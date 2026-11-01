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

#include <zorba/config.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifdef ZORBA_WITH_FILE_ACCESS
#include "util/mmap_file.h"
#endif /* ZORBA_WITH_FILE_ACCESS */

using namespace std;
using namespace zorba;

static int failures;

[[maybe_unused]] static bool assert_true( char const *expr, int line, bool result ) {
  if ( !result ) {
    cout << "FAILED, line " << line << ": " << expr << endl;
    ++failures;
  }
  return result;
}

#define ASSERT_TRUE( EXPR ) assert_true( #EXPR, __LINE__, !!(EXPR) )

#ifdef ZORBA_WITH_FILE_ACCESS

static string slurp( filesystem::path const &path ) {
  ifstream in( path, ios::binary );
  return string( istreambuf_iterator<char>( in ), istreambuf_iterator<char>() );
}

static void spew( filesystem::path const &path, char const *s ) {
  ofstream out( path, ios::binary | ios::trunc );
  out << s;
}

static void run_mmap_tests( filesystem::path const &path ) {
  { // read-only (the default)
    spew( path, "hello" );
    mmap_file f( path.string().c_str() );
    ASSERT_TRUE( f.size() == 5 );
    ASSERT_TRUE( string( f.begin(), f.end() ) == "hello" );
  }
  { // read/write: changes reach the file
    spew( path, "hello" );
    try {
      mmap_file f( path.string().c_str(), ios::in | ios::out );
      ASSERT_TRUE( string( f.begin(), f.end() ) == "hello" );
      f[0] = 'j';
    }
    catch ( std::exception const &e ) {
      cout << "in|out: " << e.what() << endl;
      ASSERT_TRUE( false );
    }
    ASSERT_TRUE( slurp( path ) == "jello" );
  }
  { // write only
    spew( path, "hello" );
    try {
      mmap_file f( path.string().c_str(), ios::out );
      f[4] = 'p';
    }
    catch ( std::exception const &e ) {
      cout << "out: " << e.what() << endl;
      ASSERT_TRUE( false );
    }
    ASSERT_TRUE( slurp( path ) == "hellp" );
  }
}

#endif /* ZORBA_WITH_FILE_ACCESS */

namespace zorba {
namespace UnitTests {

int test_mmap_file( int, char*[] ) {
#ifdef ZORBA_WITH_FILE_ACCESS
  filesystem::path const dir =
    filesystem::temp_directory_path() / "zorba_test_mmap_file";
  filesystem::create_directories( dir );
  filesystem::path const path = dir / "data.txt";
  run_mmap_tests( path );
  error_code ec;
  filesystem::remove_all( dir, ec );
#endif /* ZORBA_WITH_FILE_ACCESS */

  cout << failures << " test(s) failed\n";
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
