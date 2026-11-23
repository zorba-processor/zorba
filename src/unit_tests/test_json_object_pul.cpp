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

// Object inserts merged into one pending update list, while they are added and
// when two lists are merged: a duplicate name (JNUP0005) leaves the insert
// already in the list as it was.

#include <iostream>
#include <string>
#include <vector>

#include <zorba/zorba_exception.h>

#include "compiler/parser/query_loc.h"
#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "store/api/iterator.h"
#include "store/api/pul.h"
#include "system/globalenv.h"
#include "zorbatypes/zstring.h"

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

static store::Item_t string_item(char const *s)
{
  zstring z(s);
  store::Item_t item;
  GENV_ITEMFACTORY->createString(item, z);
  return item;
}

static store::Item_t integer(int i)
{
  store::Item_t item;
  GENV_ITEMFACTORY->createInt(item, i);
  return item;
}

// The object { "z" : 0 }.
static store::Item_t new_object()
{
  vector<store::Item_t> names(1, string_item("z"));
  vector<store::Item_t> values(1, integer(0));
  store::Item_t object;
  GENV_ITEMFACTORY->createJSONObject(object, names, values);
  return object;
}

// The pairs of OBJECT as k=v, comma-separated.
static string pairs_of(store::Item_t const &object)
{
  string result;
  store::Iterator_t it = object->getObjectKeys();
  it->open();
  store::Item_t key;
  while (it->next(key)) {
    if (!result.empty())
      result += ',';
    result += key->getStringValue().c_str();
    result += '=';
    result += object->getObjectValue(key)->getStringValue().c_str();
  }
  it->close();
  return result;
}

// Adds to PUL the insert of the pairs NAMES[i] : VALUES[i] into OBJECT.
static void insert_into(store::PUL_t &pul, store::Item_t object,
                        vector<char const*> const &names,
                        vector<int> const &values)
{
  vector<store::Item_t> nameItems, valueItems;
  for (csize i = 0; i < names.size(); ++i) {
    nameItems.push_back(string_item(names[i]));
    valueItems.push_back(integer(values[i]));
  }
  pul->addJSONObjectInsert(&QueryLoc::null, object, nameItems, valueItems);
}

static string error_name(ZorbaException const &e)
{
  return e.diagnostic().qname().localname();
}

// A duplicate after a new name: neither is added to the insert in the list.
static void test_add_duplicate_name()
{
  store::Item_t object = new_object();
  store::PUL_t pul(GENV_ITEMFACTORY->createPendingUpdateList());
  insert_into(pul, object, { "a" }, { 1 });
  string error;
  try {
    insert_into(pul, object, { "b", "a" }, { 2, 3 });
  }
  catch (ZorbaException const &e) {
    error = error_name(e);
  }
  CHECK(error == "JNUP0005");
  pul->applyUpdates(false);
  CHECK(pairs_of(object) == "z=0,a=1");
}

// The same when two lists are merged. The list merged into is not used after a
// failed merge; applied anyway, it fails as a whole.
static void test_merge_duplicate_name()
{
  store::Item_t object = new_object();
  store::PUL_t pul(GENV_ITEMFACTORY->createPendingUpdateList());
  store::PUL_t other(GENV_ITEMFACTORY->createPendingUpdateList());
  insert_into(pul, object, { "a" }, { 1 });
  insert_into(other, object, { "b", "a" }, { 2, 3 });
  string error;
  try {
    pul->mergeUpdates(other.getp());
  }
  catch (ZorbaException const &e) {
    error = error_name(e);
  }
  CHECK(error == "JNUP0005");
  error.clear();
  try {
    pul->applyUpdates(false);
  }
  catch (ZorbaException const &e) {
    error = error_name(e);
  }
  CHECK(!error.empty());
  CHECK(pairs_of(object) == "z=0");
}

int test_json_object_pul(int, char*[])
{
  failures = 0;
  test_add_duplicate_name();
  test_merge_duplicate_name();
  if (failures)
    cerr << failures << " check(s) failed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
