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

#include "diagnostics/xquery_diagnostics.h"
#include "diagnostics/util_macros.h"

#include "zorbatypes/collation_manager.h"
#include "zorbatypes/datetime.h"
#include "zorbatypes/integer.h"

#include "system/globalenv.h"

#include "types/casting.h"
#include "types/typeops.h"

#include "context/dynamic_context.h"
#include "context/static_context.h"

#include "compiler/api/compilercb.h"

#include "runtime/visitors/planiter_visitor.h"
#include "runtime/booleans/BooleanImpl.h"
#include "runtime/api/plan_iterator_wrapper.h"
#include "runtime/util/iterator_impl.h"
#include "runtime/core/internal_operators.h"
#include "runtime/core/var_iterators.h"

#include "store/api/iterator_factory.h"
#include "store/api/temp_seq.h"
#include "store/api/item_factory.h"
#include "store/api/store.h"

#include <cmath>
#include <limits>
#include <string_view>
#include <unordered_map>


namespace zorba {

SERIALIZABLE_CLASS_VERSIONS(FnBooleanIterator)
DEF_GET_NAME_AS_STRING(FnBooleanIterator)

SERIALIZABLE_CLASS_VERSIONS(OrIterator)
DEF_GET_NAME_AS_STRING(OrIterator)

SERIALIZABLE_CLASS_VERSIONS(AndIterator)
DEF_GET_NAME_AS_STRING(AndIterator)

SERIALIZABLE_CLASS_VERSIONS(CompareIterator)
DEF_GET_NAME_AS_STRING(CompareIterator)

SERIALIZABLE_TEMPLATE_INSTANCE(TypedValueCompareIterator,
                               TypedValueCompareIterator<store::XS_DOUBLE>,
                               1)
template<>
DEF_GET_NAME_AS_STRING(TypedValueCompareIterator<store::XS_DOUBLE>)

SERIALIZABLE_TEMPLATE_INSTANCE(TypedValueCompareIterator,
                               TypedValueCompareIterator<store::XS_FLOAT>,
                               2)
template<>
DEF_GET_NAME_AS_STRING(TypedValueCompareIterator<store::XS_FLOAT>)

SERIALIZABLE_TEMPLATE_INSTANCE(TypedValueCompareIterator,
                               TypedValueCompareIterator<store::XS_DECIMAL>,
                               3)
template<>
DEF_GET_NAME_AS_STRING(TypedValueCompareIterator<store::XS_DECIMAL>)

SERIALIZABLE_TEMPLATE_INSTANCE(TypedValueCompareIterator,
                               TypedValueCompareIterator<store::XS_INTEGER>,
                               4)
template<>
DEF_GET_NAME_AS_STRING(TypedValueCompareIterator<store::XS_INTEGER>)

SERIALIZABLE_TEMPLATE_INSTANCE(TypedValueCompareIterator,
                               TypedValueCompareIterator<store::XS_STRING>,
                               5)
template<>
DEF_GET_NAME_AS_STRING(TypedValueCompareIterator<store::XS_STRING>)

SERIALIZABLE_CLASS_VERSIONS(AtomicValuesEquivalenceIterator)
DEF_GET_NAME_AS_STRING(AtomicValuesEquivalenceIterator)


/*******************************************************************************
  15.1.1 fn:boolean
********************************************************************************/
FnBooleanIterator::FnBooleanIterator(
    static_context* sctx,
    const QueryLoc& loc,
    PlanIter_t& aIter,
    bool aNegate)
  :
  UnaryBaseIterator<FnBooleanIterator, PlanIteratorState>(sctx, loc, aIter),
  theNegate(aNegate)
{
}


void FnBooleanIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (UnaryBaseIterator<FnBooleanIterator, PlanIteratorState>*)this);

  ar & theNegate;
}


bool FnBooleanIterator::effectiveBooleanValue(
    const QueryLoc& loc,
    PlanState& planState,
    const PlanIterator* iter,
    bool negate)
{
  store::Item_t item;
  bool result;
  bool is_sequence;

  TypeManager* tm = iter->getStaticContext()->get_typemanager();

  if (!consumeNext(item, iter, planState))
  {
    // empty sequence => false
    result = negate ^ false;
  }
  else if (item->isNode() || item->isJSONItem())
  {
    // node => true
    result = negate ^ true;
  }
  else
  {
    store::SchemaTypeCode type = item->getTypeCode();

    store::Item_t item2;
    is_sequence = consumeNext(item2, iter, planState);

    if (!is_sequence 
        &&
        (type == store::XS_BOOLEAN ||
         TypeOps::is_subtype(type, store::XS_STRING) ||
         TypeOps::is_subtype(type, store::XS_ANY_URI) ||
         type == store::XS_UNTYPED_ATOMIC ||
         type == store::JS_NULL ||
         TypeOps::is_numeric(type)))
    {
      // atomic type xs_boolean, xs_string, xs_anyURI, xs_untypedAtomic
      // => effective boolean value is defined in the items
      bool temp = item->getEBV();
      result = negate ? (negate ^ temp) : temp;
    }
    else
    {
      if (is_sequence)
      {
        xqtref_t type = tm->create_value_type(item);

        RAISE_ERROR(err::FORG0006, loc,
        ERROR_PARAMS(ZED(BadArgTypeForFn_2o34o),
                     "", "fn:boolean",
                     ZED(EBVNotDefSeq_5),
                     ERROR_TEXT( *type )));
      }
      else
      {
        RAISE_ERROR(err::FORG0006, loc,
        ERROR_PARAMS(ZED(BadArgTypeForFn_2o34o), "", "fn:boolean" ));
      }
    }
  }

  return result;
}


bool FnBooleanIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  PlanIteratorState* aState;
  DEFAULT_STACK_INIT(PlanIteratorState, aState, planState);

  GENV_ITEMFACTORY->
  createBoolean(result,
                FnBooleanIterator::effectiveBooleanValue(this->loc,
                                                         planState,
                                                         theChild,
                                                         theNegate));
  STACK_PUSH(true, aState);
  STACK_END(aState);
}


UNARY_ACCEPT(FnBooleanIterator);


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  OrIterator                                                                 //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


OrIterator::OrIterator (
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& inChildren)
  :
  NaryBaseIterator<OrIterator, PlanIteratorState>(sctx, loc, inChildren)
{
}


