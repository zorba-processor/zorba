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
// keeps its children inline (up to two) or on the heap.

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
  size_t previous = docSize( z, 0 );
  std::cout << "children 0: " << previous << std::endl;
  // One element is at least its vptr, parent and name pointers.
  size_t const min_per_child = 3 * sizeof( void* );
  for ( int n : { 1, 2, 3, 4, 1000 } ) {
    size_t const size = docSize( z, n );
    std::cout << "children " << n << ": " << size << std::endl;
    if ( size < previous + min_per_child ) {
      std::cerr << "FAILED: " << n << " children: mem_size " << size
                << " after " << previous << std::endl;
      ++failures;
    }
    previous = size;
  }
  z->shutdown();
  StoreManager::shutdownStore( store );
  return failures ? 1 : 0;
}
/* vim:set et sw=2 ts=2: */
