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

#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <vector>

#include "util/json_parser.h"

using namespace std;
using namespace zorba;
using namespace zorba::json;

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
  cout << "+ exception: ";
  if ( json::exception const *j = dynamic_cast<json::exception const*>( &e ) ) {
    json::location const &loc = j->get_loc();
    if ( loc.file() && *loc.file() )
      cout << '"' << loc.file() << "\": ";
    cout << loc.line();
    if ( loc.column() )
      cout << ',' << loc.column();
    cout << ": ";
  }
  cout << e.what() << endl;
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

#define ASSERT_TRUE_AND_NO_EXCEPTION( EXPR ) \
  try { ASSERT_TRUE( EXPR ); } \
  catch ( std::exception const &e ) { print_exception( #EXPR, __LINE__, e ); } \
  catch ( ... ) { assert_true( #EXPR, __LINE__, false ); }

///////////////////////////////////////////////////////////////////////////////

static void test_empty_stream() {
  char const source[] = "";
  istringstream iss( source );
  parser p( iss );
  token t;
  ASSERT_NO_EXCEPTION( p.next( &t ) );
}

static void test_illegal_character() {
  char const source[] = " x ";
  istringstream iss( source );
  lexer lex( iss );
  token t;
  ASSERT_EXCEPTION( lex.next( &t ), illegal_character );
}

static void test_illegal_codepoint() {
  static char const *const sources[] = {
    " \" \\u \" ",
    " \" \\u0 \" ",
    " \" \\u00 \" ",
    " \" \\u000 \" ",
    " \" \\uG \" ",
    " \" \\u\" ",
    " \" \\uD83D \" ",
    " \" \\uD83D\\u0041 \" ",
    " \" \\uD83D\\uD83E \" ",
    " \" \\uDC4A \" ",
    0
  };

  for ( char const *const *s = sources; *s; ++s ) {
    istringstream iss( *s );
    lexer lex( iss );
    token t;
    ASSERT_EXCEPTION( lex.next( &t ), illegal_codepoint );
  }
}

static void test_illegal_escape() {
  char const source[] = "  \"  \\x  \"  ";
  istringstream iss( source );
  lexer lex( iss );
  token t;
  ASSERT_EXCEPTION( lex.next( &t ), illegal_escape );
}

static void test_illegal_literal() {
  static char const *const sources[] = {
    " f ",
    " fa ",
    " fal ",
    " fals ",
    " falsee ",
    " t ",
    " tr ",
    " tru ",
    " truee ",
    " n ",
    " nu ",
    " nul ",
    " nulll ",
    0
  };

  for ( char const *const *s = sources; *s; ++s ) {
    istringstream iss( *s );
    lexer lex( iss );
    token t;
    ASSERT_EXCEPTION( lex.next( &t ), illegal_literal );
  }
}

static void test_illegal_number() {
}

static void test_json_org_example() {
  char const source[] =
/*  1 */ "{" "\n"
/*  2 */ "  \"glossary\": {" "\n"
/*  3 */ "    \"title\": \"example glossary\"," "\n"
/*  4 */ "    \"GlossDiv\": {" "\n"
/*  5 */ "      \"title\": \"S\"," "\n"
/*  6 */ "      \"GlossList\": {" "\n"
/*  7 */ "        \"GlossEntry\": {" "\n"
/*  8 */ "          \"ID\": \"SGML\"," "\n"
/*  9 */ "          \"SortAs\": \"SGML\"," "\n"
/* 10 */ "          \"GlossTerm\": \"Standard Generalized Markup Language\"," "\n"
/* 11 */ "          \"Acronym\": \"SGML\"," "\n"
/* 12 */ "          \"Abbrev\": \"ISO 8879:1986\"," "\n"
/* 13 */ "          \"GlossDef\": {" "\n"
/* 14 */ "            \"para\": \"A meta-markup language, used to create markup languages such as DocBook.\"," "\n"
/* 15 */ "            \"GlossSeeAlso\": [\"GML\", \"XML\"]" "\n"
/* 16 */ "          }," "\n"
/* 17 */ "          \"GlossSee\": \"markup\"" "\n"
/* 18 */ "        }" "\n"
/* 19 */ "      }" "\n"
/* 20 */ "    }" "\n"
/* 21 */ "  }" "\n"
/* 22 */ "}" "\n" ;

  istringstream iss( source );
  parser p( iss );
  token t;

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  1: {
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  2: "glossary"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  2: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  2: {
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  3: "title"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  3: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  3: "example glossary"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  3: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  4: "GlossDiv"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  4: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  4: {
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  5: "title"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  5: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  5: "S"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  5: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  6: "GlossList"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  6: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  6: {
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  7: "GlossEntry"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  7: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  7: {
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  8: "ID"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  8: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  8: "SGML"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  8: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  9: "SortAs"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  9: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  9: "SGML"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  //  9: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 10: "GlossTerm"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 10: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 10: "Standard ..."
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 10: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 11: "Acronym"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 11: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 11: "SGML"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 11: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 12: "Abbrev"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 12: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 12: "ISO 8879:1986"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 12: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 13: "GlossDef"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 13: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 13: {
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 14: "para"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 14: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 14: "A meta-markup ..."
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 14: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: "GlossSeeAlso"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: [
  ASSERT_TRUE( t == token::begin_array );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: "GML"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: "XML"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 15: ]
  ASSERT_TRUE( t == token::end_array );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 16: }
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 16: ,
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 17: "GlossSee"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 17: :
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 17: "markup"
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 18: }
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 19: }
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 20: }
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 21: }
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );  // 22: }
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE( !p.next( &t ) );
}

static void test_lexer_array() {
  char const source[] = "[ 1, \"2\", false, true, null, \"\\uD83D\\uDC4A\" ]";
  istringstream iss( source );
  lexer lex( iss );
  token t;

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::begin_array );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::number );
  ASSERT_TRUE( t.get_value() == "1" );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::string );
  ASSERT_TRUE( t.get_value() == "2" );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::json_false );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::json_true );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::json_null );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::string );
  ASSERT_TRUE( t.get_value() == "\xF0\x9F\x91\x8A" );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::end_array );

  ASSERT_TRUE_AND_NO_EXCEPTION( !lex.next( &t ) );
}