void 
OrIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar, (NaryBaseIterator<OrIterator, PlanIteratorState>*)this);
}


bool
OrIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  bool res = false;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  {
    std::vector<PlanIter_t>::const_iterator ite = theChildren.begin();
    std::vector<PlanIter_t>::const_iterator end = theChildren.end();
    for (; res == false && ite != end; ++ite)
    {
      res = res ||
            FnBooleanIterator::effectiveBooleanValue(loc, planState, (*ite));
    }
  }

  STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, res), state);
  STACK_END(state);
}


NARY_ACCEPT(OrIterator);


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  AndIterator                                                                //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

AndIterator::AndIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& inChildren)
  :
  NaryBaseIterator<AndIterator, PlanIteratorState>(sctx, loc, inChildren)
{
}


void 
AndIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar, (NaryBaseIterator<AndIterator, PlanIteratorState>*)this);
}


bool
AndIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  bool res = true;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  {
    std::vector<PlanIter_t>::const_iterator ite = theChildren.begin();
    std::vector<PlanIter_t>::const_iterator end = theChildren.end();
    for (; res && ite != end; ++ite)
    {
      res = res &&
            FnBooleanIterator::effectiveBooleanValue(loc, planState, (*ite));
    }
  }

  STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, res), state);
  STACK_END(state);
}


NARY_ACCEPT(AndIterator);


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  CompareIterator                                                            //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************

********************************************************************************/
CompareIterator::CompareIterator(
     static_context* sctx,
     const QueryLoc& loc,
     PlanIter_t aChild0,
     PlanIter_t aChild1,
     CompareConsts::CompareType aCompType)
  :
  BinaryBaseIterator<CompareIterator, CompareIteratorState> ( sctx, loc, aChild0, aChild1 ),
  theCompType(aCompType),
  theTypeManager(sctx->get_typemanager()),
  theCollation(sctx->get_default_collator(loc))
{
  switch(theCompType)
  {
  case CompareConsts::GENERAL_EQUAL:
  case CompareConsts::GENERAL_NOT_EQUAL:
  case CompareConsts::GENERAL_LESS:
  case CompareConsts::GENERAL_LESS_EQUAL:
  case CompareConsts::GENERAL_GREATER:
  case CompareConsts::GENERAL_GREATER_EQUAL:
    theIsGeneralComparison = true;
    break;
  default:
    theIsGeneralComparison = false;
    break;
  }
}


CompareIterator::CompareIterator(::zorba::serialization::Archiver& ar)
  :
  BinaryBaseIterator<CompareIterator, CompareIteratorState>(ar),
  theTypeManager(NULL),
  theCollation(NULL)
{
}


void CompareIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (BinaryBaseIterator<CompareIterator, CompareIteratorState>*)this);

  SERIALIZE_ENUM(CompareConsts::CompareType, theCompType)
  ar & theIsGeneralComparison;
  SERIALIZE_TYPEMANAGER(TypeManager, theTypeManager);
  ar & theCollation;
}


BINARY_ACCEPT(CompareIterator);


/*******************************************************************************
  The items of one binding of a LET var, hashed by the value that general "="
  compares, to find the items equal to a probe without comparing it with each.

  Each item belongs to a category. A pair of items whose categories are "safe"
  for each other compares without an error: as strings (xs:string and its
  subtypes, xs:untypedAtomic; only under a codepoint collation), as numbers
  (xs:integer, xs:decimal and xs:double; an xs:integer or xs:decimal only if
  its xs:double value is below 2^53 in magnitude, so that the cast to
  xs:double that a comparison with an xs:double does is exact for integers),
  as booleans, or with a JSON null on either side (equal only to null). An
  xs:untypedAtomic is not safe with a number (its cast may fail) nor with
  null; anything else is OTHER, safe with nothing, and so is a streamable
  string, whose stream reading it would consume. A probe is looked up only
  if its category is safe for every category in the binding: then no pair
  raises an error and "=" is true exactly when an equal item is found.
********************************************************************************/
class GeneralEqualityIndex
{
public:
  enum Category : unsigned
  {
    STRING   = 1,
    UNTYPED  = 2,
    NUMBER   = 4,
    BOOLEAN  = 8,
    JSONNULL = 16,
    OTHER    = 32
  };

  struct Key
  {
    Category          theCategory = OTHER;
    double            theNumber = 0;
    std::string_view  theString;
    bool              theBoolean = false;
  };

private:
  store::TempSeq_t                                 theSeq;
  uint64_t                                         theBindingId;
  bool                                             theMemCmp;
  bool                                             theIsBuilt;
  std::vector<store::Item_t>                       theItems;
  unsigned                                         theCategories;
  std::unordered_multimap<std::string_view, csize> theStrings;
  std::unordered_multimap<double, csize>           theNumbers;
  bool                                             theHasTrue;
  bool                                             theHasFalse;
  bool                                             theHasNull;

public:
  GeneralEqualityIndex(store::TempSeq* seq, uint64_t bindingId, bool memCmp)
    :
    theSeq(seq),
    theBindingId(bindingId),
    theMemCmp(memCmp),
    theIsBuilt(false),
    theCategories(0),
    theHasTrue(false),
    theHasFalse(false),
    theHasNull(false)
  {
  }

  uint64_t getBindingId() const { return theBindingId; }

  const std::vector<store::Item_t>& getItems() const { return theItems; }

  static unsigned safeCategories(Category c)
  {
    switch (c)
    {
    case STRING:   return STRING | UNTYPED | JSONNULL;
    case UNTYPED:  return STRING | UNTYPED;
    case NUMBER:   return NUMBER | JSONNULL;
    case BOOLEAN:  return BOOLEAN | JSONNULL;
    case JSONNULL: return STRING | NUMBER | BOOLEAN | JSONNULL;
    default:       return 0;
    }
  }

  // Whether no item of the binding raises an error when compared with an item of category c.
  bool isSafeFor(Category c) const
  {
    return (theCategories & ~safeCategories(c)) == 0;
  }

