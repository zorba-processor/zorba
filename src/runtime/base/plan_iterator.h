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
#ifndef ZORBA_RUNTIME_PLAN_ITERATOR
#define ZORBA_RUNTIME_PLAN_ITERATOR

#include <chrono>
#include <stack>

#include "common/shared_types.h"

#include "diagnostics/assert.h"

#include "runtime/util/flowctl_exception.h"

#include "compiler/parser/query_loc.h"

#include "zorba/util/timer.h"
#include "zorbaserialization/class_serializer.h"
#include "zorbaserialization/serialize_template_types.h"
#include "zorbaserialization/serialize_zorba_types.h"


// Info: Forcing inlining a function in g++:
// store::Item_t next() __attribute__((always_inline)) {...}

/*******************************************************************************

  Macros to automate Duff's Device and separation of code and state.

  These macros are used in the nextImpl() method of each iterator. In general,
  the nextImpl() method will be called multiple times, and during each invocation,
  its behaviour depends on the current state of the iterator (which includes
  info about what the previous invocation(s) did). The Duff's device is a
  techique by which the point where each invocation exited from is remembered,
  and during the next invocation, execution of the method resumes at that point.
  In general, this simplifies the implementation of the method.

  DEFUALT_STACK_INIT : Gets a pointer to the iterator's state in the state block
                       and "starts" the duff's device.
  STACK_PUSH         : Causes the nextImpl() method to return with the given
                       status and saves the current line in the source file
                       into the duffs line so that when the nextImpl method
                       is called again, execution will start right after the
                       STACK_PUSH macro.
  STACK_END          : Causes the nextImpl() method to return with false,
                       indicating that there are no more results to produce.
                       It also saves the current line in the source file into
                       the duffs line so that if the nextImpl method is called
                       again, an exception will be raised.
********************************************************************************/

#define DEFAULT_STACK_INIT(stateType, stateObject, planState)           \
  stateObject = StateTraitsImpl<stateType>::getState(planState, this->theStateOffset); \
  switch (stateObject->getDuffsLine())                                  \
  {                                                                     \
  case PlanIteratorState::DUFFS_ALLOCATE_RESOURCES:

#define STACK_PUSH(status, stateObject)                                 \
  do                                                                    \
  {                                                                     \
    stateObject->setDuffsLine(__LINE__);                                \
    return status;                                                      \
  case __LINE__: ;                                                      \
   } while (0)

#define STACK_END( stateObject)                                         \
  do {                                                                  \
    stateObject->setDuffsLine(__LINE__);                                \
    return false;                                                       \
  case __LINE__:                                                        \
    stateObject->setDuffsLine(__LINE__ + 1);                            \
    [[fallthrough]];                                                    \
  case __LINE__ + 1:                                                    \
    ZORBA_ASSERT (false && "nextImpl() called past iterator end");      \
    return false;                                                       \
  default:                                                              \
    return false;                                                       \
  } while (0);                                                          \
  }


namespace zorba
{

class RuntimeCB;
class PlanIterVisitor;
class dynamic_context;
class DebuggerCommons;
class XQueryImpl;
class QueryResources;


/*******************************************************************************
  Class to represent state that is shared by all plan iterators.

  theBlock        : Pointer to the memory block that stores the local state of
                    each individual plan iterator.
  theBlockSize    : Size (in bytes) of the block.

  theGlobalDynCtxResources :
                    The QueryResources of the query that owns theGlobalDynCtx,
                    if it is not that of theCompilerCB (a function item of
                    another query is being invoked; see FunctionItem). Use
                    getGlobalDynCtxResources().
  theGlobalDynCtxIsTemporary :
                    True if theGlobalDynCtx is a temporary dctx (e.g. one used
                    to fold an expression) that function items must not keep.

  theStackDepth   : Number of user-defined function calls (and eval levels)
                    between the query body and the plan run with this state.
  theMaxStackDepth: Maximum value of theStackDepth (security policy call depth;
                    UINT32_MAX = no limit).

  theDeadline     : Point in time after which the evaluation is aborted with
                    ZXQP0068 (security policy timeout or XQuery::setTimeout).
  theDeadlineCountdown :
                    0 if there is no deadline. Otherwise, the number of
                    consumeNext calls left before the clock is read again.
                    Reading the clock on every call would cost more than the
                    calls themselves; every DEADLINE_CHECK_INTERVAL calls it is
                    negligible. Once the deadline has passed, it stays at 1 so
                    that every further consumeNext call raises the error again
                    (code that swallows the error cannot keep running).
********************************************************************************/
class PlanState
{
public:
  int8_t                  * theBlock;

