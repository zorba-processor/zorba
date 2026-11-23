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

#include <istream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "compiler/api/compiler_api.h"
#include "diagnostics/xquery_diagnostics.h"
#include "diagnostics/dict.h"
#include "diagnostics/query_hints.h"

#include "system/globalenv.h"
#include <zorba/properties.h>

#include "context/static_context.h"

#include "functions/function.h"
#include "functions/udf.h"

#include "types/typeops.h"
#include "types/root_typemanager.h"
#include "types/typemanagerimpl.h"

#include "context/uri_resolver.h"
#include "compiler/api/compiler_api_impl.h"
#include "compiler/api/compilercb.h"

#include "compiler/parser/xquery_driver.h"
#include "compiler/parsetree/parsenodes.h"
#include "compiler/parsetree/parsenode_print_xml_visitor.h"
#include "compiler/parsetree/parsenode_print_xqdoc_visitor.h"

#include "compiler/expression/expr_base.h"
#include "compiler/expression/expr_iter.h"
#include "compiler/expression/fo_expr.h"
#include "compiler/expression/function_item_expr.h"
#include "compiler/expression/expr.h"

#include "compiler/translator/translator.h"

#include "compiler/rewriter/framework/rewriter_context.h"
#include "compiler/rewriter/framework/rewriter.h"
#include "compiler/rewriter/tools/udf_graph.h"

#include "compiler/codegen/plan_visitor.h"
#include "compiler/rewriter/tools/expr_tools.h"
#ifdef ZORBA_XQUERYX
#include "compiler/xqueryx/xqueryx_to_xquery.h"
#endif

#include "runtime/base/plan_iterator.h"
#include "runtime/hof/function_item.h"
#include "util/xml_util.h"
#include "zorbatypes/URI.h"

#include "api/auditimpl.h"
#include "api/module_info_impl.h"
#include <zorba/util/timer.h>


