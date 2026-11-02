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

// Small xs:integer items are shared (created once by the item factory, like
// the two boolean items):
// - createInteger() returns the same item for a value in the cached range
//   and a new item outside it;
// - a shared item has type xs:integer exactly, and the derived integer types
//   (xs:long ... xs:byte, xs:unsigned*, xs:nonNegativeInteger, ...) never
//   get a shared xs:integer item;
// - a shared item outlives every reference handed out, also when threads
//   take and drop references concurrently.

#include <iostream>
#include <thread>
#include <vector>

#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "system/globalenv.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/schema_types.h"

using namespace std;

namespace zorba {
namespace UnitTests {

namespace {

int failures;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      cerr << __FILE__ << ':' << __LINE__ << ": check failed: "         \
           << #cond << endl;                                            \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

#define CHECK_V(cond, v)                                                \
  do {                                                                  \
    if (!(cond)) {                                                      \
      cerr << __FILE__ << ':' << __LINE__ << ": check failed: "         \
           << #cond << " (value " << (v) << ')' << endl;                \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

// The cached range: these values must be shared, the neighbours outside it
// must not.
int const cache_min = -128;
int const cache_max = 1023;

store::Item_t integer(long long v)
{
  store::Item_t item;
  GENV_ITEMFACTORY->createInteger(item, xs_integer(v));
  return item;
}

void test_sharing()
{
  for (long long v = cache_min - 100; v <= cache_max + 100; ++v)
  {
    store::Item_t const a = integer(v);
    store::Item_t const b = integer(v);
    CHECK_V(a->getTypeCode() == store::XS_INTEGER, v);
    CHECK_V(b->getTypeCode() == store::XS_INTEGER, v);
    CHECK_V(a->getIntegerValue() == xs_integer(v), v);
    CHECK_V(b->getIntegerValue() == xs_integer(v), v);
    bool const in_range = v >= cache_min && v <= cache_max;
    CHECK_V((a.getp() == b.getp()) == in_range, v);
  }

  // Values far from the range, including the xs:long limits.
  long long const far[] = { -9223372036854775807LL - 1, -1000000, 1000000,
                            9223372036854775807LL };
  for (long long v : far)
  {
    store::Item_t const a = integer(v);
    store::Item_t const b = integer(v);
    CHECK_V(a->getTypeCode() == store::XS_INTEGER, v);
    CHECK_V(a->getIntegerValue() == xs_integer(v), v);
    CHECK_V(a.getp() != b.getp(), v);
  }
}

// Creates v as every integer type that can hold it and checks that each item
// has exactly that type and is not the shared xs:integer item.
void test_derived_types(int v)
{
  store::ItemFactory *const f = GENV_ITEMFACTORY;
  store::Item_t const shared = integer(v);

  struct created {
    store::Item_t item;
    store::SchemaTypeCode type;
  };
  vector<created> items;
  store::Item_t i;

  f->createLong(i, v);  items.push_back({ i, store::XS_LONG });
  f->createInt(i, v);   items.push_back({ i, store::XS_INT });
  f->createShort(i, static_cast<xs_short>(v));
  items.push_back({ i, store::XS_SHORT });
  if (v >= -128 && v <= 127)
  {
    f->createByte(i, static_cast<xs_byte>(v));
    items.push_back({ i, store::XS_BYTE });
  }
  if (v >= 0)
  {
    f->createUnsignedLong(i, v);  items.push_back({ i, store::XS_UNSIGNED_LONG });
    f->createUnsignedInt(i, v);   items.push_back({ i, store::XS_UNSIGNED_INT });
    f->createUnsignedShort(i, static_cast<xs_unsignedShort>(v));
    items.push_back({ i, store::XS_UNSIGNED_SHORT });
    if (v <= 255)
    {
      f->createUnsignedByte(i, static_cast<xs_unsignedByte>(v));
      items.push_back({ i, store::XS_UNSIGNED_BYTE });
    }
    f->createNonNegativeInteger(i, xs_nonNegativeInteger(v));
    items.push_back({ i, store::XS_NON_NEGATIVE_INTEGER });
  }
  if (v > 0)
  {
    f->createPositiveInteger(i, xs_positiveInteger(v));
    items.push_back({ i, store::XS_POSITIVE_INTEGER });
  }
  if (v <= 0)
  {
    f->createNonPositiveInteger(i, xs_nonPositiveInteger(v));
    items.push_back({ i, store::XS_NON_POSITIVE_INTEGER });
  }
  if (v < 0)
  {
    f->createNegativeInteger(i, xs_negativeInteger(v));
    items.push_back({ i, store::XS_NEGATIVE_INTEGER });
  }

  for (created const &c : items)
  {
    CHECK_V(c.item->getTypeCode() == c.type, v);
    CHECK_V(c.item.getp() != shared.getp(), v);
    CHECK_V(c.item->getIntegerValue() == xs_integer(v), v);
  }

  // Creating the derived items did not change the shared one.
  CHECK_V(shared->getTypeCode() == store::XS_INTEGER, v);
  CHECK_V(integer(v).getp() == shared.getp(), v);
  CHECK_V(shared->getIntegerValue() == xs_integer(v), v);
}

void test_lifetime()
{
  store::Item const *first;
  {
    store::Item_t a = integer(7);
    first = a.getp();
  }
  // Every handle is gone; the factory still holds the item.
  store::Item_t const b = integer(7);
  CHECK(b.getp() == first);
  CHECK(b->getTypeCode() == store::XS_INTEGER);
  CHECK(b->getIntegerValue() == xs_integer(7));
}

void test_threads()
{
  int const nthreads = 4;
  int const rounds = 20000;
  vector<store::Item_t> held;
  vector<long> before;
  for (int v = 0; v < 16; ++v)
  {
    held.push_back(integer(v));
    before.push_back(held.back()->getRefCount());
  }
  vector<thread> threads;
  for (int t = 0; t < nthreads; ++t)
  {
    threads.emplace_back([rounds]() {
      for (int r = 0; r < rounds; ++r)
      {
        store::Item_t item;
        GENV_ITEMFACTORY->createInteger(item, xs_integer(r % 16));
        store::Item_t copy = item;
      }
    });
  }
  for (thread &t : threads)
    t.join();

  for (int v = 0; v < 16; ++v)
  {
    store::Item_t const &a = held[v];
    CHECK_V(a->getTypeCode() == store::XS_INTEGER, v);
    CHECK_V(a->getIntegerValue() == xs_integer(v), v);
    // Every thread dropped what it took: no increment or decrement was lost.
    CHECK_V(a->getRefCount() == before[v], v);
  }
}

} // namespace

int test_small_integers(int, char*[])
{
  failures = 0;
  test_sharing();
  int const samples[] = { cache_min, -100, -1, 0, 1, 5, 127, 128, 255, 256,
                          cache_max };
  for (int v : samples)
    test_derived_types(v);
  test_lifetime();
  test_threads();
  cout << (failures ? "FAILED" : "passed") << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
