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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <zorba/diagnostic_list.h>
#include <zorba/zorba_exception.h>

#include "runtime/json/json_loader.h"
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
using simplestore::json::SimpleJSONObject;

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
  // String value: parsed numbers are xs:integer, constructed ones xs:int.
  return v ? std::atoi(v->getStringValue().c_str()) : -1;
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

///////////////////////////////////////////////////////////////////////////////
// Objects of one parse share their key items; objects of every size keep
// key order and duplicate-key detection.

// Parses SOURCE with the loader jn:parse-json uses; returns the first item.
static store::Item_t parse(char const *source)
{
  istringstream iss(source);
  json::loader loader(iss);
  store::Item_t result;
  if (!loader.next(&result))
    return NULL;
  return result;
}

// Returns the error raised while parsing SOURCE, or "" if none.
static std::string parse_error(char const *source)
{
  try {
    parse(source);
  }
  catch (ZorbaException const &e) {
    return e.diagnostic().qname().localname();
  }
  return "";
}

static store::Item_t member(store::Item_t const &array, int i)
{
  return array->getArrayValue(xs_integer(i));
}

// The key item of OBJ whose string is KEY, or NULL.
static store::Item* key_item(store::Item_t const &obj, char const *key)
{
  store::Item* result = NULL;
  store::Iterator_t it = obj->getObjectKeys();
  it->open();
  store::Item_t k;
  while (it->next(k)) {
    if (k->getStringValue() == key)
      result = k.getp();
  }
  it->close();
  return result;
}

static void test_parse_shares_keys()
{
  store::Item_t a = parse(
    "[ {\"id\":1, \"name\":2}, {\"id\":3, \"name\":4}, {\"name\":5, \"id\":6} ]"
  );
  CHECK(a && a->isArray());
  if (!a)
    return;
  store::Item_t o1 = member(a, 1), o2 = member(a, 2), o3 = member(a, 3);

  // One key item per distinct key string, shared by the objects.
  CHECK(key_item(o1, "id") != NULL);
  CHECK(key_item(o1, "id") == key_item(o2, "id"));
  CHECK(key_item(o1, "id") == key_item(o3, "id"));
  CHECK(key_item(o1, "name") == key_item(o2, "name"));
  CHECK(key_item(o1, "name") == key_item(o3, "name"));

  // Each object keeps its own key order.
  CHECK(keys_of(o1) == "id,name");
  CHECK(keys_of(o2) == "id,name");
  CHECK(keys_of(o3) == "name,id");
  CHECK(value_of(o3, "id") == 6);
  CHECK(value_of(o3, "name") == 5);
}

static void test_shared_keys_mutation()
{
  store::Item_t a = parse(
    "[ {\"id\":1, \"name\":2}, {\"id\":3, \"name\":4}, {\"id\":5, \"name\":6} ]"
  );
  CHECK(a && a->isArray());
  if (!a)
    return;
  store::Item_t o1 = member(a, 1), o2 = member(a, 2), o3 = member(a, 3);
  JSONObject *const j1 = static_cast<JSONObject*>(o1.getp());
  JSONObject *const j2 = static_cast<JSONObject*>(o2.getp());
  JSONObject *const j3 = static_cast<JSONObject*>(o3.getp());

  // rename in one object does not rename the key in the others.
  CHECK(j1->rename(sharable_string("id"), sharable_string("key")));
  CHECK(keys_of(o1) == "key,name");
  CHECK(value_of(o1, "key") == 1);
  CHECK(value_of(o1, "id") == -1);
  CHECK(keys_of(o2) == "id,name");
  CHECK(value_of(o2, "id") == 3);
  CHECK(keys_of(o3) == "id,name");
  CHECK(value_of(o3, "id") == 5);

  // remove from one object leaves the others' pairs.
  CHECK(j2->remove(sharable_string("name")) != NULL);
  CHECK(keys_of(o2) == "id");
  CHECK(value_of(o1, "name") == 2);
  CHECK(value_of(o3, "name") == 6);

  // add and setValue act on one object only.
  CHECK(j3->add(sharable_string("extra"), integer(7), false));
  CHECK(j3->setValue(sharable_string("id"), integer(50)) != NULL);
  CHECK(keys_of(o3) == "id,name,extra");
  CHECK(value_of(o3, "id") == 50);
  CHECK(value_of(o2, "id") == 3);
  CHECK(value_of(o1, "extra") == -1);
  CHECK(value_of(o2, "extra") == -1);

  // A key item the parse shared is still usable by a later object.
  CHECK(j2->add(sharable_string("name"), integer(40), false));
  CHECK(keys_of(o2) == "id,name");
  CHECK(value_of(o2, "name") == 40);
  CHECK(!j2->add(sharable_string("name"), integer(41), false));
}

