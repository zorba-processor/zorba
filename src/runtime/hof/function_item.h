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
#ifndef ZORBA_RUNTIME_FUNCTION_ITEM_H
#define ZORBA_RUNTIME_FUNCTION_ITEM_H

#include "common/shared_types.h"

#include "compiler/parser/query_loc.h"

#include "store/api/item.h"

#include "context/dynamic_context.h"
#include <zorba/internal/unique_ptr.h>

namespace zorba
{

class signature;
class function_item_expr;
class FunctionItemInfo;
class QueryResources;

typedef rchandle<FunctionItemInfo> FunctionItemInfo_t;

/*******************************************************************************
  A class to hold information about a dynamic function. This info is shared
  between the FunctionItemIterator and the FunctionItems it creates.

  theCCB :
  --------
  The ccb of the query (or eval query) whose plan creates the function items.
  Set by codegen (or by fn:function-lookup).

  theLoc:
  -------
  The location where the function item expr or inline function expr appear at.

  theClosureSctx:
  ---------------
  The static context to be used when the function item is actually invoked.

  theFunction:
  ------------
  The function obj that represents the implementation of this function item.
  This is always a pointer to a user_function obj. In case of an inline function
  expr, it is an anonymous user_function obj that is created on-the-fly by the
  translator to represent the body and signature of the inline function. In case
  of a function item expr where the named function is a UDF, it is the
  user_function obj of that UDF. Finally, in case of a function item expr where
  the named function F is not a UDF, it is a user_function obj UF that is created
  on-the-fly by the translator. The signature of UF is the same as that of F, and
  its body simply invokes F. The reason why UF is built is to unify the
  implemenation of dynamic function invocation.

  theQName:
  ---------

  theArity:
  ---------
  We need to store the arity also here because the function obj above doesn't
  know about its arity in case it's a variadic function.

  theIsInline:
  ------------

  theNeedsContextItem:
  --------------------
  Whether the function is a contextual one, i.e., accesses the context item, or
  context position, or context size directly.

  theIsCoercion:
  --------------
  This is set to true if the function item is a function coercion. In this
  case the newly created function item's name is taken from the coerced
  function.

  theScopedVarsValues:
  --------------------
  Empty in the case of LiteralFunctionItem. Otherwise, the vars that are in
  scope at the place where the InlineFunction expr appears at.

  theSubstVarsValues:
  -------------------

  theScopedVarsNames:
  -------------------

  theIsGlobalVar:
  ---------------

  theVarId:
  ---------

  theScopedVarsIteratosr:
  -----------------------

********************************************************************************/
class FunctionItemInfo : public SimpleRCObject
{
public:
  CompilerCB                  * theCCB;

  QueryLoc                      theLoc;

  static_context              * theClosureSctx;

  function                    * theFunction;

  store::Item_t                 theQName;

  csize                         theArity;

  bool                          theIsInline;

  bool                          theIsCoercion;

  std::vector<var_expr*>        theInScopeVars;
  std::vector<expr*>            theInScopeVarValues;

  std::vector<store::Item_t>    theInScopeVarNames;
  std::vector<ulong>            theInScopeVarIds;

  std::vector<PlanIter_t>       theInScopeVarIterators;

  store::NsBindings             theLocalBindings;

public:
  SERIALIZABLE_CLASS(FunctionItemInfo)
  FunctionItemInfo(::zorba::serialization::Archiver& ar);
  void serialize(::zorba::serialization::Archiver& ar);

public:
  FunctionItemInfo(
      static_context* closureSctx,
      const QueryLoc& loc,
      function* func,
      const store::Item_t& qname,
      csize arity,
      bool isInline,
      bool isCoercion);

  virtual ~FunctionItemInfo();

  csize numInScopeVars() const { return theInScopeVars.size(); }

