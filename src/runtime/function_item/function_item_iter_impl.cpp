/*
 * Copyright 2006-2008 The FLWOR Foundation.
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

#include "compiler/expression/function_item_expr.h"
#include "compiler/api/compilercb.h"
#include "compiler/translator/translator.h"

#include "functions/udf.h"

#include "runtime/function_item/function_item_iter.h"
#include "runtime/api/plan_iterator_wrapper.h"
#include "runtime/util/iterator_impl.h"
#include "runtime/function_item/function_item.h"
#include "runtime/core/fncall_iterator.h"

#include "context/dynamic_context.h"
#include "context/static_context.h"

#include "types/typeops.h"

#include "store/api/item_factory.h"
#include "store/api/store.h"
#include "store/api/temp_seq.h"

#include "diagnostics/util_macros.h"

#include "system/globalenv.h"

#include "zorbamisc/ns_consts.h"


using namespace std;

namespace zorba {


/*******************************************************************************

********************************************************************************/
bool FunctionLookupIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t qname;
  store::Item_t arityItem;
  uint32_t arity;
  result = NULL;
  
  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(qname, theChildren[0], planState);
  consumeNext(arityItem, theChildren[1], planState);
  
  try
  {
    arity = to_xs_unsignedInt(arityItem->getIntegerValue());
  }
  catch ( std::range_error const& )
  {
    RAISE_ERROR(err::XPST0017, loc,
    ERROR_PARAMS(arityItem->getIntegerValue(), ZED(NoParseFnArity)));
  }
  
  try
  {
//    std::auto_ptr<CompilerCB> closureCCB;
//    closureCCB.reset(new CompilerCB(*planState.theCompilerCB));
//    closureCCB->theRootSctx = theSctx;

//    expr* fiExpr = Translator::translate_literal_function(qname, arity, closureCCB.get(), loc, true);
    expr* fiExpr = Translator::translate_literal_function(qname, arity, planState.theCompilerCB, loc, true);
    
    DynamicFunctionInfo_t dynFnInfo = static_cast<function_item_expr*>(fiExpr)->get_dynamic_fn_info();
    // dynFnInfo->theCCB = closureCCB.release();
    // dynFnInfo->theMustDeleteCCB = true;
    dynFnInfo->theCCB = NULL;
    dynFnInfo->theClosureSctx = NULL;
    uint32_t planStateSize;
    static_cast<user_function*>(dynFnInfo->theFunction.getp())->getPlan(planStateSize);

    result = new FunctionItem(dynFnInfo, new dynamic_context(planState.theGlobalDynCtx));
  }
  catch (ZorbaException const& e)
  {
    if (e.diagnostic() != err::XPST0017)
      throw;
  }
  
  if (result.getp() != NULL)
    STACK_PUSH(true, state); 
    
  STACK_END(state);
}


/*******************************************************************************

********************************************************************************/
bool FunctionNameIterator::nextImpl(
    store::Item_t& r,
    PlanState& planState) const
{
  store::Item_t lFItem;
  FunctionItem* lFunctionItem = 0;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(lFItem, theChildren[0], planState);

  // function signature guarantees that
  ZORBA_ASSERT(lFItem->isFunction());

  lFunctionItem = static_cast<FunctionItem*>(lFItem.getp());

  if (!lFunctionItem->isInline() && lFunctionItem->getFunctionName())
  {
    // non-inline function
    r = lFunctionItem->getFunctionName();
    STACK_PUSH(true, state);
  }

  STACK_END(state);
}


/*******************************************************************************

********************************************************************************/
bool FunctionArityIterator::nextImpl(
    store::Item_t& r,
    PlanState& planState) const
{
  store::Item_t lFItem;
  FunctionItem* lFunctionItem = 0;
  xs_integer    lInt;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(lFItem, theChildren[0], planState);

  // function signature guarantees that
  ZORBA_ASSERT(lFItem->isFunction());

  lFunctionItem = static_cast<FunctionItem*>(lFItem.getp());

  lInt = Integer(lFunctionItem->getArity());

  STACK_PUSH(GENV_ITEMFACTORY->createInteger(r, lInt), state);

  STACK_END(state);
}


