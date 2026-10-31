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
#pragma once
#ifndef ZORBA_COMPILER_API_COMPILERCB
#define ZORBA_COMPILER_API_COMPILERCB

#include <atomic>
#include <vector>
#include <map>

#include <zorba/config.h>

#include "common/shared_types.h"

#ifdef MSVC
// The visual studio compiler is not able to compile the type rchandle<static_context>
// without having the definition of static_context availble.
# include "context/static_context.h"
#endif

#include "compiler/expression/pragma.h"

#include "zorbaserialization/class_serializer.h"


namespace zorba 
{

#ifdef ZORBA_WITH_DEBUGGER
class DebuggerCommons;
#endif
class static_context;
class dynamic_context;
class ExprManager;
class XQueryDiagnostics;


/*******************************************************************************
  Deferred release of the compile-time and runtime state of a query (or of an
  eval query).

  Function items point into the state of the query that created them: the
  user_function objs (and the exprs of their bodies) owned by the query's
  CompilerCB, the query's static contexts, and the query's global dynamic
  context, which holds the values of the prolog variables their bodies read
  and is the parent of their closure dynamic context. The application may
  keep a function item from a query result after the query has been closed or
  destroyed, so that state must live as long as the function item.

  The owner of the state (XQueryImpl, the EvalIteratorState of an eval query,
  or a StaticContextImpl after loadProlog()) creates one QueryResources obj,
  holds a reference to it, and stores a raw pointer to it in its CompilerCB
  (CompilerCB::theResources). Each FunctionItem holds a reference to the
  QueryResources of the ccb that created it, of the ccb that owns its
  user_function (if different), and of the query that owns its global dctx
  (see FunctionItem). Nothing else changes while the query is open: no state
  is moved, and creating a function item costs a few reference-count
  increments.

  When the owner is done with the state, it calls dispose(). If no function
  item refers to it any more (the common case), dispose() destroys the state
  right away, in the order the owner used to. Otherwise the state is handed
  over to this obj ("detached") and destroyed when the last reference goes
  away.

  Cycles: a function item can be stored in a prolog variable of the query that
  created it, and so in the dctx it keeps alive. Such cycles are collected by
  collectCycles() (see runtime/hof/function_item.cpp): a trial deletion over
  the graph of detached QueryResources, the function items stored in their
  dctx variables (directly or in temp sequences, and transitively in closure
  dctxs), and the references among them. If no reference from outside that
  graph reaches this obj, the dctx variable values are released, which frees
  the whole cycle. It runs when the obj is detached, and afterwards when a
  reference that may have been the last outside one goes away: a reference to
  a function item of the graph (FunctionItem::referenceDropped()), a function
  item that refers to a detached obj, or a reference to a detached obj held
  by another QueryResources or by an owner (release()).

  Cost: a collection walks the whole graph, including the members of the
  eager temp sequences and JSON items held by the dctx variables. So that the
  collections triggered by the references a function item gains and drops
  while it is used (e.g. a function item of the graph read from a prolog
  variable, or a function item created and dropped by an escaped function)
  do not walk the graph each time, a collection that finds the graph
  referenced from outside remembers a few of the nodes referenced from
  outside from which the root is reachable ("witnesses"), together with the
  graph generation (graphGeneration()). The generation changes whenever an
  edge of any graph may have changed: a QueryResources is detached, a
  variable of a detached dctx or of a closure dctx is set, a JSON item is
  updated, or garbage is released. As long as it has not changed, the edges
  into a witness still exist (so it is alive), and while a witness still has
  more references than edges into it, the root is still referenced from
  outside, and the walk is skipped. While the dctx variables of garbage are
  released, the collections started by the function items and
  QueryResources objs that are destroyed are deferred until the release is
  done, and run once per QueryResources.

  The external function parameters of the dctx are destroyed by dispose() in
  any case, as they were before: they are opaque (e.g. the zorba-query module
  keeps the queries it prepares, with the URL resolver function items passed
  to them, in one), so cycles through them could not be collected.

  theParent:
  ----------
  For an eval query: the QueryResources of the query whose ccb the eval ccb
  copies (it shares that query's XQueryDiagnostics and DebuggerCommons).

  theDctxParent:
  --------------
  For an eval query: the QueryResources of the query that owns the parent of
  the eval dctx, if different from theParent.

  Reference counting is atomic unless ZORBA_FOR_ONE_THREAD_ONLY is set; the
  state itself, and the cycle collection, are used by one thread at a time, as
  everything else in a query.
********************************************************************************/
class QueryResources : public SyncedRCObject
{
  friend class QueryResourcesCollector;

protected:
  CompilerCB                   * theCCB;
  static_context_t               theSctx;
  dynamic_context              * theDctx;
  XQueryDiagnostics            * theDiagnostics;
#ifdef ZORBA_WITH_DEBUGGER
  DebuggerCommons              * theDebuggerCommons;
#endif
  rchandle<QueryResources>       theParent;
  rchandle<QueryResources>       theDctxParent;
  bool                           theIsDetached;