  Key getKey(const store::Item_t& item, const QueryLoc& loc) const
  {
    Key key;

    // Reading a streamable string consumes its stream, which can fail.
    if (!item->isAtomic() || item->isStreamable())
      return key;

    store::SchemaTypeCode type = item->getTypeCode();

    if (type == store::XS_UNTYPED_ATOMIC || TypeOps::is_subtype(type, store::XS_STRING))
    {
      if (theMemCmp)
      {
        const zstring& str = item->getString();
        key.theCategory = (type == store::XS_UNTYPED_ATOMIC ? UNTYPED : STRING);
        key.theString = std::string_view(str.data(), str.size());
      }
    }
    else if (type == store::XS_DOUBLE)
    {
      key.theCategory = NUMBER;
      // -0 and 0 are equal.
      key.theNumber = item->getDoubleValue().getNumber() + 0.0;
    }
    else if (TypeOps::is_subtype(type, store::XS_DECIMAL))
    {
      // The cast that the comparison with an xs:double does; an item it fails on stays OTHER.
      store::Item_t source = item;
      store::Item_t asDouble;
      try
      {
        GenericCast::castToBuiltinAtomic(asDouble, source, store::XS_DOUBLE, NULL, loc);
      }
      catch (const ZorbaException&)
      {
        return key;
      }

      double const value = asDouble->getDoubleValue().getNumber() + 0.0;
      if (std::fabs(value) < 9007199254740992.0)
      {
        key.theCategory = NUMBER;
        key.theNumber = value;
      }
    }
    else if (type == store::XS_BOOLEAN)
    {
      key.theCategory = BOOLEAN;
      key.theBoolean = item->getBooleanValue();
    }
    else if (type == store::JS_NULL)
    {
      key.theCategory = JSONNULL;
    }

    return key;
  }

  // Reads the whole binding, as the comparison without the index does before
  // it compares; later calls do nothing.
  void build(const QueryLoc& loc)
  {
    if (theIsBuilt)
      return;

    store::TempSeqIterator_t iter =
      GENV_STORE.getIteratorFactory()->createTempSeqIterator(theSeq->isLazy());

    if (theSeq->isLazy())
      iter->init(theSeq, Integer(1), Integer(std::numeric_limits<xs_long>::max()));
    else
      iter->init(theSeq);

    iter->open();

    store::Item_t item;
    while (iter->next(item))
    {
      // The error that op:hoist kept, raised as op:unhoist raises it.
      if (item->isError())
        item->getError()->polymorphic_throw();

      Key key = getKey(item, loc);
      csize const pos = theItems.size();
      theItems.push_back(item);
      theCategories |= key.theCategory;

      switch (key.theCategory)
      {
      case STRING:
      case UNTYPED:
        theStrings.emplace(key.theString, pos);
        break;
      case NUMBER:
        // NaN is equal to nothing.
        if (!std::isnan(key.theNumber))
          theNumbers.emplace(key.theNumber, pos);
        break;
      case BOOLEAN:
        (key.theBoolean ? theHasTrue : theHasFalse) = true;
        break;
      case JSONNULL:
        theHasNull = true;
        break;
      default:
        break;
      }
    }

    iter->close();
    theSeq = NULL;
    theIsBuilt = true;
  }

  // Whether "probe = s" ("s = probe" if !probeFirst) is true for an item s of
  // the binding. Requires isSafeFor(key.theCategory).
  bool contains(
      const store::Item_t& probe,
      const Key& key,
      bool probeFirst,
      const QueryLoc& loc,
      const TypeManager* tm,
      long timezone,
      XQPCollator* collation) const
  {
    switch (key.theCategory)
    {
    case STRING:
    case UNTYPED:
      // Under a codepoint collation, strings are equal iff their bytes are.
      return theStrings.find(key.theString) != theStrings.end();
    case NUMBER:
    {
      // Equal xs:double values do not make equal xs:decimal values.
      auto range = theNumbers.equal_range(key.theNumber);
      for (auto ite = range.first; ite != range.second; ++ite)
      {
        store::Item_t item0 = (probeFirst ? probe : theItems[ite->second]);
        store::Item_t item1 = (probeFirst ? theItems[ite->second] : probe);

        if (CompareIterator::generalComparison(loc,
                                               item0,
                                               item1,
                                               CompareConsts::GENERAL_EQUAL,
                                               tm,
                                               timezone,
                                               collation))
          return true;
      }
      return false;
    }
    case BOOLEAN:
      return (key.theBoolean ? theHasTrue : theHasFalse);
    case JSONNULL:
      return theHasNull;
    default:
      ZORBA_ASSERT(false);
      return false;
    }
  }
};


// A reference to a LET var, also under op:unhoist.
static LetVarIterator* letVarOperand(PlanIterator* operand)
{
  if (UnhoistIterator* unhoist = dynamic_cast<UnhoistIterator*>(operand))
  {
    std::vector<PlanIter_t>& children = unhoist->getChildren();
    if (children.size() != 1)
      return NULL;

    operand = children[0].getp();
  }

  return dynamic_cast<LetVarIterator*>(operand);
}


CompareIteratorState::CompareIteratorState()
  :
  theSeenBindingId(0)
{
}


CompareIteratorState::~CompareIteratorState()
{
}