/*******************************************************************************

********************************************************************************/
FnMapPairsIteratorState::~FnMapPairsIteratorState()
{
  if (theIsOpen)
  {
    thePlan->close(*thePlanState);
  }
}


void FnMapPairsIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  thePlanState = &planState;
  thePlan = NULL;
  theIsOpen = false;
}


void FnMapPairsIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if (theIsOpen)
  {
    thePlan->reset(planState);
  }
}


uint32_t FnMapPairsIterator::getStateSizeOfSubtree() const
{
  uint32_t size = NaryBaseIterator<FnMapPairsIterator, FnMapPairsIteratorState>::
                  getStateSizeOfSubtree();

  return size + sizeof(UDFunctionCallIteratorState);
}


void FnMapPairsIterator::openImpl(PlanState& planState, uint32_t& offset)
{
  StateTraitsImpl<FnMapPairsIteratorState>::createState(planState,
                                                theStateOffset,
                                                offset);

  StateTraitsImpl<FnMapPairsIteratorState>::initState(planState, theStateOffset);

  FnMapPairsIteratorState* state =
  StateTraitsImpl<FnMapPairsIteratorState>::getState(planState, theStateOffset);

  state->theUDFStateOffset = offset;

  offset += sizeof(UDFunctionCallIteratorState);

  std::vector<PlanIter_t>::iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter )
        {
    (*lIter)->open(planState, offset);
  }
}


bool FnMapPairsIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t child1, child2;
  std::vector<PlanIter_t> arguments;
  
  FnMapPairsIteratorState* state;
  DEFAULT_STACK_INIT(FnMapPairsIteratorState, state, planState);

  consumeNext(state->theFnItem, theChildren[0], planState);

  // function signature guarantees that
  ZORBA_ASSERT(state->theFnItem->isFunction());

  while (true)
  {
    if (!consumeNext(child1, theChildren[1], planState) ||
        !consumeNext(child2, theChildren[2], planState))
      break;
        
    if (child1.getp() && child2.getp())
    {
      {
        store::TempSeq_t seq1, seq2;
        seq1 = GENV_STORE.createTempSeq(child1);
        seq2 = GENV_STORE.createTempSeq(child2);
        store::Iterator_t seqIter1 = seq1->getIterator();
        store::Iterator_t seqIter2 = seq2->getIterator();
        seqIter1->open();
        seqIter2->open();
       
        arguments.push_back(NULL); // the first argument is expected to be the function item and it is not used
        arguments.push_back(new PlanStateIteratorWrapper(seqIter1));
        arguments.push_back(new PlanStateIteratorWrapper(seqIter2));
      }
      
      state->thePlan = static_cast<FunctionItem*>(state->theFnItem.getp())->getImplementation(arguments, planState.theCompilerCB);
      // must be opened after vars and params are set
      state->thePlan->open(planState, state->theUDFStateOffset);
      state->theIsOpen = true;

      while (consumeNext(result, state->thePlan, planState))
      {
        STACK_PUSH(true, state);
      }
      
      // need to close here early in case the plan is completely
      // consumed. Otherwise, the plan would still be opened
      // if destroyed from the state's destructor.
      state->thePlan->close(planState);
      state->theIsOpen = false;
    }
    else
    {
      result = NULL;
      STACK_PUSH(true, state);
    }
  }

  STACK_END(state);
}


/*******************************************************************************

********************************************************************************/
FnFoldLeftIteratorState::~FnFoldLeftIteratorState()
{
  if (theIsOpen)
  {
    thePlan->close(*thePlanState);
  }
}


void FnFoldLeftIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  thePlanState = &planState;
  thePlan = NULL;
  theIsOpen = false;
}


void FnFoldLeftIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if (theIsOpen)
  {
    thePlan->reset(planState);
  }
}


