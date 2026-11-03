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
#include "query_hints.h"

#include <algorithm>
#include <cstring>

namespace zorba {
namespace query_hints {

namespace {

char const use_jsoniq[] = "start the query with jsoniq version \"1.0\";";
char const math_ns[] = "http://www.w3.org/2005/xpath-functions/math";

bool is_name_char( char c ) {
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
    ( c >= '0' && c <= '9' ) || c == '_' || c == '-' || c == '.' ||
    static_cast<unsigned char>( c ) >= 0x80;
}

bool is_digit( char c ) {
  return c >= '0' && c <= '9';
}

bool is_space( char c ) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// The byte offset of a location, or npos if the query has no such location.
std::size_t offset_of( std::string const &query, unsigned long line,
                       unsigned long column ) {
  if ( !line || !column )
    return std::string::npos;
  std::size_t begin = 0;
  for ( unsigned long l = 1; l < line; ++l ) {
    begin = query.find( '\n', begin );
    if ( begin == std::string::npos )
      return std::string::npos;
    ++begin;
  }
  // The end of the query can be reported past its last character.
  return std::min( begin + column - 1, query.size() );
}

// The word that ends right before the offset, skipping whitespace.
std::string word_before( std::string const &query, std::size_t end ) {
  while ( end && is_space( query[ end - 1 ] ) )
    --end;
  std::size_t begin = end;
  while ( begin && is_name_char( query[ begin - 1 ] ) )
    --begin;
  return query.substr( begin, end - begin );
}

// "$a2-$a1": the scanner reads the name "a2-", and the error is at "$a1".
std::string dash_before_variable( std::string const &query,
                                  std::size_t offset ) {
  std::size_t dash = offset;              // "$a2- $a1" is the same mistake
  while ( dash && is_space( query[ dash - 1 ] ) )
    --dash;
  if ( dash < 2 || query[ dash - 1 ] != '-' )
    return std::string();
  --dash;

  std::size_t begin = dash;
  while ( begin && ( is_name_char( query[ begin - 1 ] ) ||
                     query[ begin - 1 ] == ':' ) )
    --begin;
  if ( begin == dash || is_digit( query[ begin ] ) ||
       query[ begin ] == '.' || query[ begin ] == '-' )
    return std::string();                 // no name: "1-$a", "(-$a"
  std::string left( query.substr( begin, dash - begin ) );

  bool const is_variable = begin && query[ begin - 1 ] == '$';
  if ( is_variable ) {
    // A variable that is being bound can not be an expression.
    std::string const word( word_before( query, begin - 1 ) );
    for ( char const *binder : { "let", "for", "variable", "at", "count" } )
      if ( word == binder )
        return std::string();
    left.insert( 0, 1, '$' );
  }

  std::size_t end = offset + 1;
  while ( end < query.size() && ( is_name_char( query[ end ] ) ||
                                  query[ end ] == ':' ) )
    ++end;
  while ( end > offset + 1 && ( query[ end - 1 ] == '-' ||
                                query[ end - 1 ] == '.' ) )
    --end;
  if ( end == offset + 1 )
    return std::string();
  return "\"" + left + "-\" is one name, because \"-\" is a name character. "
    "Write " + left + " - " + query.substr( offset, end - offset ) + ".";
}

bool starts_with( std::string const &s, char const *prefix ) {
  return s.compare( 0, std::strlen( prefix ), prefix ) == 0;
}

// The JSONiq scanner ends an unterminated string right before a "\" that does
// not start an escape, or at the end of the query. The message of the parser
// has the text it read, and the location is the start of that text.
std::string invalid_escape( std::string const &query, std::size_t offset,
                            std::string const &message,
                            std::size_t prefix_size ) {
  std::string const text( message.substr( prefix_size,
                                          message.size() - prefix_size - 1 ) );
  if ( text.empty() || text[0] != '"' )
    return std::string();                 // single quotes are the problem
  // On the first line the reported column is one too far.
  for ( std::size_t const at : { offset, offset ? offset - 1 : offset } ) {
    std::size_t const end = at + text.size();
    if ( end < query.size() && query.compare( at, text.size(), text ) == 0 &&
         query[ end ] == '\\' )
      return "In a JSONiq string \"\\\" starts an escape (\\\" \\\\ \\/ \\b \\f "
        "\\n \\r \\t \\uXXXX), and this string has one that is not valid. For a "
        "backslash write \\\\.";
  }
  return std::string();
}

} // namespace

std::string syntax_error( std::string const &query, unsigned long line,
                          unsigned long column, bool xquery_grammar,
                          std::string const &message ) {
  // The token is known from the message, whatever precedes it.
  if ( !xquery_grammar &&
       starts_with( message, "syntax error, unexpected \".\"" ) )
    return "In JSONiq the context item is $$, not \".\". Write $$, or start the "
      "query with xquery version \"3.0\"; to parse it as XQuery, where \".\" is "
      "the context item.";

  std::size_t offset = offset_of( query, line, column );
  if ( offset == std::string::npos )
    return std::string();

  char const unterminated[] = "syntax error, unterminated string literal \"";
  if ( !xquery_grammar && starts_with( message, unterminated ) )
    return invalid_escape( query, offset, message, sizeof unterminated - 1 );

  if ( query.find_first_not_of( " \t\r\n", offset ) == std::string::npos ) {
    // An error at the end of the query: look at the last token. "$a-$b" at the
    // very end is reported here, not at "$b".
    offset = query.size();
    while ( offset && is_space( query[ offset - 1 ] ) )
      --offset;
    if ( !offset )
      return std::string();
    --offset;
    if ( query[ offset ] != ']' ) {
      while ( offset && is_name_char( query[ offset ] ) )
        --offset;
      if ( query[ offset ] != '$' )
        return std::string();
    }
  } else if ( query[ offset ] != ']' && query[ offset ] != '$' && offset &&
              ( query[ offset - 1 ] == ']' || query[ offset - 1 ] == '$' ) ) {
    // On the first line the reported column is one too far.
    --offset;
  }

  if ( query[ offset ] == ']' ) {
    std::size_t prev = offset;
    while ( prev && is_space( query[ prev - 1 ] ) )
      --prev;
    if ( xquery_grammar && prev && query[ prev - 1 ] == '[' )
      return std::string( "\"[]\" is JSONiq syntax and this query is XQuery. "
        "Write $x() or jn:members($x) instead of $x[], or " ) + use_jsoniq;
    return std::string();
  }

  if ( query[ offset ] == '$' ) {
    // The scanner reads "$$" as two tokens, and the error is at the second.
    if ( xquery_grammar && offset && query[ offset - 1 ] == '$' )
      return std::string( "\"$$\" is the JSONiq context item and this query "
        "is XQuery. Write \".\" instead of \"$$\", or " ) + use_jsoniq;
    return dash_before_variable( query, offset );
  }

  return std::string();
}

std::string jsoniq_function( std::string const &local,
                             std::string const &prefix ) {
  std::string const call( prefix + ':' + local + "(...)" );
  if ( prefix == "math" )
    return "\"" + local + "\" is in the math namespace and this query is "
      "XQuery, where a function without a prefix is in fn:. Write " + call +
      " with import module namespace math = "
      "\"http://www.w3.org/2005/xpath-functions/math\"; in the prolog, or " +
      use_jsoniq;
  return "\"" + local + "\" is a JSONiq function and this query is XQuery, "
    "where a function without a prefix is in fn:. Write " + call + ", or " +
    use_jsoniq;
}

std::string empty_array_lookup() {
  return "\"[[ ]]\" selects one member and needs a position. For all the "
    "members write $x() or jn:members($x).";
}

std::string other_arity( std::string const &name,
                         std::vector<std::string> const &signatures ) {
  if ( signatures.empty() )
    return std::string();
  std::string hint( "\"" + name + "\" is declared as " );
  for ( std::size_t i = 0; i < signatures.size(); ++i ) {
    if ( i )
      hint += " and as ";
    hint += signatures[i];
  }
  return hint + ".";
}

std::string dash_in_variable( std::string const &left,
                              std::string const &right ) {
  std::string const name( "$" + left + "-" + right );
  if ( right.empty() )
    return "\"" + name + "\" is one name, because \"-\" is a name character. "
      "Put a space before the \"-\": $" + left + " - ...";
  return "\"" + name + "\" is one name, because \"-\" is a name character. "
    "Write $" + left + " - " + right + ".";
}

std::string undeclared_prefix( std::string const &prefix ) {
  if ( prefix == "math" )
    return "Add import module namespace math = "
      "\"http://www.w3.org/2005/xpath-functions/math\"; to the prolog.";
  if ( prefix == "map" )
    return "Zorba has no XQuery 3.1 map: functions. Use a JSON object, for "
      "example {| for $x in $s return { $x(\"key\") : $x } |} instead of "
      "map:merge().";
  return std::string();
}

std::string key_on_array( std::string const &key, bool jsoniq ) {
  std::string const lookup( "(\"" + key + "\")" );
  return "This item is an array, and " + lookup + " selects a member by "
    "integer position. To read this key from its members write $x()" + lookup +
    ( jsoniq ? " or $x[]" + lookup : std::string() ) +
    ", or $x(1)" + lookup + " for the first member; a predicate such as [1] "
    "filters the sequence and does not open the array.";
}

bool is_math_uri( std::string const &uri ) {
  char const end[] = "/math";
  std::size_t const size = sizeof end - 1;
  return uri.size() >= size &&
    uri.compare( uri.size() - size, size, end ) == 0;
}

std::string unknown_module( std::string const &uri, bool fetch_denied ) {
  std::string hint;
  if ( fetch_denied )
    hint = "Zorba could not load this module from any location it may read, "
      "and the security policy denies fetching it from this URI, which may "
      "hold no module at all. Check the URI.";
  if ( is_math_uri( uri ) ) {
    if ( !hint.empty() )
      hint += ' ';
    hint += std::string( "The functions pi, sin, cos and the like are in " ) +
      math_ns + ".";
  }
  return hint;
}

std::string math_function( std::string const &local, bool prefix_bound ) {
  std::string hint( "\"" + local + "\" is in " + math_ns + ", not in this "
    "namespace. Write math:" + local + "(...)" );
  if ( !prefix_bound )
    hint += std::string( " with import module namespace math = \"" ) + math_ns +
      "\"; in the prolog";
  return hint + ".";
}

std::string path_over_item( std::string const &value_type ) {
  char const object[] = "object";
  char const array[] = "array";
  if ( value_type.compare( 0, std::strlen( object ), object ) == 0 )
    return "The left side of this path is a JSON object, and path steps apply "
      "only to nodes. Select the node it holds first, for example "
      "$d(\"content\")//item.";
  if ( value_type.compare( 0, std::strlen( array ), array ) == 0 )
    return "The left side of this path is a JSON array, and path steps apply "
      "only to nodes. Read its members with $a() and select the nodes they "
      "hold.";
  return std::string();
}

std::string node_in_json() {
  return "The result holds an XML node, and the JSON output method can not "
    "represent nodes. Return serialize($node) for its markup or string($node) "
    "for its text, or return only the fields you need, such as $d(\"path\").";
}

} // namespace query_hints
} // namespace zorba
/* vim:set et sw=2 ts=2: */
