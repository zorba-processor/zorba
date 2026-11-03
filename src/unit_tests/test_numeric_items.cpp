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

// Item-level comparison of numeric items of different types, as used by the
// hash and tree indexes (and so by maps) and by group by with a fast comparison.
// F&O 3.0 4.2: "If the two operands are not of the same type, subtype
// substitution and numeric type promotion are used to obtain two operands of
// the same type."; op:numeric-equal: "Returns true if and only if the value of
// $arg1 is equal to the value of $arg2." XPath 3.0 B.1: xs:decimal promotes to
// xs:float or xs:double by casting, xs:float to xs:double.
// So 5 is not equal to 5.5e0, in either direction; truncating it is an error.
//
// ascii::itoa is checked at the limits of its argument too (a signed overflow in
// its digit index: not visible in the output of a build without UBSan).

#include <climits>
#include <cstring>
#include <iostream>
#include <vector>

#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "system/globalenv.h"
#include "util/ascii_util.h"
#include "zorbatypes/decimal.h"
#include "zorbatypes/float.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/schema_types.h"

using namespace std;
using namespace zorba;

namespace {

int failures;

void fail( char const *what, store::Item const *a, store::Item const *b ) {
  ++failures;
  cerr << "FAILED: " << what << ": " << a->show() << ", " << b->show() << endl;
}

// Runs one comparison, so that an exception is reported as a failure of this
// pair instead of ending the test.
template<typename Fn>
bool holds( Fn const &fn ) {
  try {
    return fn();
  }
  catch ( std::exception const &e ) {
    cerr << "exception: " << e.what() << endl;
    return false;
  }
}

void check_equal( store::Item const *a, store::Item const *b ) {
  if ( !holds( [&]{ return a->equals( b ); } ) )
    fail( "not equal", a, b );
  if ( !holds( [&]{ return b->equals( a ); } ) )
    fail( "not equal", b, a );
  if ( !holds( [&]{ return a->compare( b ) == 0; } ) )
    fail( "compare != 0", a, b );
  if ( !holds( [&]{ return b->compare( a ) == 0; } ) )
    fail( "compare != 0", b, a );
}

void check_less( store::Item const *a, store::Item const *b ) {
  if ( !holds( [&]{ return !a->equals( b ); } ) )
    fail( "equal", a, b );
  if ( !holds( [&]{ return !b->equals( a ); } ) )
    fail( "equal", b, a );
  if ( !holds( [&]{ return a->compare( b ) < 0; } ) )
    fail( "compare >= 0", a, b );
  if ( !holds( [&]{ return b->compare( a ) > 0; } ) )
    fail( "compare <= 0", b, a );
}

// Every integer type holding 5, and xs:decimal.
vector<store::Item_t> fives() {
  store::ItemFactory *const f = GENV_ITEMFACTORY;
  vector<store::Item_t> v;
  store::Item_t i;
  f->createInteger( i, xs_integer( 5 ) );                       v.push_back( i );
  f->createNonNegativeInteger( i, xs_nonNegativeInteger( 5 ) ); v.push_back( i );
  f->createPositiveInteger( i, xs_positiveInteger( 5 ) );       v.push_back( i );
  f->createLong( i, 5 );                                        v.push_back( i );
  f->createInt( i, 5 );                                         v.push_back( i );
  f->createShort( i, 5 );                                       v.push_back( i );
  f->createByte( i, 5 );                                        v.push_back( i );
  f->createUnsignedLong( i, 5 );                                v.push_back( i );
  f->createUnsignedInt( i, 5 );                                 v.push_back( i );
  f->createUnsignedShort( i, 5 );                               v.push_back( i );
  f->createUnsignedByte( i, 5 );                                v.push_back( i );
  f->createDecimal( i, xs_decimal( 5 ) );                       v.push_back( i );
  return v;
}

store::Item_t dbl( double d ) {
  store::Item_t i;
  GENV_ITEMFACTORY->createDouble( i, xs_double( d ) );
  return i;
}

store::Item_t flt( float d ) {
  store::Item_t i;
  GENV_ITEMFACTORY->createFloat( i, xs_float( d ) );
  return i;
}

store::Item_t ulng( uint64_t n ) {
  store::Item_t i;
  GENV_ITEMFACTORY->createUnsignedLong( i, n );
  return i;
}

void test_by_value() {
  store::Item_t const d55( dbl( 5.5 ) ), d5( dbl( 5 ) ), d4( dbl( 4.5 ) ),
                      f55( flt( 5.5f ) ), f5( flt( 5 ) ), f4( flt( 4.5f ) );
  store::Item_t dec55;
  GENV_ITEMFACTORY->createDecimal( dec55, xs_decimal( "5.5" ) );

  for ( store::Item_t const &n : fives() ) {
    check_less( n.getp(), d55.getp() );
    check_less( n.getp(), f55.getp() );
    check_less( d4.getp(), n.getp() );
    check_less( f4.getp(), n.getp() );
    check_equal( n.getp(), d5.getp() );
    check_equal( n.getp(), f5.getp() );
    if ( n->getTypeCode() != store::XS_DECIMAL )
      check_less( n.getp(), dec55.getp() );
  }
  check_equal( f55.getp(), d55.getp() );
  check_equal( dec55.getp(), d55.getp() );
  check_equal( dec55.getp(), f55.getp() );
}

// Promotion rounds: the value compared is the one cast to the floating type.
void test_rounding() {
  store::Item_t i, l;
  GENV_ITEMFACTORY->createInteger( i, xs_integer( 16777217 ) );
  GENV_ITEMFACTORY->createLong( l, 16777217 );
  store::Item_t const f( flt( 16777216.0f ) );
  check_equal( i.getp(), f.getp() );
  check_equal( l.getp(), f.getp() );
  check_equal( i.getp(), dbl( 16777217.0 ).getp() );
  check_less( f.getp(), dbl( 16777217.0 ).getp() );

  GENV_ITEMFACTORY->createLong( l, 9007199254740993LL );
  check_equal( l.getp(), dbl( 9007199254740992.0 ).getp() );
}

// xs:unsignedLong above the range of xs:long.
void test_unsigned_long() {
  store::Item_t const big( ulng( UINT64_C( 18446744073709551615 ) ) ),
                      big2( ulng( UINT64_C( 18446744073709551615 ) ) ),
                      mid( ulng( UINT64_C( 9223372036854775808 ) ) );
  store::Item_t maxl, minus1, dec;
  GENV_ITEMFACTORY->createLong( maxl, LLONG_MAX );
  GENV_ITEMFACTORY->createInteger( minus1, xs_integer( -1 ) );
  GENV_ITEMFACTORY->createDecimal( dec, xs_decimal( "18446744073709551615" ) );
  check_equal( big.getp(), big2.getp() );
  check_less( mid.getp(), big.getp() );
  check_less( maxl.getp(), big.getp() );
  check_less( maxl.getp(), mid.getp() );
  check_less( minus1.getp(), big.getp() );
  check_equal( big.getp(), dec.getp() );
  check_equal( big.getp(), dbl( 18446744073709551616.0 ).getp() );
  check_equal( big.getp(), flt( 18446744073709551616.0f ).getp() );
  check_less( mid.getp(), dbl( 18446744073709551616.0 ).getp() );
}

void test_itoa() {
  static struct { long long n; char const *s; } const signed_cases[] = {
    { LLONG_MAX, "9223372036854775807" }, { LLONG_MAX - 1, "9223372036854775806" },
    { LLONG_MIN, "-9223372036854775808" }, { LLONG_MIN + 1, "-9223372036854775807" },
    { 0, "0" }, { 7, "7" }, { -7, "-7" }, { 10, "10" }, { -10, "-10" }
  };
  ascii::itoa_buf_type buf;
  for ( auto const &c : signed_cases ) {
    if ( strcmp( ascii::itoa( c.n, buf ), c.s ) != 0 ) {
      ++failures;
      cerr << "FAILED: itoa " << c.s << " gave " << buf << endl;
    }
  }
  if ( strcmp( ascii::itoa( ULLONG_MAX, buf ), "18446744073709551615" ) != 0 ) {
    ++failures;
    cerr << "FAILED: itoa ULLONG_MAX gave " << buf << endl;
  }
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_numeric_items( int, char*[] ) {
  failures = 0;
  test_by_value();
  test_rounding();
  test_unsigned_long();
  test_itoa();
  cout << (failures ? "FAILED" : "passed") << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