  uint32_t                  theBlockSize;

  uint32_t                  theStackDepth;

  uint32_t                  theMaxStackDepth;

  CompilerCB              * theCompilerCB;

  XQueryImpl              * theQuery;

  dynamic_context         * theGlobalDynCtx;

  dynamic_context         * theLocalDynCtx;

  QueryResources          * theGlobalDynCtxResources;

  bool                      theGlobalDynCtxIsTemporary;

  std::stack<store::Item*>  theNodeConstuctionPath;

  DebuggerCommons         * theDebuggerCommons;

  std::chrono::steady_clock::time_point theDeadline;

  std::chrono::milliseconds theTimeout;   // for the error message only

  uint32_t                  theDeadlineCountdown;

  bool const                theProfile;   // cache Properties::getProfile()

  bool                      theBlockOwned;

public:
  PlanState(
      dynamic_context* globalDctx,
      dynamic_context* localDctx,
      uint32_t blockSize,
      uint32_t aStackDepth = 0,
      uint32_t aMaxStackDepth = 1024);

  PlanState(PlanState& aPlanState);

  ~PlanState();

  static constexpr uint32_t DEADLINE_CHECK_INTERVAL = 1024;

  /**
   * Sets the call depth limit and the deadline from the security policy of
   * ccb's root static context (or of the root static context if it has none);
   * an XQuery::setTimeout() value stored in ccb overrides the policy timeout.
   * The deadline starts now.
   */
  void initLimits(const CompilerCB* ccb);

  /**
   * Inherits the call depth limit and the deadline of aParent (nested plans,
   * such as eval, share the limits of the query that runs them).
   */
  void inheritLimits(const PlanState& aParent);

  void setDeadline(std::chrono::milliseconds aTimeout);

  /**
   * Raises ZXQP0069 if a call at depth aDepth would exceed the call depth
   * limit or leave too little machine stack (less than a third of it; see
   * runtime/util/stack_limit.h).
   */
  void checkCallDepth(uint32_t aDepth, const QueryLoc& loc) const;

  void checkDepth(const QueryLoc& loc) { checkCallDepth(theStackDepth, loc); }

  /**
   * The QueryResources of the query that owns theGlobalDynCtx (NULL if
   * there is none).
   */
  QueryResources* getGlobalDynCtxResources() const;

  /**
   * Called by consumeNext every DEADLINE_CHECK_INTERVAL calls when there is a
   * deadline: raises ZXQP0068 if it has passed.
   */
  void checkDeadline(const QueryLoc& loc);
};


/*******************************************************************************

 ******************************************************************************/

/**
 * Contains per-member-function profiling data.
 *
 * An init() member function is used rather than a constructor so
 * initialization is done only when profiling is enabled.
 */
struct mbr_fn {
  unsigned call_count_;
  unsigned next_count_;
  double cpu_time_;
  double wall_time_;

  void init() {
    call_count_ = 0;
    next_count_ = 0;
    cpu_time_ = 0;
    wall_time_ = 0;
  }

  void addCall() {
    ++call_count_;
  }

  void addNext() {
    ++next_count_;
  }

  void addTime( double cpu, double wall ) {
    cpu_time_ += cpu;
    wall_time_ += wall;
  }

