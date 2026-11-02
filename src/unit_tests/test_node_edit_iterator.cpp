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
// collection module's edit function replaces that node: edit swaps the contents
// of the node with the new content (UpdEditInCollection, InternalNode::swap).
// The iterator goes on with the nodes it started on, while an iterator opened
// after the edit sees the new content. The first node returned stays referenced,
// which keeps the replaced nodes alive. The cases cross the sizes at which
// element nodes change how they store their nodes, in both directions.

#include <iostream>
#include <sstream>
#include <string>

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
  char const *content;
  char const *first;    // what the iterator returns before the edit
  char const *rest;     // what it returns after the edit
  char const *all;      // what it returns after a reset()
  char const *fresh;    // what a new iterator returns after the edit
};

void test_case( Case const &c, int n ) {
  ostringstream names;
  names << "local:c" << n;
  string const qname( "xs:QName(\"" + names.str() + "\")" );

  ostringstream label;
  label << ( c.kind == CHILDREN ? "children" :
             c.kind == REVERSE ? "reverse" : "attributes" )
        << ' ' << c.before << " -> " << c.content;
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

      run( "dml:edit(dml:collection(" + qname + "), " + c.content + ");\n()" );

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
  { CHILDREN,   "<r><a/><b/></r>",        "<n><x/><y/></n>",  "a", "b", "a b", "x y" },
  { REVERSE,    "<r><a/><b/></r>",        "<n><x/><y/></n>",  "b", "a", "b a", "y x" },
  { ATTRIBUTES, "<r a='1' b='2'/>",       "<n x='1' y='2'/>", "a", "b", "",    "x y" },
  { CHILDREN,   "<r><a/></r>",            "<n><x/></n>",      "a", "",  "a",   "x" },
  { REVERSE,    "<r><a/></r>",            "<n><x/></n>",      "a", "",  "a",   "x" },
  { CHILDREN,   "<r p='1'><a/></r>",      "<n><x/><y/></n>",  "a", "",  "a",   "x y" },
  { CHILDREN,   "<r><a/><b/></r>",        "<n><x/></n>",      "a", "b", "a b", "x" },
  { REVERSE,    "<r><a/><b/></r>",        "<n><x/></n>",      "b", "a", "b a", "x" },
  { ATTRIBUTES, "<r a='1' b='2'/>",       "<n x='1'/>",       "a", "b", "",    "x" },
  { CHILDREN,   "<r><a/><b/></r>",        "<n/>",             "a", "b", "a b", "" },
  { REVERSE,    "<r><a/><b/></r>",        "<n/>",             "b", "a", "b a", "" },
  // inline to heap
  { CHILDREN,   "<r><a/></r>",            "<n><x/><y/><z/></n>", "a", "",  "a",   "x y z" },
  { CHILDREN,   "<r><a/><b/></r>",        "<n><x/><y/><z/></n>", "a", "b", "a b", "x y z" },
  { REVERSE,    "<r><a/><b/></r>",        "<n><x/><y/><z/></n>", "b", "a", "b a", "z y x" },
  { ATTRIBUTES, "<r a='1' b='2'/>",       "<n x='1' y='2' z='3'/>", "a", "b", "", "x y z" },
  // heap to inline
  { CHILDREN,   "<r><a/><b/><c/></r>",    "<n><x/></n>",      "a", "b c", "a b c", "x" },
  { REVERSE,    "<r><a/><b/><c/></r>",    "<n><x/></n>",      "c", "b a", "c b a", "x" },
  { ATTRIBUTES, "<r a='1' b='2' c='3'/>", "<n x='1'/>",       "a", "b c", "",      "x" },
  { CHILDREN,   "<r p='1'><a/><b/></r>",  "<n><x/><y/></n>",  "a", "b",   "a b",   "x y" },
  // heap to heap
  { CHILDREN,   "<r><a/><b/><c/></r>",    "<n><x/><y/><z/><w/></n>", "a", "b c", "a b c", "x y z w" },
};

} // namespace

namespace UnitTests {

int test_node_edit_iterator( int, char*[] ) {
  failures = 0;
  int n = 0;
  for ( Case const &c : cases )
    test_case( c, ++n );
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
