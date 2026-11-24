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

#include "compiler/api/compilercb.h"
#include "compiler/expression/expr_base.h"
#include "compiler/expression/expr_manager.h"

#ifdef ZORBA_WITH_DEBUGGER
#include "debugger/debugger_commons.h"
#endif

#include "context/dynamic_context.h"
#include "context/static_context.h"
#include "diagnostics/xquery_diagnostics.h"
#include "functions/udf.h"

#include <algorithm>

#include <zorba/properties.h>

#include "store/api/store.h"
#include "system/globalenv.h"

#include "zorbaserialization/serialize_template_types.h"
#include "zorbaserialization/serialize_zorba_types.h"


namespace zorba
{

SERIALIZABLE_CLASS_VERSIONS(CompilerCB)

SERIALIZABLE_CLASS_VERSIONS_2(CompilerCB::config, TYPE_CompilerCB_config)


#define DEF_PRINT_EXPR_TREE( phase )                          \
static void print_expr_tree_##phase(const expr* e, const std::string& name) { \
  std::ostream& os = Properties::instance().getDebugStream(); \
  os << "Expression tree after " << #phase                    \
     << " for " << name << "\n";                              \
  e->put(os) << std::endl;                                    \
}


DEF_PRINT_EXPR_TREE(translation);
DEF_PRINT_EXPR_TREE(optimization);


/*******************************************************************************

********************************************************************************/
CompilerCB::config::config()
  :
  opt_level(O1),
  lib_module(false),
  for_serialization_only(false),
  parse_cb(NULL)
{
  translate_cb = optimize_cb = NULL;

  // TODO: move these out
  print_item_flow = Properties::instance().getPrintItemFlow();

  if (Properties::instance().getPrintTranslated())
    translate_cb = print_expr_tree_translation;

  if (Properties::instance().getPrintOptimized())
    optimize_cb = print_expr_tree_optimization;

  force_gflwor = Properties::instance().getForceGFLWOR();
}


/*******************************************************************************

********************************************************************************/
CompilerCB::config::config(::zorba::serialization::Archiver& ar)
  :
  parse_cb(NULL),
  translate_cb(NULL),
  optimize_cb(NULL)
{
}


/*******************************************************************************

********************************************************************************/
void CompilerCB::config::serialize(::zorba::serialization::Archiver& ar)
{
  ar & force_gflwor;
  SERIALIZE_ENUM(opt_level_t, opt_level);
  ar & lib_module;
  ar & for_serialization_only;
  ar & print_item_flow;
}


/*******************************************************************************

********************************************************************************/
CompilerCB::CompilerCB(XQueryDiagnostics* errmgr, long timeout)
  :
  theXQueryDiagnostics(errmgr),
  theRootSctx(0),
#ifdef ZORBA_WITH_DEBUGGER
  theDebuggerCommons(0),
#endif
  thePhase(NONE),
  theHasEval(false),
  theIsEval(false),
  theIsLoadProlog(false),
  theIsUpdating(false),
  theIsSequential(false),
  theOuterMayRunSideEffects(false),
  theMayRunSideEffects(true),
  theMayRunSideEffectsOrUpdates(true),
  theHaveTimeout(false),
  theTimeout(timeout),
  theFoldingMillis(0),
  theDynamicCallExpr(NULL),
  theNextVisitId(1),
  theEM(new ExprManager(this)),
  theCommonLanguageEnabled(false),
  theResources(NULL)
{
  if (timeout >= 0)
    theHaveTimeout = true;
}


/*******************************************************************************
  Used by the eval iterator to create a new ccb as a copy of the ccb of the
  enclosing query.
*******************************************************************************/
CompilerCB::CompilerCB(const CompilerCB& cb)
  :
  zorba::serialization::SerializeBaseClass(cb),
  theXQueryDiagnostics(cb.theXQueryDiagnostics),
  theRootSctx(NULL),
#ifdef ZORBA_WITH_DEBUGGER
  theDebuggerCommons(cb.theDebuggerCommons),
#endif
  thePhase(NONE),
  theHasEval(false),
  theIsEval(false),
  theIsLoadProlog(false),
  theIsUpdating(false),
  theIsSequential(false),
  theOuterMayRunSideEffects(cb.theMayRunSideEffects),
  theMayRunSideEffects(true),
  theMayRunSideEffectsOrUpdates(true),
  theOuterNoHoisting(cb.theNoHoisting),
  theHaveTimeout(cb.theHaveTimeout),
  theTimeout(cb.theTimeout),
  theFoldingMillis(0),
  theDynamicCallExpr(NULL),
  theNextVisitId(cb.theNextVisitId+1),
  theConfig(cb.theConfig),
  theEM(new ExprManager(this)),
  theCommonLanguageEnabled(cb.theCommonLanguageEnabled),
  theResources(NULL)
{
}