  void removeTime( double cpu, double wall ) {
      cpu_time_ -= cpu;
      wall_time_ -= wall;
    }
};


/**
 * Contains all profiling data for an iterator.
 *
 * An init() member function is used rather than a constructor so
 * initialization is done only when profiling is enabled.
 */
struct profile_data
{
  mbr_fn data_;

  void init()
  {
    data_.init();
  }
};

/*******************************************************************************
  Base class for all iterator state objects.
********************************************************************************/
class PlanIteratorState
{
  friend class TimerWrapper;
  friend class PrinterVisitor;
public:
  static const uint32_t DUFFS_ALLOCATE_RESOURCES = 0;

private:
  uint32_t        theDuffsLine;

  profile_data    profile_data_;
  friend class PlanIterator;

#ifndef NDEBUG
public:
  bool            theIsOpened;
#endif

public:
  PlanIteratorState()
    :
    theDuffsLine(DUFFS_ALLOCATE_RESOURCES)
#ifndef NDEBUG
    , theIsOpened(false)
#endif
  {
  }

  void setDuffsLine(uint32_t v) { theDuffsLine = v; }

  uint32_t getDuffsLine() const { return theDuffsLine; }

  /*
   * Initialize the current state object.
   *
   * This method is invoked be the openImpl() method of the associated iterator
   * to initialize the state info needed by the iterator. All initialization of
   * such info should be done in this function. If resources are acquired during
   * initialization, they must be released in the destructor.
   *
   * Classes that inherit from PlanIteratorState must reimplement this method.
   * Each subclass implementation of this method must call the init() method of
   * their parent class explicitly in order to guarantee proper initialization.
   */
  void init(PlanState &planState)
  {
    theDuffsLine = DUFFS_ALLOCATE_RESOURCES;
    if ( planState.theProfile )
      profile_data_.init();
  }

  /*
   * Reset the current state object.
   *
   * This method is invoked by the resetImpl() method of the associated iterator.
   * It resets the state info so that when the nextImpl() method of the iterator
   * is called again after a resetImpl(), it will behave as if it is called for
   * the first time.
   *
   * Classes that inherit from PlanIteratorState must reimplement this method.
   * Each subclass implementation of this method must call the reset() method
   * of their parent class explicitly in order to guarantee proper reset.
   */
  void reset(PlanState&)
  {
    theDuffsLine = DUFFS_ALLOCATE_RESOURCES;
  }

  profile_data const& get_profile_data() const {
    return profile_data_;
  }
};


/*******************************************************************************

********************************************************************************/
template <class T>
class StateTraitsImpl
{
private:
  StateTraitsImpl() {} // prevent instantiation

public:
  static uint32_t getStateSize()
  {
    return sizeof(T);
  }

  static T* getState(PlanState& planState, uint32_t stateOffset)
  {
    return reinterpret_cast<T*>(planState.theBlock + stateOffset);
  }

  static void createState(PlanState& planState, uint32_t& stateOffset, uint32_t& offset)
  {
    stateOffset = offset;
    offset += StateTraitsImpl<T>::getStateSize();
    new (planState.theBlock + stateOffset)T();
  }

  static void initState(PlanState& planState, uint32_t& stateOffset)
  {
    getState(planState, stateOffset)->init(planState);
  }

  static void reset(PlanState& planState, uint32_t stateOffset)
  {
    getState( planState, stateOffset )->reset( planState );
  }

