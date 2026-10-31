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

// Hash codes of numeric values (xs:double, xs:float, xs:decimal, xs:integer):
// - fractional values must not collide wholesale (they all used to hash to
//   their integer part, so a function cache or fn:distinct-values over
//   values in (-1, 1) became quadratic);
// - values equal after numeric promotion must hash equal.

#include "stdafx.h"

#include <iostream>
#include <limits>
#include <set>

#include "zorbatypes/decimal.h"
#include "zorbatypes/float.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/schema_types.h"

using namespace std;
using namespace zorba;

namespace {

int failures;

void check( bool ok, char const *what, char const *value ) {
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << " (" << value << ')' << endl;
  }
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_numeric_hash( int, char*[] ) {
  failures = 0;

  // Fractional values spread over the hash space.
  int const n = 10000;
  set<uint32_t> doubles, floats, decimals;
  for ( int i = 1; i < n; ++i ) {
    double const d = static_cast<double>( i ) / n - 0.5;   // (-0.5, 0.5)
    doubles.insert( xs_double( d ).hash() );
    floats.insert( xs_float( static_cast<float>( d ) ).hash() );
    decimals.insert( xs_decimal( d ).hash() );
  }
  cout << n - 1 << " fractional values: " << doubles.size()
       << " distinct xs:double hashes, " << floats.size() << " xs:float, "
       << decimals.size() << " xs:decimal" << endl;
  check( doubles.size() > n * 99 / 100, "distinct xs:double hashes", "" );
  check( floats.size() > n * 99 / 100, "distinct xs:float hashes", "" );
  check( decimals.size() > n * 99 / 100, "distinct xs:decimal hashes", "" );

  // Equal after promotion => equal hash codes.
  static char const *const decimal_values[] = {
    "0", "0.5", "-0.5", "0.1", "-0.1", "3", "-3", "2.75", "-2.75",
    "123.456", "0.333", "4294967296", "4294967296.5", "-9007199254740992",
    "100000000000000000000", "0.0000001", 0
  };
  for ( char const *const *s = decimal_values; *s; ++s ) {
    xs_double const d( *s );
    xs_decimal const dec( *s );         // promotes to d
    check( dec.hash() == d.hash(), "xs:decimal vs xs:double", *s );
    xs_float const f( *s );
    if ( static_cast<double>( f.getNumber() ) == d.getNumber() )
      check( f.hash() == d.hash(), "xs:float vs xs:double", *s );
    check( dec.hash() == f.hash(), "xs:decimal vs xs:float", *s );
    if ( dec.is_xs_integer() && dec.is_xs_long() ) {
      xs_integer const i( dec );
      check( i.hash() == d.hash(), "xs:integer vs xs:double", *s );
      check( i.hash() == dec.hash(), "xs:integer vs xs:decimal", *s );
    }
  }

  check( xs_double( -0.0 ).hash() == xs_double( 0.0 ).hash(), "-0 vs 0", "" );
  check( xs_float( -0.0f ).hash() == xs_integer( 0 ).hash(), "-0 vs 0", "" );
  check( xs_double( 1e300 ).hash() != xs_double( 2e300 ).hash(),
         "xs:double beyond the xs:float range", "" );
  check( xs_float::nan().hash() == xs_double::nan().hash(), "NaN", "" );
  check( xs_float::pos_inf().hash() == xs_double::pos_inf().hash(), "INF", "" );
  check( xs_float::neg_inf().hash() == xs_double::neg_inf().hash(), "-INF", "" );

  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
