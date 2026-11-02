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

#include "types/casting.h"
#include "types/typeops.h"
#include "runtime/core/gflwor/comp_function.h"

#include <iostream>

namespace zorba {
namespace flwor {


bool SortTupleCmp::operator()(const SortTuple& t1, const SortTuple& t2) const
{
  // A sort makes no consumeNext calls. Raising here is like the type errors
  // that compare() raises: see SortTuple for what that leaves in the table.
  thePlanState->chargeWork(1, theLocation);

  ZORBA_ASSERT(t1.theKeyValues.size() == t2.theKeyValues.size());
  ZORBA_ASSERT(t1.theKeyValues.size() == theOrderSpecs->size());

  std::vector<store::Item*>::const_iterator t1iter = t1.theKeyValues.begin();
  std::vector<store::Item*>::const_iterator t1end = t1.theKeyValues.end();
  std::vector<store::Item*>::const_iterator t2iter = t2.theKeyValues.begin();

  std::vector<OrderSpec>::const_iterator orderSpecIter = theOrderSpecs->begin();

  while (t1iter != t1end)
  {
    long cmp = compare(*t1iter,
                        *t2iter,
                        orderSpecIter->theDescending,
                        orderSpecIter->theEmptyLeast,
                        orderSpecIter->theNativeCompare,
                        orderSpecIter->theCollator);
    if (cmp > 0)
    {
      return false;
    }
    else if (cmp < 0)
    {
      return true;
    }
    
    ++t1iter;
    ++t2iter;
    ++orderSpecIter;
  }
  return false;
}

          
long SortTupleCmp::compare(
      store::Item* item1,
      store::Item* item2,
      bool desc,
      bool emptyLeast,
      bool nativeCompare,
      XQPCollator* collator) const
{
  if (item1 == NULL)
  {
    if (item2 == NULL)
      return 0;
    else
      return descAsc(emptyLeast ? -1 : 1, desc);
  }
  else if (item1->isNaN())
  {
    if (item2 == NULL)
      return descAsc(emptyLeast ? 1 : -1, desc);
    else if (item2->isNaN())
      return 0;
    else
      return descAsc(emptyLeast ? -1 : 1, desc);
  }
  else if (item2 == NULL || item2->isNaN())
  {
    return descAsc(emptyLeast ? 1 : -1, desc);
  }
  else
  {
    long result;

    // danm: both valueCompare (x, NaN) and valueCompare (NaN, x) return 2.
    // That's why empty_item is needed.
    if (nativeCompare)
    {
      result = item1->compare(item2, theTimezone, collator);
    }
    else
    {
      store::Item_t ls1(item1);
      store::Item_t ls2(item2);
      result = CompareIterator::valueCompare(theLocation,
                                             ls1,
                                             ls2,
                                             theTypeManager,
                                             theTimezone,
                                             collator);
    }

    return descAsc(result , desc);
  }
}
  
/*******************************************************************************
  XQuery 3.0 3.10.8: "All the non-empty orderspec values must be convertible to
  a common type by subtype substitution and/or type promotion. The ordering is
  performed in the least common type that has a gt operator."

  The comparison function promotes each pair of keys on its own, which is not
  the same: as xs:double, 2^63-1, the xs:decimal 2^63 and the xs:double 2^63 are
  all equal, while the first two compared by themselves are not, and that is
  not even a consistent order. So promote the keys of a column that mixes
  numeric types with no subtype relation to the common type first.
********************************************************************************/
void promoteNumericSortKeys(
    std::vector<SortTuple>& sortTable,
    const std::vector<OrderSpec>& orderSpecs,
    const TypeManager* tm,
    const QueryLoc& loc)
{
  csize numSpecs = orderSpecs.size();

  for (csize i = 0; i < numSpecs; ++i)
  {
    // A native column has one static type: there is nothing to promote.
    if (orderSpecs[i].theNativeCompare)
      continue;

    bool hasDouble = false;
    bool hasFloat = false;
    bool hasDecimal = false;

    for (SortTuple& tuple : sortTable)
    {
      const store::Item* key = tuple.theKeyValues[i];
      if (key == NULL || !key->isAtomic())
        continue;

      store::SchemaTypeCode type = key->getTypeCode();
      if (TypeOps::is_subtype(type, store::XS_DOUBLE))
        hasDouble = true;
      else if (TypeOps::is_subtype(type, store::XS_FLOAT))
        hasFloat = true;
      else if (TypeOps::is_subtype(type, store::XS_DECIMAL))
        hasDecimal = true;
    }

    store::SchemaTypeCode target;
    if (hasDouble && (hasFloat || hasDecimal))
      target = store::XS_DOUBLE;
    else if (hasFloat && hasDecimal)
      target = store::XS_FLOAT;
    else
      continue;

    for (SortTuple& tuple : sortTable)
    {
      store::Item*& key = tuple.theKeyValues[i];
      if (key == NULL || !key->isAtomic())
        continue;

      store::SchemaTypeCode type = key->getTypeCode();
      if (!TypeOps::is_numeric(type) || TypeOps::is_subtype(type, target))
        continue;

      store::Item_t item(key);
      store::Item_t promoted;
      if (GenericCast::promote(promoted, item, target, NULL, tm, loc))
      {
        key->removeReference();
        key = promoted.release();
      }
    }
  }
}

} // namespace flwor
} // namespace zorba

/*
* Local variables:
* mode: c++
* End:
*/
/* vim:set et sw=2 ts=2: */