/*******************************************************************************
  General "=" with a LET var operand, when the same binding of the var is
  compared again: the var is hashed once (GeneralEqualityIndex) and each item
  of the other operand is looked up in it. Returns false if it does not apply,
  and the caller then compares as usual.

  The result and the errors are those of the nested loop of nextImpl(): the
  operands are read in the same order and as far (the whole of both operands
  before the first comparison, except that the var is not read if operand 0 is
  empty, nor operand 1 if the var is an empty operand 0), and an item of the
  other operand is looked up only if no pair it makes with an item of the var
  can raise an error; otherwise its pairs are compared in the order of the
  nested loop.
********************************************************************************/
bool CompareIterator::generalEqualByIndex(
    PlanState& planState,
    CompareIteratorState& state,
    long timezone,
    bool& found) const
{
  if (theCompType != CompareConsts::GENERAL_EQUAL)
    return false;

  // The nested loop takes the items of operand 0 in the outer loop.
  bool varIsOperand1 = true;
  LetVarIterator* var = letVarOperand(theChild1.getp());
  if (var == NULL)
  {
    var = letVarOperand(theChild0.getp());
    varIsOperand1 = false;
  }

  if (var == NULL)
    return false;

  uint64_t bindingId = 0;
  store::TempSeq* seq = var->getWholeTempSeq(planState, bindingId);

  if (seq == NULL)
    return false;

  if (!state.theIndex || state.theIndex->getBindingId() != bindingId)
  {
    if (state.theSeenBindingId != bindingId)
    {
      // Compared once: hashing would cost more than comparing.
      state.theSeenBindingId = bindingId;
      state.theIndex.reset();
      return false;
    }

    state.theIndex = std::make_unique<GeneralEqualityIndex>(
      seq, bindingId, theCollation == NULL || theCollation->doMemCmp());
  }

  GeneralEqualityIndex& index = *state.theIndex;
  PlanIterator* other = (varIsOperand1 ? theChild0 : theChild1).getp();
  std::vector<store::Item_t> others;
  store::Item_t item;

  // An error reading the var is raised again by the next evaluation, as without the index.
  auto buildIndex = [&]()
  {
    try
    {
      index.build(loc);
    }
    catch (...)
    {
      state.theIndex.reset();
      throw;
    }
  };

  found = false;

  if (varIsOperand1)
  {
    while (consumeNext(item, other, planState))
      others.push_back(item);

    if (others.empty())
      return true;

    buildIndex();
  }
  else
  {
    buildIndex();

    if (index.getItems().empty())
      return true;

    while (consumeNext(item, other, planState))
      others.push_back(item);
  }

  const std::vector<store::Item_t>& items = index.getItems();

  if (items.empty() || others.empty())
    return true;

  // Operand 1 is the var: each item of operand 0 is compared with the whole var in turn.
  if (varIsOperand1)
  {
    for (const store::Item_t& probe : others)
    {
      GeneralEqualityIndex::Key key = index.getKey(probe, loc);

      if (index.isSafeFor(key.theCategory))
      {
        found = index.contains(probe, key, true, loc, theTypeManager, timezone, theCollation);
      }
      else
      {
        for (const store::Item_t& varItem : items)
        {
          store::Item_t item0 = probe;
          store::Item_t item1 = varItem;

          if (generalComparison(loc, item0, item1, theCompType, theTypeManager, timezone, theCollation))
          {
            found = true;
            break;
          }
        }
      }

      if (found)
        break;
    }

    return true;
  }

  // Operand 0 is the var: look up only if no pair at all can raise an error.
  std::vector<GeneralEqualityIndex::Key> keys;
  keys.reserve(others.size());
  bool safe = true;

  for (const store::Item_t& probe : others)
  {
    keys.push_back(index.getKey(probe, loc));
    safe = safe && index.isSafeFor(keys.back().theCategory);
  }

  if (safe)
  {
    for (csize i = 0; i < others.size() && !found; ++i)
      found = index.contains(others[i], keys[i], false, loc, theTypeManager, timezone, theCollation);

    return true;
  }

  for (const store::Item_t& varItem : items)
  {
    for (const store::Item_t& probe : others)
    {
      store::Item_t item0 = varItem;
      store::Item_t item1 = probe;

      if (generalComparison(loc, item0, item1, theCompType, theTypeManager, timezone, theCollation))
      {
        found = true;
        return true;
      }
    }
  }

  return true;
}


/*******************************************************************************

********************************************************************************/
bool CompareIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t item0, item1, tItem0, tItem1;
  bool c0Done = false, c1Done = false, done = false, found = false;
  std::vector<store::Item_t> seq0;
  std::vector<store::Item_t> seq1;
  store::TempSeq_t tSeq0, tSeq1;
  long const timezone = planState.theLocalDynCtx->get_implicit_timezone();

  CompareIteratorState* state;
  DEFAULT_STACK_INIT(CompareIteratorState, state, planState);

  if (theIsGeneralComparison)
  {
    if (generalEqualByIndex(planState, *state, timezone, found))
    {
      done = true;
    }
    else if (consumeNext(item0, theChild0.getp(), planState))
    {
      if (consumeNext(tItem0, theChild0.getp(), planState))
      {
        seq0.push_back(item0);
        seq0.push_back(tItem0);
      }
      else
      {
        c0Done = true;
        if (consumeNext(item1, theChild1.getp(), planState))
        {
          if (consumeNext(tItem1, theChild1.getp(), planState))
          {
            seq0.push_back(item0);
            seq1.push_back(item1);
            seq1.push_back(tItem1);
          }
          else
          {
            c1Done = true;
            found = generalComparison(loc,
                                      item0,
                                      item1,
                                      theCompType,
                                      theTypeManager,
                                      timezone,
                                      theCollation);
            done = true;
          }
        }
        else
        {
          c1Done = true;
          found = false;
          done = true;
        }
      }
    }
    else
    {
      c0Done = true;
      found = false;
      done = true;
    }

    if (!done)
    {
      store::Iterator_t ite0;
      store::Iterator_t ite1;
      tSeq0 = GENV_STORE.createTempSeq(seq0);
      tSeq1 = GENV_STORE.createTempSeq(seq1);

      if (!c0Done)
      {
        ite0 = new PlanIteratorWrapper(theChild0, planState);
        tSeq0->append(ite0);
      }

      if (!c1Done)
      {
        ite1 = new PlanIteratorWrapper(theChild1, planState);
        tSeq1->append(ite1);
      }

      ite0 = tSeq0->getIterator();
      ite1 = tSeq1->getIterator();
      ite0->open();
      ite1->open();

      while (!found && ite0->next(item0))
      {
        while (!found && ite1->next(item1))
        {
          store::Item_t tmp = item0;

          if (generalComparison(loc,
                                tmp,
                                item1,
                                theCompType,
                                theTypeManager,
                                timezone,
                                theCollation))
          {
            found = true;
            break;
          }
        }

        ite1->reset();
      }
    }

    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, found), state);
  }
  else
  {
    // value comparison
    if (consumeNext(item0, theChild0.getp(), planState) &&
        consumeNext(item1, theChild1.getp(), planState))
    {
      STACK_PUSH(GENV_ITEMFACTORY->
                 createBoolean(result,
                               CompareIterator::valueComparison(loc,
                                                                item0,
                                                                item1,
                                                                theCompType,
                                                                theTypeManager,
                                                                timezone,
                                                                theCollation)),
                 state);

      assert(!consumeNext(item0, theChild0.getp(), planState) &&
             !consumeNext(item1, theChild1.getp(), planState));
    }
  }

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
bool CompareIterator::valueComparison(
    const QueryLoc& loc,
    store::Item_t& aItem0,
    store::Item_t& aItem1,
    CompareConsts::CompareType aCompType,
    const TypeManager* typemgr,
    long timezone,
    XQPCollator* aCollation)
{
  try
  {
    switch(aCompType)
    {
    case CompareConsts::VALUE_EQUAL:
    {
      return valueEqual(loc, aItem0, aItem1, typemgr, timezone, aCollation, true);
    }
    case CompareConsts::VALUE_NOT_EQUAL:
    {
      return ! valueEqual(loc, aItem0, aItem1, typemgr, timezone, aCollation, true);
    }
    case CompareConsts::VALUE_GREATER:
    {
      return valueCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) > 0;
    }
    case CompareConsts::VALUE_GREATER_EQUAL:
    {
      return valueCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) >= 0;
    }
    case CompareConsts::VALUE_LESS:
    {
      return valueCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) < 0;
    }
    case CompareConsts::VALUE_LESS_EQUAL:
    {
      return valueCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) <= 0;
    }
    default:
    {
      ZORBA_ASSERT(false);
    }
    }
  }
  catch (const ZorbaException& e)
  {
    if (e.diagnostic() == zerr::ZSTR0041_NAN_COMPARISON)
      return false;

    throw;
  }
}


