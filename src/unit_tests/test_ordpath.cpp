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
// - getRelativePosition() classifies ancestor/descendant/preceding/following;
// - ids have no fixed length: thousands of levels, built by appending, by the
//   loaders' OrdPathStack and by insertions, keep all of the above;
// - every bit pattern of the encoding decodes, at every bit offset;
// - ids that just fit inline order and compare like the longer ones.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "store/naive/ordpath.h"

using namespace std;
using zorba::simplestore::OrdPath;
using zorba::simplestore::OrdPathStack;

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

// Ids far longer than 255 bytes: a chain of first children whose every tenth
// component takes 41 bits.
void test_long_ids() {
  int const levels = 3000;
  string chain = "1";
  OrdPath appended;
  appended.setAsRoot();
  for ( int i = 1; i <= levels; ++i ) {
    int const comp = i % 10 ? 1 : 2000001;
    chain += "." + to_string( comp );
    appended.appendComp( comp );
  }
  OrdPath const deep = make( chain );
  check( dewey( deep ) == chain, "long id: round trip", "dewey" );
  check( appended == deep, "long id: appendComp", "vs compress" );
  check( deep.getLevel() == unsigned( levels + 1 ), "long id: level",
         to_string( deep.getLevel() ) );

  string const s = deep.serialize();
  check( s.size() > 2 * 255, "long id: longer than 255 bytes",
         to_string( s.size() / 2 ) );
  OrdPath back;
  back.deserialize( s );
  check( back == deep, "long id: serialize, deserialize", "" );

  OrdPath copy( deep );
  check( copy == deep, "long id: copy", "" );

  // The loaders' stack gives the same id for a chain of first children.
  string ones = "1";
  OrdPath ones_appended;
  ones_appended.setAsRoot();
  OrdPathStack ones_stack;
  ones_stack.init();
  for ( int i = 1; i <= levels; ++i ) {
    ones += ".1";
    ones_appended.appendComp( 1 );
    ones_stack.pushChild();
  }
  OrdPath from_stack;
  from_stack = ones_stack;
  check( from_stack == ones_appended, "long id: OrdPathStack", "vs appendComp" );
  check( from_stack == make( ones ), "long id: OrdPathStack", "vs compress" );
  ones_stack.popChild();
  OrdPath popped;
  popped = ones_stack;
  check( popped == make( ones.substr( 0, ones.size() - 4 ) + ".3" ),
         "long id: OrdPathStack popChild", "" );

  // Children of a long id: order, position, insertions between them.
  OrdPath const c1 = make( chain + ".1" ), c3 = make( chain + ".3" );
  check_less( deep, c1, "long id: parent before child" );
  check_less( c1, c3, "long id: siblings" );
  check( deep.getRelativePosition( c3 ) == OrdPath::DESCENDANT,
         "long id: relative position", "descendant" );
  check( c3.getRelativePosition( c1 ) == OrdPath::PRECEDING,
         "long id: relative position", "preceding" );
  check( deep.getRelativePosition2( c1 ) == OrdPath::CHILD,
         "long id: adjacent position", "child" );
  check( c1.getRelativePosition2( c3 ) == OrdPath::FOLLOWING_SIBLING,
         "long id: adjacent position", "following sibling" );

  OrdPath between, before, after;
  OrdPath::insertInto( deep, c1, c3, between );
  check_less( c1, between, "long id: insertInto" );
  check_less( between, c3, "long id: insertInto" );
  check_valid_child( deep, between, "long id: insertInto" );
  OrdPath::insertBefore( deep, c1, before );
  check_less( before, c1, "long id: insertBefore" );
  check_valid_child( deep, before, "long id: insertBefore" );
  OrdPath::insertAfter( deep, c3, after );
  check_less( c3, after, "long id: insertAfter" );
  check_valid_child( deep, after, "long id: insertAfter" );
}

// Component values at the edges of every encoding class, so that sequences
// of them put every bit pattern the decoder handles at every bit offset.
int const edge_values[] = {
  -2147483647, -1118485, -1118484, -69909, -69908, -4373, -4372, -277, -276,
  -21, -20, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 7, 8, 23, 24, 279, 280, 4375,
  4376, 69911, 69912, 1118487, 1118488, 2147483647
};

int const odd_edge_values[] = {
  -2147483647, -1118485, -69909, -4373, -277, -21, -5, -3, -1, 1, 3, 7, 23,
  279, 4375, 69911, 1118487, 2147483647
};

string join( OrdPath::DeweyID const &d ) {
  ostringstream out;
  for ( size_t i = 0; i < d.size(); ++i )
    out << ( i ? "." : "" ) << d[i];
  return out.str();
}

unsigned long odd_count( OrdPath::DeweyID const &d ) {
  unsigned long n = 0;
  for ( int32_t c : d )
    n += c % 2 != 0;
  return n;
}

