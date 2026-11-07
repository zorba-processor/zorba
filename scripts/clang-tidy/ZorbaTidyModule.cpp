/*
 * Copyright 2006-2016 zorba.io
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Zorba's clang-tidy checks, loaded as a plugin:
//   clang-tidy -load=ZorbaTidyModule.so -checks=-*,zorba-owning-memory ...
//
// zorba-owning-memory reports raw owning memory management (see
// CONTRIBUTING.md):
//   - every delete and delete[] expression;
//   - every new expression whose result is not handed straight to a smart
//     pointer (option SmartPointers): constructing one, assigning to one,
//     reset() of one, or emplacing into a container of them.
// Placement new (the expression arena of MemoryManager, a buffer) is not
// reported; new (std::nothrow) is.
//
// Option DiffFile: the output of "git diff -U0". When set, only expressions
// on the lines that the diff adds or changes are reported, so existing code
// is not reported until it is changed.

#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MAJOR < 22
// Merged into ClangTidyModule.h in LLVM 22.
#include "clang-tidy/ClangTidyModuleRegistry.h"
#endif

#include "clang/AST/ASTContext.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"

#include <string>
#include <utility>
#include <vector>

using namespace clang;
using namespace clang::ast_matchers;

namespace zorba_tidy {

namespace {

const char DefaultSmartPointers[] =
  "::zorba::rchandle;::zorba::const_rchandle;::zorba::SmartPtr;"
  "::std::unique_ptr;::std::shared_ptr";

// The lines that a "git diff -U0" adds or changes, by path relative to the
// repository root.
class ChangedLines {
public:
  bool load(llvm::StringRef path, std::string& error)
  {
    auto buffer = llvm::MemoryBuffer::getFile(path);
    if (!buffer) {
      error = buffer.getError().message();
      return false;
    }
    llvm::StringRef text = (*buffer)->getBuffer();
    std::vector<std::pair<unsigned, unsigned>>* ranges = nullptr;
    while (!text.empty()) {
      llvm::StringRef line;
      std::tie(line, text) = text.split('\n');
      if (line.consume_front("+++ ")) {
        line = line.rtrim();
        if (line == "/dev/null") {
          ranges = nullptr;
          continue;
        }
        line.consume_front("b/");
        ranges = &theFiles[line];
      } else if (ranges && line.consume_front("@@ ")) {
        // @@ -a[,b] +c[,d] @@
        size_t plus = line.find(" +");
        if (plus == llvm::StringRef::npos)
          continue;
        llvm::StringRef added = line.substr(plus + 2);
        added = added.take_until([](char c) { return c == ' '; });
        llvm::StringRef first, count;
        std::tie(first, count) = added.split(',');
        unsigned start = 0, n = 1;
        if (first.getAsInteger(10, start) ||
            (!count.empty() && count.getAsInteger(10, n)))
          continue;
        if (n > 0)
          ranges->emplace_back(start, start + n - 1);
      }
    }
    return true;
  }

  bool contains(llvm::StringRef path, unsigned line) const
  {
    llvm::SmallString<256> normalized(path);
    llvm::sys::path::remove_dots(normalized, /*remove_dot_dot=*/true);
    llvm::StringRef file = normalized;
    for (const auto& entry : theFiles) {
      llvm::StringRef rel = entry.getKey();
      if (!(file == rel ||
            (file.ends_with(rel) && file.size() > rel.size() &&
             file[file.size() - rel.size() - 1] == '/')))
        continue;
      for (const auto& range : entry.getValue())
        if (line >= range.first && line <= range.second)
          return true;
    }
    return false;
  }

private:
  llvm::StringMap<std::vector<std::pair<unsigned, unsigned>>> theFiles;
};

} // namespace

class OwningMemoryCheck : public tidy::ClangTidyCheck {
public:
  OwningMemoryCheck(llvm::StringRef Name, tidy::ClangTidyContext* Context)
    : ClangTidyCheck(Name, Context),
      theSmartPointersOption(Options.get("SmartPointers", DefaultSmartPointers)),
      theDiffFile(Options.get("DiffFile", ""))
  {
    llvm::SmallVector<llvm::StringRef, 8> names;
    llvm::StringRef(theSmartPointersOption).split(names, ';', -1, false);
    for (llvm::StringRef name : names) {
      name = name.trim();
      if (!name.empty())
        theSmartPointers.push_back(name.str());
    }
    if (!theDiffFile.empty()) {
      std::string error;
      if (theChangedLines.load(theDiffFile, error))
        theHaveDiff = true;
      else
        configurationDiag("cannot read DiffFile '%0': %1",
                          DiagnosticIDs::Error)
          << theDiffFile << error;
    }
  }