static void test_lexer_object() {
  char const source[] = "{ \"a\" : 1, \"b\" : \"2\" }";
  istringstream iss( source );
  lexer lex( iss );
  token t;

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::number );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( !lex.next( &t ) );
}

static void test_parser_array() {
  char const source[] = "[ 1, \"2\", false, true, null ]";
  istringstream iss( source );
  parser p( iss );
  token t;

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::begin_array );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::number );
  ASSERT_TRUE( t.get_value() == "1" );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::string );
  ASSERT_TRUE( t.get_value() == "2" );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::json_false );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::json_true );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::json_null );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::end_array );

  ASSERT_TRUE( !p.next( &t ) );
}

static void test_parser_object() {
  char const source[] = "{ \"a\" : 1, \"b\" : \"2\" }";
  istringstream iss( source );
  parser p( iss );
  token t;

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::begin_object );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::number );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::value_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::name_separator );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::string );

  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
  ASSERT_TRUE( t == token::end_object );

  ASSERT_TRUE( !p.next( &t ) );
}

static void test_unexpected_token() {
  token t;
  {
    char const source[] = "{ 1 }";
    istringstream iss( source );
    parser p( iss );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::begin_object );
    ASSERT_EXCEPTION( p.next( &t ), unexpected_token );
  }
  {
    char const source[] = "{ \"a\" : 1, }";
    istringstream iss( source );
    parser p( iss );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::begin_object );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::string );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::name_separator );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::number );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::value_separator );

    ASSERT_EXCEPTION( p.next( &t ), unexpected_token );
  }
  {
    char const source[] = "{ \"t\" : true \"f\" : false }";
    istringstream iss( source );
    parser p( iss );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::begin_object );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::string );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::name_separator );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::json_true );

    ASSERT_EXCEPTION( p.next( &t ), unexpected_token );
  }
  {
    char const source[] = "[ 1";
    istringstream iss( source );
    parser p( iss );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::begin_array );

    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) );
    ASSERT_TRUE( t == token::number );

    ASSERT_EXCEPTION( p.next( &t ), unexpected_token );
  }
}

