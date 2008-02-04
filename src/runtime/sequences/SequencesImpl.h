/**
 * Copyright FLWOR Foundation
 *
 * Author: David Graf (david.graf@28msec.com)
 *         Matthias Brantner (matthias.brantner@28msec.com)
 */

#ifndef XQP_SEQUENCES_IMPL_H
#define XQP_SEQUENCES_IMPL_H

#include <string>
#include <vector>
#include <stack>

#include "context/common.h"
#include "runtime/base/iterator.h"
#include "store/naive/simple_store.h"
#include "runtime/base/unarybase.h"
#include "runtime/base/binarybase.h"
#include "runtime/base/narybase.h"
#include "runtime/booleans/BooleanImpl.h"

namespace xqp
{

  /*______________________________________________________________________
  |
  | 15.1 General Functions and Operators on Sequences
  |_______________________________________________________________________*/

//15.1.1 fn:boolean
// see BooleanImpl.h

//15.1.2 op:concatenate
class FnConcatIteratorState : public  PlanIteratorState {
public:
  uint32_t theCurIter;

  void init(PlanState&);
  void reset(PlanState&);
};

NARY_ITER_STATE(FnConcatIterator, FnConcatIteratorState);

//15.1.3 fn:index-of
/*
 * If the value of $arg is the empty sequence, the function returns true; 
 * otherwise, the function returns false.
 */
class FnIndexOfIteratorState : public PlanIteratorState {
public:  
  uint32_t theCurrentPos; // the current position in the sequence
  Item_t   theSearchItem; // the item to search for

  void init(PlanState&);
  void reset(PlanState&);
  
};

NARY_ITER_STATE(FnIndexOfIterator, FnIndexOfIteratorState);

//15.1.4 fn:empty
/*
 * If the value of $arg is the empty sequence, the function returns true; 
 * otherwise, the function returns false.
 */
NARY_ITER(FnEmptyIterator);


//15.1.5 fn:exists
/*
 * If the value of $arg is not the empty sequence, the function returns true; 
 * otherwise, the function returns false.
 */
NARY_ITER(FnExistsIterator);

//15.1.6 fn:distinct-values
struct ItemCmp 
{
    bool operator() ( const Item_t& i1, const Item_t& i2) const
    {
      return CompareIterator::compare(i1, i2)<0?true:false;
    }

};

class FnDistinctValuesIteratorState : public PlanIteratorState {
public:  
  typedef std::map<Item_t, uint8_t, ItemCmp> AlreadySeenMap_t;
  typedef AlreadySeenMap_t::const_iterator   AlreadySeenConstIter_t;

  AlreadySeenMap_t theAlreadySeenMap;  

  void init(PlanState&);
  void reset(PlanState&);
};
  
class FnDistinctValuesIterator : public NaryBaseIterator<FnDistinctValuesIterator, 
                                                         FnDistinctValuesIteratorState>
{

public:
  FnDistinctValuesIterator(yy::location loc,
                           vector<PlanIter_t>& args);
 
  ~FnDistinctValuesIterator();

  Item_t 
  nextImpl(PlanState& planState);
 
  virtual void 
  accept(PlanIterVisitor&) const;

};



//15.1.7 fn:insert-before
// Returns a new sequence constructed from the value of the first parameter with the value of third parameter inserted 
// at the position specified by the value of the second parameter. 
class FnInsertBeforeIteratorState : public PlanIteratorState { 
public:  
  xqp_integer theCurrentPos; // the current position in the sequence
  xqp_integer thePosition;
  Item_t      theTargetItem;

  void init(PlanState&);
  void reset(PlanState&);
};

NARY_ITER_STATE(FnInsertBeforeIterator, FnInsertBeforeIteratorState);

//15.1.8 fn:remove
// Returns a new sequence constructed from the value of aTarget with the item at the position specified by the 
// value of aPosition removed.
class FnRemoveIteratorState : public PlanIteratorState {
public:  
  xqp_integer theCurrentPos; // the current position in the sequence
  xqp_integer thePosition; // the position to delete