  // See "Cost" in the class comment.
  struct Witness
  {
    const void * thePtr;
    int          theKind;
    long         theInternalRefs;
  };

  enum { MAX_WITNESSES = 4 };

  unsigned long                  theWitnessGeneration;
  csize                          theNumWitnesses;
  Witness                        theWitnesses[MAX_WITNESSES];

  static std::atomic<unsigned long> theGraphGeneration;

public:
  // For tests: the number of QueryResources objs alive, and the number of
  // graphs walked by collectCycles().
  static std::atomic<long>          theNumAlive;
  static std::atomic<unsigned long> theNumWalks;

public:
  QueryResources();

  ~QueryResources();

  /*
    Ends the owner's use of the given state (any of which may be NULL); the
    caller must release its reference to this obj afterwards. ccb, dctx and
    diagnostics are owned by the callee from now on, and so is the ccb's
    DebuggerCommons if ownsDebuggerCommons; sctx is moved (set to NULL), so
    that it is released before the ccb is deleted.
  */
  void dispose(
      CompilerCB* ccb,
      static_context_t& sctx,
      dynamic_context* dctx,
      XQueryDiagnostics* diagnostics,
      bool ownsDebuggerCommons,
      QueryResources* parent,
      QueryResources* dctxParent);

  bool isDetached() const { return theIsDetached; }

  /*
    Releases the given reference; if it was to a detached obj that survives,
    collects the cycles it may have been the last outside reference to.
  */
  static void release(rchandle<QueryResources>& resources);

  /*
    See the class comment. Collects the cycles through each detached obj in
    held. held lists the references to QueryResources objs that the caller
    holds until the call returns (with repetitions); they do not count as
    references from outside the graph.
  */
  static void collectCycles(QueryResources* const* held, csize numHeld);

  /*
    Returns false if the last collection of each detached obj in roots found
    it referenced from outside its graph, and that still holds (see "Cost" in
    the class comment); collectCycles() would then do nothing.
  */
  static bool mayCollect(QueryResources* const* roots, csize numRoots);

  // See "Cost" in the class comment.
  static void graphChanged()
  {
    theGraphGeneration.fetch_add(1, std::memory_order_relaxed);
  }

  static unsigned long graphGeneration()
  {
    return theGraphGeneration.load(std::memory_order_relaxed);
  }

private:
  void destroy();