  static void destroyState( PlanState &planState, uint32_t stateOffset ) {
    //
    // A bug elsewhere causes destroyState() to be called more than once on the
    // same state.  To work around that as-of-yet unfound bug, write a sentinel
    // into the raw memory after destruction and never call the destructor
    // again if the sentinal value is there.
    //
    uint32_t *const dead =
      reinterpret_cast<uint32_t*>( planState.theBlock + stateOffset );
    if ( *dead != 0xDEADBEEF ) {
      reinterpret_cast<T*>( dead )->~T();
      *dead = 0xDEADBEEF;
    }
  }
};

typedef void (mbr_fn::*method)();

class TimerWrapper
{
  bool               theProfilingEnabled;
  PlanIteratorState* theState;
  time::cpu::timer   theCPUTimer;
  time::wall::timer  theWallTimer;
  method             theMethod;

public:
  TimerWrapper(PlanIteratorState* aState, bool aProfile, method aMethod):
    theProfilingEnabled(aProfile),
    theState(aState),
    theMethod(aMethod)
  {
    if (theProfilingEnabled)
    {
      theCPUTimer.start();
      theWallTimer.start();
    }
  }

  void setState(PlanIteratorState* aState)
  {
    theState = aState;
  }

  ~TimerWrapper()
  {
    if (theProfilingEnabled && theState)
    {
      mbr_fn& lData = theState->profile_data_.data_;
      lData.addTime(theCPUTimer.elapsed(), theWallTimer.elapsed());
      if (theMethod)
      {
        (lData.*theMethod)();
      }
    }
  }
};


/*******************************************************************************
  Base class of all plan iterators.
********************************************************************************/
class PlanIterator : public SimpleRCObject
{
  friend class PlanIterWrapper;

protected:
  uint32_t           theStateOffset;

public:
  QueryLoc           loc;
  static_context   * theSctx;

#ifndef NDEBUG
  int                theId;

public:
  int getId() const  { return theId;}

  void setId(int id) { theId = id;}

  virtual std::string toString() const;
#endif

public:
  SERIALIZABLE_ABSTRACT_CLASS(PlanIterator);

  PlanIterator(zorba::serialization::Archiver& ar);

  void serialize(::zorba::serialization::Archiver& ar);

public:
  PlanIterator(static_context* sctx, const QueryLoc& loc);

  PlanIterator(const PlanIterator& it);

  ~PlanIterator();

  void setLocation(const QueryLoc& loc_) { loc = loc_; }

  const QueryLoc& getLocation() const { return loc; }

  virtual zstring getNameAsString() const;

  uint32_t getStateOffset() const { return theStateOffset; }

  static_context* getStaticContext() const { return theSctx; }

  TypeManager* getTypeManager() const;

  virtual bool isConstructor() const { return false; }

  /**
   * Accept method for the PlanIterator-Tree-Visitor
   *
   * @param PlanIterVisitor
   */
  virtual void accept(PlanIterVisitor&) const = 0;

  /**
   * Returns the size of the state which must be saved for the current iterator
   * on the state block
   */
  virtual uint32_t getStateSize() const = 0;

  /**
   * Returns the size of the state for this iterator and all its sub-iterators.
   */
  virtual uint32_t getStateSizeOfSubtree() const = 0;

  /**
   * Begin the execution of the iterator. Initializes information required for
   * the plan state and constructs the state object.
   */
  void open(PlanState& planState, uint32_t& offset)
  {
    TimerWrapper t(NULL, planState.theProfile, &mbr_fn::addCall);

    openImpl(planState, offset);

    PlanIteratorState *const state =
        StateTraitsImpl<PlanIteratorState>::getState(planState, theStateOffset);

    t.setState(state);

#ifndef NDEBUG
    // do this after openImpl because the state is created there
    ZORBA_ASSERT(!state->theIsOpened); // don't call open twice
    state->theIsOpened = true;
#endif
  }

  virtual void openImpl(PlanState& planState, uint32_t& offset) = 0;

  /**
   * Restarts the iterator so that the next 'produceNext' call will start
   * again from the beginning (should not release any resources).
   *
   * @param planState
   */
  void reset(PlanState& planState) const
  {
    PlanIteratorState* state =
        StateTraitsImpl<PlanIteratorState>::getState(planState, theStateOffset);

#ifndef NDEBUG
    ZORBA_ASSERT(state->theIsOpened);
#endif

    TimerWrapper t(state, planState.theProfile, &mbr_fn::addCall);

    resetImpl(planState);
  }

