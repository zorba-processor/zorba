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

#include <iostream>
#include <stdexcept>

#include <zorba/internal/diagnostic.h>

using namespace std;
using namespace zorba::internal::diagnostic;

///////////////////////////////////////////////////////////////////////////////

static int failures;

static bool assert_true( char const *expr, int line, bool result ) {
  if ( !result ) {
    cout << "FAILED, line " << line << ": " << expr << endl;
    ++failures;
  }
  return result;
}

static void print_exception( char const *expr, int line,
                             std::exception const &e ) {
  assert_true( expr, line, false );
  cout << "+ exception: " << e.what() << endl;
}

#define ASSERT_TRUE( EXPR ) assert_true( #EXPR, __LINE__, !!(EXPR) )

#define ASSERT_EXCEPTION( EXPR, EXCEPTION ) \
  try { EXPR; assert_true( #EXPR, __LINE__, false ); } \
  catch ( EXCEPTION const& ) { } \
  catch ( ... ) { assert_true( #EXPR, __LINE__, false ); }

#define ASSERT_NO_EXCEPTION( EXPR ) \
  try { EXPR; } \
  catch ( std::exception const &e ) { print_exception( #EXPR, __LINE__, e ); } \
  catch ( ... ) { assert_true( #EXPR, __LINE__, false ); }

#define MAKE_PARAMS(...) \
  (::zorba::internal::diagnostic::parameters(), __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////

static void test_simple() {
  {
    string s( "x$1y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xfooy" );
  }
  {
    string s( "x$1y" );
    parameters const p( MAKE_PARAMS( "" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xy" );
  }
  {
    string s( "x$1y" );
    parameters const p( MAKE_PARAMS( "\\" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x\\y" );
  }
}

static void test_braces() {
  {
    string s( "x${1}y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xfooy" );
  }
  {
    string s( "x${ 1 }y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x foo y" );
  }
  {
    string s( "x${1}y" );
    parameters const p( MAKE_PARAMS( "" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xy" );
  }
  {
    string s( "x${ 1 }y" );
    parameters const p( MAKE_PARAMS( "" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xy" );
  }
  { // $2 after a ${...3...}: pass 2 passes over the ${...3...}
    string s( "${3: }$2" );
    parameters const p( MAKE_PARAMS( "a", "b", "c" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "c: b" );
  }
  { // the same with the ${...3...} erased
    string s( "${3: }$2" );
    parameters const p( MAKE_PARAMS( "a", "b", "" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "b" );
  }
  { // $1 after a ${2}, then text
    string s( "x${2}y$1z" );
    parameters const p( MAKE_PARAMS( "a", "b" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbyaz" );
  }
  { // the template of libxml_514: $6 after a ${ 8}
    string s( "\"$5\": element has${ 8} attribute(s): \"$6\"" );
    parameters const p( MAKE_PARAMS( "", "", "", "", "e", "ids", "", "3" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "\"e\": element has 3 attribute(s): \"ids\"" );
  }
  { // a ${1} after a ${2}
    string s( "${2,}${1}" );
    parameters const p( MAKE_PARAMS( "a", "b" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "b,a" );
  }
}

static void test_conditionals() {
  { // if-true-then
    string s( "x$1?2y" );
    parameters const p( MAKE_PARAMS( "foo", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbary" );
  }
  { // if-false-then
    string s( "x$1?2y" );
    parameters const p( MAKE_PARAMS( "", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xy" );
  }
  { // if-true-then-else
    string s( "x$1?2:3y" );
    parameters const p( MAKE_PARAMS( "foo", "bar", "baz" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbary" );
  }
  { // if-false-then-else
    string s( "x$1?2:3y" );
    parameters const p( MAKE_PARAMS( "", "bar", "baz" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbazy" );
  }
  { // if-true-{then}
    string s( "x$1?{2}y" );
    parameters const p( MAKE_PARAMS( "foo", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbary" );
  }
  { // if-false-{then}
    string s( "x$1?{2}y" );
    parameters const p( MAKE_PARAMS( "", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xy" );
  }
  { // if-true-{then}-{else}
    string s( "x$1?{2}:{3}y" );
    parameters const p( MAKE_PARAMS( "foo", "bar", "baz" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbary" );
  }
  { // if-false-{then}-{else}
    string s( "x$1?{2}:{3}y" );
    parameters const p( MAKE_PARAMS( "", "bar", "baz" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbazy" );
  }
}

static void test_escapes() {
  {
    string s( "x\\$$1y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x$fooy" );
  }
  {
    string s( "x${\\11\\}}y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x1foo}y" );
  }
  {
    string s( "x$1\\?y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xfoo?y" );
  }
  {
    string s( "x$1?{bar\\}}y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbar}y" );
  }
  {
    string s( "x$1?{bar\\}}y" );
    parameters const p( MAKE_PARAMS( "" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xy" );
  }
  {
    string s( "x$1?{bar\\}}\\:{y}z" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbar}:{y}z" );
  }
  {
    string s( "x$1?{bar\\}}:{2\\}}z" );
    parameters const p( MAKE_PARAMS( "", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xbar}z" );
  }
}

static void test_literal_values() {
  //
  // Parameter values may carry user text (e.g., a piece of the input quoted
  // by a parser error message): they must be inserted literally and never be
  // scanned for substitutions again by the passes for higher parameters.
  //
  {
    string s( "x$1y$2" );
    parameters const p( MAKE_PARAMS( "$2", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x$2ybar" );
  }
  {
    string s( "x$1y$2" );
    parameters const p( MAKE_PARAMS( "${2}", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x${2}ybar" );
  }
  { // found by fuzzing (json_parse): a malformed $i?then in a value
    string s( "x$1y" );
    parameters const p( MAKE_PARAMS( "{9$4?( ", "", "", "z" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x{9$4?( y" );
  }
  {
    string s( "x$1y" );
    parameters const p( MAKE_PARAMS( "a\\$2b", "bar" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xa\\$2by" );
  }
  {
    string s( "x${1}y" );
    parameters const p( MAKE_PARAMS( "$2?{3}", "bar", "baz" ) );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "x$2?{3}y" );
  }
  { // the 9th value is unescaped like the others
    string s( "x$9y" );
    parameters const p(
      MAKE_PARAMS( "1", "2", "3", "4", "5", "6", "7", "8", "a\\b$1" )
    );
    ASSERT_NO_EXCEPTION( p.substitute( &s ) );
    ASSERT_TRUE( s == "xa\\b$1y" );
  }
}

static void test_exceptions() {
  {
    string s( "x$1?y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_EXCEPTION( p.substitute( &s ), invalid_argument );
  }
  {
    string s( "x$1?{y" );
    parameters const p( MAKE_PARAMS( "foo" ) );
    ASSERT_EXCEPTION( p.substitute( &s ), invalid_argument );
  }
}

///////////////////////////////////////////////////////////////////////////////

// The short form of a value: what the tests below expect.
static string short_form( string const &value, size_t head, size_t tail ) {
  return value.substr( 0, head ) + "\xE2\x80\xA6" +
    value.substr( value.size() - tail ) + " (" +
    to_string( value.size() ) + " bytes)";
}

static string substituted( string template_text, string const &value ) {
  parameters const p( MAKE_PARAMS( value ) );
  p.substitute( &template_text );
  return template_text;
}

static bool valid_utf8( string const &s ) {
  for ( size_t i = 0; i < s.size(); ) {
    unsigned char const b = static_cast<unsigned char>( s[i] );
    size_t const len =
      b < 0x80 ? 1 : ( b & 0xE0 ) == 0xC0 ? 2 : ( b & 0xF0 ) == 0xE0 ? 3 :
      ( b & 0xF8 ) == 0xF0 ? 4 : 0;
    if ( !len || i + len > s.size() )
      return false;
    for ( size_t j = 1; j < len; ++j )
      if ( ( static_cast<unsigned char>( s[ i + j ] ) & 0xC0 ) != 0x80 )
        return false;
    i += len;
  }
  return true;
}

static void test_bound() {
  string const at( 100, 'a' ), past( 101, 'a' ), big( 3000000, 'a' );
  ASSERT_TRUE( substituted( "$1", at ) == at );
  ASSERT_TRUE( substituted( "$1", past ) == short_form( past, 40, 40 ) );
  ASSERT_TRUE( substituted( "$1", big ) == short_form( big, 40, 40 ) );
  ASSERT_TRUE( substituted( "$1", big ).size() < 110 );

  // The three forms of a template, with a value empty, at and past the bound.
  ASSERT_TRUE( substituted( "x${: 1}y", "" ) == "xy" );
  ASSERT_TRUE( substituted( "x${: 1}y", at ) == "x: " + at + "y" );
  ASSERT_TRUE( substituted( "x${: 1}y", past ) ==
               "x: " + short_form( past, 40, 40 ) + "y" );
  ASSERT_TRUE( substituted( "$1?{[1]}:{none}", "" ) == "none" );
  ASSERT_TRUE( substituted( "$1?{[1]}:{none}", at ) == "[" + at + "]" );
  ASSERT_TRUE( substituted( "$1?{[1]}:{none}", past ) ==
               "[" + short_form( past, 40, 40 ) + "]" );
  ASSERT_TRUE( substituted( "\"$1\": text", "" ) == "\"\": text" );
  ASSERT_TRUE( substituted( "\"$1\": text", past ) ==
               "\"" + short_form( past, 40, 40 ) + "\": text" );

  // Several parameters: each has its own bound, and a value can't use another.
  {
    string s( "$1 $2 $3" );
    string const with_mark( string( 50, 'b' ) + "$3" + string( 50, 'b' ) );
    parameters const p( MAKE_PARAMS( past, with_mark, string( "c" ) ) );
    p.substitute( &s );
    ASSERT_TRUE( s == short_form( past, 40, 40 ) + " " +
                 short_form( with_mark, 40, 40 ) + " c" );
  }
}

// The characters of the escapes at every place around the cuts.
static void test_bound_escapes() {
  string const marks( "$1\\${}?:\"" );
  for ( size_t pad = 28; pad <= 44; ++pad ) {
    string const value( string( pad, 'p' ) + marks + string( 200, 'y' ) + marks +
                        string( pad, 'q' ) );
    ASSERT_TRUE( substituted( "\"$1\": text", value ) ==
                 "\"" + short_form( value, 40, 40 ) + "\": text" );
    ASSERT_TRUE( substituted( "x${: 1}y", value ) ==
                 "x: " + short_form( value, 40, 40 ) + "y" );
  }
  string const at( string( 50, '$' ) + string( 50, '\\' ) );
  ASSERT_TRUE( substituted( "$1", at ) == at );
  string const past( string( 51, '$' ) + ' ' + string( 49, '\\' ) );
  ASSERT_TRUE( substituted( "$1", past ) == short_form( past, 40, 40 ) );
}

// A cut falls between characters, wherever they start.
static void test_bound_utf8() {
  for ( string const &c : { string( "\xC3\xA9" ), string( "\xE2\x82\xAC" ),
                            string( "\xF0\x9F\x98\x80" ) } ) {
    for ( size_t shift = 0; shift < 4; ++shift ) {
      string value( shift, 'a' );
      for ( size_t i = 0; i < 80; ++i )
        value += c;
      value.append( shift, 'z' );
      string const result( substituted( "$1", value ) );
      ASSERT_TRUE( valid_utf8( result ) );
      size_t const cut = result.find( "\xE2\x80\xA6" );
      ASSERT_TRUE( cut >= 37 && cut <= 40 &&
                   value.compare( 0, cut, result, 0, cut ) == 0 );
      string const end( " (" + to_string( value.size() ) + " bytes)" );
      size_t const tail_size = result.size() - cut - 3 - end.size();
      ASSERT_TRUE( tail_size >= 37 && tail_size <= 40 &&
                   value.compare( value.size() - tail_size, tail_size,
                                  result, cut + 3, tail_size ) == 0 );
    }
  }
  // Not UTF-8 at all: still a cut, and nothing read past the ends.
  string const bytes( 300, '\x80' );
  ASSERT_TRUE( substituted( "$1", bytes ).find( "(300 bytes)" ) !=
               string::npos );
}

// A dictionary key is expanded when the message is built, after the bound.
static void test_bound_keys() {
  string const value( 3000, 'v' );
  string s( "$1" );
  parameters const p( MAKE_PARAMS( string( "~FORG0001_NoCastTo_234" ), value,
                                   string( "xs:string" ),
                                   string( "xs:integer" ) ) );
  p.substitute( &s );
  ASSERT_TRUE( s == "\"" + short_form( value, 40, 40 ) +
               "\": value of type xs:string is not castable to type xs:integer" );
  // Data that starts like a key is data.
  string const tilde( "~" + string( 300, 'k' ) );
  ASSERT_TRUE( substituted( "$1", tilde ) == short_form( tilde, 40, 40 ) );
}

// What names a resource stays whole up to the size of a path.
static void test_bound_names() {
  string const uri( "http://example.org/" + string( 400, 'u' ) );
  string const path( "/data/" + string( 300, 'd' ) + "/a file.json" );
  string const relative( string( 200, 'd' ) + "/" + string( 200, 'e' ) + ".xq" );
  string const windows( "C:\\" + string( 200, 'd' ) + "\\file.xml" );
  string const clark( "{http://example.org/" + string( 200, 'n' ) + "}local" );
  for ( string const &name : { uri, path, relative, windows, clark } )
    ASSERT_TRUE( substituted( "\"$1\"", name ) == "\"" + name + "\"" );

  // At the size of a path, and past it.
  string const at( "/" + string( 4095, 'd' ) ), past( "/" + string( 4096, 'd' ) );
  ASSERT_TRUE( substituted( "$1", at ) == at );
  ASSERT_TRUE( substituted( "$1", past ) == short_form( past, 40, 40 ) );

  // Text with a slash or a colon in it is not a name.
  string const sentence( "see: " + string( 100, 't' ) + " and/or " +
                         string( 100, 't' ) );
  ASSERT_TRUE( substituted( "$1", sentence ) == short_form( sentence, 40, 40 ) );
}

// A string in the code of Zorba is the text of the error, not data.
static void test_bound_literals() {
  char const literal[] =
    "a message of Zorba written in its code, longer than the bound of a value, "
    "which belongs to the error and has nothing of the data of a query in it";
  string s( "$1" );
  parameters const p( MAKE_PARAMS( literal ) );
  p.substitute( &s );
  ASSERT_TRUE( s == literal );
}

///////////////////////////////////////////////////////////////////////////////

namespace zorba {
namespace UnitTests {

int test_parameters( int, char*[] ) {
  test_simple();
  test_braces();
  test_conditionals();
  test_escapes();
  test_literal_values();
  test_exceptions();
  test_bound();
  test_bound_escapes();
  test_bound_utf8();
  test_bound_keys();
  test_bound_names();
  test_bound_literals();

  cout << failures << " test(s) failed\n";
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