  void init(PlanState&);
  void reset(PlanState&);
};

NARY_ITER_STATE(FnRemoveIterator, FnRemoveIteratorState);


//15.1.9 fn:reverse
class FnReverseIteratorState : public PlanIteratorState {
  public:
    std::stack<Item_t> theStack;

    void init(PlanState&);
    void reset(PlanState&);
};

NARY_ITER_STATE(FnReverseIterator, FnReverseIteratorState);

//15.1.10 fn:subsequence
// Returns the contiguous sequence of items in the value of $sourceSeq beginning at the position indicated by the value 
// of $startingLoc and continuing for the number of items indicated by the value of $length.
class FnSubsequenceIteratorState : public PlanIteratorState {
public:
  xqp_double  theStartingLoc; // second argument (startingLoc)
  xqp_double  theLength; // third optional argument (length)

  xqp_integer theCurrentPos; // the current position in the sequence
  xqp_integer theCurrentLength; // used for returning a specific number of items

  void init(PlanState&);
  void reset(PlanState&);  
};
NARY_ITER_STATE(FnSubsequenceIterator, FnSubsequenceIteratorState);

//15.1.11 fn:unordered
// no need to implement an operator for his


  /*______________________________________________________________________
  |
  | 15.2 Functions That Test the Cardinality of Sequences
  |_______________________________________________________________________*/

//15.2.1 fn:zero-or-one
NARY_ITER(FnZeroOrOneIterator);

//15.2.2 fn:one-or-more
NARY_ITER(FnOneOrMoreIterator);

//15.2.3 fn:exactly-one
NARY_ITER(FnExactlyOneIterator);

  /*______________________________________________________________________
  |
  | 15.3 Equals, Union, Intersection and Except
  |_______________________________________________________________________*/

//15.3.1 fn:deep-equal

//15.3.2 op:union

//15.3.3 op:intersect

//15.3.4 op:except


  /*______________________________________________________________________
  |
  | 15.4 Aggregate Functions
  |_______________________________________________________________________*/

//15.4.1 fn:count
NARY_ITER(FnCountIterator);

//15.4.2 fn:avg
NARY_ITER(FnAvgIterator);

//15.4.3 fn:max & 15.4.4 fn:min
class FnMinMaxIterator : public NaryBaseIterator<FnMinMaxIterator, PlanIteratorState> {
public:
  enum Type {
    MIN = 0,
    MAX
  };

private:
  Type theType;
  CompareIterator::CompareType theCompareType;

public:
    FnMinMaxIterator(yy::location loc, std::vector<PlanIter_t>& aChildren, Type aType);
    Item_t nextImpl(PlanState& aPlanState);
    virtual void accept(PlanIterVisitor& v) const;
    Type getType() const { return theType; }
};


//15.4.5 fn:sum
NARY_ITER(FnSumIterator);

  /*______________________________________________________________________
  |
  | 15.5 Functions and Operators that Generate Sequences
  |_______________________________________________________________________*/

//15.5.1 op:to
class OpToIteratorState : public PlanIteratorState {
public:
  xqp_integer theCurInt;
  xqp_integer theFirstVal;
  xqp_integer theLastVal;

  void init(PlanState&);
  void reset(PlanState&);
  
};

NARY_ITER_STATE(OpToIterator, OpToIteratorState);

//15.5.2 fn:id

//15.5.3 fn:idref


//15.5.4 fn:doc

class FnDocIterator : public UnaryBaseIterator<FnDocIterator, PlanIteratorState>
{
public:
  FnDocIterator(yy::location loc, PlanIter_t& arg);
  virtual ~FnDocIterator();

  Item_t nextImpl(PlanState& planState);
  
  virtual void accept(PlanIterVisitor&) const;
};


//15.5.5 fn:doc-available

//15.5.6 fn:collection


  /*______________________________________________________________________
  |
  | Extensions
  |_______________________________________________________________________*/


} /* namespace xqp */
#endif /* XQP_SEQUENCES_H */
/* vim:set ts=2 sw=2: */
