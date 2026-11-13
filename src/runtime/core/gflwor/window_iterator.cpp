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

#include "system/globalenv.h"

#include "compiler/api/compilercb.h"

#include "runtime/core/gflwor/window_iterator.h"
#include "runtime/core/gflwor/common.h"
#include "runtime/core/var_iterators.h"
#include "runtime/visitors/planiter_visitor.h"

#include "store/api/store.h"
#include "store/api/item_factory.h"


namespace zorba
{
namespace flwor
{

SERIALIZABLE_CLASS_VERSIONS(StartClause)

SERIALIZABLE_CLASS_VERSIONS(EndClause)

SERIALIZABLE_CLASS_VERSIONS(WindowIterator)

SERIALIZABLE_CLASS_VERSIONS(WindowVars)


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  WindowVars                                                                 //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
WindowVars::WindowVars()
{
}


/***************************************************************************//**
  Constructor
********************************************************************************/
WindowVars::WindowVars (
    const std::vector<PlanIter_t >& curVars,
    const std::vector<PlanIter_t >& prevVars,
    const std::vector<PlanIter_t >& nextVars,
    const std::vector<PlanIter_t >& posVars,
    const std::vector<PlanIter_t >& curOutVars,
    const std::vector<PlanIter_t >& prevOutVars,
    const std::vector<PlanIter_t >& nextOutVars,
    const std::vector<PlanIter_t >& posOutVars)
:
  theCurVars(curVars),
  thePrevVars(prevVars),
  theNextVars(nextVars),
  thePosVars(posVars),
  theCurOuterVars(curOutVars),
  thePrevOuterVars(prevOutVars),
  theNextOuterVars(nextOutVars),
  thePosOuterVars(posOutVars)
{
}


/***************************************************************************//**
  Destructor
********************************************************************************/
WindowVars::~WindowVars()
{
}


/***************************************************************************//**

********************************************************************************/
void WindowVars::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theCurVars;
  ar & thePrevVars;
  ar & theNextVars;
  ar & thePosVars;

