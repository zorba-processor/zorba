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
//
// PtrVector (store/naive/ptr_vector.h) holds the attributes and children of
// element and document nodes, the first two inline: its contents must behave
// like a std::vector across the inline/heap boundary in both directions.

#include <algorithm>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

#include "store/naive/node_items.h"
#include "store/naive/ptr_vector.h"

using namespace std;
using namespace zorba::simplestore;

namespace {

int failures;

typedef PtrVector<int> Vec;

int cells[16];

// The contents as indexes into cells, e.g. "0 1 5".
string show( Vec const &v ) {
  ostringstream os;
  for ( Vec::const_iterator i = v.begin(); i != v.end(); ++i )
    os << ( i == v.begin() ? "" : " " ) << ( *i ? *i - cells : -1 );
  return os.str();
}

string show_reverse( Vec const &v ) {
  ostringstream os;
  for ( Vec::const_reverse_iterator i = v.rbegin(); i != v.rend(); ++i )
    os << ( i == v.rbegin() ? "" : " " ) << ( *i ? *i - cells : -1 );
  return os.str();
}

void expect( Vec const &v, string const &want, char const *what ) {
  string const got( show( v ) );
  bool const ok = got == want && v.size() == static_cast<size_t>(
    want.empty() ? 0 : std::count( want.begin(), want.end(), ' ' ) + 1 ) &&
    v.empty() == want.empty();
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << ": got \"" << got << "\", want \""
         << want << '"' << endl;
  }
}

void check_true( bool ok, char const *what ) {
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << endl;
  }
}

void test_vector() {
  Vec v;
  expect( v, "", "empty" );
  check_true( v.capacity() == Vec::INLINE_CAPACITY && v.heap_size() == 0,
              "empty: inline" );

  v.push_back( &cells[0] );
  expect( v, "0", "push one" );
  v.push_back( &cells[1] );
  expect( v, "0 1", "push two" );
  check_true( v.heap_size() == 0, "two: still inline" );
  check_true( show_reverse( v ) == "1 0", "two: reverse" );

  // Third element: to the heap, with std::vector's growth (2 + max(2, 1)).
  v.insert( v.begin() + 1, &cells[2] );
  expect( v, "0 2 1", "insert in the middle, spill to heap" );
  check_true( v.capacity() == 4 && v.heap_size() == 4 * sizeof(int*),
              "spill: capacity 4" );
  check_true( v[0] == &cells[0] && v.front() == &cells[0] &&
              v.back() == &cells[1], "spill: indexing" );

  v.insert( v.end(), &cells[3] );
  v.insert( v.begin(), &cells[4] );
  expect( v, "4 0 2 1 3", "insert at both ends" );
  check_true( show_reverse( v ) == "3 1 2 0 4", "heap: reverse" );
  check_true( v.rend() - v.rbegin() == 5, "heap: reverse distance" );

  v.erase( v.begin() + 2 );
  expect( v, "4 0 1 3", "erase in the middle" );
  v.erase( v.begin(), v.begin() + 2 );
  expect( v, "1 3", "erase a range" );

  // Back inline when shrunk to two or fewer.
  v.shrink_to_fit();
  expect( v, "1 3", "shrink to fit" );
  check_true( v.heap_size() == 0 && v.capacity() == Vec::INLINE_CAPACITY,
              "shrink to fit: inline again" );

  // resize() adds NULLs; shrinking keeps the capacity.
  v.resize( 4 );
  expect( v, "1 3 -1 -1", "resize up" );
  v[2] = &cells[5];
  v[3] = &cells[6];
  v.resize( 3 );
  expect( v, "1 3 5", "resize down" );
  check_true( v.capacity() == 4, "resize down keeps capacity" );
  v.shrink_to_fit();
  check_true( v.capacity() == 3, "shrink to fit on the heap" );
  expect( v, "1 3 5", "shrink to fit on the heap keeps contents" );

  // Like the loader: element with two attributes, then five children.
  Vec loaded;
  loaded.resize( 2 );
  check_true( loaded.heap_size() == 0, "loader: attributes inline" );
  loaded[0] = &cells[0];
  loaded[1] = &cells[1];
  loaded.resize( 7 );
  check_true( loaded.capacity() == 7, "loader: exact heap size" );
  for ( int i = 2; i < 7; ++i )
    loaded[i] = &cells[i];
  expect( loaded, "0 1 2 3 4 5 6", "loader: contents" );

  // Range insert, as UpdReplaceElemContent's undo does.
  Vec r;
  r.push_back( &cells[9] );
  vector<int*> src;
  src.push_back( &cells[7] );
  src.push_back( &cells[8] );
  r.insert( r.begin() + 1, src.begin(), src.end() );
  expect( r, "9 7 8", "insert a range" );

  // swap: inline <-> heap, both directions.
  Vec a, b;
  a.push_back( &cells[10] );
  for ( int i = 11; i < 15; ++i )
    b.push_back( &cells[i] );
  a.swap( b );
  expect( a, "11 12 13 14", "swap: heap into inline" );
  expect( b, "10", "swap: inline into heap" );
  check_true( b.heap_size() == 0 && a.heap_size() > 0, "swap: storage moved" );
  a.swap( b );
  expect( a, "10", "swap back: inline" );
  expect( b, "11 12 13 14", "swap back: heap" );

  b.clear();
  expect( b, "", "clear" );
  b.push_back( &cells[15] );
  expect( b, "15", "push after clear" );
}

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
#ifndef ZORBA_CONTAINER_CHECKS
  // ZORBA_CONTAINER_CHECKS (test builds only) adds a generation number to
  // PtrVector.
  CHECK_SIZE( ElementNode, 88 );