static void test_parse_duplicate_keys()
{
  CHECK(parse_error("{\"a\":1, \"b\":2, \"a\":3}") == "JNDY0003");
  // The duplicate comes after the key was seen in an earlier object.
  CHECK(parse_error("[ {\"a\":1}, {\"b\":1, \"a\":2, \"a\":3} ]") == "JNDY0003");
  CHECK(parse_error("[ {\"a\":1}, {\"a\":2} ]") == "");

  // Large objects as well.
  std::string big("{");
  for (int i = 0; i < 100; ++i)
    big += "\"k" + std::to_string(i) + "\":" + std::to_string(i) + ",";
  std::string dup(big + "\"k50\":0}");
  big += "\"last\":0}";
  CHECK(parse_error(big.c_str()) == "");
  CHECK(parse_error(dup.c_str()) == "JNDY0003");

  store::Item_t o = parse(big.c_str());
  CHECK(o && o->isObject());
  if (o) {
    CHECK(value_of(o, "k0") == 0);
    CHECK(value_of(o, "k99") == 99);
    CHECK(value_of(o, "last") == 0);
    CHECK(value_of(o, "k100") == -1);
  }
}

static std::string key_name(int i)
{
  return "key" + std::to_string(i);
}

static std::string expected_keys(int from, int to, int skip = -1)
{
  std::string s;
  for (int i = from; i < to; ++i) {
    if (i == skip)
      continue;
    if (!s.empty())
      s += ',';
    s += key_name(i);
  }
  return s;
}

// Builds objects of N pairs for N on both sides of any small-object
// threshold and checks lookups, order, duplicates and updates.
static void test_object_sizes()
{
  int const sizes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17,
                        24, 31, 32, 33, 48, 64, 65, 100, 200 };
  for (int n : sizes) {
    int const before = failures;
    vector<store::Item_t> names, values;
    for (int i = 0; i < n; ++i) {
      names.push_back(sharable_string(key_name(i).c_str()));
      values.push_back(integer(i));
    }
    store::Item_t obj;
    GENV_ITEMFACTORY->createJSONObject(obj, names, values);
    JSONObject *const o = static_cast<JSONObject*>(obj.getp());

    CHECK(obj->getNumObjectPairs() == xs_integer(n));
    CHECK(std::string(keys_of(obj).c_str()) == expected_keys(0, n));
    for (int i = 0; i < n; ++i) {
      CHECK(value_of(obj, key_name(i).c_str()) == i);
      // Duplicates are refused through the same item and an equal one.
      CHECK(!o->add(names[i], integer(-2), false));
      CHECK(!o->add(unsharable_string(key_name(i).c_str()), integer(-2),
                    false));
    }
    CHECK(value_of(obj, key_name(n).c_str()) == -1);
    CHECK(value_of(obj, "") == -1);

    // createJSONObject() raises JNDY0003 for a duplicate at any position.
    vector<store::Item_t> dnames(names), dvalues(values);
    dnames.push_back(unsharable_string(key_name(n / 2).c_str()));
    dvalues.push_back(integer(0));
    std::string err;
    try {
      store::Item_t dobj;
      GENV_ITEMFACTORY->createJSONObject(dobj, dnames, dvalues);
    }
    catch (ZorbaException const &e) {
      err = e.diagnostic().qname().localname();
    }
    CHECK(err == "JNDY0003");

    // Growing by one keeps everything findable.
    CHECK(o->add(sharable_string(key_name(n).c_str()), integer(n), false));
    CHECK(std::string(keys_of(obj).c_str()) == expected_keys(0, n + 1));
    for (int i = 0; i <= n; ++i)
      CHECK(value_of(obj, key_name(i).c_str()) == i);

    // remove() from the middle keeps the order of the rest.
    int const mid = n / 2;
    CHECK(o->remove(sharable_string(key_name(mid).c_str())) != NULL);
    CHECK(o->remove(sharable_string(key_name(mid).c_str())) == NULL);
    CHECK(std::string(keys_of(obj).c_str()) == expected_keys(0, n + 1, mid));
    for (int i = 0; i <= n; ++i)
      CHECK(value_of(obj, key_name(i).c_str()) == (i == mid ? -1 : i));

    // rename() keeps the position; renaming onto an existing key fails.
    CHECK(o->rename(sharable_string(key_name(n).c_str()),
                    sharable_string("renamed")));
    CHECK(value_of(obj, key_name(n).c_str()) == -1);
    CHECK(value_of(obj, "renamed") == n);
    if (n > 1)
      CHECK(!o->rename(sharable_string("renamed"),
                       sharable_string(key_name(0).c_str())));
    CHECK(o->add(sharable_string(key_name(mid).c_str()), integer(mid), false));
    std::string expected = expected_keys(0, n, mid);
    expected += std::string(expected.empty() ? "" : ",") + "renamed," +
                key_name(mid);
    CHECK(std::string(keys_of(obj).c_str()) == expected);

    // setValue() replaces in place.
    store::Item_t old = o->setValue(sharable_string("renamed"), integer(-7));
    CHECK(old && std::atoi(old->getStringValue().c_str()) == n);
    CHECK(value_of(obj, "renamed") == -7);

    if (failures != before)
      cerr << "  (object of " << n << " pairs)" << endl;
  }
}

// A small object carries no hash map of its own: at most one pointer beyond
// its pair vector.
static void test_object_footprint()
{
  CHECK(sizeof(SimpleJSONObject) <=
        sizeof(JSONObject) +
        sizeof(std::vector<std::pair<store::Item*, store::Item*> >) +
        sizeof(void*));
}

int test_json_object(int, char*[])
{
  failures = 0;
  test_keys_from_unsharable_strings();
  test_parse_shares_keys();
  test_shared_keys_mutation();
  test_parse_duplicate_keys();
  test_object_sizes();
  test_object_footprint();
  if (failures)
    cerr << failures << " check(s) failed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
