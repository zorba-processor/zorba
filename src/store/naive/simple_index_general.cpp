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

#include "simple_index_general.h"
#include "simple_store.h"
#include "atomic_items.h"
#include "simple_item_factory.h"

#include "diagnostics/assert.h"
#include "diagnostics/util_macros.h"

#include "zorbatypes/float.h"

#include <cmath>
#include <limits>

namespace zorba 
{ 

namespace simplestore 
{

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  GeneralIndexCompareFunction                                                //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************

********************************************************************************/
GeneralIndexCompareFunction::GeneralIndexCompareFunction(
    long timezone,
    const std::string& collation)
  :
  theTimezone(timezone)
{
  if (!collation.empty())
  {
    theCollator = CollationFactory::createCollator(collation);
  }
  else
  {
    theCollator = NULL;
  } 
}


/******************************************************************************

********************************************************************************/
GeneralIndexCompareFunction::~GeneralIndexCompareFunction()
{
  delete theCollator; 
}


/******************************************************************************

********************************************************************************/
uint32_t GeneralIndexCompareFunction::hash(const store::Item* key) const
{
  uint32_t hval = FNV_32_INIT;

  if (key == NULL)
    return hval;

  return hashfun::h32<uint32_t>(key->hash(theTimezone, theCollator), hval);
}


/******************************************************************************

********************************************************************************/
bool GeneralIndexCompareFunction::equal(
    const store::Item* key1,
    const store::Item* key2) const
{
  if (key1 == NULL && key2 == NULL)
    return true;

  if (key1 == NULL || key2 == NULL)
    return false;

  // NaN is equal to no number, but its entries are kept under one key.
  if (key1->isNaN() || key2->isNaN())
    return key1->isNaN() && key2->isNaN();

  if (! key1->equals(key2, theTimezone, theCollator))
    return false;
  
  return true;
}


/*******************************************************************************

********************************************************************************/
long GeneralIndexCompareFunction::compare(
    const store::Item* key1,
    const store::Item* key2) const
{
  long result;

  if (key1 == NULL)
  {
    if (key2 != NULL)
      return -1;
  }
  else if (key2 == NULL)
  {
    if (key1 != NULL)
      return +1;
  }
  else if (key1 == IndexConditionImpl::theNegInf)
  {
    return -1;
  }
  else if (key1 == IndexConditionImpl::thePosInf)
  {
    return +1;
  }
  else if (key2 == IndexConditionImpl::theNegInf)
  {
    return +1;
  }
  else if (key2 == IndexConditionImpl::thePosInf)
  {
    return -1;
  }
  else if (key1->isNaN() || key2->isNaN())
  {
    // NaN is ordered with no number; its entries are kept first in a map.
    if (!key1->isNaN())
      return +1;

    return (key2->isNaN() ? 0 : -1);
  }
  else if ((result = key1->compare(key2, theTimezone, theCollator)))
  {
    return result;
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  GeneralIndexValue                                                          //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************

*******************************************************************************/
bool GeneralIndexValue::removeNode(
    const store::Item_t& node,
    bool untyped,
    ImageKind image)
{
  GeneralIndexValue::iterator ite = theNodes.begin();
  GeneralIndexValue::iterator end = theNodes.end();

  for (; ite != end; ++ite)
  {
    if ((*ite).theNode == node &&
        (*ite).theUntyped == untyped &&
        (*ite).theImage == image)
    {
      theNodes.erase(ite);
      return true;
    }
  }

  return false;
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndexValue::imagesOnly() const
{
  for (const_iterator ite = theNodes.begin(); ite != theNodes.end(); ++ite)
  {
    if ((*ite).theImage == NOT_IMAGE)
      return false;
  }

  return true;
}


/******************************************************************************

*******************************************************************************/
void GeneralIndexValue::addNode(
    store::Item_t& node,
    bool untyped,
    ImageKind image)
{
  csize numNodes = theNodes.size();
  theNodes.resize(numNodes + 1);
  theNodes[numNodes].theNode.transfer(node);
  theNodes[numNodes].theUntyped = untyped;
  theNodes[numNodes].theImage = image;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  GeneralIndex                                                               //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
  Whether keys of this type are stored in the xs:long map as they are.
*******************************************************************************/
static bool isLongMapType(store::SchemaTypeCode type)
{
  switch (type)
  {
  case store::XS_LONG:
  case store::XS_INT:
  case store::XS_SHORT:
  case store::XS_BYTE:
  case store::XS_UNSIGNED_INT:
  case store::XS_UNSIGNED_SHORT:
  case store::XS_UNSIGNED_BYTE:
    return true;
  default:
    return false;
  }
}


/******************************************************************************
  The xs:double and xs:float values that an item of xs:decimal or a derived
  type is promoted to, rounded as the item comparisons round them.
*******************************************************************************/
static void promoteExact(
    const AtomicItem* item,
    xs_double& doubleValue,
    xs_float& floatValue)
{
  if (isLongMapType(item->getTypeCode()))
  {
    xs_long longValue = item->getLongValue();
    doubleValue = xs_double(static_cast<double>(longValue));
    floatValue = xs_float(static_cast<float>(longValue));
  }
  else
  {
    xs_decimal decimalValue = item->getDecimalValue();
    doubleValue = xs_double(decimalValue);
    floatValue = xs_float(decimalValue);
  }
}


/******************************************************************************

*******************************************************************************/
static bool equalsLong(double doubleValue, xs_long longValue)
{
  static const double twoPow63 = 9223372036854775808.0;

  return (doubleValue >= -twoPow63 &&
          doubleValue < twoPow63 &&
          static_cast<xs_long>(doubleValue) == longValue);
}


/******************************************************************************
  The xs:float value that is equal to an xs:double value, if there is one.
*******************************************************************************/
static bool exactFloat(double doubleValue, float& floatValue)
{
  if (!(std::fabs(doubleValue) <= std::numeric_limits<float>::max()) &&
      !std::isinf(doubleValue))
    return false;

  floatValue = static_cast<float>(doubleValue);
  return static_cast<double>(floatValue) == doubleValue;
}


/******************************************************************************

*******************************************************************************/
GeneralIndex::GeneralIndex(
    const store::Item_t& qname,
    const store::IndexSpecification& spec)
  :
  IndexImpl(qname, spec),
  theKeyTypeCode(store::XS_LAST),
  theCompFunction(spec.theTimezone, spec.theCollations[0]),
  theNumUntypedEntries(0),
  theNumMultiKeyNodes(0)
{
  store::Item* typeName = spec.theKeyTypes[0].getp();

  if (typeName != NULL &&
      typeName != GET_STORE().theSchemaTypeNames[store::XS_UNTYPED_ATOMIC] &&
      typeName != GET_STORE().theSchemaTypeNames[store::XS_ANY_ATOMIC])
  {
    theKeyTypeCode = GET_STORE().theSchemaTypeCodes[spec.theKeyTypes[0].getp()];
  }
}


/******************************************************************************

*******************************************************************************/
GeneralIndex::~GeneralIndex()
{
}


/******************************************************************************

*******************************************************************************/
const XQPCollator* GeneralIndex::getCollator(csize i) const
{
  ZORBA_ASSERT(i == 0);
  return theCompFunction.getCollator();
}


/******************************************************************************

*******************************************************************************/
csize GeneralIndex::size() const
{
  assert(false);
  return 0;
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndex::insert(store::IndexKey*& key, store::Item_t& value)
{
  if (key->size() != 1)
  {
    RAISE_ERROR_NO_LOC(zerr::ZDDY0035_INDEX_GENERAL_INSERT,
    ERROR_PARAMS(getName()->getStringValue()));
  }
  return insert((*key)[0], value);
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndex::insert(store::Item_t& key, store::Item_t& node)
{
  bool found = false;
  store::Item_t castItem;
  store::Item_t node2;

  bool sorted = isSorted();

  AtomicItem* keyItem = static_cast<AtomicItem*>(key.getp());

  if (keyItem == NULL)
  {
    csize numNodes = theEmptyKeyNodes.size();
    theEmptyKeyNodes.resize(numNodes + 1);
    theEmptyKeyNodes[numNodes].transfer(node);
    return true;
  }

  if (keyItem->getBaseItem() != NULL)
  {
    keyItem = static_cast<AtomicItem*>(keyItem->getBaseItem());
    key = keyItem;
  }

  store::SchemaTypeCode keyType = keyItem->getTypeCode();

  if (isTyped())
  {
    return insertInMap(key, node, theKeyTypeCode, false);
  }

  switch (keyType)
  {
  case store::XS_BASE64BINARY:
  case store::XS_HEXBINARY:

  case store::XS_QNAME:
  case store::XS_NOTATION:

  case store::XS_GYEAR_MONTH:
  case store::XS_GYEAR:
  case store::XS_GMONTH_DAY:
  case store::XS_GDAY:
  case store::XS_GMONTH:
  {
    assert(!sorted);
    // falth through
  }

  case store::XS_ANY_URI:

  case store::XS_BOOLEAN:

  case store::XS_DATETIME:
  case store::XS_DATE:
  case store::XS_TIME:

  case store::JS_NULL:
  {
    return insertInMap(key, node, keyType, false);
  }

  case store::XS_DATETIME_STAMP:
  {
    return insertInMap(key, node, store::XS_DATETIME, false);
  }

  case store::XS_DURATION:
  case store::XS_YM_DURATION:
  case store::XS_DT_DURATION:
  {
    return insertInMap(key, node, store::XS_DURATION, false);
  }

  case store::XS_STRING:
  case store::XS_NORMALIZED_STRING:
  case store::XS_TOKEN:
  case store::XS_NMTOKEN:
  case store::XS_LANGUAGE:
  case store::XS_NAME:
  case store::XS_NCNAME:
  case store::XS_ID:
  case store::XS_IDREF:
  case store::XS_ENTITY:
  {
    return insertInMap(key, node, store::XS_STRING, false);
  }

  case store::XS_DOUBLE:
  {
    return insertInMap(key, node, store::XS_DOUBLE, false);
  }

  case store::XS_FLOAT:
  {
    return insertInMap(key, node, store::XS_FLOAT, false);
  }

  case store::XS_DECIMAL:
  case store::XS_INTEGER:
  case store::XS_NON_POSITIVE_INTEGER:
  case store::XS_NEGATIVE_INTEGER:
  case store::XS_NON_NEGATIVE_INTEGER:
  case store::XS_POSITIVE_INTEGER:
  case store::XS_UNSIGNED_LONG:
  {
    // try lossless cast to xs:long
    keyItem->castToLong(castItem);

    if (castItem != NULL)
    {
      key.transfer(castItem);
      return insertExact(key, node, store::XS_LONG);
    }

    return insertExact(key, node, store::XS_DECIMAL);
  }

  case store::XS_LONG:
  case store::XS_INT:
  case store::XS_SHORT:
  case store::XS_BYTE:
  case store::XS_UNSIGNED_INT:
  case store::XS_UNSIGNED_SHORT:
  case store::XS_UNSIGNED_BYTE:
  {
    return insertExact(key, node, store::XS_LONG);
  }

  case store::XS_UNTYPED_ATOMIC:
  {
    store::ItemHandle<UntypedAtomicItem> untypedItem = 
    static_cast<UntypedAtomicItem*>(key.getp());

    // cast to xs:string
    untypedItem->castToString(castItem);

    node2 = node;
    found = insertInMap(castItem, node2, store::XS_STRING, false);

    // No reason to cast to xs:anyUri, because when we probe with xs:string or
    // with xs:anyUri, or with xs:untypedAtomic, we unconditionally probe both
    // the STRING and the ANY_URI tables.
 
    // Compared with a numeric key or probe, an untyped value is cast to
    // xs:double.
    if (untypedItem->castToDouble(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_DOUBLE, true))
        found = true;
    }

    // try casting to xs:datetime
    else if (untypedItem->castToDateTime(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_DATETIME, true))
        found = true;
    }

    // try casting to xs:date
    else if (untypedItem->castToDate(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_DATE, true))
        found = true;
    }

    // try casting to xs:time
    else if (untypedItem->castToTime(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_TIME, true))
        found = true;
    }

    // try casting to xs:gYearMonth
    if (!sorted && untypedItem->castToGYearMonth(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_GYEAR_MONTH, true))
        found = true;
    }

    // try casting to xs:gMonthDay
    else if (!sorted && untypedItem->castToGMonthDay(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_GMONTH_DAY, true))
        found = true;
    }

    // try casting to xs:gDay
    else if (!sorted && untypedItem->castToGDay(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_GDAY, true))
        found = true;
    }

    // try casting to xs:gMonth
    else if (!sorted && untypedItem->castToGMonth(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_GMONTH, true))
        found = true;
    }

    // try casting to xs:duration
    else if (untypedItem->castToDuration(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_DURATION, true))
        found = true;
    }