#endif
  CHECK_SIZE( AttributeNode, 64 );
#ifndef ZORBA_DEBUG_STRING
  // ZORBA_DEBUG_STRING (on for Debug and RelWithDebInfo) enlarges zstring.
  CHECK_SIZE( TextNode, 48 );
#endif
#ifndef ZORBA_CONTAINER_CHECKS
  CHECK_SIZE( InternalNode::NodeVector, 24 );
#endif
}

void ok( bool cond, char const *what ) {
  if ( !cond ) {
    ++failures;
    cerr << "FAILED: " << what << endl;
  }
}

// A ContentReader whose copy throws its first `fails` times, as when memory
// runs out while an iterator copies the contents of a node or array.
class TestReader : public ContentReader {
public:
  int fails = 0;
  int copies = 0;

  explicit TestReader( void const *key ) { startReading( key ); }
  ~TestReader() { stopReading(); }

  bool reading() const { return isReading(); }

protected:
  void copyContent() {
    if ( fails > 0 ) {
      --fails;
      throw std::bad_alloc();
    }
    ++copies;
  }
};

// A change whose copy fails changes nothing and leaves the failing reader
// registered; the readers that copied before it are unregistered once.
void test_reader_failure() {
  Vec v;
  v.push_back( cells + 0 );
  TestReader failing( &v );
  TestReader first( &v );   // registered last, notified first
  failing.fails = 1;

  bool threw = false;
  try {
    v.push_back( cells + 1 );
  }
  catch ( std::bad_alloc const& ) {
    threw = true;
  }
  ok( threw, "a failing copy leaves push_back with the exception" );
  expect( v, "0", "contents after a failing copy" );
  ok( failing.reading(), "the failing reader is still registered" );
  ok( !first.reading() && first.copies == 1,
      "the reader notified before the failure copied once" );

  v.push_back( cells + 1 );
  expect( v, "0 1", "contents after the copy succeeds" );
  ok( !failing.reading() && failing.copies == 1,
      "the failing reader copied once and left the list" );
  ok( first.copies == 1, "the other reader is not notified again" );

  TestReader again( &v );
  v.erase( v.begin() );
  ok( !again.reading() && again.copies == 1, "erase notifies the reader" );
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_node_packing( int, char*[] ) {
  failures = 0;
  test_sizes();
  test_vector();
  test_reader_failure();
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
