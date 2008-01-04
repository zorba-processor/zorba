/* 
 *  Copyright 2006-2007 FLWOR Foundation.
 *  Author: Tim Kraska, David Graf
 *
 */

#ifndef XQP_ITEM_ITERATOR_H
#define XQP_ITEM_ITERATOR_H

#include "context/common.h"
#include "util/rchandle.h"
#include "util/tracer.h"
#include "compiler/parser/location.hh"
#include "runtime/base/iterator.h"
#include "runtime/base/unarybase.h"
#include "runtime/base/noarybase.h"
#include "system/zorba_engine.h"

#include <assert.h>
#include <iostream>
#include <vector>

using namespace std;
namespace xqp {

class Item;
class node;
class zorba;


class SingletonIterator;
typedef rchandle<SingletonIterator> singleton_t;


/*******************************************************************************

   Class represents an empty sequence.

********************************************************************************/
class EmptyIterator : public Batcher<EmptyIterator>
{
public:
  EmptyIterator(yy::location loc) : Batcher<EmptyIterator>(loc) {}
  EmptyIterator(const EmptyIterator& it) : Batcher<EmptyIterator>(it) {}
  ~EmptyIterator() {}
  
  Item_t nextImpl(PlanState& planState) { return NULL; }
  void resetImpl(PlanState& planState)  { }
  void releaseResourcesImpl(PlanState& planState){ }

  virtual uint32_t getStateSize() const { return 0; }
  virtual uint32_t getStateSizeOfSubtree() const { return 0; }
  virtual void setOffset(PlanState& planState, uint32_t& offset);
  
  virtual void accept(PlanIterVisitor&) const;
};


/*******************************************************************************

  Literals

********************************************************************************/
class SingletonIterator : public NoaryBaseIterator<SingletonIterator>
{
protected:
  Item_t theValue;

public:
  SingletonIterator(yy::location loc, Item_t value);

  ~SingletonIterator();
  
public:
  Item_t nextImpl(PlanState& planState);

  const Item_t& getValue() const { return theValue; }
  
  virtual void accept(PlanIterVisitor&) const;
};


/*******************************************************************************

  
********************************************************************************/

class IfThenElseIterator : public Batcher<IfThenElseIterator>
{
protected:
  class IfThenElseIteratorState : public PlanIteratorState 
  {
  public:
    bool theThenUsed;
  };

private:
  PlanIter_t theCondIter;
  PlanIter_t theThenIter;
  PlanIter_t theElseIter;
  bool theIsBooleanIter;
    
public:
  /**
   * Constructor
   * @param loc location
   * @param iterCond_arg represents condition
   * @param iterThen_arg represents then expression
   * @param iterElse_arg represents else expression
   * @param condIsBooleanIter Optional flag. If true => condition is already an iterator 
   *                              which return true or false => the conditional value
   *                              does not have to be evaluated with the effective
   *                              boolean value function
   */
  IfThenElseIterator(
        const yy::location& loc,
        PlanIter_t& aCondIter,
        PlanIter_t& aThenIter,
        PlanIter_t& aElseIter,
        bool aIsBooleanIter = false);
    
  Item_t nextImpl(PlanState& planState);
  void resetImpl(PlanState& planState);
  void releaseResourcesImpl(PlanState& planState);
  
  virtual uint32_t getStateSize() const { return sizeof(IfThenElseIteratorState); }
  virtual uint32_t getStateSizeOfSubtree() const;
  virtual void setOffset ( PlanState& planState, uint32_t& offset );
      
  virtual void accept(PlanIterVisitor&) const;
};


} /* namespace xqp */
#endif  /* XQP_ITEM_ITERATOR_H */

