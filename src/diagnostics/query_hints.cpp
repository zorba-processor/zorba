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
#include <cctype>
#include <cstring>

namespace zorba {
namespace query_hints {

namespace {

char const math_ns[] = "http://www.w3.org/2005/xpath-functions/math";
char const zorba_math_ns[] = "http://zorba.io/modules/math";

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

// Whether a version declaration that starts with this word is at this offset.
bool declaration_at( std::string const &query, std::size_t pos,
                     std::string const &word ) {
  if ( query.compare( pos, word.size(), word ) != 0 )
    return false;
  pos += word.size();
  if ( pos == query.size() || !is_space( query[ pos ] ) )
    return false;
  while ( pos < query.size() && is_space( query[ pos ] ) )
    ++pos;
  return query.compare( pos, 7, "version" ) == 0 ||
    query.compare( pos, 8, "encoding" ) == 0;
}

// Whether the first thing in the query, after whitespace and comments, is a
// version declaration that starts with this word.
bool declares( std::string const &query, std::string const &word ) {
  std::size_t pos = 0;
  while ( pos < query.size() ) {
    if ( is_space( query[ pos ] ) ) {
      ++pos;
    } else if ( query.compare( pos, 2, "(:" ) == 0 ) {
      int depth = 1;
      for ( pos += 2; pos < query.size() && depth; ) {
        if ( query.compare( pos, 2, "(:" ) == 0 ) {
          ++depth;
          pos += 2;
        } else if ( query.compare( pos, 2, ":)" ) == 0 ) {
          --depth;
          pos += 2;
        } else {
          ++pos;
        }
      }
    } else {
      break;
    }
  }
  return declaration_at( query, pos, word );
}

// How to give the query the other language. The grammar is chosen from the first
// bytes of the query and accepts only its own declaration, so the new declaration
// has to be the very first characters; and a second one would be another error.
std::string change_language( std::string const &query,
                             std::string const &from,
                             std::string const &declaration ) {
  if ( declaration_at( query, 0, from ) )
    return "replace the " + from + " version declaration with " + declaration;
  if ( declares( query, from ) )
    return "put " + declaration + " in place of the " + from + " version "
      "declaration, as the very first characters of the query, before any "
      "comment.";
  return "start the query with " + declaration + " as its very first "
    "characters, before any comment.";
}

std::string use_jsoniq( std::string const &query ) {
  return change_language( query, "xquery", "jsoniq version \"1.0\";" );
}

// The offset right after the comments and whitespace that end before this one,
// going back: the end of what precedes them, or 0.
std::size_t before_comments( std::string const &query, std::size_t end ) {
  for ( ;; ) {
    while ( end && is_space( query[ end - 1 ] ) )
      --end;
    if ( end < 2 || query.compare( end - 2, 2, ":)" ) != 0 )
      return end;
    int depth = 1;
    for ( end -= 2; end >= 2 && depth; ) {
      if ( query.compare( end - 2, 2, ":)" ) == 0 ) {
        ++depth;
        end -= 2;
      } else if ( query.compare( end - 2, 2, "(:" ) == 0 ) {
        --depth;
        end -= 2;
      } else {
        --end;
      }
    }
    if ( depth )
      return end;                         // not a comment after all
  }
}

// Whether the "." at this location comes right after an expression or a type,
// where the author can not have meant a context item.
bool dot_after_expression( std::string const &query, std::size_t offset ) {
  if ( offset == std::string::npos )
    return false;
  // On the first line the reported column is one too far.
  std::size_t dot = offset;
  if ( dot == query.size() || query[ dot ] != '.' ) {
    if ( !dot || query[ dot - 1 ] != '.' )
      return false;
    --dot;
  }
  dot = before_comments( query, dot );
  if ( !dot )
    return false;
  char const c = query[ dot - 1 ];
  if ( c == ')' || c == ']' || c == '}' || c == '"' || c == '\'' )
    return true;
  if ( !is_name_char( c ) || c == '-' )
    return false;
  std::string const word( word_before( query, dot ) );
  for ( char const *const keyword : {
          "and", "or", "not", "eq", "ne", "lt", "le", "gt", "ge", "is", "div",
          "idiv", "mod", "to", "union", "intersect", "except", "return", "then",
          "else", "in", "where", "by", "when", "satisfies" } )
    if ( word == keyword )
      return false;
  return true;
}

// Whether the "\" at this offset does not start an escape of the JSONiq scanner.
bool is_invalid_escape( std::string const &query, std::size_t backslash ) {
  std::size_t const next = backslash + 1;
  if ( next == query.size() )
    return true;
  if ( std::strchr( "\"'\\/bfnrt", query[ next ] ) && query[ next ] )
    return false;
  if ( query[ next ] != 'u' )
    return true;
  if ( query.size() - next < 5 )
    return true;
  for ( std::size_t i = next + 1; i <= next + 4; ++i )
    if ( !std::isxdigit( static_cast<unsigned char>( query[ i ] ) ) )
      return true;
  return false;
}

// The JSONiq scanner ends an unterminated string right before a "\" that does
// not start an escape, or at the end of the query. The message of the parser
// has the text it read, and the end of the location is the end of that text,
// where the "\" is if there is one. (The start of the location is not the start
// of the text when the literal spans lines.)
std::string invalid_escape( std::string const &query, std::size_t end,
                            std::string const &message,
                            std::size_t prefix_size ) {
  std::string const text( message.substr( prefix_size,
                                          message.size() - prefix_size - 1 ) );
  if ( text.empty() || text[0] != '"' || end == std::string::npos )
    return std::string();                 // single quotes are the problem
  // On the first line the reported column is one too far.
  bool found = false;
  for ( std::size_t const at : { end, end ? end - 1 : end } )
    found = found || ( at < query.size() && at >= text.size() &&
                       query[ at ] == '\\' &&
                       query.compare( at - text.size(), text.size(), text ) == 0 &&
                       is_invalid_escape( query, at ) );
  if ( !found )
    return std::string();                 // it ran to the end of the query
  return "In a JSONiq string \"\\\" starts an escape (\\\" \\' \\\\ \\/ \\b "
    "\\f \\n \\r \\t \\uXXXX), and this string has one that is not valid. For "
    "a backslash write \\\\.";
}

} // namespace

std::string syntax_error( std::string const &query, unsigned long line,
                          unsigned long column, unsigned long line_end,
                          unsigned long column_end, bool xquery_grammar,
                          std::string const &message ) {
  std::size_t offset = offset_of( query, line, column );

  // The token is known from the message; what precedes it says whether the
  // context item is what the author meant.
  if ( !xquery_grammar &&
       starts_with( message, "syntax error, unexpected \".\"" ) ) {
    if ( dot_after_expression( query, offset ) )
      return std::string();
    return std::string( "In JSONiq the context item is $$, not \".\". Write $$. "
      "To parse it as XQuery, where \".\" is the context item, " ) +
      change_language( query, "jsoniq", "xquery version \"3.0\";" );
  }

  char const unterminated[] = "syntax error, unterminated string literal \"";
  if ( !xquery_grammar && starts_with( message, unterminated ) )
    return invalid_escape( query, offset_of( query, line_end, column_end ),
                           message, sizeof unterminated - 1 );

  if ( offset == std::string::npos )
    return std::string();

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
        "Write $x() or jn:members($x) instead of $x[], or " ) +
        use_jsoniq( query );
    return std::string();
  }

