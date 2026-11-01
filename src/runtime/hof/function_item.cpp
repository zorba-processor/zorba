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

// This include needs to be kept in order to make sure the
// unique_ptr<dynamic_context> manages to dealocate the
// dynamic_context object.
#include "context/dynamic_context.h"

#include "runtime/hof/function_item.h"
#include "runtime/core/fncall_iterator.h"
#include "runtime/base/plan_iterator.h"
#include "runtime/api/plan_iterator_wrapper.h"
#include "runtime/visitors/planiter_visitor.h"

#include "compiler/api/compilercb.h"
#include "compiler/expression/var_expr.h"
#include "compiler/expression/function_item_expr.h"
#include "compiler/expression/expr_manager.h"

#include "functions/signature.h"
#include "functions/udf.h"

#include "store/api/temp_seq.h"
#include "store/api/iterator.h"

#include <map>
#include <memory>
#include <new>

#include "zorbaserialization/serialize_template_types.h"
#include "zorbaserialization/serialize_zorba_types.h"


namespace zorba
{

SERIALIZABLE_CLASS_VERSIONS(FunctionItemInfo)

SERIALIZABLE_CLASS_VERSIONS(FunctionItem)


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  FunctionItem                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************

********************************************************************************/
FunctionItemInfo::FunctionItemInfo(
    static_context* closureSctx,
    const QueryLoc& loc,
    function* func,
    const store::Item_t& qname,
    csize arity,
    bool isInline,
    bool isCoercion)
  :
  theCCB(NULL),
  theLoc(loc),
  theClosureSctx(closureSctx),
  theFunction(func),
  theQName(qname),
  theArity(arity),
  theIsInline(isInline),
  theIsCoercion(isCoercion)
{
#if 0
  std::cerr << std::endl;

  if (theFunction && theFunction->getName() != NULL)
    std::cerr << "Allocated FunctionItemInfo " << this << " for function: "
              << theFunction->getName()->getStringValue()
              << std::endl;
  else if (theQName != NULL)
    std::cerr << "Allocated FunctionItemInfo " << this << " for function: "
              << theQName->getStringValue()
              << std::endl;
  else
    std::cerr << "Allocated FunctionItemInfo " << this
              << " for anonymous function at loc: " << loc << std::endl;

  std::cerr << std::endl;
#endif
}


FunctionItemInfo::FunctionItemInfo(::zorba::serialization::Archiver& ar)
{
}


FunctionItemInfo::~FunctionItemInfo()
{
#if 0
  std::cerr << std::endl;

  if (theFunction->getName() != NULL)
    std::cerr << "DeAllocated FunctionItemInfo " << this << " for function: "
              << theFunction->getName()->getStringValue()
              << std::endl;
  else
    std::cerr << "DeAllocated FunctionItemInfo " << this << " for anonymous function"
              << std::endl;

  std::cerr << std::endl;
#endif
}


void FunctionItemInfo::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theCCB;
  ar & theClosureSctx;
  ar & theLoc;
  ar & theFunction;
  ar & theQName;
  ar & theArity;
  ar & theIsInline;
  ar & theIsCoercion;

  // These are not serialized
  // ar & theScopedVarsValues;
  // ar & theSubstVarsValues;

  ar & theInScopeVarNames;
  ar & theInScopeVarIds;

  ar & theInScopeVarIterators;

  if (ar.is_serializing_out())
  {
    uint32_t planStateSize;
    (void)static_cast<user_function*>(theFunction)->getPlan(planStateSize, 1);
  }
}