/*******************************************************************************
  Compares two items for equality according to the rules of value comparison

  @param  item0
  @param  item1
  @param  tm The type manager to use to extract the type of the 2 items and
          perform any necessary type checks and castings/promotions
  @param  timezone Optional timezone parameter used when comparing datetimes
  @param  aCollation optional collation parameter (passed as pointer to make
          it possible to be set to 0)
  @return True if the two item are equal; false otherwise.
  @throw  appropriate exception if any necesary casting/promotion fails.
  @throw  XPTY0004 if the two items are not equality comparable according to
          the table in http://www.w3.org/TR/xquery/#mapping. 
********************************************************************************/
bool CompareIterator::valueEqual(
    const QueryLoc& loc,
    store::Item_t& aItem0,
    store::Item_t& aItem1,
    const TypeManager* typemgr,
    long timezone,
    XQPCollator* aCollation,
    bool raiseError)
{
  store::Item_t castItem0, castItem1;
  if (valueCasting(loc, typemgr, aItem0, aItem1, castItem0, castItem1, raiseError))
    return equal(loc, castItem0, castItem1, typemgr, timezone, aCollation, raiseError);
  else
    return false;
}


/*******************************************************************************

********************************************************************************/
long CompareIterator::valueCompare(
    const QueryLoc& loc,
    store::Item_t& aItem0,
    store::Item_t& aItem1,
    const TypeManager* typemgr,
    long timezone,
    XQPCollator* aCollation)
{
  store::Item_t castItem0, castItem1;
  valueCasting(loc, typemgr, aItem0, aItem1, castItem0, castItem1, true);
  return compare(loc, castItem0, castItem1, typemgr, timezone, aCollation);
}


/*******************************************************************************

********************************************************************************/
bool CompareIterator::valueCasting(
    const QueryLoc& loc,
    const TypeManager* tm,
    store::Item_t& item0,
    store::Item_t& item1,
    store::Item_t& castItem0,
    store::Item_t& castItem1,
    bool raiseError)
{
  store::SchemaTypeCode type0 = item0->getTypeCode();
  store::SchemaTypeCode type1 = item1->getTypeCode();

  // all untyped Atomics to String
  if (type0 == store::XS_UNTYPED_ATOMIC)
  {
    if  (type1 == store::XS_UNTYPED_ATOMIC)
    {
      castItem0.transfer(item0);
      castItem1.transfer(item1);
    }
    else
    {
      if (!GenericCast::castToBuiltinAtomic(castItem0, item0, store::XS_STRING, NULL, loc, raiseError))
        return false;

      if (!GenericCast::promote(castItem1, item1, store::XS_STRING, NULL, tm, loc))
        castItem1.transfer(item1);
    }
  }
  else if (type1 == store::XS_UNTYPED_ATOMIC)
  {
    if (!GenericCast::promote(castItem0, item0, store::XS_STRING, NULL, tm, loc))
      castItem0.transfer(item0);

    if (!GenericCast::castToBuiltinAtomic(castItem1, item1, store::XS_STRING, NULL, loc, raiseError))
      return false;
  }
  else
  {
    if (!GenericCast::promote(castItem0, item0, type1, NULL, tm, loc))
      castItem0.transfer(item0);

    if (!GenericCast::promote(castItem1, item1, type0, NULL, tm, loc))
      castItem1.transfer(item1);
  }
  return true;
}


/*******************************************************************************
  Compares two items for their relative order with respect to a general 
  comparison operator.

  @param  item0
  @param  item1
  $param  aCompType
  @param  tm The type manager to use to extract the type of the 2 items and
          perform any necessary type checks and castings/promotions
  @param  timezone Optional timezone parameter used when comparing datetimes
  @param  aCollation optional collation parameter (passed as pointer to make
          it possible to be set to 0)
  @return True if the ordering between item0 and item1 satisfies the given
          general comparison operator; false otherwise.
  @throw  appropriate exception if any necesary casting/promotion fails.
  @throw  XPTY0004 if the two items are not order comparable.
  @throw  ZSTR0041_NAN_COMPARISON if both the items are of type xs:double or
          xs:float and at leat one of the items is NaN.
********************************************************************************/
bool CompareIterator::generalComparison(
    const QueryLoc& loc,
    store::Item_t& aItem0,
    store::Item_t& aItem1,
    CompareConsts::CompareType aCompType,
    const TypeManager* typemgr,
    long timezone,
    XQPCollator* aCollation)
{
  try
  {
    switch(aCompType)
    {
    case CompareConsts::GENERAL_EQUAL:
    {
      return generalEqual(loc, aItem0, aItem1, typemgr, timezone, aCollation);
    }
    case CompareConsts::GENERAL_NOT_EQUAL:
    {
      return !generalEqual(loc, aItem0, aItem1, typemgr, timezone, aCollation);
    }
    case CompareConsts::GENERAL_GREATER:
    {
      return generalCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) > 0;
    }
    case CompareConsts::GENERAL_GREATER_EQUAL:
    {
      return generalCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) >= 0;
    }
    case CompareConsts::GENERAL_LESS:
    {
      return generalCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) < 0;
    }
    case CompareConsts::GENERAL_LESS_EQUAL:
    {
      return generalCompare(loc, aItem0, aItem1, typemgr, timezone, aCollation) <= 0;
    }
    default:
    {
      ZORBA_ASSERT(false);
    }
    }
  }
  catch (const ZorbaException& e)
  {
    if (e.diagnostic() == zerr::ZSTR0041_NAN_COMPARISON)
      return false;
    throw;
  }
}


