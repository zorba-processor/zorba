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

// A double on the left of == against an xs:double or xs:float must compare,
// like the operand order the other way round.

#include <iostream>
#include <limits>

#include "zorbatypes/float.h"
#include "zorbatypes/schema_types.h"

using namespace std;
using namespace zorba;

namespace zorba {
namespace UnitTests {

namespace {

int failures;

void check( bool ok, char const *what ) {
  if ( !ok ) {
    ++failures;
    cerr << "FAILED: " << what << endl;
  }
}

} // namespace

int test_float_equality( int, char*[] ) {
  failures = 0;

  double const nan = numeric_limits<double>::quiet_NaN();

  check( !( 0.5 == xs_double( 1.0 ) ), "0.5 == xs:double(1)" );
  check(   1.0 == xs_double( 1.0 ),    "1 == xs:double(1)" );
  check( !( 0.0 == xs_double( 1.0 ) ), "0 == xs:double(1)" );
  check( !( 1.0 == xs_double( 0.0 ) ), "1 == xs:double(0)" );
  check( !( nan == xs_double( nan ) ), "NaN == xs:double(NaN)" );
  check( !( 0.5 == xs_float( 1.0f ) ), "0.5 == xs:float(1)" );
  check(   1.0 == xs_float( 1.0f ),    "1 == xs:float(1)" );

  check( 0.5 != xs_double( 1.0 ),      "0.5 != xs:double(1)" );
  check( xs_double( 1.0 ) != 0.5,      "xs:double(1) != 0.5" );
  check( !( xs_double( 1.0 ) == 0.5 ), "xs:double(1) == 0.5" );

  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