uint32_t FnFoldLeftIterator::getStateSizeOfSubtree() const
{
  uint32_t size = NaryBaseIterator<FnFoldLeftIterator, FnFoldLeftIteratorState>::
                  getStateSizeOfSubtree();

  return size + sizeof(UDFunctionCallIteratorState);
}


void FnFoldLeftIterator::openImpl(PlanState& planState, uint32_t& offset)
{
  StateTraitsImpl<FnFoldLeftIteratorState>::createState(planState,
                                                theStateOffset,
                                                offset);

  StateTraitsImpl<FnFoldLeftIteratorState>::initState(planState, theStateOffset);

  FnFoldLeftIteratorState* state =
  StateTraitsImpl<FnFoldLeftIteratorState>::getState(planState, theStateOffset);

  state->theUDFStateOffset = offset;

  offset += sizeof(UDFunctionCallIteratorState);

  std::vector<PlanIter_t>::iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter )
        {
    (*lIter)->open(planState, offset);
  }
}


bool FnFoldLeftIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t curSeqItem, nextSeqItem, tempItem;
  std::vector<store::Item_t> zero;
  bool haveSeqItems;
  
  FnFoldLeftIteratorState* state;
  DEFAULT_STACK_INIT(FnFoldLeftIteratorState, state, planState);

  consumeNext(state->theFnItem, theChildren[0], planState);

  // function signature guarantees that
  ZORBA_ASSERT(state->theFnItem->isFunction());

  if ((haveSeqItems = consumeNext(curSeqItem, theChildren[2], planState)))
    haveSeqItems = consumeNext(nextSeqItem, theChildren[2], planState);
  
  if (curSeqItem.getp() == NULL && nextSeqItem.getp() == NULL)
  {
    // consume and return the "zero" argument
    while (consumeNext(result, theChildren[1], planState))
    {
      STACK_PUSH(true, state);
    }
  }
  else
  {
    // read in the "zero" argument to be able to pass it to the hof
    while (consumeNext(tempItem, theChildren[1], planState))
    {
      zero.push_back(tempItem);
    }
    
    while (true)
    {
      {
        store::TempSeq_t zeroSeq = GENV_STORE.createTempSeq(zero);
        tempItem = curSeqItem; // because createTempSeq calls .release() on the item rchandle
        store::TempSeq_t seq = GENV_STORE.createTempSeq(tempItem);
        store::Iterator_t seqIter1 = zeroSeq->getIterator();
        store::Iterator_t seqIter2 = seq->getIterator();
        seqIter1->open();
        seqIter2->open();
        
        std::vector<PlanIter_t> arguments;
        arguments.push_back(NULL);
        arguments.push_back(new PlanStateIteratorWrapper(seqIter1));
        arguments.push_back(new PlanStateIteratorWrapper(seqIter2));
        if (theIsFoldRight)
          std::reverse(++arguments.begin(), arguments.end());
        
        state->thePlan = static_cast<FunctionItem*>(state->theFnItem.getp())->getImplementation(arguments, planState.theCompilerCB);
        state->thePlan->open(planState, state->theUDFStateOffset);
        state->theIsOpen = true; 
      }
      
      if (curSeqItem.isNull() || nextSeqItem.isNull())
        break;
      
      zero.clear();
      while (consumeNext(tempItem, state->thePlan, planState))
      {
        zero.push_back(tempItem);
      }
      
      state->thePlan->close(planState);
      state->theIsOpen = false;
      
      curSeqItem = nextSeqItem;
      nextSeqItem = NULL;
      if (haveSeqItems)
        haveSeqItems = consumeNext(nextSeqItem, theChildren[2], planState);
      
    } // while (true)
    
    while (consumeNext(result, state->thePlan, planState))
    {
      STACK_PUSH(true, state);
    }
    
    state->thePlan->close(planState);
    state->theIsOpen = false;
    
  } // else
  
  STACK_END(state);
}

} // namespace zorba
/* vim:set et sw=2 ts=2: */