static void test_unterminated_string() {
  char const source[] = "  \"hello  ";
  istringstream iss( source );
  lexer lex( iss );
  token t;

  ASSERT_EXCEPTION( lex.next( &t ), unterminated_string );
}

struct error_loc_case {
  char const *source;
  bool multiple;                        // allow multiple top-level values
  char const *file;                     // set by set_loc(), if any
  char const *exception;                // json::exception subclass expected
  unsigned line, column, line_end, column_end;
};

static char const* exception_name( json::exception const &e ) {
  if ( dynamic_cast<illegal_character const*>( &e ) ) return "illegal_character";
  if ( dynamic_cast<illegal_codepoint const*>( &e ) ) return "illegal_codepoint";
  if ( dynamic_cast<illegal_escape const*>( &e ) ) return "illegal_escape";
  if ( dynamic_cast<illegal_literal const*>( &e ) ) return "illegal_literal";
  if ( dynamic_cast<illegal_number const*>( &e ) ) return "illegal_number";
  if ( dynamic_cast<unexpected_token const*>( &e ) ) return "unexpected_token";
  if ( dynamic_cast<unterminated_string const*>( &e ) ) return "unterminated_string";
  return "exception";
}

// Gives its text in get areas of at most chunk characters.
class chunked_streambuf : public std::streambuf {
public:
  chunked_streambuf( char const *s, size_t chunk ) : s_( s ), chunk_( chunk ) {
    setg( s_.data(), s_.data(), s_.data() );
  }
protected:
  int_type underflow() override {
    char *const end = s_.data() + s_.size();
    if ( egptr() == end )
      return traits_type::eof();
    setg( egptr(), egptr(), std::min( egptr() + chunk_, end ) );
    return traits_type::to_int_type( *gptr() );
  }
private:
  std::string s_;
  size_t chunk_;
};

static void check_error_loc( error_loc_case const &c, istream &is,
                             char const *how ) {
  parser p( is, c.multiple );
  if ( c.file )
    p.set_loc( c.file, 1, 1 );
  token t;
  try {
    while ( p.next( &t ) )
      ;
    cout << "FAILED (" << how << "): no error for " << c.source << endl;
    ++failures;
  }
  catch ( json::exception const &e ) {
    location const &loc = e.get_loc();
    char const *const file = loc.file() ? loc.file() : "";
    if ( strcmp( exception_name( e ), c.exception ) ||
         strcmp( file, c.file ? c.file : "" ) ||
         loc.line() != c.line || loc.column() != c.column ||
         loc.line_end() != c.line_end || loc.column_end() != c.column_end ) {
      cout << "FAILED (" << how << "): for " << c.source << ": got "
           << exception_name( e ) << " at \"" << file << "\" " << loc.line()
           << ',' << loc.column() << '-' << loc.line_end() << ','
           << loc.column_end() << endl;
      ++failures;
    }
  }
}

