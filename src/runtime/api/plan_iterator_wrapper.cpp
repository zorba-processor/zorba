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

#include "runtime/api/plan_iterator_wrapper.h"

#include "runtime/base/plan_iterator.h"

namespace zorba
{

/*******************************************************************************
  class PlanIteratorWrapper
********************************************************************************/
PlanIteratorWrapper::PlanIteratorWrapper(const PlanIterator* iter, PlanState& state)
  :
  theIterator(iter),
  theStateBlock(&state)
{
  /*
  std::cerr << "--> PlanIteratorWrapper() " << this << " for iter: " << iter->getId() << " = " << typeid (*iter).name()
      << " state: " << (void*)&state << " theBlock: " << (void*)state.theBlock << std::endl;
  */
}


PlanIteratorWrapper::~PlanIteratorWrapper()
{
}


bool PlanIteratorWrapper::next(store::Item_t& result)
{
  return PlanIterator::consumeNext(result, theIterator, *theStateBlock);
}


void PlanIteratorWrapper::reset()
{
  theIterator->reset(*theStateBlock);
}


#ifndef NDEBUG
std::string PlanIteratorWrapper::toString() const
{
  std::stringstream ss;
  ss << this << " = PlanStoreIteratorWrapper iterator: " << theIterator->toString();
  return ss.str();
}
#endif


/*******************************************************************************
  class PlanStoreIteratorWrapper
********************************************************************************/
SERIALIZE_INTERNAL_METHOD(PlanStateIteratorWrapper)

void PlanStateIteratorWrapper::serialize(::zorba::serialization::Archiver& ar)
{
  PlanIterator::serialize(ar);
}

PlanStateIteratorWrapper::PlanStateIteratorWrapper(const PlanIterator* iter, PlanState& state)
  :
  PlanIterator(NULL, QueryLoc()),
  theIterator(iter),
  theStateBlock(&state)
{
}


PlanStateIteratorWrapper::~PlanStateIteratorWrapper()
{
}


bool PlanStateIteratorWrapper::produceNext(store::Item_t& result, PlanState& planState) const
{
  return theIterator->produceNext(result, *theStateBlock);
}


bool PlanStateIteratorWrapper::next(store::Item_t& result)
{
  return PlanIterator::consumeNext(result, theIterator, *theStateBlock);
}


void PlanStateIteratorWrapper::reset(PlanState& planState) const
{
  theIterator->reset(*theStateBlock);
}


void PlanStateIteratorWrapper::accept(PlanIterVisitor& v) const
{
}


} /* namespace zorba */
/* vim:set et sw=2 ts=2: */
