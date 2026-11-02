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

// Item::mem_size() of an XML node includes its subtree, whether the element
// keeps its children inline (up to two) or on the heap, and counts the heap
// array by its capacity: a slot is not counted when it is inline. The parser
// sizes the array exactly, so the third child adds an array of 3 slots and the
// fourth one more slot.

#include <iostream>
#include <sstream>

#include <zorba/iterator.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>

using namespace zorba;

static size_t docSize( Zorba *z, int children ) {
  std::ostringstream q;
  q << "parse-xml(concat('<r>', string-join(for $i in 1 to " << children
    << " return '<i/>', ''), '</r>'))";
  XQuery_t query = z->compileQuery( q.str() );
  Iterator_t it = query->iterator();
  it->open();
  Item item;
  size_t size = 0;
  if ( it->next( item ) )
    size = item.mem_size();
  item.close();
  it->close();
  query->close();
  return size;
}

int node_mem_size( int, char*[] ) {
  void *store = StoreManager::getStore();
  Zorba *z = Zorba::getInstance( store );
  int failures = 0;
  size_t sizes[5];
  for ( int n = 0; n <= 4; ++n ) {
    sizes[n] = docSize( z, n );
    std::cout << "children " << n << ": " << sizes[n] << std::endl;
  }
  size_t const child = sizes[1] - sizes[0];
  size_t const slot = sizeof( void* );
  // One element is at least its vptr, parent and name pointers.
  if ( sizes[1] < sizes[0] || child < 3 * slot ) {
    std::cerr << "FAILED: first child adds " << child << std::endl;
    ++failures;
  }
  size_t const want[] = { child, child + 3 * slot, child + slot };
  for ( int n = 1; n <= 3; ++n ) {
    size_t const step = sizes[n + 1] - sizes[n];
    if ( step != want[n - 1] ) {
      std::cerr << "FAILED: child " << n + 1 << " adds " << step << ", want "
                << want[n - 1] << std::endl;
      ++failures;
    }
  }
  size_t const many = docSize( z, 1000 );
  std::cout << "children 1000: " << many << std::endl;
  if ( many < sizes[4] + 996 * child ) {
    std::cerr << "FAILED: 1000 children: " << many << std::endl;
    ++failures;
  }
  z->shutdown();
  StoreManager::shutdownStore( store );
  return failures ? 1 : 0;
}
/* vim:set et sw=2 ts=2: */
