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

// An iterator over a non-lazy temp sequence (store/naive/simple_temp_seq.h)
// that exists while the sequence changes: append() adds items (and may move
// the sequence's storage), init() replaces its contents and purge() releases
// them. The iterator returns the positions it was given when it was
// initialized (the whole sequence as it was then, or a range of it), up to
// the current end of the sequence; open() and reset() go back to the first
// of these positions. It never returns an item the sequence released.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "store/api/iterator.h"
#include "store/api/iterator_factory.h"
#include "store/api/store.h"
#include "store/api/temp_seq.h"
#include "system/globalenv.h"
#include "zorbatypes/integer.h"

using namespace std;

namespace zorba {
namespace {

int failures;

// A temp sequence of the integers first, first+1, ..., first+count-1.
store::TempSeq_t integers( long first, long count ) {
  vector<store::Item_t> items( count );
  for ( long i = 0; i < count; ++i )
    GENV_ITEMFACTORY->createInteger( items[i], xs_integer( first + i ) );
  return GENV_STORE.createTempSeq( items );
}

// An open iterator over the integers first, ..., first+count-1.
store::Iterator_t source( long first, long count ) {
  store::Iterator_t it( integers( first, count )->getIterator() );
  it->open();
  return it;
}

// What remains in the iterator, separated by spaces.
string drain( store::Iterator *it ) {
  ostringstream os;
  store::Item_t item;
  bool first = true;
  while ( it->next( item ) ) {
    os << ( first ? "" : " " ) << item->getIntegerValue().toString().c_str();
    first = false;
  }
  return os.str();
}

void expect( char const *what, string const &got, string const &want ) {
  if ( got != want ) {
    ++failures;
    cerr << "FAILED: " << what << ": got \"" << got << "\", want \"" << want
         << '"' << endl;
  }
}

// The items appended after the iterator was initialized are not part of it;
// the storage of the sequence moves (many items are appended to two).
void test_append_reset() {
  store::TempSeq_t seq( integers( 1, 2 ) );
  store::Iterator_t it( seq->getIterator() );
  it->open();
  expect( "append: before", drain( it.getp() ), "1 2" );
  seq->append( source( 3, 1000 ) );
  it->reset();
  expect( "append: reset", drain( it.getp() ), "1 2" );
  it->open();
  expect( "append: open", drain( it.getp() ), "1 2" );
  store::Iterator_t all( seq->getIterator() );
  all->open();
  expect( "append: a new iterator", drain( all.getp() ).substr( 0, 8 ),
          "1 2 3 4 " );
}

// The same with an iterator over a range of the sequence.
void test_append_range() {
  store::TempSeq_t seq( integers( 1, 4 ) );
  store::TempSeqIterator_t it(
    GENV_ITERATOR_FACTORY->createTempSeqIterator( false ) );
  it->init( seq, xs_integer( 2 ), xs_integer( 3 ) );
  it->open();
  seq->append( source( 5, 1000 ) );
  it->reset();
  expect( "range: reset", drain( it.getp() ), "2 3" );
  it->open();
  expect( "range: open", drain( it.getp() ), "2 3" );
}

// init() replaces the contents: the iterator returns its positions in the new
// contents, up to their end.
void test_init() {
  store::TempSeq_t seq( integers( 1, 3 ) );
  store::Iterator_t it( seq->getIterator() );
  it->open();
  seq->init( source( 10, 1000 ) );
  it->reset();
  expect( "init: more items", drain( it.getp() ), "10 11 12" );
  seq->init( source( 20, 2 ) );
  it->reset();
  expect( "init: fewer items", drain( it.getp() ), "20 21" );
}

// After purge() the iterator returns nothing: the items were released.
void test_purge() {
  store::TempSeq_t seq( integers( 1, 3 ) );
  store::Iterator_t it( seq->getIterator() );
  it->open();
  store::Item_t first;
  it->next( first );
  seq->purge();
  expect( "purge: next", drain( it.getp() ), "" );
  it->reset();
  expect( "purge: reset", drain( it.getp() ), "" );
}

} // namespace

namespace UnitTests {

int test_temp_seq_iterator( int, char*[] ) {
  failures = 0;
  Zorba::getInstance( StoreManager::getStore() );
  test_append_reset();
  test_append_range();
  test_init();
  test_purge();
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