static void test_error_locations() {
  static error_loc_case const cases[] = {
    { "[\n  x ]", false, nullptr, "illegal_character", 2, 3, 2, 3 },
    { "[\t\tx]", false, nullptr, "illegal_character", 1, 4, 1, 4 },
    { "\r\n[\r\n 1,\r\n x]", false, nullptr, "illegal_character", 4, 2, 4, 2 },
    { "[\"ab\\x\"]", false, nullptr, "illegal_escape", 1, 6, 1, 6 },
    { "[\"\\u00G0\"]", false, nullptr, "illegal_codepoint", 1, 4, 1, 7 },
    { "[\"\\u12", false, nullptr, "illegal_codepoint", 1, 4, 1, 7 },
    { "[\"\\uD800\\u0041\"]", false, nullptr, "illegal_codepoint", 1, 4, 1, 14 },
    { "[\"\\uD800x\"]", false, nullptr, "illegal_codepoint", 1, 4, 1, 9 },
    { "[\"abc", false, nullptr, "unterminated_string", 1, 2, 1, 6 },
    { "[\n\"abc\ndef", false, nullptr, "unterminated_string", 2, 1, 3, 4 },
    { "[tru", false, nullptr, "illegal_literal", 1, 2, 1, 5 },
    { "[trux]", false, nullptr, "illegal_literal", 1, 2, 1, 5 },
    { "[nulls]", false, nullptr, "illegal_literal", 1, 2, 1, 6 },
    { "[-", false, nullptr, "illegal_number", 1, 2, 1, 3 },
    { "[-x]", false, nullptr, "illegal_number", 1, 2, 1, 3 },
    { "[01]", false, nullptr, "illegal_number", 1, 2, 1, 3 },
    { "[1.]", false, nullptr, "illegal_number", 1, 2, 1, 4 },
    { "[1.", false, nullptr, "illegal_number", 1, 2, 1, 4 },
    { "[1e]", false, nullptr, "illegal_number", 1, 2, 1, 4 },
    { "[1e+", false, nullptr, "illegal_number", 1, 2, 1, 5 },
    { "[12a]", false, nullptr, "illegal_number", 1, 2, 1, 4 },
    { "[1.5x]", false, nullptr, "illegal_number", 1, 2, 1, 5 },
    { "[1e5x]", false, nullptr, "illegal_number", 1, 2, 1, 5 },
    { "{\"a\" 1}", false, nullptr, "unexpected_token", 1, 6, 1, 6 },
    { "[1 2]", false, nullptr, "unexpected_token", 1, 4, 1, 4 },
    { "{\"a\":1,", false, nullptr, "unexpected_token", 1, 7, 1, 7 },
    { "{\"a\":1}\n{\"b\":tru", true, nullptr, "illegal_literal", 2, 6, 2, 9 },
    { "{\"a\":1}\n{\"b\":tru", true, "events.jsonl", "illegal_literal", 2, 6, 2, 9 },
    { "{\"a\":1}\n{\"b\":\"x", true, "events.jsonl", "unterminated_string", 2, 6, 2, 8 },
    { "[1", false, nullptr, "unexpected_token", 1, 2, 1, 2 },
    { "[1,", false, nullptr, "unexpected_token", 1, 3, 1, 3 },
    { "{", false, nullptr, "unexpected_token", 1, 1, 1, 1 },
    { "{\"a\"", false, nullptr, "unexpected_token", 1, 2, 1, 4 },
    { "{\"a\":", false, nullptr, "unexpected_token", 1, 2, 1, 4 },
    { "[[1]", false, nullptr, "unexpected_token", 1, 4, 1, 4 },
    { "[{\"a\":[1,{\"b\":2}],\n\"c\":}]", false, nullptr, "unexpected_token", 2, 5, 2, 5 },
    { "[\"a\nb\\q\"]", false, nullptr, "illegal_escape", 2, 3, 2, 3 },
    { "[\"\\n\nx", false, nullptr, "unterminated_string", 1, 2, 2, 2 },
    { "[123456789012345678901234567890x]", false, nullptr, "illegal_number", 1, 2, 1, 32 },
    { "[1]\n[2]\n[3,\n", true, nullptr, "unexpected_token", 3, 3, 3, 3 },
  };

  for ( error_loc_case const &c : cases ) {
    {
      istringstream iss( c.source );
      check_error_loc( c, iss, "istringstream" );
    }
    for ( size_t chunk : { 1, 2, 3, 5 } ) {
      chunked_streambuf buf( c.source, chunk );
      istream is( &buf );
      check_error_loc( c, is, "chunks" );
    }
  }
  {
    // columns wrap as their type does on a line longer than it counts
    std::string const source = "[" + std::string( 70000, ' ' ) + "x]";
    error_loc_case const c = {
      source.c_str(), false, nullptr, "illegal_character", 1, 4466, 1, 4466
    };
    istringstream iss( source );
    check_error_loc( c, iss, "istringstream" );
  }
}