/*******************************************************************************

********************************************************************************/
CompilerCB::CompilerCB(::zorba::serialization::Archiver& ar)
  :
  ::zorba::serialization::SerializeBaseClass(),
  theXQueryDiagnostics(NULL),
  theRootSctx(NULL),
#ifdef ZORBA_WITH_DEBUGGER
  theDebuggerCommons(NULL),
#endif
  thePhase(RUNTIME),
  theHasEval(false),
  theIsEval(false),
  theOuterMayRunSideEffects(true),
  theMayRunSideEffects(true),
  theMayRunSideEffectsOrUpdates(true),
  theNextVisitId(1),
  theEM(new ExprManager(this)),
  theResources(NULL)
{
}


/*******************************************************************************

********************************************************************************/
CompilerCB::~CompilerCB()
{
  // Release the anonymous udfs (and their plans) before the expressions they
  // point to are destroyed.
  theAnonymousUDFs.clear();

  delete theEM;
}


/*******************************************************************************

********************************************************************************/
void CompilerCB::addAnonymousUDF(user_function* udf)
{
  theAnonymousUDFs.push_back(udf);
}


/*******************************************************************************
  class QueryResources (collectCycles() is in runtime/hof/function_item.cpp)
********************************************************************************/
std::atomic<unsigned long> QueryResources::theGraphGeneration(1);

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
namespace
{
// See QueryResources::CollectorLock.
std::recursive_mutex theCollectorMutex;

// The detached QueryResources objs alive, under the collector lock (see
// "Threads" in the class comment of QueryResources).
std::vector<QueryResources*> theDetachedObjs;
}

std::atomic<unsigned long> QueryResources::theDetachEpoch(0);

QueryResources::CollectorLock::CollectorLock()
  :
  theStoreLock(GENV_STORE.getGlobalLock(), Lock::READ),
  theGuard(theCollectorMutex)
{
}
#else
QueryResources::CollectorLock::CollectorLock()
{
}
#endif

std::atomic<long> QueryResources::theNumAlive(0);

std::atomic<unsigned long> QueryResources::theNumWalks(0);

std::atomic<unsigned long> QueryResources::theFailWalkAtEdge(0);


long query_resources_alive()
{
  return QueryResources::theNumAlive.load();
}


unsigned long query_resources_walks()
{
  return QueryResources::theNumWalks.load();
}


void query_resources_fail_walk(unsigned long edge)
{
  QueryResources::theFailWalkAtEdge.store(edge);
}


QueryResources::QueryResources()
  :
  theCCB(NULL),
  theDctx(NULL),
  theDiagnostics(NULL),
#ifdef ZORBA_WITH_DEBUGGER
  theDebuggerCommons(NULL),
#endif
  theIsDetached(false),
#ifndef ZORBA_FOR_ONE_THREAD_ONLY
  theDetachedAt(0),
#endif
  theWitnessGeneration(0),
  theNumWitnesses(0)
{
  ++theNumAlive;
}


QueryResources::~QueryResources()
{
  destroy();

  // The parents may now be reachable only through cycles.
  release(theParent);
  release(theDctxParent);

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
  if (theDetachedAt != 0)
  {
    CollectorLock lock;
    theDetachedObjs.erase(
        std::find(theDetachedObjs.begin(), theDetachedObjs.end(), this));
  }
#endif

  --theNumAlive;
}


void QueryResources::dispose(
    CompilerCB* ccb,
    static_context_t& sctx,
    dynamic_context* dctx,
    XQueryDiagnostics* diagnostics,
    bool ownsDebuggerCommons,
    QueryResources* parent,
    QueryResources* dctxParent)
{
  assert(theCCB == NULL && theDctx == NULL && !theIsDetached);

  theCCB = ccb;
  theSctx = sctx;
  sctx = NULL;
  theDctx = dctx;
  theDiagnostics = diagnostics;
#ifdef ZORBA_WITH_DEBUGGER
  theDebuggerCommons =
    (ownsDebuggerCommons && ccb != NULL ? ccb->theDebuggerCommons : NULL);
#else
  (void)ownsDebuggerCommons;
#endif

  // See the class comment. May release function items created by the query,
  // and thereby references to this obj.
  if (theDctx)
    theDctx->release_parameters();

  if (getRefCount() > 1)
  {
    // Function items created by the query may still be alive: keep the state
    // until the last of them is destroyed, unless only cycles keep it.
    theParent = parent;
    theDctxParent = (dctxParent != parent ? dctxParent : NULL);

    // From here on the collections on other threads walk the state handed
    // over above.
#ifndef ZORBA_FOR_ONE_THREAD_ONLY
    detach();
#else
    theIsDetached.store(true, std::memory_order_release);
#endif

    // From now on the dctx variables are edges of a graph.
    if (theDctx)
      theDctx->track_writes();
    graphChanged();

    // The caller's reference does not count.
    QueryResources* held[1] = { this };
    collectCycles(held, 1);
  }

  if (getRefCount() == 1)
  {
    theIsDetached.store(false, std::memory_order_relaxed);
    theParent = NULL;
    theDctxParent = NULL;
    destroy();
  }
}