/*******************************************************************************
  Compares two items for equality according to the rules of general comparison

  @param  item0
  @param  item1
  @param  tm The type manager to use to extract the type of the 2 items and
          perform any necessary type checks and castings/promotions
  @param  timezone Optional timezone parameter used when comparing datetimes
  @param  aCollation optional collation parameter (passed as pointer to make
          it possible to be set to 0)
  @return True if the two item are equal; false otherwise.
  @throw  appropriate exception if any necesary casting/promotion fails.
  @throw  XPTY0004 if the two items are not equality comparable.
********************************************************************************/
bool CompareIterator::generalEqual(
    const QueryLoc& loc,
    store::Item_t& aItem0,
    store::Item_t& aItem1,
    const TypeManager* typemgr,
    long timezone,
    XQPCollator* aCollation)
{
  store::Item_t castItem0, castItem1;
  generalCasting(loc, typemgr, aItem0, aItem1, castItem0, castItem1);
  return equal(loc, castItem0, castItem1, typemgr, timezone, aCollation, true);
}


/*******************************************************************************
  Compares two items for ordering according to the rules of general comparison

  @param  item0
  @param  item1
  @param  tm The type manager to use to extract the type of the 2 items and
          perform any necessary type checks and castings/promotions
  @param  timezone Optional timezone parameter used when comparing datetimes
  @param  aCollation optional collation parameter (passed as pointer to make
          it possible to be set to 0)
  @return < 0 if item0 is less than item1, 0 if the two items are equal, or
          > 0 if item0 is greater than item1.
  @throw  appropriate exception if any necesary casting/promotion fails.
  @throw  XPTY0004 if the two items are not order comparable.
  @throw  ZSTR0041_NAN_COMPARISON if both the items are of type xs:double or
          xs:float and at leat one of the items is NaN.
********************************************************************************/
long CompareIterator::generalCompare(
    const QueryLoc& loc,
    store::Item_t& aItem0,
    store::Item_t& aItem1,
    const TypeManager* typemgr,
    long timezone,
    XQPCollator* aCollation)
{
  store::Item_t castItem0, castItem1;
  generalCasting(loc, typemgr, aItem0, aItem1, castItem0, castItem1);
  return compare(loc, castItem0, castItem1, typemgr, timezone, aCollation);
}


/*******************************************************************************

********************************************************************************/
void CompareIterator::generalCasting(
    const QueryLoc& loc,
    const TypeManager* tm,
    store::Item_t& item0,
    store::Item_t& item1,
    store::Item_t& castItem0,
    store::Item_t& castItem1)
{
  store::SchemaTypeCode type0 = item0->getTypeCode();
  store::SchemaTypeCode type1 = item1->getTypeCode();

  if (type0 == store::XS_UNTYPED_ATOMIC)
  {
    if (TypeOps::is_numeric(type1))
    {
      GenericCast::castToBuiltinAtomic(castItem0, item0, store::XS_DOUBLE, NULL, loc);

      GenericCast::promote(castItem1, item1, store::XS_DOUBLE, NULL, tm, loc);
    }
    else if (type1 == store::XS_UNTYPED_ATOMIC)
    {
      castItem0.transfer(item0);
      castItem1.transfer(item1);
    }
    else if (TypeOps::is_subtype(type1, store::XS_STRING))
    {
      GenericCast::castToBuiltinAtomic(castItem0, item0, store::XS_STRING, NULL, loc);
      castItem1.transfer(item1);
    }
    else
    {
      GenericCast::castToBuiltinAtomic(castItem0, item0, type1, NULL, loc);
      castItem1.transfer(item1);
    }
  }
  else if (type1 == store::XS_UNTYPED_ATOMIC)
  {
    if (TypeOps::is_numeric(type0))
    {
      GenericCast::castToBuiltinAtomic(castItem1, item1, store::XS_DOUBLE, NULL, loc);
      GenericCast::promote(castItem0, item0, store::XS_DOUBLE, NULL, tm, loc);
    }
    else if (TypeOps::is_subtype(type0, store::XS_STRING))
    {
      GenericCast::castToBuiltinAtomic(castItem1, item1, store::XS_STRING, NULL, loc);
      castItem0.transfer(item0);
    }
    else
    {
      GenericCast::castToBuiltinAtomic(castItem1, item1, type0, NULL, loc);
      castItem0.transfer(item0);
    }
  }
  else
  {
    if (!GenericCast::promote(castItem0, item0, type1, NULL, tm, loc))
      castItem0.transfer(item0);

    if (!GenericCast::promote(castItem1, item1, type0, NULL, tm, loc))
      castItem1.transfer(item1);
  }
}