namespace zorba
{

static void print_ast_tree(const parsenode *n, const std::string& name)
{
  std::cout << "AST for " << name << "\n";
  print_parsetree_xml(std::cout, n);
  std::cout << std::endl;
}


/*******************************************************************************

********************************************************************************/
XQueryCompiler::XQueryCompiler(CompilerCB* aCompilerCB)
  :
  theCompilerCB(aCompilerCB)
{
}


/*******************************************************************************

********************************************************************************/
XQueryCompiler::~XQueryCompiler()
{
}


/*******************************************************************************

********************************************************************************/
void XQueryCompiler::parseOnly(
    std::istream& aXQuery,
    const zstring& aFileName)
{
  parsenode_t pn = parse(aXQuery, aFileName);
}


/*******************************************************************************

********************************************************************************/
void XQueryCompiler::xqdoc(
    std::istream&         aXQuery,
    const zstring&        aFileName,
    store::Item_t&        aResult,
    const store::Item_t&  aDateTime,
    uint32_t              aOptions)
{
  parsenode_t lAST = parse(aXQuery, aFileName);

  print_parsetree_xqdoc(aResult, lAST.getp(),
                        aFileName.c_str(), aDateTime, aOptions);
}


/*******************************************************************************

********************************************************************************/
bool XQueryCompiler::getLanguageMode(std::stringstream& s) const
{
  const size_t lPeekSize = 7;
  char lPeek[lPeekSize];
  s.get(lPeek, lPeekSize);
  s.clear();
  s.seekg(0, s.beg);

  bool lXQueryMode;

  if (strncmp(lPeek, "jsoniq", lPeekSize - 1) == 0)
  {
    lXQueryMode = false;
  }
  else if (strncmp(lPeek, "xquery", lPeekSize - 1) == 0)
  {
    lXQueryMode = true;
  }
  else
  {
    if (theCompilerCB->theRootSctx) // could be null in eval
    {
      StaticContextConsts::language_kind_t lKind
        = theCompilerCB->theRootSctx->language_kind();
      lXQueryMode = (lKind != StaticContextConsts::language_kind_jsoniq);
    }
    else
    {
      lXQueryMode = true;
    }
  }
  return lXQueryMode;
}

/*******************************************************************************

********************************************************************************/
parsenode_t XQueryCompiler::parse(std::istream& aXQuery, const zstring& aFileName)
{
  // TODO: move these out
  if (Properties::instance().getPrintAST())
  {
    theCompilerCB->theConfig.parse_cb = print_ast_tree;
  }

  // The text is copied into a std::string, never through a stream insertion:
  // a stream that cannot grow only sets a state bit, and a cut text could still
  // be a valid query. std::bad_alloc goes to the caller.
  std::string xquery_text;

#ifdef ZORBA_XQUERYX
  char* converted_xquery_str = NULL;
  std::string   xquery_str;
  bool is_xqueryx = false;

  {
    char strtemp[1000];
    do
    {
      strtemp[0] = 0;
      aXQuery.read(strtemp, sizeof(strtemp)-1);
      strtemp[aXQuery.gcount()] = 0;
      xquery_str += strtemp;
    }
    while(aXQuery.gcount() == (sizeof(strtemp)-1));
  }

  XQueryXConvertor* xqxconvertor = GENV.getXQueryXConvertor();

  if(xqxconvertor->isXQueryX((char*)xquery_str.c_str()))
  {
    // identify XQueryX by content:
    // root tag =
    // "<prefix:module ... xmlns:prefix="http://www.w3.org/2005/XQueryX" ... > "

    is_xqueryx = true;
    //translate from xqueryx to xquery using XSLT
    //read all input stream into std::string

    converted_xquery_str = xqxconvertor->XQueryX2XQuery(
        xquery_str.c_str(),
        theCompilerCB->theRootSctx ?
          theCompilerCB->theRootSctx->get_security_policy() :
          GENV_ROOT_STATIC_CONTEXT.get_security_policy());

#ifndef NDEBUG
    printf ("\n\n%s", converted_xquery_str);  // debug
#endif
    // Freed right after the copy, also when the copy fails.
    try
    {
      xquery_text = converted_xquery_str;
    }
    catch (...)
    {
      xqxconvertor->freeResult(converted_xquery_str);
      converted_xquery_str = NULL;
      throw;
    }
    xqxconvertor->freeResult(converted_xquery_str);
    converted_xquery_str = NULL;
  }
  else
  {
    xquery_text.swap(xquery_str);
  }
#else // ZORBA_XQUERYX

  xquery_text.assign(std::istreambuf_iterator<char>(aXQuery),
                     std::istreambuf_iterator<char>());
#endif

  std::stringstream xquery_stream(std::move(xquery_text));

  theCompilerCB->setPhase(CompilerCB::PARSING);

  parsenode_t node;
  theCompilerCB->setPhase(CompilerCB::NONE);
  
  bool lXQueryMode = getLanguageMode(xquery_stream);

  if (lXQueryMode)
  {
    xquery_driver lDriver(&*theCompilerCB, xquery_driver::XQUERY_GRAMMAR);
    lDriver.parse_stream(xquery_stream, aFileName);
    node =  lDriver.get_expr();
  }
  else
  {
    xquery_driver lDriver(&*theCompilerCB, xquery_driver::JSONIQ_GRAMMAR);
    lDriver.parse_stream(xquery_stream, aFileName);
    node =  lDriver.get_expr();
  }

#ifdef ZORBA_XQUERYX
  if (is_xqueryx && converted_xquery_str != NULL)
  {
    xqxconvertor->freeResult(converted_xquery_str);
  }
#endif



  const parsenode& lNode = *node;
  if (typeid (lNode) == typeid (ParseErrorNode))
  {
    ParseErrorNode* pen = static_cast<ParseErrorNode *>(&*node);
    XQueryException e(XQUERY_EXCEPTION_VAR(pen->err,
    ERROR_PARAMS(ERROR_TEXT( pen->msg )), ERROR_LOC(pen->get_location())));
    e.set_hint(query_hints::syntax_error(xquery_stream.str(),
                                         pen->get_location().getLineBegin(),
                                         pen->get_location().getColumnBegin(),
                                         pen->get_location().getLineEnd(),
                                         pen->get_location().getColumnEnd(),
                                         lXQueryMode,
                                         pen->msg.str()).c_str());
    throw e;
  }

  return node;
}

/*******************************************************************************

********************************************************************************/
ModuleInfo* XQueryCompiler::parseInfo(
    std::istream& aXQuery,
    const zstring& aFileName)
{
  parsenode_t lParseNode = parse(aXQuery, aFileName);

  const parsenode& lNodeRef = *lParseNode;
  if (typeid (lNodeRef) == typeid (ParseErrorNode))
  {
    ParseErrorNode* pen = static_cast<ParseErrorNode *>(lParseNode.getp());
    throw XQUERY_EXCEPTION_VAR(pen->err, 
    ERROR_PARAMS(ERROR_TEXT( pen->msg )), ERROR_LOC(pen->get_location()));
  }

  LibraryModule* lLibModule = dynamic_cast<LibraryModule*>(lParseNode.getp());

  zstring lTargetNamespace;

  if (lLibModule)
  {
    ModuleDecl* lDecl = lLibModule->get_decl().getp();
    lTargetNamespace = lDecl->get_target_namespace();
  }

  return new ModuleInfoImpl(lTargetNamespace);
}

/*******************************************************************************

********************************************************************************/
PlanIter_t XQueryCompiler::compile(
    std::istream& aXQuery,
    const zstring& aFileName,
    ulong& nextDynamicVarId)
{
  audit::Event* ae = theCompilerCB->theRootSctx->get_audit_event();
  zorba::audit::ScopedRecord sar(ae);

  const char* lFileName = aFileName.c_str();

  audit::ScopedAuditor<const char*>
  filenameAudit(sar, zorba::audit::XQUERY_COMPILATION_FILENAME, lFileName);

  parsenode_t lAST;

  {
    time::Timer lTimer;

    audit::DurationAuditor
    durationAudit(sar, audit::XQUERY_COMPILATION_PARSE_DURATION, lTimer);

    lAST = parse(aXQuery, aFileName);

    if (theCompilerCB->theConfig.lib_module &&
        dynamic_cast<LibraryModule*>(lAST.getp()) != NULL)
    {
      lAST = createMainModule(lAST, aXQuery, aFileName);
    }
  }

  return compile(lAST, true, nextDynamicVarId, sar);
}


/*******************************************************************************

********************************************************************************/
PlanIter_t XQueryCompiler::compile(
    const parsenode_t& ast,
    bool applyPUL,
    ulong& nextDynamicVarId,
    audit::ScopedRecord& auditRecord)
{
  expr* rootExpr;

  rootExpr = translate(ast, auditRecord);

  rootExpr = optimize(rootExpr, auditRecord);

  theCompilerCB->theMayRunSideEffects =
    theCompilerCB->theOuterMayRunSideEffects ||
    expr_tools::may_run_side_effects(rootExpr);

  theCompilerCB->theMayRunSideEffectsOrUpdates =
    theCompilerCB->theMayRunSideEffects ||
    expr_tools::may_run_side_effects_or_updates(rootExpr);

  PlanIter_t plan = codegen(rootExpr, nextDynamicVarId, auditRecord);

  seal(rootExpr);

  return plan;
}


/*******************************************************************************

********************************************************************************/
expr* XQueryCompiler::translate(
    parsenode_t parsenode,
    audit::ScopedRecord& auditRecord)
{
  time::Timer timer;

  audit::DurationAuditor
  durationAudit(auditRecord,
                audit::XQUERY_COMPILATION_TRANSLATION_DURATION,
                timer);

  theCompilerCB->setPhase(CompilerCB::TRANSLATION);

  expr* rootExpr = zorba::translate(*parsenode, theCompilerCB);

  theCompilerCB->setPhase(CompilerCB::NONE);

  if (rootExpr == NULL)
  {
    // TODO: can this happen?
    throw ZORBA_EXCEPTION(zerr::ZAPI0002_XQUERY_COMPILATION_FAILED);
  }

  return rootExpr;
}


/*******************************************************************************

********************************************************************************/
expr* XQueryCompiler::optimize(expr* rootExpr, audit::ScopedRecord& auditRecord)
{
  time::Timer timer;

  audit::DurationAuditor
  durationAudit(auditRecord,
                audit::XQUERY_COMPILATION_OPTIMIZATION_DURATION,
                timer);

  theCompilerCB->setPhase(CompilerCB::OPTIMIZATION);

  expr_tools::classify_calls(rootExpr);

  // Build the call-graph among the udfs that are actually used in the query
  // program.
  UDFGraph udfGraph(rootExpr);

  // By default all UDFs are marked as deterministic. Now, we find which udfs
  // are actually non-deterministic and mark them as such. This has to be done
  // before optimization.
  udfGraph.inferDeterminism();

  if (theCompilerCB->theConfig.opt_level <= CompilerCB::config::O0)
  {
    theCompilerCB->setPhase(CompilerCB::NONE);
    return rootExpr;
  }

  // Optimize the udfs.
  udfGraph.optimizeUDFs(theCompilerCB);

  // Optimize the main expr (i.e., the top expr of the main module).
  RewriterContext rCtx(theCompilerCB,
                       rootExpr,
                       NULL,
                       "Optimizing main program",
                       rootExpr->get_sctx()->is_in_ordered_mode());

  GENV_COMPILERSUBSYS.getDefaultOptimizingRewriter()->rewrite(rCtx);

  rootExpr = rCtx.getRoot();

  if (theCompilerCB->theConfig.optimize_cb != NULL)
    theCompilerCB->theConfig.optimize_cb(rootExpr, "main query");

  theCompilerCB->setPhase(CompilerCB::NONE);

  return rootExpr;
}


/*******************************************************************************

********************************************************************************/
PlanIter_t XQueryCompiler::codegen(
    expr* rootExpr,
    ulong& nextDynamicVarId,
    audit::ScopedRecord& auditRecord)
{
  time::Timer timer;

  audit::DurationAuditor
  durationAudit(auditRecord,
                audit::XQUERY_COMPILATION_CODEGENERATION_DURATION,
                timer);

  theCompilerCB->setPhase(CompilerCB::CODEGEN);

  PlanIter_t plan = zorba::codegen("main query",
                                   rootExpr,
                                   theCompilerCB,
                                   nextDynamicVarId);

  theCompilerCB->setPhase(CompilerCB::NONE);

  return plan;
}


namespace
{

/*******************************************************************************
  The udfs of a compilation that seal() optimizes and generates the plans of,
  in the order their plans were generated lazily before (by the first call that
  opened): those that the main query names, each call after its arguments, then
  those that their bodies name.
********************************************************************************/
class SealedFunctions
{
public:
  struct Entry
  {
    user_function * theUDF;
    ulong           theNextVarId;
    // Named by the main query or by a body it reaches: the optimizer already
    // saw it, or it was optimized when its plan was generated.
    bool            theIsReached;
  };