  QueryResources(const QueryResources&);
  QueryResources& operator=(const QueryResources&);
};

typedef rchandle<QueryResources> QueryResources_t;


// For tests (test/api/function_item_lifetime.cpp): see
// QueryResources::theNumAlive and QueryResources::theNumWalks.
ZORBA_DLL_PUBLIC long query_resources_alive();

ZORBA_DLL_PUBLIC unsigned long query_resources_walks();


/*******************************************************************************
  There is one CompilerCB per query plus one CompilerCB per invocation of an
  eval or xqdoc expression that appears in the query. The query-level ccb is
  created by the constructor of the XQueryImpl obj and remains alive for the
  whole duration of the query (including runtime). Each eval/xqdoc-level ccb is
  created as a copy of the query-level ccb during the execution of the eval/xqdoc
  expr.

  theXQueryDiagnostics :
  ----------------------
  Pointer to the query's XQueryDiagnostics obj. (see src/api/xqueryimpl.h). The eval
  CompilerCBs share the query's XQueryDiagnostics.

  theSctxMap :
  ------------
  A query-level (or eval-level) map that stores the sctx objs that need to be
  kept around for the whole duration of a query (including runtime). In non-
  DEBUGGER mode, the map stores only for root sctx of each module. In DEBUGGER
  mode, it stores all the sctxs created by each module. Each sctx stored in
  this map has an associated numeric id, and theSctxMap actually maps these
  numeric ids to their associated sctx objs. The map is modified by the methods
  TranslatorImpl::end_visit(ModuleImport) and TranslatorImpl::push_scope().

  theRootSctx :
  -------------
  The root static ctx for the query or for one of the query's eval exprs. For
  an eval expr, its root sctx is a child of the query's root sctx. For the query,
  its root sctx may be (a) a child of a user-provided sctx, or (b) if the query
  is a load-prolog query, the user-provided sctx, or (c) if the user did not
  provide any sctx, a child of zorba's root sctx.

  theDebuggerCommons :
  --------------------

  theHasEval :
  ------------
  True if there is an eval expr within the compilation unit covered by this CCB.

  theIsEval :
  -----------
  True if this is the CCB for an eval query. This flag is needed to determine
  if a PUL returned by the main program must be applied or not.

  theIsLoadProlog :
  -----------------
  Whether this is a load-prolog query or not (load-prolog queries are created
  internally by the StaticContextImpl::loadProlog() method).

  theIsUpdating :
  ---------------
  Set to true if the root expr of the query or eval expr is an updating expr.

  theHaveTimeout, theTimeout :
  ----------------------------
  Set by XQuery::setTimeout(): if theHaveTimeout, theTimeout (seconds, 0 = none)
  replaces the timeout of the security policy (see PlanState::initLimits).

  theTempIndexCounter :
  ---------------------
  A counter used to create unique names for temporary (query-specific) indexes
  created to perform hashjoins (see rewriter/rules/index_join_rule.cpp).

  thePragmas:
  -------------
  A multimap from expr* to pragma such that not every expression needs
  to keep it's own list of pragmas. Since the expr* pointer is only valid
  until codegen finished, the pragmas can only be used in the compiler.


  theConfig.lib_module :
  ----------------------
  If true, then if the query string that is given by the user is a library
  module, zorba will wrap it in a dummy main module and compile/execute that
  dummy module (see  XQueryCompiler::createMainModule() method). This flag is
  a copy of the lib_module flag in Zorba_CompilerHints_t.

  theConfig.for_serialization_only :
  ----------------------------------
  This flag is a copy of the for_serialization_only flag in Zorba_CompilerHints_t.

  theConfig.parse_cb :
  Pointer to the function to call to print the AST that results from parsing
  the query.

  theConfig.translate_cb :
  ------------------------
  Pointer to the function to call to print the expr tree that results from
  translating the query AST.
  
  theCommonLanguageEnabled :
  ------------------------
  This is set to true when the XQuery and JSONiq common-language is enabled. The
  flag belongs to the static context, but it is also held here because it needs
  to be available during compilation and translation in order to raise warnings.

********************************************************************************/
class CompilerCB : public zorba::serialization::SerializeBaseClass
{
public:
  struct config : public zorba::serialization::SerializeBaseClass
  {
    typedef enum
    {
      O0,
      O1,
      O2
    } opt_level_t;