  if ( query[ offset ] == '$' ) {
    // The scanner reads "$$" as two tokens, and the error is at the second.
    if ( xquery_grammar && offset && query[ offset - 1 ] == '$' )
      return std::string( "\"$$\" is the JSONiq context item and this query "
        "is XQuery. Write \".\" instead of \"$$\", or " ) +
        use_jsoniq( query );
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
      "\"http://www.w3.org/2005/xpath-functions/math\"; in the prolog, or "
      "start the query with jsoniq version \"1.0\"; as its very first "
      "characters, before any comment.";
  return "\"" + local + "\" is a JSONiq function and this query is XQuery, "
    "where a function without a prefix is in fn:. Write " + call + ", or "
    "start the query with jsoniq version \"1.0\"; as its very first characters, "
    "before any comment.";
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
  // Only "-" after it: the operand is still to come.
  if ( right.find_first_not_of( '-' ) == std::string::npos )
    return "\"" + name + "\" is one name, because \"-\" is a name character. "
      "Put a space before the \"-\": $" + left + " - " + right + "...";
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

namespace {

// A selector that reads the same between double quotes once decoded.
bool reads_as_literal( std::string const &key ) {
  for ( char const c : key ) {
    unsigned char const u = static_cast<unsigned char>( c );
    if ( c == '"' || c == '\\' || u < 0x20 || u == 0x7F )
      return false;
  }
  return true;
}

} // namespace

std::string key_on_array( std::string const &key, bool jsoniq ) {
  std::string const lookup(
    "(\"" + ( reads_as_literal( key ) ? key : std::string( "key" ) ) + "\")" );
  return "This item is an array, and " + lookup + " selects a member by "
    "integer position. To read this key from its members write $x()" + lookup +
    ( jsoniq ? " or $x[]" + lookup : std::string() ) +
    ", or $x(1)" + lookup + " for the first member; a predicate such as [1] "
    "filters the sequence and does not open the array.";
}

namespace {

// A URI without its scheme and "www.": http or https, with or without www, is
// the same mistake.
std::string without_scheme( std::string const &uri ) {
  std::size_t begin = 0;
  for ( char const *const scheme : { "http://", "https://" } )
    if ( starts_with( uri, scheme ) )
      begin = std::strlen( scheme );
  if ( uri.compare( begin, 4, "www." ) == 0 )
    begin += 4;
  return uri.substr( begin );
}

bool is_w3c_math_uri( std::string const &uri ) {
  return without_scheme( uri ) == "w3.org/2005/xpath-functions/math";
}

// The Zorba math module, and its old namespace.
bool is_zorba_math_uri( std::string const &uri ) {
  std::string const name( without_scheme( uri ) );
  return name == "zorba.io/modules/math" ||
    name == "zorba-xquery.com/modules/math";
}

} // namespace

bool is_math_uri( std::string const &uri ) {
  return is_w3c_math_uri( uri ) || is_zorba_math_uri( uri );
}

std::string unknown_module( std::string const &uri, bool fetch_denied ) {
  std::string hint;
  if ( fetch_denied )
    hint = "Zorba could not load this module from any location it may read, "
      "and the security policy denies fetching it from this URI, which may "
      "hold no module at all. Check the URI.";
  std::string pointer;
  if ( is_w3c_math_uri( uri ) )
    pointer = std::string( "The functions pi, sin, cos and the like are in " ) +
      math_ns + ".";
  else if ( is_zorba_math_uri( uri ) && uri != zorba_math_ns )
    pointer = std::string( "The Zorba math module is " ) + zorba_math_ns + ".";
  if ( !pointer.empty() && !hint.empty() )
    hint += ' ';
  return hint + pointer;
}

std::string math_function( std::string const &local, bool prefix_bound,
                           bool prefix_taken ) {
  std::string hint( "\"" + local + "\" is in " + math_ns + ", not in this "
    "namespace. " );
  if ( prefix_taken )
    return hint + "The prefix math is bound to another namespace here: replace "
      "that binding, or import this one under another prefix: import module "
      "namespace mf = \"" + math_ns + "\"; and write mf:" + local + "(...).";
  hint += "Write math:" + local + "(...)";
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
