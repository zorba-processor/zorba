/*
 * Copyright 2026 Zorba contributors
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

// xs:decimal values held by store items:
// - take one small heap block (the M_APM_struct with the digits right after
//   it, sized to fit) instead of a struct plus an 84-byte digit array;
// - give exactly the results of the same value computed afresh: canonical
//   string, equality, ordering, hash, arithmetic, rounding, mixed with
//   values that were never stored;
// - stay correct when a copy outlives the item and is then assigned a value
//   that needs more digits than fit in place.

#include <iostream>
#include <string>
#include <vector>

#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "system/globalenv.h"
#include "zorbatypes/decimal.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/m_apm.h"
#include "zorbatypes/schema_types.h"
#include "zorbatypes/zstring.h"

using namespace std;
using namespace zorba;

namespace {

int failures;

void check( bool ok, string const &what ) {
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << endl;
  }
}

string str( Decimal const &d ) {
  return d.toString().c_str();
}

store::Item_t stored( Decimal const &d ) {
  store::Item_t item;
  GENV_ITEMFACTORY->createDecimal( item, d );
  return item;
}

char const *const values[] = {
  "0", "-0", "-0.0", "0.000", "+1", "1.0", "1.50", "100", "1000000",
  "-123.25", ".5", "5.", "  12.5  ", "0.001", "0.000000000000000001",
  "0.0000000000000000001",                  // 19 places: printed rounded
  "0.1234567890123456789",
  "999999999999999999", "1234567890123456", "12345678901234567",
  "9007199254740992", "9007199254740993", "-9007199254740993",
  "9223372036854775807", "-9223372036854775808",
  "9223372036854775808", "-9223372036854775809",
  "9223372036854775807.5", "18446744073709551616",
  "123456789012345678901234567890.123456789",
  "-0.000000000000000000000000000000000001",
  "1000000000000000000000000000000000000000000000000000000000000000000000"
  "0000000000000000000000000000000000000000000000000000000000000000000000"
  "000000000000000000000000000000000000000000000000000000000000.5",
};

} // namespace

namespace zorba {
namespace UnitTests {

int test_decimal_storage( int, char*[] ) {
  failures = 0;

  vector<Decimal> fresh, kept;
  vector<store::Item_t> items;
  for ( char const *s : values ) {
    Decimal const f( s );
    store::Item_t const item( stored( f ) );
    Decimal const k( item->getDecimalValue() );
    string const what( string( "\"" ) + s + '"' );

    // Storage: one block of at most 32 B of struct plus the digits rounded
    // up to 8 bytes (at least 8). That is 40 B up to 16 digits.
    // alloc_size() is the number of significant digits
    size_t const digits = f.alloc_size();
    size_t const digit_bytes = ( ( digits + 1 ) / 2 + 7 ) / 8 * 8;
    size_t const limit = 32 + ( digit_bytes < 8 ? 8 : digit_bytes );
    check( k.heap_bytes() <= limit,
           what + ": stored value takes " + to_string( k.heap_bytes() ) +
           " B, expected at most " + to_string( limit ) );
    if ( digits <= 16 )
      check( k.heap_bytes() <= 40, what + ": more than 40 B" );

    // Same value, same results.
    check( str( k ) == str( f ), what + ": toString " + str( k ) + " vs " +
           str( f ) );
    check( zstring( item->getStringValue() ) == f.toString(),
           what + ": item string value" );
    check( k == f && !( k != f ) && k.compare( f ) == 0 &&
           f.compare( k ) == 0 && k <= f && k >= f && !( k < f ),
           what + ": comparison with the fresh value" );
    check( k.hash() == f.hash(), what + ": hash" );
    check( item->hash() == f.hash(), what + ": item hash" );
    check( k.sign() == f.sign(), what + ": sign" );
    check( k.is_xs_long() == f.is_xs_long() &&
           k.is_xs_int() == f.is_xs_int() &&
           k.is_xs_integer() == f.is_xs_integer(), what + ": is_xs_*" );
    check( str( k.round() ) == str( f.round() ) &&
           str( k.round( xs_integer( 2 ) ) ) == str( f.round( xs_integer( 2 ) ) ) &&
           str( k.round( xs_integer( -3 ) ) ) == str( f.round( xs_integer( -3 ) ) ) &&
           str( k.roundHalfToEven( xs_integer( 1 ) ) ) ==
             str( f.roundHalfToEven( xs_integer( 1 ) ) ) &&
           str( k.floor() ) == str( f.floor() ) &&
           str( k.ceil() ) == str( f.ceil() ) &&
           str( -k ) == str( -f ), what + ": rounding" );
    check( k.toString( 5 ) == f.toString( 5 ), what + ": toString(5)" );

    fresh.push_back( f );
    kept.push_back( k );
    items.push_back( item );
  }

  // Arithmetic between stored and fresh values, both ways.
  for ( size_t i = 0; i < fresh.size(); ++i ) {
    for ( size_t j = 0; j < fresh.size(); ++j ) {
      Decimal const &fa = fresh[i], &fb = fresh[j];
      Decimal const &ka = kept[i], &kb = kept[j];
      string const what( string( "\"" ) + values[i] + "\" op \"" +
                         values[j] + '"' );
      check( str( ka + kb ) == str( fa + fb ) &&
             str( ka + fb ) == str( fa + fb ), what + ": +" );
      check( str( ka - kb ) == str( fa - fb ) &&
             str( fa - kb ) == str( fa - fb ), what + ": -" );
      check( str( ka * kb ) == str( fa * fb ) &&
             str( ka * fb ) == str( fa * fb ), what + ": *" );
      if ( fb.sign() != 0 ) {
        check( str( ka / kb ) == str( fa / fb ) &&
               str( fa / kb ) == str( fa / fb ), what + ": /" );
        check( str( ka % kb ) == str( fa % fb ) &&
               str( ka % fb ) == str( fa % fb ), what + ": %" );
      }
      check( ka.compare( kb ) == fa.compare( fb ) &&
             ka.compare( fb ) == fa.compare( fb ), what + ": compare" );
      check( ( ka == kb ) == ( fa == fb ), what + ": ==" );
      check( items[i]->equals( items[j].getp() ) == ( fa == fb ), what + ": equals" );
      if ( fa == fb )
        check( items[i]->hash() == items[j]->hash(), what + ": equal hash" );
    }
  }

  // A copy that outlives its item, then is assigned values that need more
  // digit space than the stored value has.
  for ( char const *s : values ) {
    Decimal d;
    {
      store::Item_t item( stored( Decimal( s ) ) );
      d = item->getDecimalValue();
    }
    string const what( string( "\"" ) + s + "\" reassigned" );
    d = "123456789012345678901234567890123456789.987654321";
    check( str( d ) == "123456789012345678901234567890123456789.987654321",
           what + ": " + str( d ) );
    d = 42LL;
    check( str( d ) == "42", what + " 42: " + str( d ) );
    d = 18446744073709551615ULL;
    check( str( d ) == "18446744073709551615", what + " ULL: " + str( d ) );
    Decimal e;
    {
      store::Item_t item( stored( Decimal( s ) ) );
      e = item->getDecimalValue();
    }
    e += Decimal( "0.5" );
    check( str( e ) == str( Decimal( s ) + Decimal( "0.5" ) ),
           what + ": += " + str( e ) );
  }

  // Storing a value that is already stored shares it.
  {
    store::Item_t const a( stored( Decimal( "123.25" ) ) );
    store::Item_t const b( stored( a->getDecimalValue() ) );
    check( b->getDecimalValue().heap_bytes() <= 40, "re-stored value" );
    check( a->equals( b.getp() ) && a->hash() == b->hash(), "re-stored equals" );
  }

  if ( failures )
    cerr << failures << " failure(s)" << endl;
  else
    cout << "decimal_storage: all checks passed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
