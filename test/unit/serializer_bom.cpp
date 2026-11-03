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

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

#include <zorba/config.h>
#include <zorba/options.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>

using namespace zorba;

/**
 * Serializes "a" with byte-order-mark=yes and compares the octets.
 */
static bool check( Zorba *z, char const *method, char const *encoding,
                   std::string const &expected ) {
  Zorba_SerializerOptions_t opts;
  opts.set( "method", method );
  opts.set( "omit-xml-declaration", "yes" );
  opts.set( "encoding", encoding );
  opts.set( "byte-order-mark", "yes" );
  XQuery_t const q = z->compileQuery( "'a'" );
  std::ostringstream out;
  q->execute( out, &opts );
  std::string const actual( out.str() );
  std::cout << method << ' ' << encoding << ':';
  for ( unsigned char const c : actual ) {
    char buf[4];
    std::snprintf( buf, sizeof buf, " %02X", c );
    std::cout << buf;
  }
  std::cout << std::endl;
  return actual == expected;
}

int serializer_bom( int, char*[] ) {
  void *const store = StoreManager::getStore();
  Zorba *const z = Zorba::getInstance( store );
  bool ok = true;
  try {
    std::string const utf8( "\xEF\xBB\xBF" "a" );
    ok &= check( z, "text", "UTF-8", utf8 );
    ok &= check( z, "text", "utf-8", utf8 );
    ok &= check( z, "xml", "UTF-8", utf8 );
#ifndef ZORBA_NO_ICU
    ok &= check( z, "text", "UTF-16LE", std::string( "\xFF\xFE" "a\0", 4 ) );
    ok &= check( z, "text", "UTF-16BE", std::string( "\xFE\xFF\0a", 4 ) );
    ok &= check( z, "xml", "UTF-16LE", std::string( "\xFF\xFE" "a\0", 4 ) );
    ok &= check( z, "xml", "UTF-16BE", std::string( "\xFE\xFF\0a", 4 ) );
    // One signature, in the order of the code unit that follows.
    Zorba_SerializerOptions_t opts;
    opts.set( "method", "text" );
    opts.set( "encoding", "UTF-16" );
    opts.set( "byte-order-mark", "yes" );
    XQuery_t const q = z->compileQuery( "'a'" );
    std::ostringstream out;
    q->execute( out, &opts );
    std::string const s( out.str() );
    std::cout << "text UTF-16 size " << s.size() << std::endl;
    ok &= s == std::string( "\xFF\xFE" "a\0", 4 ) ||
          s == std::string( "\xFE\xFF\0a", 4 );
#endif /* ZORBA_NO_ICU */
  }
  catch ( std::exception const &e ) {
    std::cerr << e.what() << std::endl;
    ok = false;
  }
  z->shutdown();
  StoreManager::shutdownStore( store );
  return ok ? 0 : 1;
}