  CompilerCB              * theCCB;
  std::vector<Entry>        theEntries;
  std::set<user_function*>  theSeen;

  explicit SealedFunctions(CompilerCB* ccb) : theCCB(ccb) {}

  // nextVarId is the one the calls of the function pass to getPlan().
  void add(function* f, ulong nextVarId, bool isReached)
  {
    if (f == NULL || !f->isUdf())
      return;

    user_function* udf = static_cast<user_function*>(f);

    // A udf of another compilation is sealed by it; one without a body is
    // external or comes from a plan.
    if (udf->getCCB() != theCCB ||
        udf->getBody() == NULL ||
        udf->isSealed() ||
        !theSeen.insert(udf).second)
      return;

    theEntries.push_back(Entry{udf, nextVarId, isReached});
  }

  void addNamed(expr* e, bool isReached)
  {
    ExprIterator iter(e);
    while (!iter.done())
    {
      if (**iter != NULL)
        addNamed(**iter, isReached);

      iter.next();
    }

    if (e->get_expr_kind() == fo_expr_kind)
    {
      add(static_cast<fo_expr*>(e)->get_func(), 1, isReached);
    }
    else if (e->get_expr_kind() == function_item_expr_kind)
    {
      function_item_expr* fi = static_cast<function_item_expr*>(e);
      add(fi->get_function(), fi->get_fi_info()->firstLocalVarId(), isReached);
    }
  }
};


// Computes the cached type of every expression of e. One that cannot be typed
// keeps raising its error where it is typed.
void computeTypes(expr* e)
{
  ExprIterator iter(e);
  while (!iter.done())
  {
    if (**iter != NULL)
      computeTypes(**iter);

    iter.next();
  }

  try
  {
    (void)e->get_return_type();
  }
  catch (ZorbaException const&)
  {
  }
}


// Makes every constructor in e copy its input nodes.
void copyInputNodes(expr* e)
{
  ExprIterator iter(e);
  while (!iter.done())
  {
    if (**iter != NULL)
      copyInputNodes(**iter);

    iter.next();
  }

  if (e->get_expr_kind() == doc_expr_kind)
    static_cast<doc_expr*>(e)->setCopyInputNodes();
  else if (e->get_expr_kind() == elem_expr_kind)
    static_cast<elem_expr*>(e)->setCopyInputNodes();
}

} // namespace


/*******************************************************************************
  Sealed compilation: optimizes and generates the plan of every udf of this
  compilation that its main query can reach, and computes the types of their
  expressions, so that running the query, or compiling another query against a
  static context it shares, changes nothing in them.

  A prolog loaded with loadProlog, and a query with eval or fn:function-lookup,
  can have any function they declare called by another compilation: then every
  declared function is sealed. The functions that the compilation does not call
  are optimized as the compilation of a query that calls them did before, and
  their constructors copy their input nodes, since nothing here knows how a
  caller uses the nodes they return.

  Optimizing and generating a plan lazily raised their errors when a call
  opened: they are kept in the udf and raised there (user_function::seal()).
  An error that the compilation of a caller found in a function that this
  compilation does not call (inferring its determinism, computing its cache
  settings) is raised by the compilation of a query that calls it statically
  (UDFGraph::inferDeterminism()).
********************************************************************************/
void XQueryCompiler::seal(expr* rootExpr)
{
  CompilerCB* ccb = theCompilerCB;

  bool const allDeclared = (ccb->theIsLoadProlog ||
                            ccb->theHasEval ||
                            ccb->theHasFunctionLookup);

  SealedFunctions sealed(ccb);
  sealed.addNamed(rootExpr, true);

  std::set<user_function*> declared;
  bool declaredAdded = false;

  for (csize i = 0; ; ++i)
  {
    if (i == sealed.theEntries.size())
    {
      if (!allDeclared || declaredAdded)
        break;

      declaredAdded = true;

      std::vector<static_context*> sctxs;
      sctxs.push_back(ccb->theRootSctx);

      CompilerCB::SctxMap::const_iterator ite = ccb->theSctxMap.begin();
      for (; ite != ccb->theSctxMap.end(); ++ite)
        sctxs.push_back(ite->second.getp());

      std::set<const static_context*> visitedSctxs;
      std::vector<function*> functions;

      for (csize j = 0; j < sctxs.size(); ++j)
      {
        if (sctxs[j] != NULL)
          sctxs[j]->get_udfs(functions, visitedSctxs);
      }

      for (csize j = 0; j < functions.size(); ++j)
      {
        if (static_cast<user_function*>(functions[j])->getCCB() == ccb)
          declared.insert(static_cast<user_function*>(functions[j]));

        sealed.add(functions[j], 1, false);
      }

      if (i == sealed.theEntries.size())
        break;
    }

    SealedFunctions::Entry entry = sealed.theEntries[i];
    user_function* udf = entry.theUDF;

    if (entry.theIsReached)
    {
      try
      {
        udf->optimize();
      }
      catch (ZorbaException const& e)
      {
        udf->setSealError(e);
      }
    }
    else
    {
      UDFGraph graph(udf);

      try
      {
        graph.inferDeterminism();
      }
      catch (ZorbaException const& e)
      {
        udf->setStaticError(e);
      }

      try
      {
        if (ccb->theConfig.opt_level > CompilerCB::config::O0)
          graph.optimizeUDFs(ccb);
      }
      catch (ZorbaException const& e)
      {
        udf->setSealError(e);
      }
    }

    sealed.addNamed(udf->getBody(), entry.theIsReached);
  }

  if (allDeclared)
  {
    for (csize i = 0; i < sealed.theEntries.size(); ++i)
      copyInputNodes(sealed.theEntries[i].theUDF->getBody());
  }

  // The queries compiled against the static context of a prolog include its
  // program, which initializes its variables (translator, end_visit of
  // MainModule): they read it, they do not change it.
  if (ccb->theIsLoadProlog)
  {
    expr* program = ccb->theRootSctx->get_query_expr();

    copyInputNodes(rootExpr);
    computeTypes(rootExpr);

    if (program != NULL && program != rootExpr)
    {
      copyInputNodes(program);
      computeTypes(program);
    }
  }

  for (csize i = 0; i < sealed.theEntries.size(); ++i)
  {
    SealedFunctions::Entry const& entry = sealed.theEntries[i];
    user_function* udf = entry.theUDF;

    // The compilation of a caller computed them for the calls it generated.
    if (declared.count(udf) != 0)
    {
      try
      {
        udf->computeCacheSettings(ccb->theXQueryDiagnostics);
      }
      catch (ZorbaException const& e)
      {
        udf->setStaticError(e);
      }
    }

    computeTypes(udf->getBody());

    udf->seal(entry.theNextVarId);
  }
}


/******************************************************************************
  This is a small helper class used when the user wants to compile a library
  module. The ONLY place it is used (and should be used) is in the
  XQueryCompiler::createMainModule method below.
  QQQ When we have the ability to compile a library module indepedently, this
  rather hacky class can go away. At that time, we can also eliminate the
  "stream URL" hack in StreamResource - this is the only place in the code where
  we use the two-arg StreamResource constructor.
*******************************************************************************/
class FakeLibraryModuleURLResolver : public internal::URLResolver
{
public:
  FakeLibraryModuleURLResolver(
      zstring const& aLibraryModuleFilename,
      std::istream& aStream)
    :
    theLibraryModuleFilename(aLibraryModuleFilename),
    theStream(aStream)
  {}