  ar & theCurOuterVars;
  ar & thePrevOuterVars;
  ar & theNextOuterVars;
  ar & thePosOuterVars;
}


/***************************************************************************//**

********************************************************************************/
void WindowVars::accept(PlanIterVisitor& v, bool startClause) const
{
  zstring varName;

  if (startClause)
    varName = "start-";
  else
    varName = "end-";

  if (!theCurVars.empty())
  {
    varName += "CurrentIn";
    v.beginVisitWinCondVariable(varName, theCurVars);
    v.endVisitWinCondVariable();
  }

  if (!thePrevVars.empty())
  {
    varName += "PrevIn";
    v.beginVisitWinCondVariable(varName, thePrevVars);
    v.endVisitWinCondVariable();
  }

  if (!theNextVars.empty())
  {
    varName += "NextIn";
    v.beginVisitWinCondVariable(varName, theNextVars);
    v.endVisitWinCondVariable();
  }

  if (!thePosVars.empty())
  {
    varName += "PosIn";
    v.beginVisitWinCondVariable(varName, thePosVars);
    v.endVisitWinCondVariable();
  }

  if (!theCurOuterVars.empty())
  {
    varName += "CurrentOut";
    v.beginVisitWinCondVariable(varName, theCurOuterVars);
    v.endVisitWinCondVariable();
  }

  if (!thePrevOuterVars.empty())
  {
    varName += "PrevOut";
    v.beginVisitWinCondVariable(varName, thePrevOuterVars);
    v.endVisitWinCondVariable();
  }

  if (!theNextOuterVars.empty())
  {
    varName += "NextOut";
    v.beginVisitWinCondVariable(varName, theNextOuterVars);
    v.endVisitWinCondVariable();
  }

  if (!thePosOuterVars.empty())
  {
    varName += "PosOut";
    v.beginVisitWinCondVariable(varName, thePosOuterVars);
    v.endVisitWinCondVariable();
  }
}


/*******************************************************************************
  Binds the variables inside the window clause.

  @param planState The PlanState
  @param aInputSeq The underlying input sequence
  @param aPosition The position of the current item within the input sequence
                   (counting starts with 1).
********************************************************************************/
void WindowVars::bindIntern(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  store::Item_t item;

  if (!theCurVars.empty())
  {
    inputSeq->getItem(xs_integer(pos), item);

    bindVariables(item, theCurVars, planState);
  }

  if (!thePrevVars.empty())
  {
    if (pos > 1)
      inputSeq->getItem(xs_integer(pos - 1), item);
    else
      item = NULL;

    bindVariables(item, thePrevVars, planState);
  }

  if (!theNextVars.empty())
  {
    inputSeq->getItem(xs_integer(pos + 1), item);

    bindVariables(item, theNextVars, planState);
  }

  if (!thePosVars.empty())
  {
    GENV_ITEMFACTORY->createInteger(item, xs_integer(pos));

    bindVariables(item, thePosVars, planState);
  }
}


/*******************************************************************************
  Binds the variables outside the window clause.

  @param planState The PlanState
  @param inputSeq The underlying input sequence
  @param pos The position of the current item within the input sequence
                   (counting starts with 1).
********************************************************************************/
void WindowVars::bindExtern(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  store::Item_t item;

  if (!theCurOuterVars.empty())
  {
    inputSeq->getItem(xs_integer(pos), item);

    bindVariables(item, theCurOuterVars, planState);
  }

  if (!thePrevOuterVars.empty())
  {
    if (pos > 1)
      inputSeq->getItem(xs_integer(pos - 1), item);
    else
      item = NULL;

    bindVariables(item, thePrevOuterVars, planState);
  }

  if (!theNextOuterVars.empty())
  {
    inputSeq->getItem(xs_integer(pos + 1), item);

    bindVariables(item, theNextOuterVars, planState);
  }

  if (!thePosOuterVars.empty())
  {
    GENV_ITEMFACTORY->createInteger(item, Integer(pos));

    bindVariables(item, thePosOuterVars, planState);
  }
}


bool WindowVars::hasInnerItemRefs() const
{
  return !theCurVars.empty() || !thePrevVars.empty() || !theNextVars.empty();
}


bool WindowVars::hasOuterItemRefs() const
{
  return (!theCurOuterVars.empty() ||
          !thePrevOuterVars.empty() ||
          !theNextOuterVars.empty());
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  StartClause                                                                //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************

********************************************************************************/
StartClause::StartClause(
    PlanIter_t aStartClauseIter,
    WindowVars& aWindowVars)
  :
  theStartClauseIter(aStartClauseIter),
  theWindowVars(aWindowVars)
{
}


/***************************************************************************//**

********************************************************************************/
StartClause::~StartClause()
{
}


void StartClause::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theStartClauseIter;
  ar & theWindowVars;
}


/***************************************************************************//**

********************************************************************************/
uint32_t StartClause::getStateSizeOfSubtree() const
{
  return theStartClauseIter->getStateSizeOfSubtree();
}


/***************************************************************************//**

********************************************************************************/
void StartClause::accept(PlanIterVisitor& v) const
{
  theWindowVars.accept(v, true);
  theStartClauseIter->accept(v);
}


/***************************************************************************//**

********************************************************************************/
void StartClause::open(PlanState& planState, uint32_t& aOffset) const
{
  theStartClauseIter->open(planState, aOffset);
}


/***************************************************************************//**

********************************************************************************/
void StartClause::reset(PlanState& planState) const
{
  theStartClauseIter->reset(planState);
}


/***************************************************************************//**

********************************************************************************/
void StartClause::close(PlanState& planState) const
{
  theStartClauseIter->close(planState);
}


/***************************************************************************//**
  Evaluate the start condition expr on the item that is pointed to by the given
  position within the temp sequence that materializes the result of the domain
  expr.
********************************************************************************/
bool StartClause::evaluate(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  theWindowVars.bindIntern(planState, inputSeq, pos);

  return evalToBool(theStartClauseIter, planState);
}


/***************************************************************************//**
  Bind the references to the external start-condition variables to the values
  corresponding to the item that is pointed to by the given position within
  the temp sequence that materializes the result of the domain expr.
********************************************************************************/
void StartClause::bindExtern(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  theWindowVars.bindExtern(planState, inputSeq, pos);
}


/***************************************************************************//**

********************************************************************************/
void StartClause::bindIntern(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  theWindowVars.bindIntern(planState, inputSeq, pos);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  EndClause                                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  If the EndClause is missing in the case of a tumbling window, this constructor
  should be used.
********************************************************************************/
EndClause::EndClause()
  :
  theEndClauseIter(NULL),
  theOnlyEnd(false),
  theHasEndClause(false)
{
}


/***************************************************************************//**

********************************************************************************/
EndClause::EndClause(
    PlanIter_t aEndClauseIter,
    WindowVars& aWindowVars,
    bool aOnlyEnd)
  :
  theEndClauseIter(aEndClauseIter),
  theWindowVars(aWindowVars),
  theOnlyEnd(aOnlyEnd),
  theHasEndClause(true)
{
}


/***************************************************************************//**

********************************************************************************/
EndClause::~EndClause()
{
}


/***************************************************************************//**

********************************************************************************/
void EndClause::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theEndClauseIter;
  ar & theWindowVars;
  ar & theOnlyEnd;
  ar & theHasEndClause;
}


/***************************************************************************//**

********************************************************************************/
uint32_t EndClause::getStateSizeOfSubtree() const
{
  if (theHasEndClause)
    return theEndClauseIter->getStateSizeOfSubtree();
  else
    return 0;
}


/***************************************************************************//**

********************************************************************************/
void EndClause::accept(PlanIterVisitor& v) const
{
  //TODO more output
  if (theHasEndClause)
  {
    theWindowVars.accept(v, false);
    theEndClauseIter->accept(v);
  }
}


/***************************************************************************//**

********************************************************************************/
void EndClause::open(PlanState& planState, uint32_t& aOffset) const
{
  if (theHasEndClause)
    theEndClauseIter->open(planState, aOffset);
}


/***************************************************************************//**

********************************************************************************/
void EndClause::reset(PlanState& planState) const
{
  if (theHasEndClause)
    theEndClauseIter->reset(planState);
}


/***************************************************************************//**

********************************************************************************/
void EndClause::close(PlanState& planState) const
{
  if (theHasEndClause)
    theEndClauseIter->close(planState);
}


/***************************************************************************//**

********************************************************************************/
bool EndClause::evaluate(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  theWindowVars.bindIntern(planState, inputSeq, pos);

  return evalToBool(theEndClauseIter, planState);
}


/***************************************************************************//**

********************************************************************************/
void EndClause::bindIntern(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  theWindowVars.bindIntern(planState, inputSeq, pos);
}


/***************************************************************************//**

********************************************************************************/
void EndClause::bindExtern(
    PlanState& planState,
    const store::TempSeq_t& inputSeq,
    const ulong pos) const
{
  theWindowVars.bindExtern(planState, inputSeq, pos);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  WindowDef                                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


WindowDef::~WindowDef()
{
}

WindowDef::WindowDef(ulong aStartPos) : theStartPos(aStartPos), theEndPos(0)
{
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  WindowState                                                                //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

WindowState::WindowState() : theDomainSeq (NULL) , theCurInputPos(1), thePurgedUpTo(0)
{
}


WindowState::~WindowState()
{
}


void WindowState::init(PlanState& aState)
{
  PlanIteratorState::init(aState);
  theCurInputPos = 1;
  thePurgedUpTo = 0;
  theDomainSeq = NULL;
}


void WindowState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  theCurInputPos = 1;
  thePurgedUpTo = 0;
  theDomainSeq = NULL;
  theOpenWindows.clear();
  theUnclosedWindows.clear();
}



/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  WindowIterator                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/***************************************************************************//**

********************************************************************************/
WindowIterator::WindowIterator(
    static_context* sctx,
    const QueryLoc& loc,
    WindowType windowType,
    PlanIter_t tupleIter,
    PlanIter_t domainIter,
    PlanIter_t treatIter,
    store::Item* varName,
    const std::vector<PlanIter_t>& varRefs,
    StartClause& startClause,
    EndClause& endClause,
    bool lazyEval,
    bool endCondSameForAllWindows)
  :
  PlanIterator(sctx, loc),
  theWindowType(windowType),
  theTupleIter(tupleIter),
  theInputIter(domainIter),
  theTreatIter(treatIter),
  theVarName(varName),
  theStartClause(startClause),
  theEndClause(endClause),
  theLazyEval(lazyEval),
  theEndCondSameForAllWindows(endCondSameForAllWindows)
{
  castIterVector<LetVarIterator>(theVarRefs, varRefs);

  // The inner vars of the end condition read the current item and its
  // neighbors, which doGarbageCollection keeps for every window.
  theWindowsHoldItems = (!theVarRefs.empty() ||
                         theStartClause.theWindowVars.hasInnerItemRefs() ||
                         theStartClause.theWindowVars.hasOuterItemRefs() ||
                         theEndClause.theWindowVars.hasOuterItemRefs());
}


/***************************************************************************//**

********************************************************************************/
WindowIterator::~WindowIterator()
{
}


/***************************************************************************//**

********************************************************************************/
void WindowIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar, (PlanIterator*)this);
  SERIALIZE_ENUM(WindowType, theWindowType);
  ar & theTupleIter;
  ar & theInputIter;
  ar & theTreatIter;
  ar & theVarName;
  ar & theVarRefs;
  ar & theStartClause;
  ar & theEndClause;
  ar & theLazyEval;
  ar & theEndCondSameForAllWindows;
  ar & theWindowsHoldItems;
}