    // These casts can succeed together with one of the casts above.
    if (!sorted && untypedItem->castToGYear(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_GYEAR, true))
        found = true;
    }

    if (!sorted && untypedItem->castToHexBinary(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_HEXBINARY, true))
        found = true;
    }

    if (!sorted && untypedItem->castToBase64Binary(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_BASE64BINARY, true))
        found = true;
    }

    if (untypedItem->castToBoolean(castItem))
    {
      node2 = node;
      if (insertInMap(castItem, node2, store::XS_BOOLEAN, true))
        found = true;
    }

    return found;
  }

  default:
  {
    RAISE_ERROR_NO_LOC(zerr::ZDTY0012_INDEX_KEY_TYPE_ERROR, 
    ERROR_PARAMS(getName()->getStringValue()));
    return false;
  }
  }
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndex::insertInMap(
    store::Item_t& key,
    store::Item_t& node,
    store::SchemaTypeCode targetMap,
    bool untyped,
    GeneralIndexValue::ImageKind image)
{
  if (untyped)
    ++theNumUntypedEntries;

  if (isSorted())
  {
    GeneralTreeIndex* idx = static_cast<GeneralTreeIndex*>(this);
    return idx->insertInMap(key, node, idx->theMaps[targetMap], untyped, image);
  }
  else
  {
    GeneralHashIndex* idx = static_cast<GeneralHashIndex*>(this);
    return idx->insertInMap(key, node, idx->theMaps[targetMap], untyped, image);
  }
}


/******************************************************************************
  The images (see GeneralIndex) of a key that goes into the xs:long or the
  xs:decimal map: none in a tree index, and in a hash index the promotions of
  the key to xs:double and xs:float, except one that is equal to an xs:long key
  exactly.
*******************************************************************************/
void GeneralIndex::getImages(
    const AtomicItem* key,
    store::SchemaTypeCode targetMap,
    store::Item_t& doubleImage,
    store::Item_t& floatImage) const
{
  doubleImage = NULL;
  floatImage = NULL;

  if (isSorted())
    return;

  xs_double doubleValue;
  xs_float floatValue;
  promoteExact(key, doubleValue, floatValue);

  bool longKey = (targetMap == store::XS_LONG);

  if (!longKey || !equalsLong(doubleValue.getNumber(), key->getLongValue()))
    GET_FACTORY().createDouble(doubleImage, doubleValue);

  if (!longKey || !equalsLong(floatValue.getNumber(), key->getLongValue()))
    GET_FACTORY().createFloat(floatImage, floatValue);
}


/******************************************************************************
  Inserts a key of xs:decimal or a derived type in the xs:long or the
  xs:decimal map, with its images.
*******************************************************************************/
bool GeneralIndex::insertExact(
    store::Item_t& key,
    store::Item_t& node,
    store::SchemaTypeCode targetMap)
{
  bool found = false;
  store::Item_t doubleImage;
  store::Item_t floatImage;
  store::Item_t node2;

  getImages(static_cast<AtomicItem*>(key.getp()), targetMap,
            doubleImage, floatImage);

  if (doubleImage != NULL)
  {
    node2 = node;
    if (insertInMap(doubleImage, node2, store::XS_DOUBLE, false,
                    GeneralIndexValue::DOUBLE_IMAGE))
      found = true;
  }

  if (floatImage != NULL)
  {
    node2 = node;
    if (insertInMap(floatImage, node2, store::XS_FLOAT, false,
                    GeneralIndexValue::FLOAT_IMAGE))
      found = true;
  }

  if (insertInMap(key, node, targetMap, false))
    found = true;

  return found;
}