  virtual ~FakeLibraryModuleURLResolver()
  {}

  virtual internal::Resource* resolveURL(
      const zstring& aUrl,
      const internal::EntityData* aEntityData)
  {
    // Since we know this URL resolver will only be used when compiling the
    // stub query, and the only URI that query will contain is the one we're
    // interested in, then we don't actually need to check the URL at all -
    // just return the stream. This is a good thing, because due to URI
    // mapping, aUrl will probably be different than the one we're expecting.
    // If we returned NULL here, Zorba's built-in URL Resolvers would attempt
    // to load the resource from somewhere else, and that may result in the
    // wrong information being compiled. See bug 992304.
    assert (theStream.good());
    // Pass a nullptr StreamReleaser; memory ownership of the istream remains
    // with the caller of this method.
    // QQQ We can remove this third argument when we can compile modules individually
    return new internal::StreamResource(&theStream, nullptr, theLibraryModuleFilename);
  }

private:
  zstring theLibraryModuleFilename;
  std::istream& theStream;
};


/******************************************************************************
  Create a dummy main module to wrap a library module.
******************************************************************************/
parsenode_t XQueryCompiler::createMainModule(
    parsenode_t aLibraryModule,
    std::istream& aXQuery,
    const zstring& aFileName)
{
  //get the namespace from the LibraryModule
  LibraryModule* mod_ast = dynamic_cast<LibraryModule *>(&*aLibraryModule);
  if (!mod_ast)
    throw ZORBA_EXCEPTION(zerr::ZAPI0002_XQUERY_COMPILATION_FAILED,
    ERROR_PARAMS(ZED(BadLibraryModule)));

  const zstring& lib_namespace = mod_ast->get_decl()->get_target_namespace();

  URI lURI(lib_namespace);
  if(!lURI.is_absolute())
  {
    throw XQUERY_EXCEPTION(err::XQST0046,
    ERROR_PARAMS(lURI.toString(), ZED(MustBeAbsoluteURI)),
      ERROR_LOC(mod_ast->get_decl()->get_location()));
  }

  // Set up the original query stream as the result of resolving the
  // library module's URI
  aXQuery.clear();
  aXQuery.seekg(0);

  FakeLibraryModuleURLResolver* aFakeResolver =
    new FakeLibraryModuleURLResolver(aFileName, aXQuery);

  theCompilerCB->theRootSctx->add_url_resolver(aFakeResolver);

  // create a dummy main module and parse it
  std::stringstream lDocStream;
  zstring tmp;
  zorba::xml::escape(lib_namespace, &tmp);
  lDocStream << "import module namespace m = '" << tmp << "'; 1";
  return parse(lDocStream, aFileName);
}


/*******************************************************************************

********************************************************************************/
XQueryCompilerSubsystem::XQueryCompilerSubsystem()
{
}


XQueryCompilerSubsystem::~XQueryCompilerSubsystem()
{
}


std::unique_ptr<XQueryCompilerSubsystem> XQueryCompilerSubsystem::create()
{
  return std::unique_ptr<XQueryCompilerSubsystem>(new XQueryCompilerSubsystemImpl());
}

}
/* vim:set et sw=2 ts=2: */