    typedef void (* expr_callback) (const expr *, const std::string& name);

    typedef void (* ast_callback) (const parsenode *, const std::string& name);

    bool           force_gflwor;
    opt_level_t    opt_level;
    bool           lib_module;
    bool           for_serialization_only;
    ast_callback   parse_cb;
    expr_callback  translate_cb;
    expr_callback  optimize_cb;
    bool           print_item_flow;  // TODO: move to RuntimeCB

   public:
    SERIALIZABLE_CLASS(config);
    config(::zorba::serialization::Archiver& ar);

    config();

    ~config() {}

    void serialize(::zorba::serialization::Archiver& ar);
  };

  typedef enum
  {
    NONE,
    PARSING,
    TRANSLATION,
    OPTIMIZATION,
    CODEGEN,
    RUNTIME
  } ProcessingPhase;

  typedef std::map<csize, static_context_t> SctxMap;

  typedef std::multimap<const expr*, pragma*>  PragmaMap;

  typedef PragmaMap::const_iterator PragmaMapIter;

public:
  XQueryDiagnostics       * theXQueryDiagnostics;

  SctxMap                   theSctxMap;

  static_context          * theRootSctx;

#ifdef ZORBA_WITH_DEBUGGER
  DebuggerCommons         * theDebuggerCommons;
#endif

  ProcessingPhase           thePhase;

  bool                      theHasEval;

  bool                      theIsEval;

  bool                      theIsLoadProlog;

  bool                      theIsUpdating;

  bool                      theIsSequential;

  bool                      theHaveTimeout;

  uint32_t                  theTimeout;

  uint8_t                   theNextVisitId;

  config                    theConfig;

  ExprManager       * const theEM;

  PragmaMap                 thePragmas;
  
  bool                      theCommonLanguageEnabled;

  // Owns the anonymous user_function objects created by the translator
  // (inline functions, function items for builtins/type constructors, and
  // coercion wrappers). They are referenced elsewhere only through raw
  // pointers (FunctionItemInfo::theFunction), and their bodies live in theEM,
  // so they share the lifetime of this ccb. Not copied, not serialized.
  std::vector<function_t>   theAnonymousUDFs;

  // Not owned; set by the owner of this ccb (see QueryResources). NULL for a
  // ccb whose function items cannot outlive it. Not copied, not serialized.
  QueryResources          * theResources;

public:
  SERIALIZABLE_CLASS(CompilerCB);
  CompilerCB(::zorba::serialization::Archiver& ar);
  void serialize(::zorba::serialization::Archiver& ar);

public:
  CompilerCB(XQueryDiagnostics*, long timeout = -1);

  CompilerCB(const CompilerCB& ccb);

  ~CompilerCB();

  ProcessingPhase getPhase() const { return thePhase; }

  void setPhase(ProcessingPhase v) { thePhase = v; }

  bool isLoadPrologQuery() const { return theIsLoadProlog; }

  void setLoadPrologQuery() { theIsLoadProlog = true; }

  void setIsUpdating(bool aIsUpdating) { theIsUpdating = aIsUpdating; }

  bool isUpdating() const { return theIsUpdating; }

  void setIsSequential(bool aIsSequential) {theIsSequential = aIsSequential;}

  bool isSequential() const { return theIsSequential;}

  static_context* getStaticContext(int id);

  ExprManager* getExprManager() const { return theEM; }

  void addAnonymousUDF(user_function* udf);

  uint8_t getVisitId() { return theNextVisitId++; }

  //
  // Pragmas
  //
  void add_pragma(const expr* e, pragma* p);

  void lookup_pragmas(const expr* e, std::vector<pragma*>& pragmas) const;

  bool lookup_pragma(const expr* e, const zstring& localname, pragma*&) const;
};


}

#endif

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