/***************************************************************************//**

********************************************************************************/
zstring WindowIterator::getNameAsString() const {
  return "WindowIterator";
}


uint32_t WindowIterator::getStateSize() const
{
  return StateTraitsImpl<WindowState>::getStateSize();
}


/***************************************************************************//**

********************************************************************************/
uint32_t WindowIterator::getStateSizeOfSubtree() const
{
  int32_t size = this->getStateSize();
  size += theTupleIter->getStateSizeOfSubtree();
  size += theInputIter->getStateSizeOfSubtree();
  size += theStartClause.getStateSizeOfSubtree();
  size += theEndClause.getStateSizeOfSubtree();

  if (theTreatIter)
    size += theTreatIter->getStateSizeOfSubtree();

  return size;
}


/***************************************************************************//**

********************************************************************************/
void WindowIterator::accept(PlanIterVisitor& v) const
{
  if (!v.hasToVisit(this))
    return;

  v.beginVisit(*this);

  v.beginVisitWindowVariable(theVarName->getStringValue().str(), theVarRefs);

  theInputIter->accept(v);

  v.endVisitWindowVariable();

  theStartClause.accept(v);
  theEndClause.accept(v);

  theTupleIter->accept(v);

  if (theTreatIter)
    theTreatIter->accept(v);

  v.endVisit(*this);
}