void QueryResources::release(rchandle<QueryResources>& resources)
{
  if (resources == NULL)
    return;

  unsigned long const epoch = beginRelease();

  if (resources->isDetached() && resources->getRefCount() > 1)
  {
    rchandle<QueryResources> hold;
    hold.transfer(resources);
    QueryResources* held[1] = { hold.getp() };
    collectCycles(held, 1);
  }
  else
  {
    resources = NULL;
    endRelease(epoch);
  }
}


#ifndef ZORBA_FOR_ONE_THREAD_ONLY
/*******************************************************************************
  See "Threads" in the class comment. Under the lock, so that a release that
  sees the new epoch finds this obj listed once it gets the lock.
********************************************************************************/
void QueryResources::detach()
{
  CollectorLock lock;

  theIsDetached.store(true, std::memory_order_release);

  theDetachedAt = theDetachEpoch.fetch_add(1, std::memory_order_seq_cst) + 1;
  theDetachedObjs.push_back(this);

  // Pairs with the fence of endRelease(): the collection that follows reads
  // the reference counts after it.
  std::atomic_thread_fence(std::memory_order_seq_cst);
}


/*******************************************************************************
  See "Threads" in the class comment: collects the cycles through the objs
  detached after epoch. Runs in releases, so a failure (std::bad_alloc) must
  not escape: the cycles are then kept, as when a walk fails.
********************************************************************************/
void QueryResources::collectDetachedSince(unsigned long epoch)
{
  CollectorLock lock;

  std::vector<QueryResources_t> roots;
  std::vector<QueryResources*> held;

  try
  {
    for (QueryResources* res : theDetachedObjs)
    {
      // A count of 0: the obj is being destroyed and waits for the lock to
      // leave the list.
      if (res->theDetachedAt > epoch && res->theRefCount.incrementUnlessZero())
      {
        QueryResources_t root(res);
        res->removeReference();
        roots.push_back(root);
        held.push_back(res);
      }
    }
  }
  catch (const std::exception&)
  {
    return;
  }

  if (!held.empty())
    collectCycles(&held[0], held.size());
}
#endif


void QueryResources::destroy()
{
  delete theDctx;
  theDctx = NULL;

  theSctx = NULL;

#ifdef ZORBA_WITH_DEBUGGER
  delete theDebuggerCommons;
  theDebuggerCommons = NULL;
#endif

  delete theCCB;
  theCCB = NULL;

  delete theDiagnostics;
  theDiagnostics = NULL;
}


/*******************************************************************************

********************************************************************************/
void CompilerCB::serialize(::zorba::serialization::Archiver& ar)
{
  ar.set_ccb(this);

  ar & theHasEval;
  ar & theIsEval;
  ar & theIsLoadProlog;
  ar & theIsUpdating;
  ar & theIsSequential;
  ar & theSctxMap;
  ar & theRootSctx;
#ifdef ZORBA_WITH_DEBUGGER
  ar & theDebuggerCommons;
#endif
  if (!ar.is_serializing_out())
  {
    //don't serialize this
    theXQueryDiagnostics = NULL;
  }
  ar & theConfig;
  ar & theHaveTimeout;
  ar & theTimeout;
  ar & theCommonLanguageEnabled;
}



/*******************************************************************************

********************************************************************************/
static_context* CompilerCB::getStaticContext(int c)
{
  SctxMap::iterator lIter;
  lIter = theSctxMap.find(c);
  assert(lIter != theSctxMap.end());
  return lIter->second.getp();
}


/***************************************************************************//**

********************************************************************************/
void CompilerCB::add_pragma(const expr* e, pragma* p)
{
  thePragmas.insert(std::make_pair(e, p));
}


void
CompilerCB::lookup_pragmas(const expr* e, std::vector<pragma*>& pragmas) const
{
  pragmas.clear();

  std::pair<PragmaMapIter, PragmaMapIter> lRange = thePragmas.equal_range(e);
  while (lRange.first != lRange.second)
  {
    pragmas.push_back(lRange.first->second);
    ++lRange.first;
  }
}

bool
CompilerCB::lookup_pragma(
    const expr* e,
    const zstring& localname,
    pragma*& p) const
{
  std::pair<PragmaMapIter, PragmaMapIter> lRange = thePragmas.equal_range(e);
  while (lRange.first != lRange.second)
  {
    if (lRange.first->second->theQName->getLocalName() == localname)
    {
      p = lRange.first->second;
      return true;
    }
    ++lRange.first;
  }
  return false;
}



} /* namespace zorba */
/* vim:set et sw=2 ts=2: */