/*******************************************************************************

********************************************************************************/
void FunctionItemInfo::add_variable(expr* var, var_expr* substVar)
{
  theInScopeVarValues.push_back(var);
  theInScopeVars.push_back(substVar);
  theInScopeVarNames.push_back(substVar->get_name());
  theInScopeVarIds.push_back(substVar->get_unique_id());
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  FunctionItem                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


FunctionItem::FunctionItem(const FunctionItem& aFunctionItem)
  :
  store::Item(store::Item::FUNCTION),
  theResources(aFunctionItem.theResources),
  theFunctionResources(aFunctionItem.theFunctionResources),
  theGlobalResources(aFunctionItem.theGlobalResources),
  theFunctionItemInfo(aFunctionItem.theFunctionItemInfo),
  theArity(aFunctionItem.theArity),
  theClosureDctx(aFunctionItem.getDctx()),
  theGlobalDctx(aFunctionItem.theGlobalDctx),
  theClosureDctxOwned(false),
  theInternalRefs(0),
  theInternalRefsGeneration(0)
{
  theArgValues.insert(theArgValues.begin(), aFunctionItem.theArgValues.begin(), aFunctionItem.theArgValues.end());

  // The closure dctx is shared, not copied: keep its owner alive.
  if (theClosureDctx != NULL)
  {
    if (aFunctionItem.theClosureDctxOwned)
      theClosureDctxOwner = const_cast<FunctionItem*>(&aFunctionItem);
    else
      theClosureDctxOwner = aFunctionItem.theClosureDctxOwner;
  }
}

/*******************************************************************************

********************************************************************************/
FunctionItem::FunctionItem(
    const FunctionItemInfo_t& fiInfo,
    dynamic_context* closureDctx,
    const PlanState& planState)
  :
  store::Item(store::Item::FUNCTION),
  theFunctionItemInfo(fiInfo),
  theArity(fiInfo->theArity),
  theClosureDctx(closureDctx),
  theGlobalDctx(NULL),
  theClosureDctxOwned(true),
  theInternalRefs(0),
  theInternalRefsGeneration(0)
{
  assert(theFunctionItemInfo->theFunction->isUdf());
  theArgValues.resize(theArity);

  // The closure dctx is the local dctx of the body when the item is invoked:
  // its variables are edges of the graph QueryResources::collectCycles()
  // examines.
  if (theClosureDctx != NULL)
    theClosureDctx->track_writes();

  // See theResources, theFunctionResources and theGlobalResources.
  CompilerCB* ccb = theFunctionItemInfo->theCCB;

  if (ccb != NULL)
    theResources = ccb->theResources;

  CompilerCB* udfCCB =
  static_cast<user_function*>(theFunctionItemInfo->theFunction)->getCCB();

  if (udfCCB != NULL && udfCCB->theResources != theResources.getp())
    theFunctionResources = udfCCB->theResources;

  // A global dctx that nothing keeps alive is not remembered (see
  // theGlobalDctx).
  theGlobalResources = planState.getGlobalDynCtxResources();

  if (theGlobalResources != NULL && !planState.theGlobalDynCtxIsTemporary)
    theGlobalDctx = planState.theGlobalDynCtx;

#if 0
  if (theFunctionItemInfo->theFunction->getName() != NULL)
    std::cerr << "Allocated FunctionItem " << this << " for function: "
              << theFunctionItemInfo->theFunction->getName()->getStringValue()
              << std::endl;
  else
    std::cerr << "Allocated FunctionItem " << this << " for anonymous function"
              << std::endl;
#endif
}


/*******************************************************************************

********************************************************************************/
FunctionItem::FunctionItem(::zorba::serialization::Archiver& ar)
  :
  store::Item(store::Item::FUNCTION),
  theClosureDctx(NULL),
  theGlobalDctx(NULL),
  theClosureDctxOwned(false),
  theInternalRefs(0),
  theInternalRefsGeneration(0)
{
}


/*******************************************************************************

********************************************************************************/
FunctionItem::~FunctionItem()
{
#if 0
  if (theFunctionItemInfo->theFunction->getName() != NULL)
    std::cerr << "DeAllocated FunctionItem " << this << " for function: "
              << theFunctionItemInfo->theFunction->getName()->getStringValue()
              << std::endl;
  else
    std::cerr << "DeAllocated FunctionItem " << this << " for anonymous function"
              << std::endl;
#endif
  if (theClosureDctxOwned)
    delete theClosureDctx;

  if (refersToDetached())
  {
    // This item may have held the last reference from outside a cycle (see
    // QueryResources::collectCycles()). Release everything else first: it
    // may point into the state the QueryResources keep.
    theArgValues.clear();
    theFunctionItemInfo = NULL;
    theClosureDctxOwner = NULL;

    rchandle<QueryResources> held[3];
    held[0].transfer(theResources);
    held[1].transfer(theFunctionResources);
    held[2].transfer(theGlobalResources);

    QueryResources* heldPtrs[3] = { held[0].getp(), held[1].getp(), held[2].getp() };
    QueryResources::collectCycles(heldPtrs, 3);
  }
}


/*******************************************************************************

********************************************************************************/
bool FunctionItem::refersToDetached() const
{
  return
    (theResources != NULL && theResources->isDetached()) ||
    (theFunctionResources != NULL && theFunctionResources->isDetached()) ||
    (theGlobalResources != NULL && theGlobalResources->isDetached());
}


/*******************************************************************************
  See theInternalRefs.
********************************************************************************/
void FunctionItem::referenceDropped(long remaining)
{
  if (theInternalRefsGeneration == QueryResources::graphGeneration())
  {
    if (remaining > theInternalRefs)
      return;
  }
  else if (!refersToDetached())
  {
    return;
  }

  collectCycles();
}


/*******************************************************************************
  See theInternalRefs.
********************************************************************************/
void FunctionItem::collectCycles()
{
  QueryResources* roots[3] =
  { theResources.getp(), theFunctionResources.getp(), theGlobalResources.getp() };

  if (!QueryResources::mayCollect(roots, 3))
    return;

  rchandle<QueryResources> held[3];
  held[0] = theResources;
  held[1] = theFunctionResources;
  held[2] = theGlobalResources;

  // "this" may be destroyed by the collection: only the copies are used
  // from here on.
  QueryResources* heldPtrs[3] = { held[0].getp(), held[1].getp(), held[2].getp() };
  QueryResources::collectCycles(heldPtrs, 3);
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  QueryResources::collectCycles()                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
  Trial deletion over the graph of a detached QueryResources (see the class
  comment of QueryResources in compiler/api/compilercb.h).

  The nodes are QueryResources objs, function items, JSON objects and arrays,
  and (eager) temp sequences; the edges are the references among them that
  this class can see:

  - a detached QueryResources (or the root) refers to the values of its dctx
    variables and to its parents;
  - a function item refers to its QueryResources, to the item that owns its
    closure dctx, and to the values of its closure dctx variables;
  - a JSON object or array refers to its members;
  - an eager temp sequence refers to its items.

  A node whose reference count is greater than the number of edges into it is
  referenced from outside the graph, and so is every node reachable from it.
  If the root is not, nothing outside the graph can reach it any more: the
  graph consists of cycles only, and releasing the dctx variable values of the
  unreachable QueryResources frees it. References this class cannot see (e.g.
  items inside a lazy temp sequence or in partial-application arguments) only
  make it conservative: they count as references from outside.
********************************************************************************/
class QueryResourcesCollector
{
public:
  // Set while a graph is being built: the reference count changes it causes
  // (temporary handles) must not start a collection.
  static thread_local bool theIsBuilding;

  // Set while garbage is being released: the collections started meanwhile
  // only add their roots here (see QueryResources::collectCycles()).
  static thread_local std::vector<QueryResources_t>* thePending;

  enum Kind { RESOURCES, ITEM, JSON, SEQ };

private:
  struct Node
  {
    Kind                theKind;
    const void        * thePtr;
    long                theRefs;
    long                theInternalRefs;
    bool                theIsLive;
    bool                theReachesRoot;
    std::vector<Node*>  theOut;
    std::vector<Node*>  theIn;
  };

  class BuildingGuard
  {
    bool theSaved;
  public:
    BuildingGuard() : theSaved(theIsBuilding) { theIsBuilding = true; }
    ~BuildingGuard() { theIsBuilding = theSaved; }
  };

  class IteratorGuard
  {
    store::Iterator* theIter;
  public:
    IteratorGuard(store::Iterator* iter) : theIter(iter) { theIter->open(); }
    ~IteratorGuard() { theIter->close(); }
  };

  QueryResources * const * theHeld;
  csize                    theNumHeld;

  // See QueryResources::theFailWalkAtEdge.
  unsigned long            theFailAtEdge;
  unsigned long            theNumEdges;

  std::map<const void*, Node*> theNodes;
  std::vector<Node*>           theToVisit;

public:
  QueryResourcesCollector(QueryResources* const* held, csize numHeld)
    :
    theHeld(held),
    theNumHeld(numHeld),
    theFailAtEdge(QueryResources::theFailWalkAtEdge.load(std::memory_order_relaxed)),
    theNumEdges(0)
  {
  }

  ~QueryResourcesCollector()
  {
    std::map<const void*, Node*>::iterator ite = theNodes.begin();
    for (; ite != theNodes.end(); ++ite)
      delete ite->second;
  }

  /*
    The reference count of a node, not counting the references in held.
  */
  static long refCount(
      int kind,
      const void* ptr,
      QueryResources* const* held,
      csize numHeld)
  {
    switch (kind)
    {
    case RESOURCES:
    {
      const QueryResources* res = static_cast<const QueryResources*>(ptr);
      long refs = res->getRefCount();
      for (csize i = 0; i < numHeld; ++i)
      {
        if (held[i] == res)
          --refs;
      }
      return refs;
    }
    case ITEM:
      return static_cast<const FunctionItem*>(ptr)->getRefCount();
    case JSON:
      return static_cast<const store::Item*>(ptr)->getRefCount();
    case SEQ:
      return static_cast<const store::TempSeq*>(ptr)->getRefCount();
    }
    return 0;
  }

  /*
    Returns true if the last collection of root found it referenced from
    outside its graph and still does (see "Cost" in the class comment of
    QueryResources).
  */
  static bool isKnownLive(
      const QueryResources* root,
      QueryResources* const* held,
      csize numHeld)
  {
    if (root->theWitnessGeneration != QueryResources::graphGeneration())
      return false;

    for (csize i = 0; i < root->theNumWitnesses; ++i)
    {
      const QueryResources::Witness& w = root->theWitnesses[i];
      if (refCount(w.theKind, w.thePtr, held, numHeld) > w.theInternalRefs)
        return true;
    }
    return false;
  }

  /*
    Returns true if root is unreachable from outside its graph, after adding
    to garbage the QueryResources objs of the graph whose dctx variables must
    be released. Otherwise records in root the witnesses, and in the function
    items of the graph how many references the graph has to them.
  */
  bool run(QueryResources* root, std::vector<QueryResources_t>& garbage)
  {
    unsigned long generation = QueryResources::graphGeneration();

    ++QueryResources::theNumWalks;

    Node* rootNode;
    {
      BuildingGuard guard;

      rootNode = getNode(RESOURCES, root);

      while (!theToVisit.empty())
      {
        Node* node = theToVisit.back();
        theToVisit.pop_back();
        visit(node);
      }
    }

    // Mark everything reachable from a node referenced from outside.
    std::vector<Node*> work;
    std::map<const void*, Node*>::iterator ite = theNodes.begin();
    for (; ite != theNodes.end(); ++ite)
    {
      Node* node = ite->second;
      if (node->theRefs > node->theInternalRefs)
      {
        node->theIsLive = true;
        work.push_back(node);
      }
    }

    while (!work.empty())
    {
      Node* node = work.back();
      work.pop_back();

      std::vector<Node*>::iterator outIte = node->theOut.begin();
      for (; outIte != node->theOut.end(); ++outIte)
      {
        if (!(*outIte)->theIsLive)
        {
          (*outIte)->theIsLive = true;
          work.push_back(*outIte);
        }
      }
    }

    if (rootNode->theIsLive)
    {
      recordWitnesses(root, rootNode, generation);

      for (ite = theNodes.begin(); ite != theNodes.end(); ++ite)
      {
        Node* node = ite->second;
        if (node->theKind == ITEM)
        {
          FunctionItem* item = const_cast<FunctionItem*>(
          static_cast<const FunctionItem*>(node->thePtr));

          // An item can be part of several graphs: keep the largest count
          // seen in this generation.
          if (item->theInternalRefsGeneration != generation ||
              item->theInternalRefs < node->theInternalRefs)
          {
            item->theInternalRefs = node->theInternalRefs;
          }
          item->theInternalRefsGeneration = generation;
        }
      }
      return false;
    }

    root->theWitnessGeneration = 0;

    for (ite = theNodes.begin(); ite != theNodes.end(); ++ite)
    {
      Node* node = ite->second;
      if (node->theKind == RESOURCES && !node->theIsLive)
      {
        QueryResources* res = const_cast<QueryResources*>(
        static_cast<const QueryResources*>(node->thePtr));
        if (res == root || res->theIsDetached)
          garbage.push_back(res);
      }
    }
    return true;
  }

private:
  /*
    Records in root up to MAX_WITNESSES nodes referenced from outside the
    graph from which root is reachable, preferring other nodes to
    QueryResources objs: the references to those come and go with every
    function item created by the query.
  */
  void recordWitnesses(QueryResources* root, Node* rootNode, unsigned long generation)
  {
    std::vector<Node*> work;
    rootNode->theReachesRoot = true;
    work.push_back(rootNode);

    while (!work.empty())
    {
      Node* node = work.back();
      work.pop_back();

      std::vector<Node*>::iterator inIte = node->theIn.begin();
      for (; inIte != node->theIn.end(); ++inIte)
      {
        if (!(*inIte)->theReachesRoot)
        {
          (*inIte)->theReachesRoot = true;
          work.push_back(*inIte);
        }
      }
    }

    root->theNumWitnesses = 0;

    for (int pass = 0; pass < 2; ++pass)
    {
      std::map<const void*, Node*>::iterator ite = theNodes.begin();
      for (; ite != theNodes.end(); ++ite)
      {
        Node* node = ite->second;

        if (root->theNumWitnesses == static_cast<csize>(QueryResources::MAX_WITNESSES))
          break;

        if (!node->theReachesRoot ||
            node->theRefs <= node->theInternalRefs ||
            (node->theKind == RESOURCES) != (pass == 1))
          continue;

        QueryResources::Witness& w = root->theWitnesses[root->theNumWitnesses++];
        w.thePtr = node->thePtr;
        w.theKind = node->theKind;
        w.theInternalRefs = node->theInternalRefs;
      }
    }

    root->theWitnessGeneration = generation;
  }

  Node* getNode(Kind kind, const void* ptr)
  {
    std::map<const void*, Node*>::iterator ite = theNodes.find(ptr);
    if (ite != theNodes.end())
      return ite->second;

    std::unique_ptr<Node> node(new Node);
    node->theKind = kind;
    node->thePtr = ptr;
    node->theRefs = refCount(kind, ptr, theHeld, theNumHeld);
    node->theInternalRefs = 0;
    node->theIsLive = false;
    node->theReachesRoot = false;

    Node* raw = node.get();
    theNodes[ptr] = raw;
    node.release();
    theToVisit.push_back(raw);
    return raw;
  }

  void addEdge(Node* from, Kind kind, const void* to)
  {
    if (++theNumEdges == theFailAtEdge)
    {
      QueryResources::theFailWalkAtEdge.store(0);
      throw std::bad_alloc();
    }

    Node* node = getNode(kind, to);
    ++node->theInternalRefs;
    from->theOut.push_back(node);
    node->theIn.push_back(from);
  }

  void addResources(Node* from, const QueryResources* res)
  {
    if (res != NULL)
      addEdge(from, RESOURCES, res);
  }

  void addItem(Node* from, const store::Item* item)
  {
    if (item == NULL)
      return;

    if (item->isFunction())
      addEdge(from, ITEM, static_cast<const FunctionItem*>(item));
    else if (item->isJSONItem())
      addEdge(from, JSON, item);
  }

  /*
    Adds an edge to each item that iter returns. The handle to each item is
    released before the edge is added, so that the reference count recorded
    for a new node does not include it.
  */
  void addItems(Node* from, store::Iterator* iter)
  {
    IteratorGuard guard(iter);
    store::Item_t item;
    while (iter->next(item))
    {
      const store::Item* raw = item.getp();
      item = NULL;
      addItem(from, raw);
    }
  }

  void addVariables(Node* from, const dynamic_context* dctx)
  {
    if (dctx == NULL)
      return;

    const std::vector<dynamic_context::VarValue>& vars = dctx->get_variables();
    std::vector<dynamic_context::VarValue>::const_iterator ite = vars.begin();
    for (; ite != vars.end(); ++ite)
    {
      if (ite->hasItemValue())
        addItem(from, ite->theValue.item);
      else if (ite->hasSeqValue())
        addEdge(from, SEQ, ite->theValue.temp_seq);
    }
  }

  void visit(Node* node)
  {
    switch (node->theKind)
    {
    case RESOURCES:
    {
      const QueryResources* res = static_cast<const QueryResources*>(node->thePtr);
      // An attached QueryResources is referenced by its owner: it is live,
      // and what its dctx holds does not matter.
      if (res->theIsDetached)
      {
        addVariables(node, res->theDctx);
        addResources(node, res->theParent.getp());
        addResources(node, res->theDctxParent.getp());
      }
      break;
    }
    case ITEM:
    {
      const FunctionItem* item = static_cast<const FunctionItem*>(node->thePtr);
      addResources(node, item->theResources.getp());
      addResources(node, item->theFunctionResources.getp());
      addResources(node, item->theGlobalResources.getp());
      addItem(node, item->theClosureDctxOwner.getp());
      if (item->theClosureDctxOwned)
        addVariables(node, item->theClosureDctx);
      break;
    }
    case JSON:
    {
      const store::Item* json = static_cast<const store::Item*>(node->thePtr);
      if (json->isArray())
      {
        store::Iterator_t iter = json->getArrayValues();
        addItems(node, iter.getp());
      }
      else
      {
        store::Iterator_t keys = json->getObjectKeys();
        IteratorGuard guard(keys.getp());
        store::Item_t key;
        while (keys->next(key))
        {
          store::Item_t value = json->getObjectValue(key);
          const store::Item* raw = value.getp();
          value = NULL;
          addItem(node, raw);
        }
      }
      break;
    }
    case SEQ:
    {
      store::TempSeq* seq =
      const_cast<store::TempSeq*>(static_cast<const store::TempSeq*>(node->thePtr));
      if (!seq->isLazy())
      {
        store::Iterator_t iter = seq->getIterator();
        addItems(node, iter.getp());
      }
      break;
    }
    }
  }
};


thread_local bool QueryResourcesCollector::theIsBuilding = false;

thread_local std::vector<QueryResources_t>* QueryResourcesCollector::thePending = NULL;


bool QueryResources::mayCollect(QueryResources* const* roots, csize numRoots)
{
  if (QueryResourcesCollector::theIsBuilding)
    return false;

  for (csize i = 0; i < numRoots; ++i)
  {
    QueryResources* root = roots[i];

    if (root != NULL && root->theIsDetached &&
        !QueryResourcesCollector::isKnownLive(root, NULL, 0))
      return true;
  }
  return false;
}


void QueryResources::collectCycles(QueryResources* const* held, csize numHeld)
{
  if (QueryResourcesCollector::theIsBuilding)
    return;

  if (QueryResourcesCollector::thePending != NULL)
  {
    // Garbage is being released: defer (see "Cost" in the class comment).
    std::vector<QueryResources_t>& pending = *QueryResourcesCollector::thePending;

    for (csize i = 0; i < numHeld; ++i)
    {
      QueryResources* root = held[i];

      if (root == NULL || !root->theIsDetached)
        continue;

      bool seen = false;
      for (csize j = 0; j < pending.size() && !seen; ++j)
        seen = (pending[j].getp() == root);

      if (!seen)
        pending.push_back(QueryResources_t(root));
    }
    return;
  }

  for (csize i = 0; i < numHeld; ++i)
  {
    QueryResources* root = held[i];

    if (root == NULL || !root->theIsDetached)
      continue;

    bool seen = false;
    for (csize j = 0; j < i; ++j)
      seen |= (held[j] == root);
    if (seen)
      continue;

    if (QueryResourcesCollector::isKnownLive(root, held, numHeld))
      continue;

    std::vector<QueryResources_t> garbage;
    {
      QueryResourcesCollector collector(held, numHeld);
      if (!collector.run(root, garbage))
        continue;
    }

    // Releasing the values frees the cycles; the handles in garbage keep the
    // QueryResources objs alive until all of them are done. The collections
    // started by what this destroys are deferred until then.
    std::vector<QueryResources_t> pending;
    {
      struct PendingGuard
      {
        PendingGuard(std::vector<QueryResources_t>* pending)
        {
          QueryResourcesCollector::thePending = pending;
        }
        ~PendingGuard()
        {
          QueryResourcesCollector::thePending = NULL;
        }
      } guard(&pending);

      graphChanged();

      std::vector<QueryResources_t>::iterator ite = garbage.begin();
      for (; ite != garbage.end(); ++ite)
      {
        if ((*ite)->theDctx)
          (*ite)->theDctx->release_variables();
      }

      garbage.clear();
    }

    if (!pending.empty())
    {
      std::vector<QueryResources*> pendingPtrs;
      for (csize j = 0; j < pending.size(); ++j)
        pendingPtrs.push_back(pending[j].getp());

      collectCycles(&pendingPtrs[0], pendingPtrs.size());
    }
  }
}


/*******************************************************************************

********************************************************************************/
void FunctionItem::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theFunctionItemInfo;
  ar & theArity;
  ar & theArgValues;
}


/*******************************************************************************

********************************************************************************/
const store::Item_t FunctionItem::getFunctionName() const
{
  return theFunctionItemInfo->theQName;
}


/*******************************************************************************
  Returns the arity of the function before any partial application
********************************************************************************/
csize FunctionItem::getStartArity() const
{
  return theFunctionItemInfo->theArity;
}


/*******************************************************************************

********************************************************************************/
const signature& FunctionItem::getSignature() const
{
  return theFunctionItemInfo->theFunction->getSignature();
}


/*******************************************************************************
  This function will return true if the pos-th argument of the function
  has been partially applied, i.e. theArgumentsValues[pos] is not NULL
********************************************************************************/
bool FunctionItem::isArgumentApplied(csize pos) const
{
  assert(pos < theArgValues.size());
  return (theArgValues[pos].getp() != NULL);
}


/*******************************************************************************

********************************************************************************/
void FunctionItem::setArgumentValue(csize pos, const PlanIter_t& value)
{
  theArity--;

  // find the pos-th NULL value and fill it
  for (csize i = 0; i < theArgValues.size(); ++i)
  {
    if (theArgValues[i] == NULL)
    {
      if (pos == 0)
      {
        theArgValues[i] = value;
        return;
      }
      else
      {
        pos--;
      }
    }
  }

  assert(false);
}


/*******************************************************************************

********************************************************************************/
ulong FunctionItem::getMaxInScopeVarId() const
{
  return theFunctionItemInfo->
         theInScopeVarIds[theFunctionItemInfo->theInScopeVarIds.size()-1];
}


/*******************************************************************************
  The getImplementation function assumes the dynChildren vector comes from a
  DynamicFnCallIterator, and as such, the first element of dynChildren is
  the function item itself, so it will be skipped.
********************************************************************************/
PlanIter_t FunctionItem::getImplementation(
    const std::vector<PlanIter_t>& dynChildren,
    CompilerCB* ccb)
{
  std::vector<PlanIter_t> args;
  args.resize(theArgValues.size());

  std::vector<PlanIter_t>::iterator argsIte = args.begin();
  std::vector<PlanIter_t>::iterator ite = theArgValues.begin();
  std::vector<PlanIter_t>::const_iterator ite2 = dynChildren.begin();
  ++ite2; // skip the first child because it's the function item

  for( ; argsIte != args.end(); ++argsIte, ++ite)
  {
    if (*ite != NULL)
    {
      *argsIte = *ite;
      static_cast<PlanStateIteratorWrapper*>(ite->getp())->reset();
    }
    else
    {
      *argsIte = *ite2;
      ++ite2;
    }
  }

  expr* dummy = ccb->theEM->
  create_function_item_expr(NULL,
                            NULL,
                            theFunctionItemInfo->theLoc,
                            false,
                            false);
  
  PlanIter_t udfCallIterator = theFunctionItemInfo->theFunction->
  codegen(ccb,
          theFunctionItemInfo->theClosureSctx,
          theFunctionItemInfo->theLoc,
          args,
          *dummy);

  UDFunctionCallIterator* udfIter =
  static_cast<UDFunctionCallIterator*>(udfCallIterator.getp());

  udfIter->setDynamic();
  udfIter->setFunctionItem(this);

  return udfCallIterator;
}


/*******************************************************************************

********************************************************************************/
PlanIter_t FunctionItem::getImplementation(CompilerCB* ccb)
{
  std::vector<PlanIter_t> args;

  expr* dummy = ccb->theEM->
  create_function_item_expr(NULL,
                            NULL,
                            theFunctionItemInfo->theLoc,
                            false,
                            false);
  
  PlanIter_t udfCallIterator = theFunctionItemInfo->theFunction->
  codegen(ccb,
          theFunctionItemInfo->theClosureSctx,
          theFunctionItemInfo->theLoc,
          args,
          *dummy);

  UDFunctionCallIterator* udfIter =
  static_cast<UDFunctionCallIterator*>(udfCallIterator.getp());

  udfIter->setDynamic();
  udfIter->setFunctionItem(this);

  return udfCallIterator;
}


/*******************************************************************************

********************************************************************************/
zstring FunctionItem::show() const
{
  std::ostringstream lRes;
  lRes << getFunctionName()->getStringValue();
  if (!isInline())
    lRes << "#" << getArity() << " (" << theFunctionItemInfo->theLoc << ")";
  return lRes.str();
}


} //namespace zorba
/* vim:set et sw=2 ts=2: */
