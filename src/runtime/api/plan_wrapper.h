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
#ifndef ZORBA_RUNTIME_API_PLAN_WRAPPER
#define ZORBA_RUNTIME_API_PLAN_WRAPPER

#include "common/shared_types.h"

#include "store/api/iterator.h"


#include <memory>

#include <zorba/item.h>
#include <api/serialization/serializable.h>


namespace zorba
{

class PlanState;
class DebuggerRuntime;
class XQueryImpl;
class dynamic_context;


/*******************************************************************************
  Wrapper used to drive the evaluation of an iterator (sub)tree.

  Execution limits: if parentState is given (a plan nested in a running query,
  such as eval), the plan runs one call level deeper and shares the parent's
  call depth limit and deadline. Otherwise, the limits come from the security
  policy of the ccb's root static context (see PlanState::initLimits), and the
  deadline starts when the plan is opened.

  The wrapper wraps the root iterator of the (sub)tree. It is responsible
  for allocating and deallocating the plan state that is shared by all
  iterators in the (sub)tree. In general, it hides internal functionality
  like separation of code and execution, or garabage collection, and it
  provides a simple interface that the application can use.

  - theIterator :
  Pointer to the root iterator of the plan to be executed by this PlanWrapper.
  NOTE: theIterator MUST be a RAW pointer, NOT an rchandle. This is because
  several PlanWrappers (created from different clones of the same query) may
  be accessing the same plan iterator, but PlanIterator does not do sunchronized
  ref counting. It is the responsibility of the user of a plan wrapper to do
  memory management for the actual plan (the plan wrapper does not assume
  ownership of the plan).

  - theDynamicContext : 
  Most of the time this is empty. If a NULL dctx is given to the constructor
  of "this", the constructor allocates a dctx and theDynamicContext owns it.
********************************************************************************/
class PlanWrapper : public store::Iterator
{
  friend class DynamicFunctionInvocationIterator;
  friend class DebuggerRuntime;
  friend class EvalIteratorState;

protected:

  PlanIterator       * theIterator;

  std::unique_ptr<dynamic_context> theDynamicContext;

  PlanState          * thePlanState;

  bool                 theIsOpen;

  bool                 theHasParent;

  store::Iterator_t    theExitValue;

public:

  PlanWrapper(
        PlanIterator*     iter,
        CompilerCB*       ccb,
        dynamic_context*  dynamicContext,
        XQueryImpl*       query,
        const PlanState*  parentState = NULL);

  ~PlanWrapper();

  void open();

  bool next(store::Item_t& item);

  bool skip(int64_t count);

  void reset();

  void close();

  void checkDepth(const QueryLoc& loc);

  dynamic_context* dctx() { return theDynamicContext.get(); }

  void profile() const;

  bool isOpen() { return theIsOpen; }

#ifndef NDEBUG
  virtual std::string toString() const;
#endif    
};

} /* namespace zorba */
#endif

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