/*******************************************************************************
  Checks if the two passed items contain the same value (without performing any
  castings or promotions on the two items). 

  @param  item0
  @param  item1
  @param  tm The type manager to use to extract the type of the 2 items
  @param  timezone Optional timezone parameter used when comparing datetimes
  @param  aCollation optional collation parameter (passed as pointer to make
          it possible to be set to 0)
  @return True if the two item are equal; false otherwise.
  @throw  XPTY0004 if the two items are not equality comparable according to
          the table in http://www.w3.org/TR/xquery/#mapping. 
********************************************************************************/
bool CompareIterator::equal(
    const QueryLoc& loc,
    const store::Item_t& item0,
    const store::Item_t& item1,
    const TypeManager* tm,
    long timezone,
    XQPCollator* collation,
    bool raiseError)
{
  store::SchemaTypeCode type0 = item0->getTypeCode();
  store::SchemaTypeCode type1 = item1->getTypeCode();

  if (TypeOps::is_subtype(type0, type1))
  {
    return item1->equals(item0, timezone, collation);
  }
  else if (TypeOps::is_subtype(type1, type0))
  {
    return item0->equals(item1, timezone, collation);
  }
  else
  {
    // There are 3 cases when two types are comparable without one being a
    // subtype of the other: (a) they belong to different branches under of
    // the type-inheritance subtree rooted at xs:integer, (b) they belong to
    // different branches under of the type-inheritance subtree rooted at
    // xs::duration (i.e. one is xs:yearMonthDuration and the other is
    // xs:dayTimeDuration).
    // The same case happens when there are two types derived from xs:NOTATION.
    // (c) either of the types is a subtype of NULL.
    if (TypeOps::is_subtype(type0, store::XS_INTEGER) &&
        TypeOps::is_subtype(type1, store::XS_INTEGER))
    {
      return (item0->getIntegerValue() == item1->getIntegerValue());
    }
    else if (TypeOps::is_subtype(type0, store::XS_DURATION) &&
             TypeOps::is_subtype(type1, store::XS_DURATION))
    {
      return (item0->getDurationValue() == item1->getDurationValue());
    }
    else if (TypeOps::is_subtype(type0, store::XS_NOTATION) &&
             TypeOps::is_subtype(type1, store::XS_NOTATION))
    {
      return item0->equals(item1);
    }
    else if (TypeOps::is_subtype(type0, store::JS_NULL))
    {
      return item0->equals(item1);
    }
    else if (TypeOps::is_subtype(type1, store::JS_NULL))
    {
      return item1->equals(item0);
    }
    else if (raiseError)
    {
      xqtref_t type0 = tm->create_value_type(item0.getp());
      xqtref_t type1 = tm->create_value_type(item1.getp());

      RAISE_ERROR(err::XPTY0004, loc,
      ERROR_PARAMS(ZED(BadType_23o), ERROR_TEXT( *type0 ), ZED(NoCompareWithType_4), ERROR_TEXT( *type1 )));
    }
    else
      return false;
  }
}


/*******************************************************************************
  Performs an order-comparison between two given items (without performing any
  castings or promotions on the two items). 

  @param  item0
  @param  item1
  @param  tm The type manager to use to extract the type of the 2 items
  @param  timezone Optional timezone parameter used when comparing datetimes
  @param  aCollation optional collation parameter (passed as pointer to make
          it possible to be set to 0)
  @return < 0 if item0 is less than item1, 0 if the two items are equal, or
          > 0 if item0 is greater than item1.
  @throw  XPTY0004 if the two items are not order comparable according to
          the table in http://www.w3.org/TR/xquery/#mapping. 
  @throw  ZSTR0041_NAN_COMPARISON if both the items are of type xs:double or
          xs:float and at leat one of the items is NaN.
********************************************************************************/
long CompareIterator::compare(
    const QueryLoc& loc,
    const store::Item_t& item0,
    const store::Item_t& item1,
    const TypeManager* tm,
    long timezone,
    XQPCollator* collation)
{
  store::SchemaTypeCode type0 = item0->getTypeCode();
  store::SchemaTypeCode type1 = item1->getTypeCode();

  // xs:gYearMonth, xs:gYear, xs:gMonthDay, xs:gDay and xs:gMonth have
  // equality but no order: F&O 3.0 10.4.13-10.4.17 define only the -equal
  // operators, so lt/gt/le/ge (XQuery 3.0 3.7.1, B.2) raise XPTY0004.
  auto const is_gregorian = [](store::SchemaTypeCode t) {
    return t >= store::XS_GYEAR_MONTH && t <= store::XS_GMONTH;
  };
  if (is_gregorian(type0) || is_gregorian(type1))
  {
    xqtref_t type0 = tm->create_value_type(item0.getp());
    xqtref_t type1 = tm->create_value_type(item1.getp());

    RAISE_ERROR(err::XPTY0004, loc,
    ERROR_PARAMS(ZED(BadType_23o), ERROR_TEXT( *type0 ), ZED(NoCompareWithType_4), ERROR_TEXT( *type1 )));
  }

  try
  {
    if (TypeOps::is_subtype(type0, store::XS_DURATION) &&
        TypeOps::is_subtype(type1, store::XS_DURATION))
    {
      if (type0 == type1 && type0 != store::XS_DURATION)
      {
        return item0->compare(item1, timezone, collation);
      }
      else
      {
        xqtref_t type0 = tm->create_value_type(item0.getp());
        xqtref_t type1 = tm->create_value_type(item1.getp());

        RAISE_ERROR(err::XPTY0004, loc,
        ERROR_PARAMS(ZED(BadType_23o), ERROR_TEXT( *type0 ), ZED(NoCompareWithType_4), ERROR_TEXT( *type1 )));
      }
    }
    else if (TypeOps::is_subtype(type1, type0))
    {
      return item0->compare(item1, timezone, collation);
    }
    else if (TypeOps::is_subtype(type0, type1))
    {
      return -item1->compare(item0, timezone, collation);
    }
    else
    {
      // There is 1 case when two types are order-comparable without one being a
      // subtype of the other: they belong to different branches under of the
      // type-inheritance subtree rooted at xs:integer.
      if (TypeOps::is_subtype(type0, store::XS_INTEGER) &&
          TypeOps::is_subtype(type1, store::XS_INTEGER))
      {
        return item0->getIntegerValue().compare(item1->getIntegerValue());
      }
      // jn:null is always smaller than any other atomic value type
      else if (TypeOps::is_subtype(type0, store::JS_NULL))
      {
        return -1;
      }
      else if (TypeOps::is_subtype(type1, store::JS_NULL))
      {
        return 1;
      }
      else
      {
        xqtref_t type0 = tm->create_value_type(item0.getp());
        xqtref_t type1 = tm->create_value_type(item1.getp());

        RAISE_ERROR(err::XPTY0004, loc,
        ERROR_PARAMS(ZED(BadType_23o), ERROR_TEXT( *type0 ), ZED(NoCompareWithType_4), ERROR_TEXT( *type1 )));
      }
    }
  }
  catch(const ZorbaException& e)
  {
    // For example, two QName items do not have an order relationship.
    if (e.diagnostic() == zerr::ZSTR0040_TYPE_ERROR)
    {
      xqtref_t type0 = tm->create_value_type(item0.getp());
      xqtref_t type1 = tm->create_value_type(item1.getp());

      RAISE_ERROR(err::XPTY0004, loc,
      ERROR_PARAMS(ZED(BadType_23o), ERROR_TEXT( *type0 ), ZED(NoStrictCompareWithType_4), ERROR_TEXT( *type1 )));
    }

    throw;
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  TypedValueCompareIterator                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

template <store::SchemaTypeCode ATC>
void TypedValueCompareIterator<ATC>::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<TypedValueCompareIterator<ATC>, PlanIteratorState>*)this);

  SERIALIZE_ENUM(CompareConsts::CompareType, theCompType);
  ar & theCollation;
}