// Gives its text, then throws instead of reporting the end.
class throwing_streambuf : public std::streambuf {
public:
  explicit throwing_streambuf( char const *s ) : s_( s ) {
    setg( s_.data(), s_.data(), s_.data() + s_.size() );
  }
protected:
  int_type underflow() override {
    throw std::runtime_error( "buffer failure" );
  }
private:
  std::string s_;
};

// Has no get area and reports how many characters it holds, so that the lexer
// reads several in one go; throws after giving `given` of them.
class failing_after_streambuf : public std::streambuf {
public:
  failing_after_streambuf( char const *s, size_t given ) :
    s_( s ), given_( given ) {}
protected:
  std::streamsize showmanyc() override {
    return static_cast<std::streamsize>( s_.size() - pos_ );
  }
  int_type underflow() override {
    if ( pos_ == given_ )
      throw std::runtime_error( "buffer failure" );
    return traits_type::to_int_type( s_[ pos_ ] );
  }
  int_type uflow() override {
    int_type const c = underflow();
    ++pos_;
    return c;
  }
private:
  std::string s_;
  size_t given_;
  size_t pos_ = 0;
};

static void test_throwing_streambuf_mid_read() {
  {
    // the characters given before the failure are lexed, then badbit is set
    failing_after_streambuf buf( "[1,2]", 3 );
    istream is( &buf );
    parser p( is );
    token t;
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_array );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::number );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::value_separator );
    ASSERT_EXCEPTION( p.next( &t ), unexpected_token );
    ASSERT_TRUE( is.bad() );
  }
  {
    // with badbit in the exception mask, the exception propagates after them
    failing_after_streambuf buf( "[1,2]", 3 );
    istream is( &buf );
    is.exceptions( ios::badbit );
    parser p( is );
    token t;
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_array );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::number );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::value_separator );
    ASSERT_EXCEPTION( p.next( &t ), std::runtime_error );
    ASSERT_TRUE( is.bad() );
  }
}

static void test_throwing_streambuf() {
  {
    // as istream::get() does: the exception sets badbit and reads as the end
    throwing_streambuf buf( "[1," );
    istream is( &buf );
    parser p( is );
    token t;
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_array );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::number );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::value_separator );
    ASSERT_EXCEPTION( p.next( &t ), unexpected_token );
    ASSERT_TRUE( is.bad() );
  }
  {
    // with badbit in the exception mask, the buffer's exception propagates
    throwing_streambuf buf( "[1," );
    istream is( &buf );
    is.exceptions( ios::badbit );
    parser p( is );
    token t;
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_array );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::number );
    ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::value_separator );
    ASSERT_EXCEPTION( p.next( &t ), std::runtime_error );
    ASSERT_TRUE( is.bad() );
  }
}

static void test_stream_state_at_end() {
  {
    // reading past the end sets eofbit and failbit, as istream::get() does
    istringstream iss( "[1]" );
    parser p( iss, true );
    token t;
    while ( p.next( &t ) )
      ;
    ASSERT_TRUE( iss.eof() );
    ASSERT_TRUE( iss.fail() );
    ASSERT_TRUE( !iss.bad() );
  }
  {
    // peeking at the end sets eofbit only, as istream::peek() does
    istringstream iss( "12" );
    lexer lex( iss );
    token t;
    ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) && t == token::number );
    ASSERT_TRUE( iss.eof() );
    ASSERT_TRUE( !iss.fail() );
  }
}

