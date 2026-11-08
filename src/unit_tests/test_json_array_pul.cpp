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

// Array update primitives added to one pending update list. Queries build one
// list per updating expression and merge them; these tests cover the merging
// done while primitives are added to a single list.

#include <iostream>
#include <string>
#include <vector>

#include <zorba/zorba_exception.h>

#include "compiler/parser/query_loc.h"
#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "store/api/pul.h"
#include "system/globalenv.h"
#include "zorbatypes/integer.h"

using namespace std;

namespace zorba {
namespace UnitTests {

static int failures;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      cerr << __FILE__ << ':' << __LINE__ << ": check failed: "         \
           << #cond << endl;                                            \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

static store::Item_t integer(int i)
{
  store::Item_t item;
  GENV_ITEMFACTORY->createInteger(item, xs_integer(i));
  return item;
}

// The array [1, 2, ..., size].
static store::Item_t array_of(int size)
{
  vector<store::Item_t> members;
  for (int i = 1; i <= size; ++i)
    members.push_back(integer(i));
  store::Item_t array;
  GENV_ITEMFACTORY->createJSONArray(array, members);
  return array;
}

// The members of ARRAY, comma-separated.
static string members_of(store::Item_t const &array)
{
  string result;
  xs_integer size = array->getArraySize();
  for (int i = 1; xs_integer(i) <= size; ++i) {
    if (!result.empty())
      result += ',';
    result += array->getArrayValue(xs_integer(i))->getStringValue().c_str();
  }
  return result;
}

static store::PUL_t new_pul()
{
  store::PUL_t pul(GENV_ITEMFACTORY->createPendingUpdateList());
  return pul;
}

static void insert_at(store::PUL_t &pul, store::Item_t array, int pos, int value)
{
  store::Item_t position = integer(pos);
  vector<store::Item_t> members(1, integer(value));
  pul->addJSONArrayInsert(&QueryLoc::null, array, position, members);
}

static void append_to(store::PUL_t &pul, store::Item_t array, int value)
{
  vector<store::Item_t> members(1, integer(value));
  pul->addJSONArrayAppend(&QueryLoc::null, array, members);
}

static void delete_at(store::PUL_t &pul, store::Item_t array, int pos)
{
  store::Item_t position = integer(pos);
  pul->addJSONArrayDelete(&QueryLoc::null, array, position);
}

static void replace_at(store::PUL_t &pul, store::Item_t array, int pos, int value)
{
  store::Item_t position = integer(pos);
  store::Item_t newValue = integer(value);
  pul->addJSONArrayReplaceValue(&QueryLoc::null, array, position, newValue);
}

// Returns the local name of the error raised by F on a fresh list for
// [1, 2, 3], or "" if none.
template<typename F>
static string error_of(F f)
{
  store::Item_t array = array_of(3);
  store::PUL_t pul = new_pul();
  try {
    f(pul, array);
  }
  catch (ZorbaException const &e) {
    return e.diagnostic().qname().localname();
  }
  return "";
}

static void test_append_after_insert()
{
  store::Item_t array = array_of(3);
  store::PUL_t pul = new_pul();
  insert_at(pul, array, 2, 9);
  append_to(pul, array, 7);
  append_to(pul, array, 8);
  pul->applyUpdates(false);
  CHECK(members_of(array) == "1,9,2,3,7,8");
}

static void test_delete_and_replace_last()
{
  store::Item_t array = array_of(3);
  store::PUL_t pul = new_pul();
  replace_at(pul, array, 3, 4);
  delete_at(pul, array, 3);
  pul->applyUpdates(false);
  CHECK(members_of(array) == "1,2");

  array = array_of(3);
  pul = new_pul();
  delete_at(pul, array, 3);
  replace_at(pul, array, 3, 4);
  pul->applyUpdates(false);
  CHECK(members_of(array) == "1,2");
}

static void test_delete_and_replace_before_others()
{
  store::Item_t array = array_of(4);
  store::PUL_t pul = new_pul();
  delete_at(pul, array, 2);
  insert_at(pul, array, 1, 9);
  replace_at(pul, array, 3, 5);
  replace_at(pul, array, 2, 6);
  pul->applyUpdates(false);
  CHECK(members_of(array) == "9,1,5,4");
}

static void two_replaces_and_delete(store::PUL_t &pul, store::Item_t array)
{
  replace_at(pul, array, 2, 4);
  delete_at(pul, array, 2);
  replace_at(pul, array, 2, 5);
}

static void delete_and_two_replaces(store::PUL_t &pul, store::Item_t array)
{
  delete_at(pul, array, 2);
  replace_at(pul, array, 2, 4);
  replace_at(pul, array, 2, 5);
}

static void test_two_replaces_with_delete()
{
  CHECK(error_of(two_replaces_and_delete) == "JNUP0009");
  CHECK(error_of(delete_and_two_replaces) == "JNUP0009");
}

int test_json_array_pul(int, char*[])
{
  failures = 0;
  test_append_after_insert();
  test_delete_and_replace_last();
  test_delete_and_replace_before_others();
  test_two_replaces_with_delete();
  if (failures)
    cerr << failures << " check(s) failed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
