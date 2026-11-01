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

#include <iostream>
#include <vector>

#include "store/api/copymode.h"
#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "store/api/iterator.h"
#include "store/naive/json_items.h"
#include "system/globalenv.h"
#include "zorbatypes/zstring.h"

using namespace std;

namespace zorba {
namespace UnitTests {

using simplestore::json::JSONObject;

static int failures;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      cerr << __FILE__ << ':' << __LINE__ << ": check failed: "         \
           << #cond << endl;                                            \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

// A string item whose zstring representation is unsharable: the non-const
// operator[] marks it so, and copies of an unsharable representation are deep
// clones rather than shared references.
static store::Item_t unsharable_string(char const *s)
{
  zstring z(s);
  (void)z[0];
  store::Item_t item;
  GENV_ITEMFACTORY->createString(item, z);
  return item;
}

static store::Item_t sharable_string(char const *s)
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

static int value_of(store::Item_t const &obj, char const *key)
{
  store::Item_t v = obj->getObjectValue(sharable_string(key));
  return v ? v->getIntValue() : -1;
}

static zstring keys_of(store::Item_t const &obj)
{
  zstring result;
  store::Iterator_t it = obj->getObjectKeys();
  it->open();
  store::Item_t key;
  while (it->next(key)) {
    if (!result.empty())
      result += ',';
    result += key->getStringValue();
  }
  it->close();
  return result;
}

static void test_keys_from_unsharable_strings()
{
  vector<store::Item_t> names, values;
  names.push_back(unsharable_string("first-key"));
  values.push_back(integer(1));
  names.push_back(unsharable_string("second-key"));
  values.push_back(integer(2));
  names.push_back(unsharable_string("third-key"));
  values.push_back(integer(3));

  store::Item_t obj;
  GENV_ITEMFACTORY->createJSONObject(obj, names, values);

  CHECK(keys_of(obj) == "first-key,second-key,third-key");
  CHECK(value_of(obj, "first-key") == 1);
  CHECK(value_of(obj, "second-key") == 2);
  CHECK(value_of(obj, "third-key") == 3);
  CHECK(value_of(obj, "fourth-key") == -1);

  JSONObject *const o = static_cast<JSONObject*>(obj.getp());

  // add(): a duplicate key is refused, a new one is appended.
  CHECK(!o->add(unsharable_string("second-key"), integer(20), false));
  CHECK(o->add(unsharable_string("fourth-key"), integer(4), false));
  CHECK(value_of(obj, "fourth-key") == 4);

  // rename(): the new name has to be findable, the old one gone.
  CHECK(o->rename(sharable_string("second-key"),
                  unsharable_string("renamed-key")));
  CHECK(value_of(obj, "second-key") == -1);
  CHECK(value_of(obj, "renamed-key") == 2);
  CHECK(!o->rename(sharable_string("first-key"),
                   unsharable_string("renamed-key")));

  // setValue() and remove() find keys stored from unsharable strings.
  store::Item_t old = o->setValue(unsharable_string("third-key"), integer(30));
  CHECK(old && old->getIntValue() == 3);
  CHECK(value_of(obj, "third-key") == 30);
  CHECK(o->remove(unsharable_string("first-key")) != NULL);
  CHECK(value_of(obj, "first-key") == -1);

  CHECK(keys_of(obj) == "renamed-key,third-key,fourth-key");

  // copy(): the copy owns its keys too.
  store::CopyMode mode;
  store::Item_t copy = obj->copy(NULL, mode);
  CHECK(keys_of(copy) == "renamed-key,third-key,fourth-key");
  CHECK(value_of(copy, "renamed-key") == 2);
  CHECK(value_of(copy, "third-key") == 30);
  CHECK(value_of(copy, "fourth-key") == 4);
}

int test_json_object(int, char*[])
{
  failures = 0;
  test_keys_from_unsharable_strings();
  if (failures)
    cerr << failures << " check(s) failed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