/***************************************************************************//**

********************************************************************************/
void WindowIterator::openImpl(PlanState& planState, uint32_t& offset)
{
  StateTraitsImpl<WindowState>::createState(planState, theStateOffset, offset);
  StateTraitsImpl<WindowState>::initState(planState, theStateOffset);

  theTupleIter->open(planState, offset);
  theInputIter->open(planState, offset);

  if (theTreatIter)
    theTreatIter->open(planState, offset);

  theStartClause.open(planState, offset);
  theEndClause.open(planState, offset);
}


/***************************************************************************//**

********************************************************************************/
void WindowIterator::resetImpl(PlanState& planState) const
{
  WindowState* lState = StateTraitsImpl<WindowState>::getState(planState,
                                                               theStateOffset);
  lState->reset(planState);

  theTupleIter->reset(planState);
  theInputIter->reset(planState);
  theStartClause.reset(planState);
  theEndClause.reset(planState);

  if (theTreatIter)
    theTreatIter->reset(planState);
}


/***************************************************************************//**

********************************************************************************/
void WindowIterator::closeImpl(PlanState& planState)
{
  theTupleIter->close(planState);
  theInputIter->close(planState);
  theStartClause.close(planState);
  theEndClause.close(planState);

  if (theTreatIter)
    theTreatIter->close(planState);

  StateTraitsImpl<WindowState>::destroyState(planState, theStateOffset);
}