/******************************************************************************
  Removes what insertExact() inserted.
*******************************************************************************/
bool GeneralIndex::removeExact(
    const store::Item_t& key,
    const store::Item_t& node,
    store::SchemaTypeCode targetMap)
{
  store::Item_t doubleImage;
  store::Item_t floatImage;

  getImages(static_cast<AtomicItem*>(key.getp()), targetMap,
            doubleImage, floatImage);

  bool found = removeFromMap(key, node, targetMap, false);

  if (doubleImage != NULL &&
      removeFromMap(doubleImage, node, store::XS_DOUBLE, false,
                    GeneralIndexValue::DOUBLE_IMAGE))
    found = true;

  if (floatImage != NULL &&
      removeFromMap(floatImage, node, store::XS_FLOAT, false,
                    GeneralIndexValue::FLOAT_IMAGE))
    found = true;

  return found;
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndex::remove(const store::Item_t& key, const store::Item_t& node)
{
  bool found = false;
  store::Item_t castItem;

  bool sorted = isSorted();

  AtomicItem* keyItem = static_cast<AtomicItem*>(key.getp());

  if (keyItem == NULL)
  {
    std::vector<store::Item_t>::iterator ite =
    std::find(theEmptyKeyNodes.begin(), theEmptyKeyNodes.end(), node);

    ZORBA_ASSERT(ite != theEmptyKeyNodes.end());

    theEmptyKeyNodes.erase(ite);
    return true;
  }

  if (keyItem->getBaseItem() != NULL)
  {
    keyItem = static_cast<AtomicItem*>(keyItem->getBaseItem());
  }

  if (isTyped())
  {
    return removeFromMap(key, node, theKeyTypeCode, false);
  }

  store::SchemaTypeCode keyType = keyItem->getTypeCode();

  switch (keyType)
  {
  case store::XS_BASE64BINARY:
  case store::XS_HEXBINARY:

  case store::XS_QNAME:
  case store::XS_NOTATION:

  case store::XS_GYEAR_MONTH:
  case store::XS_GYEAR:
  case store::XS_GMONTH_DAY:
  case store::XS_GDAY:
  case store::XS_GMONTH:
  {
    assert(!sorted);
    // falth through
  }

  case store::XS_ANY_URI:

  case store::XS_BOOLEAN:

  case store::XS_DATETIME:
  case store::XS_DATE:
  case store::XS_TIME:

  case store::JS_NULL:
  {
    return removeFromMap(key, node, keyType, false);
  }

  case store::XS_DATETIME_STAMP:
  {
    return removeFromMap(key, node, store::XS_DATETIME, false);
  }

  case store::XS_DURATION:
  case store::XS_YM_DURATION:
  case store::XS_DT_DURATION:
  {
    return removeFromMap(key, node, store::XS_DURATION, false);
  }

  case store::XS_STRING:
  case store::XS_NORMALIZED_STRING:
  case store::XS_TOKEN:
  case store::XS_NMTOKEN:
  case store::XS_LANGUAGE:
  case store::XS_NAME:
  case store::XS_NCNAME:
  case store::XS_ID:
  case store::XS_IDREF:
  case store::XS_ENTITY:
  {
    return removeFromMap(key, node, store::XS_STRING, false);
  }

  case store::XS_DOUBLE:
  {
    return removeFromMap(key, node, store::XS_DOUBLE, false);
  }

  case store::XS_FLOAT:
  {
    return removeFromMap(key, node, store::XS_FLOAT, false);
  }

  case store::XS_DECIMAL:
  case store::XS_INTEGER:
  case store::XS_NON_POSITIVE_INTEGER:
  case store::XS_NEGATIVE_INTEGER:
  case store::XS_NON_NEGATIVE_INTEGER:
  case store::XS_POSITIVE_INTEGER:
  case store::XS_UNSIGNED_LONG:
  {
    // try lossless cast to xs:long
    keyItem->castToLong(castItem);

    if (castItem != NULL)
      return removeExact(castItem, node, store::XS_LONG);

    return removeExact(keyItem, node, store::XS_DECIMAL);
  }

  case store::XS_LONG:
  case store::XS_INT:
  case store::XS_SHORT:
  case store::XS_BYTE:
  case store::XS_UNSIGNED_INT:
  case store::XS_UNSIGNED_SHORT:
  case store::XS_UNSIGNED_BYTE:
  {
    return removeExact(keyItem, node, store::XS_LONG);
  }

  case store::XS_UNTYPED_ATOMIC:
  {
    store::ItemHandle<UntypedAtomicItem> untypedItem = 
    static_cast<UntypedAtomicItem*>(key.getp());

    // cast to xs:string
    untypedItem->castToString(castItem);

    found = removeFromMap(castItem, node, store::XS_STRING, false);

    if (untypedItem->castToDouble(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_DOUBLE, true))
        found = true;
    }

    // try casting to xs:datetime
    else if (untypedItem->castToDateTime(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_DATETIME, true))
        found = true;
    }

    // try casting to xs:date
    else if (untypedItem->castToDate(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_DATE, true))
        found = true;
    }

    // try casting to xs:time
    else if (untypedItem->castToTime(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_TIME, true))
        found = true;
    }

    // try casting to xs:gYearMonth
    if (!sorted && untypedItem->castToGYearMonth(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_GYEAR_MONTH, true))
        found = true;
    }

    // try casting to xs:gMonthDay
    else if (!sorted && untypedItem->castToGMonthDay(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_GMONTH_DAY, true))
        found = true;
    }

    // try casting to xs:gDay
    else if (!sorted && untypedItem->castToGDay(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_GDAY, true))
        found = true;
    }

    // try casting to xs:gMonth
    else if (!sorted && untypedItem->castToGMonth(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_GMONTH, true))
        found = true;
    }

    // try casting to xs:duration
    else if (untypedItem->castToDuration(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_DURATION, true))
        found = true;
    }

    if (!sorted && untypedItem->castToGYear(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_GYEAR, true))
        found = true;
    }

    if (!sorted && untypedItem->castToHexBinary(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_HEXBINARY, true))
        found = true;
    }

    if (!sorted && untypedItem->castToBase64Binary(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_BASE64BINARY, true))
        found = true;
    }

    if (untypedItem->castToBoolean(castItem))
    {
      if (removeFromMap(castItem, node, store::XS_BOOLEAN, true))
        found = true;
    }

    return found;
  }

  default:
    ZORBA_ASSERT(false);
  }

  return true;
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndex::remove(
    const store::IndexKey* key,
    const store::Item_t& node,
    bool all)
{
  ZORBA_ASSERT(false);
}


