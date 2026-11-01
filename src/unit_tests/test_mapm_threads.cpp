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

// Concurrent MAPM arithmetic: xs:decimal (always MAPM), xs:integer (MAPM in
// ZORBA_WITH_BIG_INTEGER builds) and the MAPM functions with internal caches
// (pow, exp, log, sin, sqrt, divide), from several threads at once. Every
// thread must compute exactly what a single thread computes. Meant to be run
// under ThreadSanitizer as well (see the tsan presets).
//
// This works at the library level rather than through queries: the default
// build has ZORBA_FOR_ONE_THREAD_ONLY=ON, which compiles out the locking of
// the rest of the engine (store reference counts, QName pool, ...).

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zorba/config.h>

#include "zorbatypes/decimal.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/m_apm.h"
#include "zorbatypes/schema_types.h"
#include "zorbatypes/zstring.h"

using namespace std;
using namespace zorba;

namespace {

// Shared by all threads: copies of it share its M_APM_struct.
Decimal const theShared( "0.123456789" );

string mapm_str( MAPM const &m ) {
  char buf[256];
  m.toString( buf, 40 );
  return buf;
}

string decimal_work( int seed ) {
  Decimal sum;
  Decimal const rate( "1.0825" );
  for ( int i = 1; i <= 400; ++i ) {
    Decimal const x( i + seed );
    Decimal copy( theShared );            // shared refcount
    copy += x * rate;                     // copy-on-write
    sum += copy / Decimal( 7 );
    sum -= Decimal( "0.001" ) * theShared;
  }
  string r( sum.roundHalfToEven( xs_integer( 12 ) ).toString().c_str() );
  Decimal const big( "123456789012345678901234567890.123456789" );
  r += ' ';
  r += ( big * theShared % Decimal( "3.7" ) ).toString().c_str();
  r += ' ';
  r += ( big / Decimal( 3 ) ).floor().toString().c_str();
  r += ' ';
  r += Decimal( "2" ).sqrt().toString().c_str();
  r += ( theShared < Decimal( 1 ) ) ? " lt" : " ge";
  return r;
}

string integer_work( int seed ) {
  xs_integer sum( 0 );
  for ( int i = 1; i <= 400; ++i ) {
    xs_integer const n( i + seed );
    sum += n * n - n / xs_integer( 7 ) + n % xs_integer( 13 );
  }
  string r( sum.toString().c_str() );
#ifdef ZORBA_WITH_BIG_INTEGER
  xs_integer f( 1 );
  for ( int i = 2; i <= 60; ++i )
    f *= xs_integer( i );
  r += ' ';
  r += f.toString().c_str();
  r += ' ';
  r += ( f / xs_integer( "99999999999999999999" ) ).toString().c_str();
  r += ' ';
  r += ( f % xs_integer( 1000000007 ) ).toString().c_str();
#endif
  return r;
}

string function_work( int seed ) {
  MAPM const x( 2.5 + seed / 1000.0 );
  string r;
  r += mapm_str( x.pow( MAPM( "1.3" ), 40 ) );
  r += ' ';
  r += mapm_str( x.exp( 40 ) );
  r += ' ';
  r += mapm_str( x.log( 40 ) );
  r += ' ';
  r += mapm_str( x.sin( 40 ) );
  r += ' ';
  r += mapm_str( x.sqrt( 60 ) );
  r += ' ';
  r += mapm_str( x.divide( MAPM( 7 ), 50 ) );
  r += ' ';
  r += mapm_str( x.ipow( 25, 40 ) );
  return r;
}

string work( int seed ) {
  return decimal_work( seed ) + " | " + integer_work( seed ) + " | " +
         function_work( seed );
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_mapm_threads( int, char*[] ) {
  unsigned const num_threads =
    std::clamp( std::thread::hardware_concurrency(), 4u, 8u );
  int const num_seeds = 6;
  int const iterations = 8;

  vector<string> expected( num_seeds );
  auto start = chrono::steady_clock::now();
  for ( int i = 0; i < iterations; ++i )
    for ( int s = 0; s < num_seeds; ++s )
      expected[s] = work( s );
  double const single =
    chrono::duration<double>( chrono::steady_clock::now() - start ).count();

  for ( int s = 0; s < num_seeds; ++s )
    cout << "seed " << s << ": " << expected[s] << endl;

  atomic<unsigned> mismatches( 0 );
  atomic<unsigned> errors( 0 );

  start = chrono::steady_clock::now();
  vector<thread> threads;
  for ( unsigned t = 0; t < num_threads; ++t ) {
    threads.emplace_back( [&, t]() {
      for ( int i = 0; i < iterations; ++i ) {
        for ( int j = 0; j < num_seeds; ++j ) {
          int const s = ( j + t ) % num_seeds;
          try {
            string const r( work( s ) );
            if ( r != expected[s] && mismatches++ < 5 )
              cerr << "thread " << t << " seed " << s << ": got\n  " << r
                   << "\nexpected\n  " << expected[s] << endl;
          }
          catch ( std::exception const &e ) {
            if ( errors++ < 5 )
              cerr << "thread " << t << " seed " << s << ": " << e.what()
                   << endl;
          }
        }
      }
    } );
  }
  for ( thread &th : threads )
    th.join();
  double const multi =
    chrono::duration<double>( chrono::steady_clock::now() - start ).count();

  cout << num_threads << " threads x " << iterations << " iterations x "
       << num_seeds << " seeds: " << mismatches << " mismatches, " << errors
       << " errors" << endl;
  cout << "time: 1 thread " << single << " s, " << num_threads << " threads "
       << multi << " s (" << num_threads << "x the work)" << endl;

  return ( mismatches == 0 && errors == 0 ) ? 0 : 1;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
