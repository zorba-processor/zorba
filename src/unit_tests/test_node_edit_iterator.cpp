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

// An iterator over the children or attributes of a node stays open while the
// node is updated: the collection module's edit function replaces the node,
// which swaps its contents with the new content (UpdEditInCollection,
// InternalNode::swap), or an update inserts or deletes its children or
// attributes. The iterator goes on with the nodes the node had when it
// started, while an iterator opened after the update sees the new content.
// Every case runs twice: with the first node returned still referenced by the
// caller and with no reference at all, where only the iterator can keep the
// replaced or deleted nodes alive. The cases cross the sizes at which element
// nodes change how they store their nodes, in both directions.
//
// The test also prints, without checking them, the time of one iterator
// start and close and of one reference to a node, taken in the same process.

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <zorba/iterator.h>
#include <zorba/store_manager.h>
#include <zorba/xquery_exception.h>
#include <zorba/zorba.h>

#include "api/unmarshaller.h"
#include "store/api/item.h"
#include "store/api/iterator.h"
#include "store/api/iterator_factory.h"
#include "system/globalenv.h"
#include "zorbatypes/zstring.h"

using namespace std;

namespace zorba {
namespace {

int failures;

enum Kind { CHILDREN, REVERSE, ATTRIBUTES };

char const *const prolog =
  "import module namespace ddl = "
  "\"http://zorba.io/modules/store/dynamic/collections/ddl\";\n"
  "import module namespace dml = "
  "\"http://zorba.io/modules/store/dynamic/collections/dml\";\n";

// Runs a query to its end and returns its first item, if any.
Item run( string const &body ) {
  Zorba *const z = Zorba::getInstance( StoreManager::getStore() );
  XQuery_t q = z->compileQuery( prolog + body );
  Iterator_t it = q->iterator();
  it->open();
  Item first, i;
  if ( it->next( first ) )
    while ( it->next( i ) )
      ;
  i = Item();
  it->close();
  q->close();
  return first;
}

store::Iterator_t open_iterator( Kind kind, store::Item *node ) {
  store::Iterator_t it;
  if ( kind == CHILDREN )
    it = node->getChildren();
  else if ( kind == ATTRIBUTES )
    it = node->getAttributes();
  else {
    store::ChildrenReverseIterator *const r =
      GENV_ITERATOR_FACTORY->createChildrenReverseIterator();
    it = store::Iterator_t( r );
    r->init( const_cast<store::Item const*>( node ) );
  }
  it->open();
  return it;
}

// The local names of what remains in the iterator, separated by spaces.
string drain( store::Iterator_t const &it ) {
  ostringstream os;
  store::Item_t node;
  bool first = true;
  while ( it->next( node ) ) {
    os << ( first ? "" : " " )
       << node->getNodeName()->getLocalName().c_str();
    first = false;
  }
  return os.str();
}

void expect( string const &name, char const *what, string const &got,
             string const &want ) {
  if ( got != want ) {
    ++failures;
    cerr << "FAILED: " << name << ": " << what << ": got \"" << got
         << "\", want \"" << want << '"' << endl;
  }
}

struct Case {
  Kind kind;
  char const *before;
  char const *update;   // an update of $n, the node
  char const *first;    // what the iterator returns before the edit
  char const *rest;     // what it returns after the edit
  char const *all;      // what it returns after a reset()
  char const *fresh;    // what a new iterator returns after the edit
};

void test_case( Case const &c, int n, bool hold_first ) {
  ostringstream names;
  names << "local:c" << n;
  string const qname( "xs:QName(\"" + names.str() + "\")" );

  ostringstream label;
  label << ( c.kind == CHILDREN ? "children" :
             c.kind == REVERSE ? "reverse" : "attributes" )
        << ' ' << c.before << ": " << c.update
        << ( hold_first ? "" : " (no reference)" );
  string const name( label.str() );

  try {
    run( "ddl:create(" + qname + ");\ndml:insert-last(" + qname + ", "
         + c.before + ");\n()" );

    store::Item_t node(
      Unmarshaller::getInternalItem( run( "dml:collection(" + qname + ")" ) ) );
    {
      store::Iterator_t it( open_iterator( c.kind, node.getp() ) );
      store::Item_t first;
      if ( !it->next( first ) ) {
        ++failures;
        cerr << "FAILED: " << name << ": no first node" << endl;
        return;
      }
      expect( name, "first node", first->getNodeName()->getLocalName().c_str(),
              c.first );
      if ( !hold_first )
        first = NULL;

      run( "let $n := dml:collection(" + qname + ") return " + c.update
           + ";\n()" );

      expect( name, "rest of the open iterator", drain( it ), c.rest );
      if ( c.kind != ATTRIBUTES ) {
        it->reset();
        expect( name, "open iterator after reset", drain( it ), c.all );
      }
      it->close();

      store::Iterator_t fresh( open_iterator( c.kind, node.getp() ) );
      expect( name, "new iterator", drain( fresh ), c.fresh );
      fresh->close();
    }
    node = NULL;
    run( "dml:truncate(" + qname + ");\nddl:delete(" + qname + ");\n()" );
  }
  catch ( ZorbaException const &e ) {
    ++failures;
    cerr << "FAILED: " << name << ": " << e.what() << endl;
  }
}

// The cases that swap two inline lists of the same size come first: they end
// with a wrong result instead of a wild pointer when the iterator reads
// the storage of the node it holds.
Case const cases[] = {
  // inline to inline
  { CHILDREN,   "<r><a/><b/></r>",        "dml:edit($n, <n><x/><y/></n>)",  "a", "b", "a b", "x y" },
  { REVERSE,    "<r><a/><b/></r>",        "dml:edit($n, <n><x/><y/></n>)",  "b", "a", "b a", "y x" },
  { ATTRIBUTES, "<r a='1' b='2'/>",       "dml:edit($n, <n x='1' y='2'/>)", "a", "b", "",    "x y" },
  { CHILDREN,   "<r><a/></r>",            "dml:edit($n, <n><x/></n>)",      "a", "",  "a",   "x" },
  { REVERSE,    "<r><a/></r>",            "dml:edit($n, <n><x/></n>)",      "a", "",  "a",   "x" },
  { CHILDREN,   "<r p='1'><a/></r>",      "dml:edit($n, <n><x/><y/></n>)",  "a", "",  "a",   "x y" },
  { CHILDREN,   "<r><a/><b/></r>",        "dml:edit($n, <n><x/></n>)",      "a", "b", "a b", "x" },
  { REVERSE,    "<r><a/><b/></r>",        "dml:edit($n, <n><x/></n>)",      "b", "a", "b a", "x" },
  { ATTRIBUTES, "<r a='1' b='2'/>",       "dml:edit($n, <n x='1'/>)",       "a", "b", "",    "x" },
  { CHILDREN,   "<r><a/><b/></r>",        "dml:edit($n, <n/>)",             "a", "b", "a b", "" },
  { REVERSE,    "<r><a/><b/></r>",        "dml:edit($n, <n/>)",             "b", "a", "b a", "" },
  // inline to heap
  { CHILDREN,   "<r><a/></r>",            "dml:edit($n, <n><x/><y/><z/></n>)", "a", "",  "a",   "x y z" },
  { CHILDREN,   "<r><a/><b/></r>",        "dml:edit($n, <n><x/><y/><z/></n>)", "a", "b", "a b", "x y z" },
  { REVERSE,    "<r><a/><b/></r>",        "dml:edit($n, <n><x/><y/><z/></n>)", "b", "a", "b a", "z y x" },
  { ATTRIBUTES, "<r a='1' b='2'/>",       "dml:edit($n, <n x='1' y='2' z='3'/>)", "a", "b", "", "x y z" },
  // heap to inline
  { CHILDREN,   "<r><a/><b/><c/></r>",    "dml:edit($n, <n><x/></n>)",      "a", "b c", "a b c", "x" },
  { REVERSE,    "<r><a/><b/><c/></r>",    "dml:edit($n, <n><x/></n>)",      "c", "b a", "c b a", "x" },
  { ATTRIBUTES, "<r a='1' b='2' c='3'/>", "dml:edit($n, <n x='1'/>)",       "a", "b c", "",      "x" },
  { CHILDREN,   "<r p='1'><a/><b/></r>",  "dml:edit($n, <n><x/><y/></n>)",  "a", "b",   "a b",   "x y" },
  // heap to heap
  { CHILDREN,   "<r><a/><b/><c/></r>",    "dml:edit($n, <n><x/><y/><z/><w/></n>)", "a", "b c", "a b c", "x y z w" },

  // Updates that insert or delete children or attributes of the node.
  { CHILDREN,   "<r><a/><b/></r>",        "insert node <x/> as first into $n", "a", "b",   "a b",   "x a b" },
  { CHILDREN,   "<r><a/><b/><c/></r>",    "insert node <x/> as first into $n", "a", "b c", "a b c", "x a b c" },
  { CHILDREN,   "<r><a/><b/><c/></r>",    "insert node <x/> as last into $n",  "a", "b c", "a b c", "a b c x" },
  { CHILDREN,   "<r><a/><b/><c/></r>",    "insert node (<x/>, <y/>, <z/>, <w/>) after $n/a",
                                          "a", "b c", "a b c", "a x y z w b c" },
  { CHILDREN,   "<r><a/><b/><c/></r>",    "delete node $n/b",                  "a", "b c", "a b c", "a c" },
  { CHILDREN,   "<r><a/><b/><c/></r>",    "delete node $n/*",                  "a", "b c", "a b c", "" },
  { CHILDREN,   "<r><a/><b/><c/></r>",    "replace node $n/b with <x/>",       "a", "b c", "a b c", "a x c" },
  { CHILDREN,   "<r p='1' q='2'><a/><b/><c/></r>", "delete node $n/@p",        "a", "b c", "a b c", "a b c" },
  { CHILDREN,   "<r p='1'><a/><b/><c/></r>", "insert node attribute q {'2'} into $n",
                                          "a", "b c", "a b c", "a b c" },
  { REVERSE,    "<r><a/><b/><c/></r>",    "insert node <x/> as last into $n",  "c", "b a", "c b a", "x c b a" },
  { REVERSE,    "<r><a/><b/><c/></r>",    "delete node $n/b",                  "c", "b a", "c b a", "c a" },
  { ATTRIBUTES, "<r a='1' b='2' c='3'/>", "insert node attribute x {'4'} into $n", "a", "b c", "", "a b c x" },
  { ATTRIBUTES, "<r a='1' b='2' c='3'/>", "delete node $n/@b",                 "a", "b c", "",      "a c" },
  { ATTRIBUTES, "<r a='1' b='2' c='3'/>", "insert node <x/> as first into $n", "a", "b c", "",      "a b c" },
};

// Median of 5, in ns per call of f.
template<class F>
double time_ns( int calls, F const &f ) {
  vector<double> runs;
  for ( int r = 0; r < 5; ++r ) {
    chrono::steady_clock::time_point const t0 = chrono::steady_clock::now();
    for ( int i = 0; i < calls; ++i )
      f();
    chrono::duration<double,nano> const d = chrono::steady_clock::now() - t0;
    runs.push_back( d.count() / calls );
  }
  sort( runs.begin(), runs.end() );
  return runs[2];
}

void print_costs() {
  store::Item_t node( Unmarshaller::getInternalItem(
    run( "dml:collection(xs:QName(\"local:cost\"))" ) ) );
  int const calls = 1000000;
  store::Item const *const raw = node.getp();
  double const reference = time_ns( calls, [raw]() {
    store::Item_t held( const_cast<store::Item*>( raw ) );
  } );
  store::ChildrenIterator *const r =
    GENV_ITERATOR_FACTORY->createChildrenIterator();
  store::Iterator_t keep( r );
  double const iterator = time_ns( calls, [raw, r]() {
    r->init( raw );
    r->close();
  } );
  cout << "cost: node reference " << reference << " ns, iterator init+close "
       << iterator << " ns" << endl;
}

} // namespace

namespace UnitTests {

int test_node_edit_iterator( int, char*[] ) {
  failures = 0;
  int n = 0;
  run( "ddl:create(xs:QName(\"local:cost\"));\n"
       "dml:insert-last(xs:QName(\"local:cost\"), <r><a/></r>);\n()" );
  print_costs();
  for ( Case const &c : cases )
    test_case( c, ++n, true );
  for ( Case const &c : cases )
    test_case( c, ++n, false );
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