// Ids whose encoding puts a given byte where the decoder reads its next byte;
// each of these bytes once decoded wrongly. The parent is the id without its
// last component.
void test_decode_table() {
  static char const *const ids[] = {
    "1.-1118484.1", "1.1.-1118485", "1.-1118485.1", "1.-2147483647",
    "1.1.-2147483647.3",      // 0000 0000
    "1.1.1.1.-1.6.3",         // 0001 1111
    "1.1.1.1.1.1.1",          // 0101 0101
    "1.1.1.3", "1.1.1.2.1",   // 0101 0110
    "1.1.1.1.3.0.-3.1",       // 1010 0100
    "1.1.1.1.3.0.6.3",        // 1010 0111
    nullptr
  };
  for ( char const *const *s = ids; *s; ++s ) {
    OrdPath::DeweyID const d = parse( *s );
    OrdPath const p = make( *s );
    check( dewey( p ) == *s, "decode table", string( *s ) + " -> " + dewey( p ) );
    check( p.getLevel() == odd_count( d ), "decode table: level", *s );

    OrdPath::DeweyID const parent_d( d.begin(), d.end() - 1 );
    OrdPath::DeweyID grand = parent_d;
    while ( !grand.empty() && grand.back() % 2 == 0 )
      grand.pop_back();
    OrdPath parent;
    parent.compress( grand );
    OrdPath after;
    OrdPath::insertAfter( parent, p, after );
    check_less( p, after, ( string( "decode table: insertAfter " ) + *s ).c_str() );
    check_valid_child( parent, after, "decode table: insertAfter" );
    // Nothing fits before the smallest component.
    if ( d.back() > -2147483647 ) {
      OrdPath before;
      OrdPath::insertBefore( parent, p, before );
      check_less( before, p,
                  ( string( "decode table: insertBefore " ) + *s ).c_str() );
      check_valid_child( parent, before, "decode table: insertBefore" );
    }
    check( parent.getRelativePosition2( p ) == OrdPath::CHILD,
           "decode table: child", *s );
  }
}

// Every id 1.a.b.c.o (o odd) made of edge values decodes to itself, and its
// level is the number of its odd components.
void test_decode_all_patterns() {
  int reported = 0;
  for ( int a : edge_values )
    for ( int b : edge_values )
      for ( int c : edge_values )
        for ( int o : odd_edge_values ) {
          OrdPath::DeweyID const d = { 1, a, b, c, o };
          OrdPath p;
          p.compress( d );
          string const want = join( d );
          int const before = failures;
          check( dewey( p ) == want, "decode", want + " -> " + dewey( p ) );
          check( p.getLevel() == odd_count( d ), "decode: level", want );
          if ( failures != before && ++reported >= 20 ) {
            cerr << "  (more decode failures not shown)" << endl;
            return;
          }
        }
}

// The relative position of y to x for ids that end with an odd component,
// from their components.
OrdPath::RelativePosition expected_position( OrdPath::DeweyID const &x,
                                             OrdPath::DeweyID const &y ) {
  size_t i = 0;
  while ( i < x.size() && i < y.size() && x[i] == y[i] )
    ++i;
  if ( i == x.size() && i == y.size() )
    return OrdPath::SELF;
  if ( i == x.size() )
    return OrdPath::DESCENDANT;
  if ( i == y.size() )
    return OrdPath::ANCESTOR;
  return x[i] < y[i] ? OrdPath::FOLLOWING : OrdPath::PRECEDING;
}

// Ids around the 63 bits that fit inline: chains of 1s, each 2 bits, with
// one or two more components. Order, equality and relative position follow
// from the components, whichever ids are inline and whichever are not.
void test_inline_boundary() {
  int reported = 0;
  for ( int m = 24; m <= 34; ++m ) {
    vector<OrdPath::DeweyID> ids;
    OrdPath::DeweyID chain( 1, 1 );
    for ( int i = 0; i < m; ++i ) {
      chain.push_back( 1 );
      ids.push_back( chain );
    }
    for ( int a : edge_values )
      for ( int o : odd_edge_values ) {
        OrdPath::DeweyID d = chain;
        d.push_back( a );
        d.push_back( o );
        ids.push_back( d );
      }
    for ( int o : odd_edge_values ) {
      OrdPath::DeweyID d = chain;
      d.push_back( o );
      ids.push_back( d );
    }

    vector<OrdPath> paths( ids.size() );
    for ( size_t i = 0; i < ids.size(); ++i )
      paths[i].compress( ids[i] );

    for ( size_t i = 0; i < ids.size(); ++i )
      for ( size_t j = 0; j < ids.size(); ++j ) {
        OrdPath::RelativePosition const pos =
          expected_position( ids[i], ids[j] );
        bool const less = pos == OrdPath::DESCENDANT ||
                          pos == OrdPath::FOLLOWING;
        bool const ok_less = ( paths[i] < paths[j] ) == less;
        bool const ok_greater = ( paths[j] > paths[i] ) == less;
        bool const ok_equal = ( paths[i] == paths[j] ) == ( pos == OrdPath::SELF );
        bool const ok_position = paths[i].getRelativePosition( paths[j] ) == pos;
        if ( ok_less && ok_greater && ok_equal && ok_position )
          continue;
        string const what = join( ids[i] ) + " vs " + join( ids[j] );
        check( ok_less, "inline boundary: <", what );
        check( ok_greater, "inline boundary: >", what );
        check( ok_equal, "inline boundary: ==", what );
        check( ok_position, "inline boundary: relative position", what );
        if ( ++reported >= 20 ) {
          cerr << "  (more inline boundary failures not shown)" << endl;
          return;
        }
      }
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
  test_long_ids();
  test_decode_table();
  test_decode_all_patterns();
  test_inline_boundary();
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