// Each token as text: type, numeric type, value and location.
static vector<std::string> tokens_of( istream &is, bool multiple ) {
  vector<std::string> result;
  parser p( is, multiple );
  token t;
  while ( p.next( &t ) ) {
    location const loc( t.get_loc() );
    ostringstream oss;
    oss << t.get_type() << ' ' << t.get_numeric_type() << " [" << t.text()
        << "] " << loc.line() << ',' << loc.column() << '-' << loc.line_end()
        << ',' << loc.column_end();
    result.push_back( oss.str() );
  }
  return result;
}

static void test_buffer_boundaries() {
  static char const source[] =
    "{\"key\":\"value\",\"n\":-12.5e+3,\"esc\":\"a\\\"b\\u00e9\\nc\",\n"
    " \"multi\nline\":[true,false,null,0,1.25,\"\"],\"x\":{}}\r\n"
    "{\"key\":\"value2\",\"n\":7}\n";
  istringstream iss( source );
  vector<std::string> const expected( tokens_of( iss, true ) );
  ASSERT_TRUE( expected.size() == 43 );
  for ( size_t chunk = 1; chunk <= 9; ++chunk ) {
    chunked_streambuf buf( source, chunk );
    istream is( &buf );
    ASSERT_TRUE( tokens_of( is, true ) == expected );
  }

  // values across the end of the lexer's largest buffer (64 KiB)
  std::string big( "[" );
  while ( big.size() < 200000 )
    big += "\"abcdefghijklmnopqrstuvwxyz\",1234567.890123,\"\\u0041x\",";
  big += "0]";
  istringstream big_iss( big );
  vector<std::string> const big_tokens( tokens_of( big_iss, false ) );
  chunked_streambuf big_buf( big.c_str(), 4093 );
  istream big_is( &big_buf );
  ASSERT_TRUE( tokens_of( big_is, false ) == big_tokens );
  size_t values = 0;
  for ( std::string const &t : big_tokens )
    values += t.find( " [abcdefghijklmnopqrstuvwxyz] " ) != std::string::npos ||
              t.find( " [1234567.890123] " ) != std::string::npos ||
              t.find( " [Ax] " ) != std::string::npos;
  ASSERT_TRUE( values == ( big_tokens.size() - 3 ) / 2 );
}

static void test_token_copy_holds_value() {
  istringstream iss( "[\"first\",\"second\"]" );
  lexer lex( iss );
  token t;
  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  token const copy( t );
  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE_AND_NO_EXCEPTION( lex.next( &t ) );
  ASSERT_TRUE( copy.get_value() == "first" );
  ASSERT_TRUE( t.get_value() == "second" );
}

static void test_sync() {
  istringstream iss( "[1] [22]  {}" );
  parser p( iss, true );
  token t;
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_array );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::number );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::end_array );
  p.sync();
  ASSERT_TRUE( iss.tellg() == streampos( 3 ) );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_array );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t.get_value() == "22" );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::end_array );
  p.sync();
  ASSERT_TRUE( iss.tellg() == streampos( 8 ) );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::begin_object );
  ASSERT_TRUE_AND_NO_EXCEPTION( p.next( &t ) && t == token::end_object );
}

///////////////////////////////////////////////////////////////////////////////

namespace zorba {
namespace UnitTests {

int test_json_parser( int, char*[] ) {

  // lexer-only tests
  test_lexer_array();
  test_lexer_object();
  test_illegal_character();
  test_illegal_codepoint();
  test_illegal_escape();
  test_illegal_literal();
  test_illegal_number();
  test_unterminated_string();

  // parser tests
  test_empty_stream();
  test_parser_array();
  test_parser_object();
  test_unexpected_token();
  test_json_org_example();
  test_error_locations();
  test_throwing_streambuf();
  test_throwing_streambuf_mid_read();
  test_stream_state_at_end();
  test_buffer_boundaries();
  test_token_copy_holds_value();
  test_sync();

  cout << failures << " test(s) failed\n";
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
