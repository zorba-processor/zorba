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

// OrdPath (store/naive/ordpath.h) defines document order and the labels given
// to nodes inserted by update expressions. Checked here:
// - compress() followed by decompression (via show()) returns the same dewey id;
// - the byte-wise order of compressed ids is document order;
// - insertBefore/insertAfter/insertInto produce ids strictly between the
//   neighbours, also when applied repeatedly;
// - getRelativePosition() classifies ancestor/descendant/preceding/following.

#include "stdafx.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "store/naive/ordpath.h"

using namespace std;
using zorba::simplestore::OrdPath;

namespace {

int failures;

void check( bool ok, char const *what, string const &detail ) {
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << " (" << detail << ')' << endl;
  }
}

OrdPath::DeweyID parse( string const &s ) {
  OrdPath::DeweyID d;
  istringstream in( s );
  string comp;
  while ( getline( in, comp, '.' ) )
    d.push_back( atoi( comp.c_str() ) );
  return d;
}

OrdPath make( string const &s ) {
  OrdPath p;
  p.compress( parse( s ) );
  return p;
}

// show() is "<hex bytes> <dewey id>"; return the dewey id part.
string dewey( OrdPath const &p ) {
  string const s( p.show().c_str() );
  return s.substr( s.rfind( ' ' ) + 1 );
}

string name( OrdPath const &p ) {
  return string( p.show().c_str() );
}

void check_less( OrdPath const &a, OrdPath const &b, char const *what ) {
  string const d = name( a ) + " < " + name( b );
  check( a < b, what, d );
  check( !( b < a ), what, d + " (reversed)" );
  check( b > a, what, d + " (operator>)" );
  check( !( a == b ), what, d + " (operator==)" );
}

void test_round_trip() {
  static char const *const ids[] = {
    "1", "1.1", "1.3", "1.5.7", "1.2.1", "1.4.2.1", "1.-1", "1.-3.1",
    "1.31.33", "1.33.1", "1.1000001", "1.3215639", "1.1.1.1.1.1.1.1.1.1.1.1",
    // several components packed into the last byte of the encoding
    "1.2.0.0.0.1", "1.2.0.0.-1", "1.0.0.0.0.0.1", "1.2.0.0.0.0.0.0.0.1",
    nullptr
  };
  for ( char const *const *s = ids; *s; ++s ) {
    OrdPath const p = make( *s );
    check( p.isValid(), "round trip: valid", *s );
    check( dewey( p ) == *s, "round trip", string( *s ) + " -> " + dewey( p ) );
    OrdPath copy( p );
    check( copy == p, "round trip: copy", *s );
    OrdPath assigned;
    assigned = p;
    check( dewey( assigned ) == *s, "round trip: assignment", *s );
  }

  OrdPath root;
  root.setAsRoot();
  check( root.isRoot(), "root", "isRoot" );
  check( dewey( root ) == "1", "root", dewey( root ) );
  check( root == make( "1" ), "root", "setAsRoot vs compress(1)" );
}

void test_ordering() {
  // Document order: ancestors first, then children left to right; even
  // components ("carets") sit between the odd siblings they separate.
  static char const *const in_order[] = {
    "1", "1.-3", "1.-1", "1.1", "1.1.1", "1.1.3", "1.1.3.1", "1.1.5",
    "1.2.1", "1.2.3", "1.3", "1.3.1", "1.5", "1.5.1", "1.7", "1.31", "1.33",
    "1.33.1", "1.1000001", "1.1000003", "1.3215639",
    nullptr
  };
  vector<OrdPath> paths;
  for ( char const *const *s = in_order; *s; ++s )
    paths.push_back( make( *s ) );

  for ( size_t i = 0; i < paths.size(); ++i ) {
    check( paths[i] == paths[i], "ordering: reflexive", name( paths[i] ) );
    check( !( paths[i] < paths[i] ), "ordering: irreflexive", name( paths[i] ) );
    for ( size_t j = i + 1; j < paths.size(); ++j )
      check_less( paths[i], paths[j], "ordering" );
  }
}

// An id produced by an insertion must decode to a dewey id that compresses
// back to the same bytes, and must be a child of the given parent.
void check_valid_child( OrdPath const &parent, OrdPath const &p,
                        char const *what ) {
  string const d = dewey( p );
  check( make( d ) == p, what, "round trip: " + name( p ) );
  check( parent.getRelativePosition( p ) == OrdPath::DESCENDANT, what,
         "descendant of " + name( parent ) + ": " + name( p ) );
}

enum Side { ALTERNATE, KEEP_LEFT, KEEP_RIGHT };