/***************************************************************************//**
  Bind the window var to the items between the given starting and ending items
  of the domain sequence.
********************************************************************************/
void WindowIterator::bindVariable(
    PlanState& planState,
    store::TempSeq_t& inputSeq,
    ulong startPos,
    ulong endPos) const
{
  xs_integer const lStartPos(startPos);
  xs_integer const lEndPos(endPos);

  for (std::vector<LetVarIter_t>::const_iterator varIter = theVarRefs.begin();
       varIter != theVarRefs.end();
       ++varIter)
  {
    (*varIter)->bind(inputSeq, planState, lStartPos, lEndPos);
  }
}


/***************************************************************************//**
  Test the end condition of the candidate windows on the current item. Before
  evaluating the end condition expr, we must rebind the internal vars of the
  start condition, because those variables may be referenced in the end cond
  expr. If the condition has one value for all windows, it needs neither that
  nor more than one evaluation.
********************************************************************************/
void WindowIterator::checkEndCondition(
    WindowState* state,
    PlanState& planState) const
{
  std::vector<WindowDef*>& unclosed = state->theUnclosedWindows;

  if (unclosed.empty())
    return;

  ulong const pos = state->theCurInputPos;

  if (theEndCondSameForAllWindows)
  {
    if (theEndClause.evaluate(planState, state->theDomainSeq, pos))
    {
      for (WindowDef* window : unclosed)
        window->theEndPos = pos;

      unclosed.clear();
    }

    return;
  }

  csize numUnclosed = 0;

  for (WindowDef* window : unclosed)
  {
    theStartClause.bindIntern(planState,
                              state->theDomainSeq,
                              window->theStartPos);

    if (theEndClause.evaluate(planState, state->theDomainSeq, pos))
      window->theEndPos = pos;
    else
      unclosed[numUnclosed++] = window;
  }

  unclosed.resize(numUnclosed);
}


/***************************************************************************//**
  Release the items of the domain sequence that no reader can ask for anymore.
  This runs only after the tuple of the last returned window has been consumed:
  the consumer pulled the next tuple, so nothing reads the previous binding of
  the window var. A clause that keeps the window var past its tuple (order by,
  group by, a function item) has copied its items by then, and holds them with
  its own references.

  What remains readable by position, with p the current position:

  - p, as the "previous" item of the next position (p + 1 is not purged, nor
    anything after it);
  - if windows hold items: from the item before the start of the first open
    window on. That window, and the ones after it, bind the "previous" of their
    start, their start, the items between start and end, and the neighbors of
    their end. The first open window has the smallest start.
********************************************************************************/
void WindowIterator::doGarbageCollection(WindowState* state) const
{
  ulong firstKept = state->theCurInputPos;

  if (theWindowsHoldItems && !state->theOpenWindows.empty())
  {
    ulong const startPos = state->theOpenWindows.front().theStartPos;
    firstKept = startPos > 1 ? startPos - 1 : 1;
  }

  ulong const purgeTo = firstKept - 1;

  if (purgeTo > state->thePurgedUpTo)
  {
    state->theDomainSeq->purgeUpTo(xs_integer(purgeTo));
    state->thePurgedUpTo = purgeTo;
  }
}