template <store::SchemaTypeCode ATC>
void TypedValueCompareIterator<ATC>::accept(PlanIterVisitor& v) const
{
  if (!v.hasToVisit(this))
    return;

  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator iter =  this->theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd =  this->theChildren.end();
  for ( ; iter != lEnd; ++iter )
  {
    (*iter)->accept(v);
  }

  v.endVisit(*this);
}


template<store::SchemaTypeCode ATC>
bool TypedValueCompareIterator<ATC>::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t lItem0, lItem1;
  bool bRes;
  bool neq = false;
  long cmp;
  long const timezone = planState.theLocalDynCtx->get_implicit_timezone();

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (CONSUME(lItem0, 0) && CONSUME(lItem1, 1))
  {
    switch (theCompType)
    {
    case CompareConsts::VALUE_NOT_EQUAL:
      neq = true;
      [[fallthrough]];

    case CompareConsts::VALUE_EQUAL:
    {
      bRes = lItem0->equals(lItem1, timezone, theCollation);
      if (neq)
        bRes = ! bRes;

      break;
    }

    default:
    {
      try
      {
        cmp = lItem0->compare(lItem1, timezone, theCollation);

        switch (theCompType)
        {
        case CompareConsts::VALUE_LESS:
          bRes = (cmp < 0);
          break;
        case CompareConsts::VALUE_GREATER:
          bRes = (cmp > 0);
          break;
        case CompareConsts::VALUE_LESS_EQUAL:
          bRes = (cmp <= 0);
          break;
        case CompareConsts::VALUE_GREATER_EQUAL:
          bRes = (cmp >= 0);
          break;
        default:
          ZORBA_ASSERT(false);
        } // switch (theCompType)
      }
      catch (const ZorbaException& e)
      {
        if (e.diagnostic() == zerr::ZSTR0041_NAN_COMPARISON)
          bRes = false;
        else
          throw;
      }
    } // default
    } // switch (theCompType)

    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, bRes), state);

    assert(!CONSUME(lItem0, 0) && !CONSUME(lItem1, 1));
  }

  STACK_END(state);
}


template class TypedValueCompareIterator<store::XS_DOUBLE>;
template class TypedValueCompareIterator<store::XS_FLOAT>;
template class TypedValueCompareIterator<store::XS_DECIMAL>;
template class TypedValueCompareIterator<store::XS_INTEGER>;
template class TypedValueCompareIterator<store::XS_STRING>;



/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  AtomicValuesEquivalenceIterator                                            //
//                                                                             //
//  http://www.w3.org/TR/xquery-11/#dt-equivalence-two-atomic-values           //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

AtomicValuesEquivalenceIterator::AtomicValuesEquivalenceIterator(
    static_context* sctx,
    const QueryLoc& loc,
    PlanIter_t aChild0,
    PlanIter_t aChild1)
  :
  BinaryBaseIterator<AtomicValuesEquivalenceIterator, PlanIteratorState>(sctx, loc, aChild0, aChild1),
  theTypeManager(sctx->get_typemanager()),
  theCollation(sctx->get_default_collator(loc))
{
}


AtomicValuesEquivalenceIterator::AtomicValuesEquivalenceIterator(
    ::zorba::serialization::Archiver& ar)
  :
  BinaryBaseIterator<AtomicValuesEquivalenceIterator, PlanIteratorState>(ar),
  theTypeManager(NULL),
  theCollation(NULL)
{
}


void AtomicValuesEquivalenceIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (BinaryBaseIterator<AtomicValuesEquivalenceIterator, PlanIteratorState>*)this);

  SERIALIZE_TYPEMANAGER(TypeManager, theTypeManager);
  ar & theCollation;
}


BINARY_ACCEPT(AtomicValuesEquivalenceIterator);


bool AtomicValuesEquivalenceIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t lItem0, lItem1;
  store::SchemaTypeCode type0;
  store::SchemaTypeCode type1;
  bool are_equivalent;
  long const timezone = planState.theLocalDynCtx->get_implicit_timezone();

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(lItem0, theChild0.getp(), planState))
  {
    if (consumeNext(lItem1, theChild1.getp(), planState))
    {
      type0 = lItem0->getTypeCode();
      type1 = lItem1->getTypeCode();

      if ((TypeOps::is_subtype(type0, store::XS_FLOAT) ||
           TypeOps::is_subtype(type0, store::XS_DOUBLE))
          &&
          (TypeOps::is_subtype(type1, store::XS_FLOAT) ||
           TypeOps::is_subtype(type1, store::XS_DOUBLE))
          &&
          lItem0->isNaN() && lItem1->isNaN())
      {
        STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, true), state);
      }
      else
      {
        try
        {
          are_equivalent = CompareIterator::valueEqual(loc,
                                                       lItem0,
                                                       lItem1,
                                                       theTypeManager,
                                                       timezone,
                                                       theCollation,
                                                       true);
        }
        catch (ZorbaException const& e)
        {
          if (e.diagnostic() == err::XPTY0004)
            are_equivalent = false;
          else
            throw;
        }

        STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, are_equivalent),
                   state);
      }
    }
    else
    {
      STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, false), state);
    }
  }
  else if (consumeNext(lItem1, theChild1.getp(), planState))
  {
    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, false), state);
  }
  else
  {
    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(result, true), state);
  }

  STACK_END(state);
}


} // namespace zorba
/* vim:set et sw=2 ts=2: */
