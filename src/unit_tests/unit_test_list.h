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
#pragma once
#ifndef ZORBA_UNIT_TEST_LIST_H
#define ZORBA_UNIT_TEST_LIST_H

#include <iostream>

#include <zorba/config.h>

namespace zorba {
namespace UnitTests {

  int runUriTest(int argc, char* argv[]);

  int runDebuggerProtocolTest(int argc, char* argv[]);

  int test_ato_( int, char*[] );
  int test_base64( int, char*[] );
  int test_base64_streambuf( int, char*[] );
  int test_fs_util( int, char*[] );
  int test_hashmaps( int argc, char* argv[] );
  int test_hexbinary( int argc, char* argv[] );
  int test_hexbinary_streambuf( int argc, char* argv[] );

#ifndef ZORBA_NO_ICU
  int test_icu_streambuf( int, char*[] );
#endif /* ZORBA_NO_ICU */

  int test_json_parser( int, char*[] );
  int test_mem_sizeof( int, char*[] );
  int test_parameters( int, char*[] );
  int test_string( int, char*[] );
  int test_time( int, char*[] );
  int test_time_parse( int, char*[] );

#ifndef ZORBA_NO_FULL_TEXT
  int test_stemmer( int, char*[] );
  int test_thesaurus( int, char*[] );
  int test_tokenizer( int, char*[] );
#endif /* ZORBA_NO_FULL_TEXT */

#ifndef ZORBA_HAVE_UNIQUE_PTR
  int test_unique_ptr( int, char*[] );
#endif /* ZORBA_HAVE_UNIQUE_PTR */

#ifndef ZORBA_HAVE_UNORDERED_MAP
  int test_unordered_map( int, char*[] );
#endif /* ZORBA_HAVE_UNORDERED_MAP */

#ifndef ZORBA_HAVE_UNORDERED_SET
  int test_unordered_set( int, char*[] );
#endif /* ZORBA_HAVE_UNORDERED_SET */

  int test_utf8_streambuf( int, char*[] );
  int test_uuid( int, char*[] );

  void initializeTestList();

} // namespace UnitTests
} // namespace zorba
#endif /* ZORBA_UNIT_TEST_LIST_H */
/* vim:set et sw=2 ts=2: */