// Insert repeatedly into the gap between two siblings. ALTERNATE narrows the
// gap from both sides; KEEP_LEFT always inserts right after the left sibling,
// KEEP_RIGHT always right before the right sibling.
void test_repeated_insert_into( char const *parent_id, char const *left_id,
                                char const *right_id ) {
  static int const rounds = 200;
  OrdPath const parent = make( parent_id );
  for ( Side side : { ALTERNATE, KEEP_LEFT, KEEP_RIGHT } ) {
    string const what = string( "repeated insertInto below " ) + parent_id +
      ( side == ALTERNATE ? " (alternate)" :
        side == KEEP_LEFT ? " (keep left)" : " (keep right)" );
    OrdPath left = make( left_id ), right = make( right_id );
    for ( int i = 0; i < rounds; ++i ) {
      OrdPath m;
      OrdPath::insertInto( parent, left, right, m );
      int const before = failures;
      check_less( left, m, what.c_str() );
      check_less( m, right, what.c_str() );
      check_valid_child( parent, m, what.c_str() );
      if ( failures != before ) {
        cerr << "  at round " << i << endl;
        break;
      }
      if ( side == KEEP_LEFT || ( side == ALTERNATE && i % 2 == 0 ) )
        right = m;
      else
        left = m;
    }
  }
}

void test_insert() {
  OrdPath const parent = make( "1" );
  OrdPath const a = make( "1.1" );
  OrdPath const b = make( "1.3" );

  OrdPath mid;
  OrdPath::insertInto( parent, a, b, mid );
  check_less( a, mid, "insertInto: after left sibling" );
  check_less( mid, b, "insertInto: before right sibling" );
  check_less( parent, mid, "insertInto: after parent" );
  check( parent.getRelativePosition( mid ) == OrdPath::DESCENDANT,
         "insertInto: stays a child of the parent", name( mid ) );

  OrdPath before;
  OrdPath::insertBefore( parent, a, before );
  check_less( parent, before, "insertBefore: after parent" );
  check_less( before, a, "insertBefore: before sibling" );

  OrdPath after;
  OrdPath::insertAfter( parent, b, after );
  check_less( b, after, "insertAfter: after sibling" );
  check( parent.getRelativePosition( after ) == OrdPath::DESCENDANT,
         "insertAfter: stays a child of the parent", name( after ) );

  // Repeated insertion into the same gap.
  test_repeated_insert_into( "1", "1.1", "1.3" );
  test_repeated_insert_into( "1.3", "1.3.1", "1.3.3" );
  test_repeated_insert_into( "1.3.5.7", "1.3.5.7.1", "1.3.5.7.3" );
  test_repeated_insert_into( "1.2.1", "1.2.1.-1", "1.2.1.1" );

  // Repeated insertBefore/insertAfter at the ends.
  OrdPath first = a, last = b;
  for ( int i = 0; i < 30; ++i ) {
    OrdPath f, l;
    OrdPath::insertBefore( parent, first, f );
    OrdPath::insertAfter( parent, last, l );
    check_less( f, first, "repeated insertBefore" );
    check_less( last, l, "repeated insertAfter" );
    first = f;
    last = l;
  }
  check_less( parent, first, "repeated insertBefore: after parent" );

  // Below a non-root parent.
  OrdPath const p2 = make( "1.3" );
  OrdPath const c1 = make( "1.3.1" );
  OrdPath const c2 = make( "1.3.3" );
  OrdPath c;
  OrdPath::insertInto( p2, c1, c2, c );
  check_less( c1, c, "insertInto below 1.3" );
  check_less( c, c2, "insertInto below 1.3" );
  check( p2.getRelativePosition( c ) == OrdPath::DESCENDANT,
         "insertInto below 1.3: descendant of parent", name( c ) );
}

void test_relative_position() {
  struct Case { char const *self, *other; OrdPath::RelativePosition pos; };
  static Case const cases[] = {
    { "1.3",   "1.3",     OrdPath::SELF },
    { "1.3",   "1.3.1",   OrdPath::DESCENDANT },
    { "1.3",   "1.3.5.1", OrdPath::DESCENDANT },
    { "1.3.1", "1.3",     OrdPath::ANCESTOR },
    { "1.3.5", "1",       OrdPath::ANCESTOR },
    { "1.3",   "1.5",     OrdPath::FOLLOWING },
    { "1.3",   "1.5.1",   OrdPath::FOLLOWING },
    { "1.3.1", "1.5",     OrdPath::FOLLOWING },
    { "1.5",   "1.3",     OrdPath::PRECEDING },
    { "1.5",   "1.3.1",   OrdPath::PRECEDING },
    { "1.3.3", "1.3.1",   OrdPath::PRECEDING },
    { "1.5.1", "1.1.1",   OrdPath::PRECEDING },
  };
  for ( Case const &c : cases ) {
    OrdPath const self = make( c.self ), other = make( c.other );
    check( self.getRelativePosition( other ) == c.pos,
           "relative position", string( c.self ) + " vs " + c.other );
  }
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_ordpath( int, char*[] ) {
  failures = 0;
  test_round_trip();
  test_ordering();
  test_insert();
  test_relative_position();
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
