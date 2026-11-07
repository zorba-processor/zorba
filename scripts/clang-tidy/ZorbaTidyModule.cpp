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

#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"

using namespace clang::ast_matchers;

namespace zorba_tidy {

class OwningMemoryCheck : public clang::tidy::ClangTidyCheck {
public:
  OwningMemoryCheck(llvm::StringRef Name, clang::tidy::ClangTidyContext* Context)
    : ClangTidyCheck(Name, Context) {}

  bool isLanguageVersionSupported(const clang::LangOptions& LangOpts) const override {
    return LangOpts.CPlusPlus;
  }

  void registerMatchers(MatchFinder*) override {}

  void check(const MatchFinder::MatchResult&) override {}
};

class ZorbaTidyModule : public clang::tidy::ClangTidyModule {
public:
  void addCheckFactories(clang::tidy::ClangTidyCheckFactories& Factories) override {
    Factories.registerCheck<OwningMemoryCheck>("zorba-owning-memory");
  }
};

} // namespace zorba_tidy

static clang::tidy::ClangTidyModuleRegistry::Add<zorba_tidy::ZorbaTidyModule>
  X("zorba-module", "Zorba's own checks.");