/***************************************************************************//**
  Whether the domain is read as it goes. For a sliding window, theLazyEval was
  decided by the query that compiled it, which is not always the one that runs
  it: the body of a function loaded with StaticContext::loadProlog(), or of a
  function item made by another query, is compiled by that prolog or query. No
  side effect may run in the query being run either.
********************************************************************************/
bool WindowIterator::readsDomainLazily(const PlanState& planState) const
{
  if (!theLazyEval || theWindowType == TUMBLING)
    return theLazyEval;

  const CompilerCB* ccb = planState.theCompilerCB;

  return ccb != NULL && !ccb->theMayRunSideEffects;
}


/***************************************************************************//**

********************************************************************************/
bool WindowIterator::nextImpl(store::Item_t& aResult, PlanState& planState) const
{
  store::Iterator_t iterator;

  WindowState* state;
  DEFAULT_STACK_INIT(WindowState, state, planState);

  // Pull the next tuple from the input stream
  while (consumeNext(aResult, theTupleIter, planState))
  {
    // Create the temp sequence where to materialize the result of the domain
    // expr (lazily if readsDomainLazily).
    iterator = new PlanIteratorWrapper(theInputIter, planState);
    state->theDomainSeq = GENV_STORE.createTempSeq(iterator, readsDomainLazily(planState));

    // Its clever to switch quite early to avoid a lot of if-else statements
    if (theWindowType == WindowIterator::SLIDING)
    {
      // Get the next item from the domain sequence
      // TODO: can the xs_integer be hoisted?
      while (state->theDomainSeq->containsItem(xs_integer(state->theCurInputPos)))
      {
        // If the current item satisfies the start condition, create a candidate
        // window starting at the current domain item.
        if (theStartClause.evaluate(planState,
                                    state->theDomainSeq,
                                    state->theCurInputPos))
        {
          state->theOpenWindows.push_back(WindowDef(state->theCurInputPos));
          state->theUnclosedWindows.push_back(&state->theOpenWindows.back());
        }

        // For each candidate window, check if the current domain item satisfies
        // the end condition.
        checkEndCondition(state, planState);

        // Try to return closed windows to the consumer iterator. Notice that
        // windows must be sorted according to the position of their starting
        // items in the domain sequence. So, we can return a closed window only
        // if it appears as the first window in state->theOpenWindows.
        while (!state->theOpenWindows.empty() &&
               state->theOpenWindows.front().theEndPos != 0)
        {
          // The current window is closed and its starting item is before the
          // stating items of all other windows (open or closed) in the domain
          // sequence. So, (a) bind the window var and the external vars of 
          // the start and end conditions, (b) remove the window from the set
          // of candidate windows, (c) return to the caller a new tuple that
          // consists of the current input tuple augmented with one column per
          // variable that was bound in this step, and (d) purge from the domain
          // temp seq the items that only this window needed.
          theStartClause.bindExtern(planState,
                                    state->theDomainSeq,
                                    state->theOpenWindows.front().theStartPos);

          theEndClause.bindExtern(planState,
                                  state->theDomainSeq,
                                  state->theOpenWindows.front().theEndPos);

          bindVariable(planState,
                       state->theDomainSeq,
                       state->theOpenWindows.front().theStartPos,
                       state->theOpenWindows.front().theEndPos);

          state->theOpenWindows.pop_front();

          if (theTreatIter)
          {
            store::Item_t tmp;
            while (consumeNext(tmp, theTreatIter, planState))
            {
              ;
            }

            theTreatIter->reset(planState);
          }

          STACK_PUSH(true, state);

          doGarbageCollection(state);
        }

        doGarbageCollection(state);

        ++state->theCurInputPos;
      }
    }
    else //Tumpling window
    {
      // Doing this switch now also avoids further overhad
      if (theEndClause.theHasEndClause)
      {
        while (state->theDomainSeq->containsItem(xs_integer(state->theCurInputPos)))
        {
          if (state->theOpenWindows.empty() &&
              theStartClause.evaluate(planState,
                                      state->theDomainSeq,
                                      state->theCurInputPos))
          {
            theStartClause.bindExtern(planState,
                                      state->theDomainSeq,
                                      state->theCurInputPos);

            state->theOpenWindows.push_back(state->theCurInputPos);
          }

          if ( !state->theOpenWindows.empty() &&
               theEndClause.evaluate(planState,
                                     state->theDomainSeq,
                                     state->theCurInputPos))
          {
            theEndClause.bindExtern(planState,
                                    state->theDomainSeq,
                                    state->theCurInputPos);

            bindVariable(planState,
                         state->theDomainSeq,
                         state->theOpenWindows[0].theStartPos,
                         state->theCurInputPos);

            state->theOpenWindows.pop_back();

            assert(state->theOpenWindows.empty());

            if (theTreatIter)
            {
              store::Item_t tmp;
              while (consumeNext(tmp, theTreatIter, planState))
              {
                ;
              }

              theTreatIter->reset(planState);
            }

            STACK_PUSH(true, state);
          }

          doGarbageCollection(state);

          ++state->theCurInputPos;
        }
      }
      else
      {
        while (state->theDomainSeq->containsItem(xs_integer(state->theCurInputPos)))
        {
          if (theStartClause.evaluate(planState,
                                      state->theDomainSeq,
                                      state->theCurInputPos))
          {
            if (!state->theOpenWindows.empty())
            {
              //In no case there should be more than 1 position inside
              assert(state->theOpenWindows.size() == 1);

              theStartClause.bindExtern(planState,
                                        state->theDomainSeq,
                                        state->theOpenWindows[0].theStartPos);

              bindVariable(planState,
                           state->theDomainSeq,
                           state->theOpenWindows[0].theStartPos,
                           state->theCurInputPos  - 1);

              state->theOpenWindows.pop_back();

              assert(state->theOpenWindows.empty());

              if (theTreatIter)
              {
                store::Item_t tmp;
                while (consumeNext(tmp, theTreatIter, planState))
                {
                  ;
                }

                theTreatIter->reset(planState);
              }

              STACK_PUSH(true, state);
            }

            state->theOpenWindows.push_back(state->theCurInputPos);
          }

          doGarbageCollection(state);

          ++state->theCurInputPos;
        }
      }
    }

    // The domain sequence has ended. Return the windows that remain, in order.
    // Unclosed windows get the last item as their end, or are dropped if the end
    // condition says "only end".
    while (!state->theOpenWindows.empty())
    {
      if (!theEndClause.theOnlyEnd || state->theOpenWindows.front().theEndPos != 0)
      {
        if (state->theOpenWindows.front().theEndPos == 0)
          state->theOpenWindows.front().theEndPos = state->theCurInputPos - 1;

        bindVariable(planState,
                     state->theDomainSeq,
                     state->theOpenWindows.front().theStartPos,
                     state->theOpenWindows.front().theEndPos);

        theStartClause.bindExtern(planState,
                                  state->theDomainSeq,
                                  state->theOpenWindows.front().theStartPos);

        theEndClause.bindExtern(planState,
                                state->theDomainSeq,
                                state->theOpenWindows.front().theEndPos);

        state->theOpenWindows.pop_front();

        if (theTreatIter)
        {
          store::Item_t tmp;
          while (consumeNext(tmp, theTreatIter, planState))
          {
            ;
          }

          theTreatIter->reset(planState);
        }

        STACK_PUSH(true, state);

        doGarbageCollection(state);
      }
      else
      {
        state->theOpenWindows.pop_front();
      }
    }

    theInputIter->reset(planState);
    state->reset(planState);
  }

  STACK_PUSH(false, state);
  STACK_END(state);
}

  } //Namespace flwor
}//Namespace zorba
/* vim:set et sw=2 ts=2: */