  void add_variable(expr* var, var_expr* substVar);
};


/*******************************************************************************
  A FunctionItem is created during FunctionItemIterator::nextImpl().

  theResources:
  -------------
  The QueryResources of theFunctionItemInfo->theCCB. It keeps alive the
  compile-time state of the query that created this function item
  (user_function objs, exprs, static contexts) when the application keeps the
  item after that query is closed or destroyed. See QueryResources in
  compiler/api/compilercb.h.

  theFunctionResources:
  ---------------------
  The QueryResources of the ccb that owns theFunctionItemInfo->theFunction,
  if different from theResources (e.g. fn:function-lookup, evaluated by one
  query, of a UDF declared by another query). NULL otherwise.

  theGlobalResources:
  -------------------
  The QueryResources of the query that owns theGlobalDctx.

  theClosureDctxOwner:
  --------------------
  For a function item created by partial application (copy constructor): the
  function item that owns theClosureDctx, kept alive as long as this one.

  theFunctionItemInfo:
  --------------------
  The associated function-item info.

  theArity:
  ---------
  The arity of the function item will decrease when a partial application is used.

  theClosureDctx:
  ---------------
  The dynamic context obj to be used as the local dctx of the function, when the
  function item is dynamically invoked. This dctx is allocated during
  FunctionItemIterator::nextImpl(). At that time it is also populated with the
  values of the outer variables. Its parent is theGlobalDctx.

  theGlobalDctx:
  --------------
  The global dctx of the plan that created this function item: the values of
  the prolog variables that the function body reads, wherever the item is
  invoked (XQuery 3.0: the non-local variable bindings of the function's
  defining context). NULL if that dctx was temporary (e.g. constant folding),
  in which case the body uses the global dctx of the invoking plan.

  theArgValues:
  -------------

  theInternalRefs:
  ----------------
  Non-zero if this item is stored in the dctx of a detached QueryResources
  (see QueryResources::collectCycles()): the number of references to it from
  that graph when it was last examined. When its reference count drops to
  that number or below, the cycles through it may have become unreachable.

********************************************************************************/
class FunctionItem : public store::Item, public zorba::serialization::SerializeBaseClass
{
  friend class QueryResourcesCollector;

protected:
  // Declared first, so that they are released last.
  rchandle<QueryResources>        theResources;
  rchandle<QueryResources>        theFunctionResources;
  rchandle<QueryResources>        theGlobalResources;
  store::Item_t                   theClosureDctxOwner;

  FunctionItemInfo_t              theFunctionItemInfo;

  csize                           theArity;

  dynamic_context*                theClosureDctx;

  dynamic_context*                theGlobalDctx;

  std::vector<PlanIter_t>         theArgValues;

  bool                            theClosureDctxOwned;

  long                            theInternalRefs;

  SYNC_CODE(mutable RCLock        theRCLock;)


public:
  SERIALIZABLE_CLASS(FunctionItem)
  FunctionItem(::zorba::serialization::Archiver& ar);
  void serialize(::zorba::serialization::Archiver& ar);

public:
  FunctionItem(const FunctionItem& aFunctionItem);

  /*
    closureDctx: see theClosureDctx (NULL if there are no outer variables).
    planState: the state of the plan that creates the item.
  */
  FunctionItem(
      const FunctionItemInfo_t& fiInfo,
      dynamic_context* closureDctx,
      const PlanState& planState);

  ~FunctionItem();

  SYNC_CODE(RCLock* getRCLock() const { return &theRCLock; })

  // Called by store::Item::removeReference() when the reference count has
  // dropped to remaining (> 0).
  void referenceDropped(long remaining)
  {
    if (theInternalRefs != 0 && remaining <= theInternalRefs)
      collectCycles();
  }

  bool isInline() const { return theFunctionItemInfo->theIsInline; }
  
  bool isCoercion() const { return theFunctionItemInfo->theIsCoercion; }

  const store::Item_t getFunctionName() const;

  csize getArity() const { return theArity; }
  
  csize getStartArity() const;

  const signature& getSignature() const;

  dynamic_context* getDctx() const { return theClosureDctx; }

  dynamic_context* getGlobalDctx() const { return theGlobalDctx; }

  QueryResources* getGlobalResources() const { return theGlobalResources.getp(); }

  ulong getMaxInScopeVarId() const;

  const std::vector<PlanIter_t>& getArgValues() const { return theArgValues; }

  void setArgumentValue(csize pos, const PlanIter_t& value);

  bool isArgumentApplied(csize pos) const;

  PlanIter_t getImplementation(
      const std::vector<PlanIter_t>& argValues,
      CompilerCB* ccb);
  
  PlanIter_t getImplementation(CompilerCB* ccb);

  zstring show() const;

private:
  void collectCycles();
};


}//end of zorba namespace

#endif
/* vim:set et sw=2 ts=2: */