  bool isLanguageVersionSupported(const LangOptions& LangOpts) const override
  {
    return LangOpts.CPlusPlus;
  }

  void storeOptions(tidy::ClangTidyOptions::OptionMap& Opts) override
  {
    Options.store(Opts, "SmartPointers", theSmartPointersOption);
    Options.store(Opts, "DiffFile", theDiffFile);
  }

  void registerMatchers(MatchFinder* Finder) override
  {
    Finder->addMatcher(cxxDeleteExpr().bind("delete"), this);
    Finder->addMatcher(
      cxxNewExpr(unless(isInstantiationDependent())).bind("new"), this);
  }

  void check(const MatchFinder::MatchResult& Result) override
  {
    if (const auto* del = Result.Nodes.getNodeAs<CXXDeleteExpr>("delete")) {
      if (del->getArgument()->isTypeDependent() &&
          isInPrimaryTemplate(*del, *Result.Context))
        return; // reported, if at all, in the instantiations
      report(del->getBeginLoc(), *Result.SourceManager,
             "%select{delete|delete[]}0 expression: own the object with a "
             "smart pointer (std::unique_ptr, rchandle) instead",
             del->isArrayForm() ? 1 : 0);
      return;
    }

    const auto* nw = Result.Nodes.getNodeAs<CXXNewExpr>("new");
    if (!nw || isPlacement(*nw))
      return;
    switch (consumer(*nw, *Result.Context)) {
    case Consumer::SmartPointer:
    case Consumer::Dependent:
      return;
    case Consumer::Other:
      report(nw->getBeginLoc(), *Result.SourceManager,
             "%select{new|new[]}0 expression whose result is not handed "
             "straight to a smart pointer: use std::make_unique, or construct "
             "the std::unique_ptr or rchandle from it",
             nw->isArray() ? 1 : 0);
      return;
    }
  }

private:
  enum class Consumer { SmartPointer, Dependent, Other };

  template<class T>
  static bool isInPrimaryTemplate(const T& node, ASTContext& ctx)
  {
    DynTypedNodeList parents = ctx.getParents(node);
    while (!parents.empty()) {
      const DynTypedNode& p = parents[0];
      if (const auto* fd = p.get<FunctionDecl>())
        return fd->isDependentContext();
      if (const auto* rd = p.get<CXXRecordDecl>())
        return rd->isDependentContext();
      parents = ctx.getParents(p);
    }
    return false;
  }

  // Placement new with a non-nothrow argument: an arena or a buffer owns
  // the memory.
  static bool isPlacement(const CXXNewExpr& nw)
  {
    if (nw.getNumPlacementArgs() == 0)
      return false;
    for (const Expr* arg : nw.placement_arguments()) {
      const CXXRecordDecl* rd =
        arg->getType().getNonReferenceType()->getAsCXXRecordDecl();
      if (!(rd && rd->getName() == "nothrow_t" &&
            rd->getDeclContext()->isStdNamespace()))
        return true;
    }
    return false;
  }

  bool isSmart(QualType type, ASTContext& ctx) const
  {
    type = type.getNonReferenceType();
    if (type->isPointerType())
      return false;
    const CXXRecordDecl* rd = type->getAsCXXRecordDecl();
    if (!rd)
      return false;
    std::vector<llvm::StringRef> names(theSmartPointers.begin(),
                                       theSmartPointers.end());
    return !match(cxxRecordDecl(isSameOrDerivedFrom(hasAnyName(names))),
                  *rd, ctx).empty();
  }

  // The element type of a container specialization (first template
  // argument), for emplace calls.
  bool isContainerOfSmart(QualType type, ASTContext& ctx) const
  {
    const auto* spec = llvm::dyn_cast_or_null<ClassTemplateSpecializationDecl>(
      type.getNonReferenceType()->getAsCXXRecordDecl());
    if (!spec || spec->getTemplateArgs().size() == 0)
      return false;
    const TemplateArgument& arg = spec->getTemplateArgs()[0];
    return arg.getKind() == TemplateArgument::Type &&
           isSmart(arg.getAsType(), ctx);
  }