/******************************************************************************

*******************************************************************************/
bool GeneralIndex::removeFromMap(
    const store::Item_t& key,
    const store::Item_t& node,
    store::SchemaTypeCode targetMap,
    bool untyped,
    GeneralIndexValue::ImageKind image)
{
  bool found;

  if (isSorted())
  {
    GeneralTreeIndex* idx = static_cast<GeneralTreeIndex*>(this);
    found = idx->removeFromMap(key, node, idx->theMaps[targetMap], untyped, image);
  }
  else
  {
    GeneralHashIndex* idx = static_cast<GeneralHashIndex*>(this);
    found = idx->removeFromMap(key, node, idx->theMaps[targetMap], untyped, image);
  }

  if (found && untyped)
    --theNumUntypedEntries;

  return found;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Hash Map General Index                                                     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************

*******************************************************************************/
GeneralHashIndex::GeneralHashIndex(
    const store::Item_t& qname,
    const store::IndexSpecification& spec)
  :
  GeneralIndex(qname, spec),
  theSingleMap(NULL)
{
  assert(getNumColumns() == 1);

  memset(reinterpret_cast<void*>(theMaps), 0, store::XS_LAST * sizeof(IndexMap*));

  if (isTyped())
  {
    theSingleMap = new IndexMap(theCompFunction, 1024, spec.theIsThreadSafe);

    theMaps[theKeyTypeCode] = theSingleMap;
  }
}


/******************************************************************************

*******************************************************************************/
GeneralHashIndex::~GeneralHashIndex()
{
  for (csize i = 0; i < store::XS_LAST; ++i)
  {
    if (theMaps[i] == NULL)
      continue;

    IndexMap::iterator ite = theMaps[i]->begin();
    IndexMap::iterator end = theMaps[i]->end();
 
    for (; ite != end; ++ite)
    {
      //std::cout << "Index Entry Delete [" << (*ite).first << "," 
      //          << (*ite).second << "]" << std::endl;
      
      const_cast<store::Item*>((*ite).first)->removeReference();
      delete (*ite).second;
    }

    delete theMaps[i];
  }
}


/******************************************************************************

*******************************************************************************/
void GeneralHashIndex::clear()
{
  for (csize i = 0; i < store::XS_LAST; ++i)
  {
    if (theMaps[i] == NULL)
      continue;

    IndexMap::iterator ite = theMaps[i]->begin();
    IndexMap::iterator end = theMaps[i]->end();
 
    for (; ite != end; ++ite)
    {
      //std::cout << "Index Entry Delete [" << (*ite).first << "," 
      //          << (*ite).second << "]" << std::endl;
      
      const_cast<store::Item*>((*ite).first)->removeReference();
      delete (*ite).second;
    }

    theMaps[i]->clear();
  }

  theEmptyKeyNodes.clear();
  theNumUntypedEntries = 0;
  theNumMultiKeyNodes = 0;
}


/******************************************************************************

*******************************************************************************/
store::Index::KeyIterator_t GeneralHashIndex::keys() const
{
  return new KeyIterator(theMaps);
}


/******************************************************************************

*******************************************************************************/
bool GeneralHashIndex::insertInMap(
    store::Item_t& key,
    store::Item_t& node,
    IndexMap*& targetMap,
    bool untyped,
    GeneralIndexValue::ImageKind image)
{
  GeneralIndexValue* valueSet = NULL;

  if (targetMap == NULL)
    targetMap = new IndexMap(theCompFunction, 1024, theSpec.theIsThreadSafe);

  if (targetMap->get(key, valueSet))
  {
    // NaN keys share an entry but are not equal to each other.
    if (isUnique() && !key->isNaN())
    {
      RAISE_ERROR_NO_LOC(zerr::ZDDY0024_INDEX_UNIQUE_VIOLATION,
      ERROR_PARAMS(theQname->getStringValue()));
    }

    valueSet->addNode(node, untyped, image);

    return true;
  }

  valueSet = new GeneralIndexValue();
  valueSet->addNode(node, untyped, image);
  
  //std::cout << "Index Entry Insert [" << key << "," 
  //          << valueSet << "]" << std::endl;
  
  targetMap->insert(key.getp(), valueSet);
  key.release(); // ownership of the key passes to the index.

  return false;
}


/******************************************************************************

*******************************************************************************/
bool GeneralHashIndex::removeFromMap(
    const store::Item_t& key,
    const store::Item_t& node,
    IndexMap* targetMap,
    bool untyped,
    GeneralIndexValue::ImageKind image)
{
  assert(targetMap);
  assert(key != NULL);

  IndexMap::iterator pos = targetMap->find(key);

  if (pos != targetMap->end())
  {
    GeneralIndexValue* valueSet = (*pos).second;

    bool found = valueSet->removeNode(node, untyped, image);

    if (valueSet->empty())
    {
      // Released after the erase, which hashes the key.
      const store::Item* mapKey = (*pos).first;
      delete valueSet;
      targetMap->erase(pos);
      const_cast<store::Item*>(mapKey)->removeReference();
    }

    return found;
  }

  return false;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  GeneralHashIndex::KeyIterator                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************

********************************************************************************/
GeneralHashIndex::KeyIterator::KeyIterator(IndexMap* const* aMaps)
  :
  theMaps(aMaps)
{
}

/******************************************************************************

*******************************************************************************/
GeneralHashIndex::KeyIterator::~KeyIterator()
{
}

/******************************************************************************
 * moves theIterator to the next available iterator from theMaps
*******************************************************************************/
void GeneralHashIndex::KeyIterator::setNextIter()
{
  for ( ;
    theCurType < store::XS_LAST && theMaps[theCurType] == 0; 
    ++theCurType)
  {}

  if (theCurType != store::XS_LAST)
    theIterator = theMaps[theCurType]->begin();
}

/******************************************************************************
 * moves theIterator to the next key that is not only the xs:double image of
 * keys listed from the xs:long or xs:decimal map
*******************************************************************************/
void GeneralHashIndex::KeyIterator::skipImageKeys()
{
  while (theCurType != store::XS_LAST)
  {
    if (theIterator == theMaps[theCurType]->end())
    {
      ++theCurType;
      setNextIter();
    }
    else if ((*theIterator).second->imagesOnly())
    {
      ++theIterator;
    }
    else
    {
      return;
    }
  }
}

/******************************************************************************

*******************************************************************************/
void GeneralHashIndex::KeyIterator::open()
{
  // start with first type from zorba::store::SchemaTypeCode enum
  theCurType = 0;
  // Index doesn't contain keys from all atomic types -> we need to skip the
  // empty ones.
  setNextIter();
  skipImageKeys();
}


/******************************************************************************

*******************************************************************************/
bool GeneralHashIndex::KeyIterator::next(store::IndexKey& aKey)
{
  // XS_LAST marks the end of the type list -> we stop
  if (theCurType == store::XS_LAST)
    return false;

  const store::Item* lKeyItem = (*theIterator).first;
  // the general index doesn't return an IndexKey object because
  // it doesn't support multi-value keys -> we push the key directly
  // into the passed IndexKey.
  aKey.empty();
  aKey.push_back(const_cast<zorba::store::Item*>(lKeyItem));

  ++theIterator;
  skipImageKeys();

  return true;
}


/******************************************************************************

*******************************************************************************/
void GeneralHashIndex::KeyIterator::close()
{
  theCurType = store::XS_LAST;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Tree General Index                                                         //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************

*******************************************************************************/
GeneralTreeIndex::GeneralTreeIndex(
    const store::Item_t& qname,
    const store::IndexSpecification& spec)
  :
  GeneralIndex(qname, spec),
  theSingleMap(NULL)
{
  assert(getNumColumns() == 1);

  memset(reinterpret_cast<void*>(theMaps), 0, store::XS_LAST * sizeof(IndexMap*));

  if (isTyped())
  {
    theSingleMap = new IndexMap(theCompFunction);

    theMaps[theKeyTypeCode] = theSingleMap;
  }
}


/******************************************************************************

*******************************************************************************/
GeneralTreeIndex::~GeneralTreeIndex()
{
  for (csize i = 0; i < store::XS_LAST; ++i)
  {
    if (theMaps[i] == NULL)
      continue;

    IndexMap::iterator ite = theMaps[i]->begin();
    IndexMap::iterator end = theMaps[i]->end();
 
    for (; ite != end; ++ite)
    {
      //std::cout << "Index Entry Delete [" << (*ite).first << "," 
      //          << (*ite).second << "]" << std::endl;
      
      const_cast<store::Item*>((*ite).first)->removeReference();
      delete (*ite).second;
    }

    delete theMaps[i];
  }
}


/******************************************************************************

*******************************************************************************/
void GeneralTreeIndex::clear()
{
  for (csize i = 0; i < store::XS_LAST; ++i)
  {
    if (theMaps[i] == NULL)
      continue;

    IndexMap::iterator ite = theMaps[i]->begin();
    IndexMap::iterator end = theMaps[i]->end();
 
    for (; ite != end; ++ite)
    {
      //std::cout << "Index Entry Delete [" << (*ite).first << "," 
      //          << (*ite).second << "]" << std::endl;
      
      const_cast<store::Item*>((*ite).first)->removeReference();
      delete (*ite).second;
    }

    theMaps[i]->clear();
  }

  theEmptyKeyNodes.clear();
  theNumUntypedEntries = 0;
  theNumMultiKeyNodes = 0;
}


/******************************************************************************

*******************************************************************************/
bool GeneralTreeIndex::insertInMap(
    store::Item_t& key,
    store::Item_t& node,
    IndexMap*& targetMap,
    bool untyped,
    GeneralIndexValue::ImageKind image)
{
  GeneralIndexValue* valueSet = NULL;

  if (targetMap == NULL)
    targetMap = new IndexMap(theCompFunction);

  IndexMap::iterator pos = targetMap->find(key);

  if (pos != targetMap->end())
  {
    // NaN keys share an entry but are not equal to each other.
    if (isUnique() && !key->isNaN())
    {
      RAISE_ERROR_NO_LOC(zerr::ZDDY0024_INDEX_UNIQUE_VIOLATION,
      ERROR_PARAMS(theQname->getStringValue()));
    }

    pos->second->addNode(node, untyped, image);

    return true;
  }

  valueSet = new GeneralIndexValue();
  valueSet->addNode(node, untyped, image);
  
  //std::cout << "Index Entry Insert [" << key << "," 
  //          << valueSet << "]" << std::endl;
  
  targetMap->insert(IndexMapPair(key.getp(), valueSet));
  key.release(); // ownership of the key obj passes to the index.

  return false;
}


/******************************************************************************

*******************************************************************************/
bool GeneralTreeIndex::removeFromMap(
    const store::Item_t& key,
    const store::Item_t& node,
    IndexMap* targetMap,
    bool untyped,
    GeneralIndexValue::ImageKind image)
{
  assert(targetMap);
  assert(key != NULL);

  IndexMap::iterator pos = targetMap->find(key);

  if (pos != targetMap->end())
  {
    GeneralIndexValue* valueSet = (*pos).second;

    bool found = valueSet->removeNode(node, untyped, image);

    if (valueSet->empty())
    {
      const_cast<store::Item*>((*pos).first)->removeReference();
      delete valueSet;
      targetMap->erase(pos);
    }

    return found;
  }

  return false;
}


/******************************************************************************

*******************************************************************************/
store::Index::KeyIterator_t GeneralTreeIndex::keys() const
{
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  GeneralTreeIndex::KeyIterator                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************

*******************************************************************************/
GeneralTreeIndex::KeyIterator::~KeyIterator()
{
}


/******************************************************************************

*******************************************************************************/
void GeneralTreeIndex::KeyIterator::open()
{
}


/******************************************************************************

*******************************************************************************/
bool GeneralTreeIndex::KeyIterator::next(store::IndexKey&)
{
  return false;
}


/******************************************************************************

*******************************************************************************/
void GeneralTreeIndex::KeyIterator::close()
{
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  ProbeGeneralIndexIterator                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


ProbeGeneralIndexIterator::ProbeGeneralIndexIterator(const store::Index_t& idx)
  :
  theIndex(static_cast<GeneralIndex*>(idx.getp())),
  theIsUntypedProbe(false),
  theIsFullProbe(false),
  theSelectsEmptyKeys(false),
  theEmptyKeyPos(0),
  theNumberKind(NOT_NUMBER)
{
}


/*******************************************************************************

********************************************************************************/
void ProbeGeneralIndexIterator::checkStringKeyType(const AtomicItem* key) const
{
  if (key == NULL)
    return;

  store::SchemaTypeCode keyType = key->getTypeCode();

  // A value comparison casts an untyped key to xs:string, which compares
  // with null.
  if (keyType != store::XS_UNTYPED_ATOMIC &&
      keyType != store::JS_NULL &&
      keyType != store::XS_ANY_URI &&
      keyType != store::XS_STRING &&
      keyType != store::XS_NORMALIZED_STRING &&
      keyType != store::XS_TOKEN &&
      keyType != store::XS_NMTOKEN &&
      keyType != store::XS_LANGUAGE &&
      keyType != store::XS_NAME &&
      keyType != store::XS_NCNAME &&
      keyType != store::XS_ID &&
      keyType != store::XS_IDREF &&
      keyType != store::XS_ENTITY)
  {
    RAISE_ERROR_NO_LOC(err::XPTY0004,
    ERROR_PARAMS(ZED(NoUntypedKeyNodeValue_2), theIndex->getName()->getStringValue()));
  }
}


/*******************************************************************************
  
********************************************************************************/
void ProbeGeneralIndexIterator::init(const store::IndexCondition_t& cond,
                                     const xs_integer& aSkip)
{
  theProbeKind = cond->getKind();
  theCondition = static_cast<GeneralIndexCondition*>(cond.getp());

  // The runtime calls init once per search item on the same iterator.
  theIsUntypedProbe = false;
  theIsFullProbe = false;
  theSelectsEmptyKeys = false;
  theNumberKind = NOT_NUMBER;
  theResultSets.clear();
  theMapBegins.clear();
  theMapEnds.clear();

  if (theProbeKind == store::IndexCondition::POINT_VALUE ||
      theProbeKind == store::IndexCondition::BOX_VALUE)
  {
    if (theIndex->theNumMultiKeyNodes > 0)
    {
      RAISE_ERROR_NO_LOC(err::XPTY0004,
      ERROR_PARAMS(ZED(NoMultiKeyNodeValues_2), theIndex->getName()->getStringValue()));
    }

    if (theIndex->theNumUntypedEntries > 0)
    {
      checkStringKeyType(theCondition->theKey.getp());
      checkStringKeyType(theCondition->theLowerBound.getp());
      checkStringKeyType(theCondition->theUpperBound.getp());
    }
  }

  if (theProbeKind == store::IndexCondition::POINT_VALUE ||
      theProbeKind == store::IndexCondition::POINT_GENERAL)
  {
    initPoint();
  }
  else
  {
    static_cast<ProbeGeneralTreeIndexIterator*>(this)->initBox();
  }
}


/******************************************************************************
  
********************************************************************************/
void ProbeGeneralIndexIterator::initPoint()
{
  // Note: the runtime (or compiler) makes sure that the search key is not NULL
  // and its type is a subtype of the index key type. Furthermore, in the case 
  // of a point-value probe, the type of the search item cannot be untyped, 
  // because untyped search keys either don't match the index key type, or are 
  // cast to xs:string. 
  AtomicItem_t& key = theCondition->theKey;

  bool sorted = theIndex->isSorted();

  if (theIndex->isTyped())
  {
    probeMap(theIndex->theKeyTypeCode, key.getp());
  }
  else
  {
    store::Item_t castItem;

    store::SchemaTypeCode keyType = key->getTypeCode();

    switch (keyType)
    {
    case store::XS_QNAME:
    case store::XS_NOTATION:
    case store::XS_BASE64BINARY:
    case store::XS_HEXBINARY:
    case store::XS_GYEAR_MONTH:
    case store::XS_GYEAR:
    case store::XS_GMONTH_DAY:
    case store::XS_GDAY:
    case store::XS_GMONTH:
    {
      assert(!sorted);
    }

    case store::XS_BOOLEAN:
    case store::XS_DATETIME:
    case store::XS_DATE:
    case store::XS_TIME:
    case store::JS_NULL:
    {
      probeMap(keyType, key.getp());
      break;
    }

    case store::XS_DATETIME_STAMP:
    {
      probeMap(store::XS_DATETIME, key.getp());
      break;
    }

    case store::XS_DURATION:
    case store::XS_YM_DURATION:
    case store::XS_DT_DURATION:
    {
      probeMap(store::XS_DURATION, key.getp()); 
      break;
    }

    case store::XS_ANY_URI:
    {
      probeMap(store::XS_ANY_URI, key.getp()); 

      if (haveMap(store::XS_STRING))
      {
        zstring tmp;
        key->getStringValue2(tmp);
        GET_FACTORY().createString(castItem, tmp);

        probeMap(store::XS_STRING, castItem.getp());
      }

      break;
    }

    case store::XS_STRING:
    case store::XS_NORMALIZED_STRING:
    case store::XS_TOKEN:
    case store::XS_NMTOKEN:
    case store::XS_LANGUAGE:
    case store::XS_NAME:
    case store::XS_NCNAME:
    case store::XS_ID:
    case store::XS_IDREF:
    case store::XS_ENTITY:
    {
      probeMap(store::XS_STRING, key.getp());

      if (haveMap(store::XS_ANY_URI))
      {
        zstring tmp;
        key->getStringValue2(tmp);
        GET_FACTORY().createAnyURI(castItem, tmp);

        probeMap(store::XS_ANY_URI, castItem.getp());
      }

      break;
    }

    case store::XS_DOUBLE:
    case store::XS_FLOAT:
    case store::XS_DECIMAL:
    case store::XS_INTEGER:
    case store::XS_NON_POSITIVE_INTEGER:
    case store::XS_NEGATIVE_INTEGER:
    case store::XS_NON_NEGATIVE_INTEGER:
    case store::XS_POSITIVE_INTEGER:
    case store::XS_UNSIGNED_LONG:
    case store::XS_LONG:
    case store::XS_INT:
    case store::XS_SHORT:
    case store::XS_BYTE:
    case store::XS_UNSIGNED_INT:
    case store::XS_UNSIGNED_SHORT:
    case store::XS_UNSIGNED_BYTE:
    {
      probeNumber(key.getp());
      break;
    }

    case store::XS_UNTYPED_ATOMIC:
    {
      ZORBA_ASSERT(theProbeKind == store::IndexCondition::POINT_GENERAL);

      theIsUntypedProbe = true;

      store::ItemHandle<UntypedAtomicItem> untypedItem = 
      static_cast<UntypedAtomicItem*>(key.getp());

      // cast to xs:string
      if (haveMap(store::XS_STRING))
      {
        untypedItem->castToString(castItem); 
        probeMap(store::XS_STRING, castItem.getp());
      }

      // cast to xs:anyURI
      if (haveMap(store::XS_ANY_URI) && untypedItem->castToUri(castItem))
      {
        probeMap(store::XS_ANY_URI, castItem.getp());
      }

      // Compared with a number, an untyped value is cast to xs:double.
      bool haveNumbers = (haveMap(store::XS_LONG) ||
                          haveMap(store::XS_DECIMAL) ||
                          haveMap(store::XS_DOUBLE) ||
                          haveMap(store::XS_FLOAT));

      if (haveNumbers && untypedItem->castToDouble(castItem))
      {
        probeNumber(static_cast<AtomicItem*>(castItem.getp()));
      }

      // try casting to xs:datetime
      else if (haveMap(store::XS_DATETIME) &&
               untypedItem->castToDateTime(castItem))
      {
        probeMap(store::XS_DATETIME, castItem.getp());
      }

      // try casting to xs:date
      else if (haveMap(store::XS_DATE) &&
               untypedItem->castToDate(castItem))
      {
        probeMap(store::XS_DATE, castItem.getp());
      }

      // try casting to xs:time
      else if (haveMap(store::XS_TIME) &&
               untypedItem->castToTime(castItem))
      {
        probeMap(store::XS_TIME, castItem.getp());
      }

      // try casting to xs:gYearMonth
      if (!sorted && 
          haveMap(store::XS_GYEAR_MONTH) &&
          untypedItem->castToGYearMonth(castItem))
      {
        probeMap(store::XS_GYEAR_MONTH, castItem.getp());
      }

      // try casting to xs:gMonthDay
      else if (!sorted && 
               haveMap(store::XS_GMONTH_DAY) &&
               untypedItem->castToGMonthDay(castItem))
      {
        probeMap(store::XS_GMONTH_DAY, castItem.getp());
      }

      // try casting to xs:gDay
      else if (!sorted &&
               haveMap(store::XS_GDAY) &&
               untypedItem->castToGDay(castItem))
      {
        probeMap(store::XS_GDAY, castItem.getp());
      }

      // try casting to xs:gMonth
      else if (!sorted &&
               haveMap(store::XS_GMONTH) &&
               untypedItem->castToGMonth(castItem))
      {
        probeMap(store::XS_GMONTH, castItem.getp());
      }

      // try casting to xs:duration
      else if (haveMap(store::XS_DURATION) &&
               untypedItem->castToDuration(castItem))
      {
        probeMap(store::XS_DURATION, castItem.getp());
      }

      // These casts can succeed together with one of the casts above.
      if (!sorted &&
          haveMap(store::XS_GYEAR) &&
          untypedItem->castToGYear(castItem))
      {
        probeMap(store::XS_GYEAR, castItem.getp());
      }

      if (!sorted &&
          haveMap(store::XS_HEXBINARY) &&
          untypedItem->castToHexBinary(castItem))
      {
        probeMap(store::XS_HEXBINARY, castItem.getp());
      }

      if (!sorted &&
          haveMap(store::XS_BASE64BINARY) &&
          untypedItem->castToBase64Binary(castItem))
      {
        probeMap(store::XS_BASE64BINARY, castItem.getp());
      }

      if (haveMap(store::XS_BOOLEAN) && untypedItem->castToBoolean(castItem))
      {
        probeMap(store::XS_BOOLEAN, castItem.getp());
      }
      
      break;
    }

    default:
    {
      RAISE_ERROR_NO_LOC(zerr::ZDTY0012_INDEX_KEY_TYPE_ERROR,
      ERROR_PARAMS(theIndex->getName()->getStringValue()));
    }
    }
  }
}


/******************************************************************************
  
********************************************************************************/
void ProbeGeneralIndexIterator::initBox()
{
  GeneralTreeIndex* idx = static_cast<GeneralTreeIndex*>(theIndex.getp());

  bool haveLower = theCondition->theRangeFlags.theHaveLowerBound;
  bool haveUpper = theCondition->theRangeFlags.theHaveUpperBound;

  assert(theProbeKind == store::IndexCondition::BOX_VALUE ||
         !(haveLower && haveUpper));

  theSelectsEmptyKeys = (!haveLower && !haveUpper);

  AtomicItem_t& lowerKey = theCondition->theLowerBound;
  AtomicItem_t& upperKey = theCondition->theUpperBound;
  store::Item_t lowerAltKey;
  store::Item_t upperAltKey;

  zstring tmp;

  if (idx->isTyped())
  {
    // Note: the runtime (or compiler) makes sure that each search key is a
    // subtype of the index key type.
    probeMap(theIndex->theKeyTypeCode, lowerKey, upperKey);
  }
  else if (haveLower || haveUpper)
  {
    // jn:null() is less than every other atomic value and equal to itself
    // (JSONiq), so a null bound selects the other keys by itself, and a null
    // key is selected by the bounds alone. An untyped bound is not compared
    // with a null key.
    GeneralIndexCondition::RangeFlags& flags = theCondition->theRangeFlags;

    struct RangeFlagsRestorer
    {
      GeneralIndexCondition::RangeFlags& theFlags;
      const GeneralIndexCondition::RangeFlags theGiven;
      ~RangeFlagsRestorer() { theFlags = theGiven; }
    } restorer{flags, flags};

    bool nullLower = (haveLower && lowerKey->getTypeCode() == store::JS_NULL);
    bool nullUpper = (haveUpper && upperKey->getTypeCode() == store::JS_NULL);

    bool selectsNullKeys =
      (!haveLower || (nullLower && flags.theLowerBoundIncl)) &&
      (!haveUpper ||
       (nullUpper ?
        flags.theUpperBoundIncl :
        upperKey->getTypeCode() != store::XS_UNTYPED_ATOMIC));

    if (nullLower)
      haveLower = flags.theHaveLowerBound = false;

    if (!haveLower && !haveUpper)
    {
      theIsFullProbe = true;

      for (csize i = 0; i < store::XS_LAST; ++i)
      {
        if (idx->theMaps[i] == NULL || i == store::JS_NULL)
          continue;

        theMapBegins.push_back(idx->theMaps[i]->begin());
        theMapEnds.push_back(idx->theMaps[i]->end());
      }
    }

    store::Item_t castItem;

    store::SchemaTypeCode lowerKeyType = 
    (haveLower ? lowerKey->getTypeCode() : store::XS_LAST);

    store::SchemaTypeCode upperKeyType = 
    (haveUpper ? upperKey->getTypeCode() : store::XS_LAST);

    store::SchemaTypeCode keyType = 
    (nullUpper ? store::JS_NULL :
     lowerKeyType < upperKeyType ? lowerKeyType : upperKeyType);

    switch (keyType)
    {
    case store::XS_BOOLEAN:
    case store::XS_DATETIME:
    case store::XS_DATE:
    case store::XS_TIME:
    {
      probeMap(keyType, lowerKey, upperKey);
      break;
    }

    case store::XS_DATETIME_STAMP:
    {
      probeMap(store::XS_DATETIME, lowerKey, upperKey);
      break;
    }

    // No other key is at most null; without bounds left, the other keys were
    // selected above.
    case store::JS_NULL:
    case store::XS_LAST:
    {
      break;
    }

    case store::XS_DURATION:
    case store::XS_YM_DURATION:
    case store::XS_DT_DURATION:
    {
      probeMap(store::XS_DURATION, lowerKey, upperKey);
      break;
    }

    case store::XS_ANY_URI:
    {
      probeMap(store::XS_ANY_URI, lowerKey, upperKey);

      if (idx->theMaps[store::XS_STRING])
      {
        if (haveLower)
        {
          lowerKey->getStringValue2(tmp);
          GET_FACTORY().createString(lowerAltKey, tmp);
        }

        if (haveUpper)
        {
          upperKey->getStringValue2(tmp);
          GET_FACTORY().createString(upperAltKey, tmp);
        }
        
        probeMap(store::XS_STRING, lowerAltKey, upperAltKey);
      }

      break;
    }

    case store::XS_STRING:
    case store::XS_NORMALIZED_STRING:
    case store::XS_TOKEN:
    case store::XS_NMTOKEN:
    case store::XS_LANGUAGE:
    case store::XS_NAME:
    case store::XS_NCNAME:
    case store::XS_ID:
    case store::XS_IDREF:
    case store::XS_ENTITY:
    {
      probeMap(store::XS_STRING, lowerKey, upperKey);

      if (idx->theMaps[store::XS_ANY_URI])
      {
        if (haveLower)
        {
          lowerKey->getStringValue2(tmp);
          GET_FACTORY().createAnyURI(lowerAltKey, tmp);
        }

        if (haveUpper)
        {
          upperKey->getStringValue2(tmp);
          GET_FACTORY().createAnyURI(upperAltKey, tmp);
        }

        probeMap(store::XS_ANY_URI, lowerAltKey, upperAltKey);
      }

      break;
    }

    case store::XS_DOUBLE:
    case store::XS_FLOAT:
    case store::XS_DECIMAL:
    case store::XS_INTEGER:
    case store::XS_NON_POSITIVE_INTEGER:
    case store::XS_NEGATIVE_INTEGER:
    case store::XS_NON_NEGATIVE_INTEGER:
    case store::XS_POSITIVE_INTEGER:
    case store::XS_UNSIGNED_LONG:
    case store::XS_LONG:
    case store::XS_INT:
    case store::XS_SHORT:
    case store::XS_BYTE:
    case store::XS_UNSIGNED_INT:
    case store::XS_UNSIGNED_SHORT:
    case store::XS_UNSIGNED_BYTE:
    {
      probeNumberRange(lowerKey.getp(), upperKey.getp());
      break;
    }

    case store::XS_UNTYPED_ATOMIC:
    {
      assert(theProbeKind == store::IndexCondition::BOX_GENERAL);

      theIsUntypedProbe = true;

      store::ItemHandle<UntypedAtomicItem> untypedItem = 
      (haveLower ? 
       static_cast<UntypedAtomicItem*>(lowerKey.getp()) :
       static_cast<UntypedAtomicItem*>(upperKey.getp()));

      store::Item_t altKey;

      // cast to xs:string
      if (idx->theMaps[store::XS_STRING])
      {
        untypedItem->castToString(altKey);
        probeMap(store::XS_STRING, altKey, altKey);
      }

      // cast to xs:anyURI
      if (idx->theMaps[store::XS_ANY_URI] && untypedItem->castToUri(altKey))
      {
        probeMap(store::XS_ANY_URI, altKey, altKey);
      }

      // Compared with a number, an untyped value is cast to xs:double.
      if (untypedItem->castToDouble(altKey))
      {
        probeNumberRange(altKey, altKey);
      }

      // try casting to xs:datetime
      else if (idx->theMaps[store::XS_DATETIME] && untypedItem->castToDateTime(altKey))
      {
        probeMap(store::XS_DATETIME, altKey, altKey);
      }

      // try casting to xs:date
      else if (idx->theMaps[store::XS_DATE] && untypedItem->castToDate(altKey))
      {
        probeMap(store::XS_DATE, altKey, altKey);
      }

      // try casting to xs:time
      else if (idx->theMaps[store::XS_TIME] && untypedItem->castToTime(altKey))
      {
        probeMap(store::XS_TIME, altKey, altKey);
      }

      // try casting to xs:duration
      else if (idx->theMaps[store::XS_DURATION] && untypedItem->castToDuration(altKey))
      {
        probeMap(store::XS_DURATION, altKey, altKey);
      }

      // This cast can succeed together with one of the casts above.
      if (idx->theMaps[store::XS_BOOLEAN] && untypedItem->castToBoolean(altKey))
      {
        probeMap(store::XS_BOOLEAN, altKey, altKey);
      }

      break;
    }

    default:
    {
      RAISE_ERROR_NO_LOC(zerr::ZDTY0012_INDEX_KEY_TYPE_ERROR,
      ERROR_PARAMS(theIndex->getName()->getStringValue()));
    }
    }

    if (selectsNullKeys && idx->theMaps[store::JS_NULL] != NULL)
    {
      theMapBegins.push_back(idx->theMaps[store::JS_NULL]->begin());
      theMapEnds.push_back(idx->theMaps[store::JS_NULL]->end());
    }
  }
  else
  {
    theIsFullProbe = true;

    for (csize i = 0; i < store::XS_LAST; ++i)
    {
      if (idx->theMaps[i] == NULL)
        continue;

      theMapBegins.push_back(idx->theMaps[i]->begin());
      theMapEnds.push_back(idx->theMaps[i]->end());
    }
  }
}


/******************************************************************************
  Probes the numeric maps (see GeneralIndex) with a numeric search key.
********************************************************************************/
void ProbeGeneralIndexIterator::probeNumber(const AtomicItem* key)
{
  store::SchemaTypeCode keyType = key->getTypeCode();

  theNumberKind = (keyType == store::XS_DOUBLE ? DOUBLE_NUMBER :
                   keyType == store::XS_FLOAT ? FLOAT_NUMBER :
                   EXACT_NUMBER);

  // NaN is equal to no number.
  if (key->isNaN())
    return;

  if (theIndex->isSorted())
  {
    probeMap(store::XS_LONG, key);
    probeMap(store::XS_DECIMAL, key);
    probeMap(store::XS_DOUBLE, key);
    probeMap(store::XS_FLOAT, key);
    return;
  }

  store::Item_t castItem;

  // An xs:long key that the search key is equal to but not exactly is found
  // by its image.
  if (isLongMapType(keyType))
    probeMap(store::XS_LONG, key);
  else if (key->castToLong(castItem))
    probeMap(store::XS_LONG, castItem.getp());
  else if (theNumberKind == EXACT_NUMBER)
    probeMap(store::XS_DECIMAL, key);

  if (theNumberKind == EXACT_NUMBER)
  {
    xs_double doubleValue;
    xs_float floatValue;
    promoteExact(key, doubleValue, floatValue);

    GET_FACTORY().createDouble(castItem, doubleValue);
    probeMap(store::XS_DOUBLE, castItem.getp());

    GET_FACTORY().createFloat(castItem, floatValue);
    probeMap(store::XS_FLOAT, castItem.getp());
  }
  else if (theNumberKind == DOUBLE_NUMBER)
  {
    probeMap(store::XS_DOUBLE, key);

    float floatValue;
    if (exactFloat(key->getDoubleValue().getNumber(), floatValue))
    {
      GET_FACTORY().createFloat(castItem, xs_float(floatValue));
      probeMap(store::XS_FLOAT, castItem.getp());
    }
  }
  else
  {
    GET_FACTORY().createDouble(castItem, key->getDoubleValue());
    probeMap(store::XS_DOUBLE, castItem.getp());

    probeMap(store::XS_FLOAT, key);
  }
}


/******************************************************************************
  Probes the numeric maps of a tree index with numeric bounds.
********************************************************************************/
void ProbeGeneralIndexIterator::probeNumberRange(
    const store::Item* lowerKey,
    const store::Item* upperKey)
{
  GeneralIndexCondition::RangeFlags& flags = theCondition->theRangeFlags;

  // NaN is less than and greater than no number.
  if ((flags.theHaveLowerBound && lowerKey->isNaN()) ||
      (flags.theHaveUpperBound && upperKey->isNaN()))
    return;

  probeMap(store::XS_LONG, lowerKey, upperKey);
  probeMap(store::XS_DECIMAL, lowerKey, upperKey);
  probeMap(store::XS_DOUBLE, lowerKey, upperKey);
  probeMap(store::XS_FLOAT, lowerKey, upperKey);
}


/******************************************************************************

*******************************************************************************/
void ProbeGeneralIndexIterator::probeMap(
    store::SchemaTypeCode targetMap,
    const store::Item* key)
{
  // NaN is equal to no number, although NaN keys share a map entry.
  if (key->isNaN())
    return;

  if (theIndex->isSorted())
  {
    static_cast<ProbeGeneralTreeIndexIterator*>(this)->
    probeMap(static_cast<GeneralTreeIndex*>(theIndex.getp())->theMaps[targetMap],
             key);
  }
  else
  {
    static_cast<ProbeGeneralHashIndexIterator*>(this)->
    probeMap(static_cast<GeneralHashIndex*>(theIndex.getp())->theMaps[targetMap],
             key);
  }
}


/*******************************************************************************

********************************************************************************/
void ProbeGeneralIndexIterator::probeMap(
    store::SchemaTypeCode targetMap,
    const store::Item* lowerKey,
    const store::Item* upperKey)
{
    static_cast<ProbeGeneralTreeIndexIterator*>(this)->
    probeMap(static_cast<GeneralTreeIndex*>(theIndex.getp())->theMaps[targetMap],
             lowerKey,
             upperKey);
}


/*******************************************************************************

********************************************************************************/
bool ProbeGeneralIndexIterator::haveMap(store::SchemaTypeCode targetMap) const
{
  if (theIndex->isSorted())
  {
    GeneralTreeIndex* idx = static_cast<GeneralTreeIndex*>(theIndex.getp());
    return (idx->theMaps[targetMap] != NULL);
  }
  else
  {
    GeneralHashIndex* idx = static_cast<GeneralHashIndex*>(theIndex.getp());
    return (idx->theMaps[targetMap] != NULL);
  }
}


/******************************************************************************

********************************************************************************/
void ProbeGeneralIndexIterator::open()
{
  if (theProbeKind == store::IndexCondition::POINT_VALUE ||
      theProbeKind == store::IndexCondition::POINT_GENERAL)
  {
    theResultSetsEnd = theResultSets.end();
    theResultSetsIte = theResultSets.begin();

    for (; theResultSetsIte != theResultSetsEnd; ++theResultSetsIte)
    {
      assert(*theResultSetsIte != NULL);

      theIte = (*theResultSetsIte)->begin();
      theEnd = (*theResultSetsIte)->end();
        
      break;
    }
  }
  else
  {
    theCurrentMap = 0;
    theEmptyKeyPos = 0;

    if (!theMapBegins.empty())
    {
      assert(!theMapEnds.empty());

      theMapIte = theMapBegins[theCurrentMap];

      if (theMapIte != theMapEnds[theCurrentMap])
      {
        GeneralIndexValue* resultSet = theMapIte->second;
        theIte = resultSet->begin();
        theEnd = resultSet->end();
      }
    }
  }
}


/******************************************************************************

********************************************************************************/
void ProbeGeneralIndexIterator::reset()
{
  open();
}


/******************************************************************************

********************************************************************************/
void ProbeGeneralIndexIterator::close()
{
}


/******************************************************************************

********************************************************************************/
bool ProbeGeneralIndexIterator::skipsImage(
    GeneralIndexValue::ImageKind image) const
{
  switch (image)
  {
  case GeneralIndexValue::DOUBLE_IMAGE:
    return theIsFullProbe || theNumberKind != DOUBLE_NUMBER;

  case GeneralIndexValue::FLOAT_IMAGE:
    return theIsFullProbe || theNumberKind != FLOAT_NUMBER;

  default:
    return false;
  }
}


/******************************************************************************
  TODO : need sync on result vector
********************************************************************************/
bool ProbeGeneralIndexIterator::next(store::Item_t& result)
{
  if (theProbeKind == store::IndexCondition::POINT_VALUE ||
      theProbeKind == store::IndexCondition::POINT_GENERAL)
  {
    while (theResultSetsIte != theResultSetsEnd)
    {
      while (theIte != theEnd)
      {
        if (theIsUntypedProbe && (*theIte).theUntyped)
        {
          assert(theProbeKind == store::IndexCondition::POINT_GENERAL);

          ++theIte;
          continue;
        }

        if (skipsImage((*theIte).theImage))
        {
          ++theIte;
          continue;
        }

        result = (*theIte).theNode;
        
        ++theIte;
        return true;
      }

      ++theResultSetsIte;

      if (theResultSetsIte != theResultSetsEnd)
      {
        theIte = (*theResultSetsIte)->begin();
        theEnd = (*theResultSetsIte)->end();
      }
    }
  }
  else
  {
    while (theCurrentMap < theMapBegins.size())
    {
      while (theMapIte != theMapEnds[theCurrentMap])
      {
        while (theIte != theEnd)
        {
          if ((theIsFullProbe || theIsUntypedProbe) && (*theIte).theUntyped)
          {
            ++theIte;
            continue;
          }

          if (skipsImage((*theIte).theImage))
          {
            ++theIte;
            continue;
          }

          result = (*theIte).theNode;
          
          ++theIte;
          return true;
        }

        ++theMapIte;
        if (theMapIte == theMapEnds[theCurrentMap])
          break;

        GeneralIndexValue* resultSet = theMapIte->second;
        theIte = resultSet->begin();
        theEnd = resultSet->end();
      }

      ++theCurrentMap;
      if (theCurrentMap == theMapBegins.size())
        break;

      theMapIte = theMapBegins[theCurrentMap];

      if (theMapIte != theMapEnds[theCurrentMap])
      {
        GeneralIndexValue* resultSet = theMapIte->second;
        theIte = resultSet->begin();
        theEnd = resultSet->end();
      }
    }

    if (theSelectsEmptyKeys &&
        theEmptyKeyPos < theIndex->theEmptyKeyNodes.size())
    {
      result = theIndex->theEmptyKeyNodes[theEmptyKeyPos++];
      return true;
    }
  }

  return false;
}


/******************************************************************************
 The implementation here doesn't really give anything in terms of
 performance but other implementations might be able to provide more
 efficient ones.
********************************************************************************/
void ProbeGeneralIndexIterator::count(store::Item_t& result)
{
  xs_integer res = xs_integer(0);

  open();

  store::Item_t tmp;
  while (next(tmp))
    ++res;

  close();

  GET_FACTORY().createInteger(result, res);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  ProbeHashGeneralIndexIterator                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************

********************************************************************************/
ProbeGeneralHashIndexIterator::ProbeGeneralHashIndexIterator(
    const store::Index_t& index) 
  :
  ProbeGeneralIndexIterator(index)
{
}


/******************************************************************************

********************************************************************************/
void ProbeGeneralHashIndexIterator::probeMap(
    const GeneralHashIndex::IndexMap* targetMap,
    const store::Item* key)
{
  if (targetMap == NULL)
    return;

  GeneralHashIndex::EntryIterator ite = const_cast<GeneralHashIndex::IndexMap*>
  (targetMap)->find(const_cast<store::Item*>(key));

  if (ite != targetMap->end())
  {
    theResultSets.push_back(NULL);
    theResultSets[theResultSets.size() - 1] = (*ite).second;
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  ProbeGeneralTreeIndexIterator                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************

********************************************************************************/
ProbeGeneralTreeIndexIterator::ProbeGeneralTreeIndexIterator(
    const store::Index_t& index) 
  :
  ProbeGeneralIndexIterator(index)
{
}


/******************************************************************************

********************************************************************************/
void ProbeGeneralTreeIndexIterator::probeMap(
    const GeneralTreeIndex::IndexMap* targetMap,
    const store::Item* key)
{
  if (targetMap == NULL)
    return;

  // Several xs:decimal keys can equal one xs:float value. Not equal_range,
  // which may assume one match per key in a std::map (libc++ does).
  GeneralTreeIndex::EntryIterator end = targetMap->upper_bound(key);

  for (GeneralTreeIndex::EntryIterator ite = targetMap->lower_bound(key);
       ite != end;
       ++ite)
  {
    theResultSets.push_back((*ite).second);
  }
}


/******************************************************************************
  
********************************************************************************/
void ProbeGeneralTreeIndexIterator::probeMap(
    const GeneralTreeIndex::IndexMap* map,
    const store::Item* lowerKey,
    const store::Item* upperKey)
{
  if (map == NULL)
    return;

  GeneralIndexCondition::RangeFlags& flags = theCondition->theRangeFlags;

  bool haveLower = flags.theHaveLowerBound;
  bool haveUpper = flags.theHaveUpperBound;
  bool lowerIncl = flags.theLowerBoundIncl;
  bool upperIncl = flags.theUpperBoundIncl;

  assert(theProbeKind != store::IndexCondition::BOX_GENERAL ||
         !(haveLower && haveUpper));

  // NaN is less than and greater than no number.
  if ((haveLower && lowerKey->isNaN()) || (haveUpper && upperKey->isNaN()))
    return;

  GeneralTreeIndex::EntryIterator begin;
  GeneralTreeIndex::EntryIterator end;

  if (haveLower)
  {
    begin = (lowerIncl ? map->lower_bound(lowerKey) : map->upper_bound(lowerKey));
  }
  else
  {
    // NaN keys come first in a map (see GeneralIndexCompareFunction); only a
    // probe without bounds selects them.
    begin = map->begin();
    if (haveUpper && begin != map->end() && begin->first->isNaN())
      ++begin;
  }

  if (haveUpper)
  {
    end = (upperIncl ? map->upper_bound(upperKey) : map->lower_bound(upperKey));

    // begin lies past end if its key is above the upper bound. The bounds
    // are not compared with each other: out of order as given, they can be
    // equal once promoted to the type of the keys.
    if (begin != end &&
        (begin == map->end() ||
         theIndex->theCompFunction.compare(begin->first, upperKey) > 0))
      end = begin;
  }
  else
  {
    end = map->end();
  }

  theMapBegins.push_back(begin);
  theMapEnds.push_back(end);
}



} // namespace simplestore
} // namespace zorba
/* vim:set et sw=2 ts=2: */
