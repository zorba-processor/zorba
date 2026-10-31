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
#include "stdafx.h"

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

#include <zorba/properties.h>

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
  theHaveTimeout(false),
  theTimeout(timeout),
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
  theHaveTimeout(cb.theHaveTimeout),
  theTimeout(cb.theTimeout),
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

std::atomic<long> QueryResources::theNumAlive(0);

std::atomic<unsigned long> QueryResources::theNumWalks(0);


long query_resources_alive()
{
  return QueryResources::theNumAlive.load();
}


unsigned long query_resources_walks()
{
  return QueryResources::theNumWalks.load();
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
    theIsDetached = true;

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
    theIsDetached = false;
    theParent = NULL;
    theDctxParent = NULL;
    destroy();
  }
}


void QueryResources::release(rchandle<QueryResources>& resources)
{
  if (resources == NULL)
    return;

  if (resources->theIsDetached && resources->getRefCount() > 1)
  {
    rchandle<QueryResources> hold;
    hold.transfer(resources);
    QueryResources* held[1] = { hold.getp() };
    collectCycles(held, 1);
  }
  else
  {
    resources = NULL;
  }
}


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
