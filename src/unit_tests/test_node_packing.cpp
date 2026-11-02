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

// Size of the XML node classes of the simple store (store/naive/node_items.h)
// on 64-bit targets. Every parsed element is one ElementNode; with glibc
// malloc a request of n bytes takes max(32, round16(n + 8)) bytes, so 88 bytes
// is the largest ElementNode that fits a 96-byte chunk (bench/store).

#include <iostream>

#include "store/naive/node_items.h"

using namespace std;
using namespace zorba::simplestore;

namespace {

int failures;

void check( bool ok, char const *what, size_t actual, size_t limit ) {
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << " is " << actual << " bytes, limit "
         << limit << endl;
  }
}

#define CHECK_SIZE( T, LIMIT ) \
  check( sizeof( T ) <= (LIMIT), "sizeof(" #T ")", sizeof( T ), (LIMIT) )

void test_sizes() {
  if ( sizeof( void* ) != 8 )
    return;
  CHECK_SIZE( ElementNode, 88 );
  CHECK_SIZE( AttributeNode, 64 );
  CHECK_SIZE( TextNode, 48 );
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_node_packing( int, char*[] ) {
  failures = 0;
  test_sizes();
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