  Consumer consumer(const CXXNewExpr& nw, ASTContext& ctx) const
  {
    const Expr* cur = &nw;
    DynTypedNodeList parents = ctx.getParents(*cur);
    while (!parents.empty()) {
      const DynTypedNode& p = parents[0];
      if (const auto* vd = p.get<VarDecl>())
        return vd->getType()->isDependentType() ? Consumer::Dependent
                                                : Consumer::Other;
      const auto* e = p.get<Expr>();
      if (!e)
        return Consumer::Other;
      if (e->isTypeDependent() || e->isValueDependent() ||
          llvm::isa<ParenListExpr, CXXUnresolvedConstructExpr>(e))
        return Consumer::Dependent;

      if (llvm::isa<ParenExpr, ImplicitCastExpr, MaterializeTemporaryExpr,
                    CXXBindTemporaryExpr, ExprWithCleanups>(e) ||
          (llvm::isa<ExplicitCastExpr>(e) && e->getType()->isPointerType())) {
        cur = e;
        parents = ctx.getParents(*cur);
        continue;
      }
      if (const auto* co = llvm::dyn_cast<ConditionalOperator>(e)) {
        if (co->getCond() == cur)
          return Consumer::Other;
        cur = e;
        parents = ctx.getParents(*cur);
        continue;
      }
      if (const auto* ce = llvm::dyn_cast<CXXConstructExpr>(e))
        return isSmart(ce->getType(), ctx) ? Consumer::SmartPointer
                                           : Consumer::Other;
      if (const auto* oc = llvm::dyn_cast<CXXOperatorCallExpr>(e)) {
        if (oc->getOperator() == OO_Equal && oc->getNumArgs() == 2 &&
            oc->getArg(1) == cur && isSmart(oc->getArg(0)->getType(), ctx))
          return Consumer::SmartPointer;
        return Consumer::Other;
      }
      if (const auto* mc = llvm::dyn_cast<CXXMemberCallExpr>(e)) {
        const CXXMethodDecl* md = mc->getMethodDecl();
        if (md && md->getIdentifier()) {
          llvm::StringRef name = md->getName();
          QualType object = mc->getObjectType();
          if (name == "reset" && isSmart(object, ctx))
            return Consumer::SmartPointer;
          if (name.starts_with("emplace") && isContainerOfSmart(object, ctx))
            return Consumer::SmartPointer;
        }
        return Consumer::Other;
      }
      return Consumer::Other;
    }
    return Consumer::Other;
  }

  template<class Arg>
  void report(SourceLocation loc, const SourceManager& sm,
              llvm::StringRef message, Arg arg)
  {
    if (theHaveDiff) {
      SourceLocation spelling = sm.getSpellingLoc(loc);
      SourceLocation expansion = sm.getExpansionLoc(loc);
      if (isChanged(spelling, sm)) {
        // reported where written
      } else if (isChanged(expansion, sm)) {
        loc = expansion;
      } else {
        return;
      }
    }
    diag(loc, message) << arg;
  }

  bool isChanged(SourceLocation loc, const SourceManager& sm) const
  {
    PresumedLoc presumed = sm.getPresumedLoc(loc, /*UseLineDirectives=*/false);
    return presumed.isValid() &&
           theChangedLines.contains(presumed.getFilename(), presumed.getLine());
  }

  std::string theSmartPointersOption;
  std::vector<std::string> theSmartPointers;
  std::string theDiffFile;
  ChangedLines theChangedLines;
  bool theHaveDiff = false;
};

class ZorbaTidyModule : public tidy::ClangTidyModule {
public:
  void addCheckFactories(tidy::ClangTidyCheckFactories& Factories) override
  {
    Factories.registerCheck<OwningMemoryCheck>("zorba-owning-memory");
  }
};

} // namespace zorba_tidy

static clang::tidy::ClangTidyModuleRegistry::Add<zorba_tidy::ZorbaTidyModule>
  X("zorba-module", "Zorba's own checks.");
