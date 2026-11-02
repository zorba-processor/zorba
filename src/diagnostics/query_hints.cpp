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

#include <cstring>

namespace zorba {
namespace query_hints {

namespace {

char const use_jsoniq[] = "start the query with jsoniq version \"1.0\";";

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
  std::size_t const offset = begin + column - 1;
  return offset < query.size() ? offset : std::string::npos;
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
  if ( offset < 2 || query[ offset - 1 ] != '-' )
    return std::string();

  std::size_t begin = offset - 1;
  while ( begin && ( is_name_char( query[ begin - 1 ] ) ||
                     query[ begin - 1 ] == ':' ) )
    --begin;
  if ( begin == offset - 1 || is_digit( query[ begin ] ) ||
       query[ begin ] == '.' || query[ begin ] == '-' )
    return std::string();                 // no name: "1-$a", "(-$a"
  std::string left( query.substr( begin, offset - 1 - begin ) );

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

} // namespace

std::string syntax_error( std::string const &query, unsigned long line,
                          unsigned long column, bool xquery_grammar ) {
  std::size_t offset = offset_of( query, line, column );
  if ( offset == std::string::npos )
    return std::string();

  // After a prolog on the same line the reported column can be one too far.
  if ( query[ offset ] != ']' && query[ offset ] != '$' && offset &&
       ( query[ offset - 1 ] == ']' || query[ offset - 1 ] == '$' ) )
    --offset;

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

std::string key_on_array( std::string const &key ) {
  std::string const lookup( "(\"" + key + "\")" );
  return "This item is an array, and " + lookup + " selects a member by "
    "integer position. To read this key from its members write $x()" + lookup +
    ", or $x(1)" + lookup + " for the first member; a predicate such as [1] "
    "filters the sequence and does not open the array.";
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