  virtual void resetImpl(PlanState& planState) const = 0;

  /**
   * Finish the execution of the iterator. Releases all resources and destroys
   * the according plan state* objects. Make sure that no exception is throw
   * when destroying the states. Otherwise we will have a lot of memory leaks.
   *
   * @param planState
   */
  void close(PlanState& planState)
  {
    PlanIteratorState* state =
        StateTraitsImpl<PlanIteratorState>::getState(planState, theStateOffset);

    TimerWrapper t(state, planState.theProfile, NULL);

    closeImpl(planState);

#ifndef NDEBUG
    state->theIsOpened = false;
#endif
  }

  virtual void closeImpl(PlanState& planState) = 0;

  /**
   * Return the number of items in the sequence that is computed by this
   * iterator. The base implementation of this method simply computes the
   * whole sequence and counts its items. However, the count() method is
   * redefined by specific plan iterators that can compute their count
   * without computing the whole result sequence. One such example is the
   * iterator that computes the dml:collection() function.
   */
  bool count(store::Item_t& result, PlanState& planState) const
  {
    PlanIteratorState* state =
        StateTraitsImpl<PlanIteratorState>::getState(planState, theStateOffset);

    TimerWrapper t(state, planState.theProfile, NULL);

    return countImpl(result, planState);
  }

  virtual bool countImpl(store::Item_t& result, PlanState& planState) const;

  /**
   * Skip a number of items from the Plan's sequence. Classes can overwrite
   * this functions to optimize the skipping by jumping directly to the
   * desired position in the sequence.
   *
   * Returns true if the entire sequence has been consumed, false otherwise.
   *
   * @param count the number of items to be skipped
   * @param planState the state plan
   *
   */
  bool skip(int64_t count, PlanState &planState) const
  {
    PlanIteratorState* state =
        StateTraitsImpl<PlanIteratorState>::getState(planState, theStateOffset);

    TimerWrapper t(state, planState.theProfile, NULL);

    return skipImpl(count, planState);
  }

  virtual bool skipImpl(int64_t count, PlanState &planState) const;

  /**
   * Produce the next item and return it to the caller. Implicitly, the first
   * call of 'producNext' initializes the iterator and allocates resources
   * (main memory, file descriptors, etc.).
   *
   * @param stateBLock
   */
  bool produceNext(store::Item_t& result, PlanState& planState) const
  {
    PlanIteratorState *const state =
      StateTraitsImpl<PlanIteratorState>::getState(planState, theStateOffset);

#ifndef NDEBUG
    ZORBA_ASSERT(state->theIsOpened);
#endif
    TimerWrapper t(state, planState.theProfile, &mbr_fn::addNext);

    return nextImpl(result, planState);
  }

  virtual bool nextImpl(store::Item_t& result, PlanState& planState) const = 0;

  /**
   * Static Method: Makes the given iterator produce its next result and returns
   * that result to the caller.
   */
#ifndef NDEBUG
  static bool consumeNext(
        store::Item_t& result,
        const PlanIterator* iter,
        PlanState& planState);
#else
  static bool consumeNext(
        store::Item_t& result,
        const PlanIterator* iter,
        PlanState& planState)
  {
    if (planState.theDeadlineCountdown != 0 &&
        --planState.theDeadlineCountdown == 0)
      planState.checkDeadline(iter->loc);

    return iter->produceNext(result, planState);
  }
#endif
};

#ifndef NDEBUG
/*******************************************************************************
  Reset the global iterator ID counter, used for debugging purposes. Called by
  the plan serialization when loading a new plan.
********************************************************************************/
void reset_global_iterator_id_counter();
#endif


} /* namespace zorba */

#define DEF_GET_NAME_AS_STRING(...) \
  zstring __VA_ARGS__::getNameAsString() const { return #__VA_ARGS__; }

#endif  /* ZORBA_ITERATOR_H */

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
