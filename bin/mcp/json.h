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

#ifndef ZORBA_MCP_JSON_H
#define ZORBA_MCP_JSON_H

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace zorba {
namespace mcp {

/**
 * A JSON value for the MCP protocol messages: small, self-contained and
 * strict (RFC 8259). Numbers keep their source text, so a request id is
 * echoed exactly as received; object members keep their order.
 */
class Json {
public:
  enum class Type { Null, Bool, Number, String, Array, Object };
  typedef std::vector<Json> array_type;
  typedef std::vector<std::pair<std::string,Json> > object_type;

  Json() : theType( Type::Null ), theBool( false ) { }
  Json( std::nullptr_t ) : Json() { }
  Json( bool b ) : theType( Type::Bool ), theBool( b ) { }
  Json( int n ) : Json( static_cast<long long>( n ) ) { }
  Json( long n ) : Json( static_cast<long long>( n ) ) { }
  Json( unsigned n ) : Json( static_cast<unsigned long long>( n ) ) { }
  Json( long long n );
  Json( unsigned long n ) : Json( static_cast<unsigned long long>( n ) ) { }
  Json( unsigned long long n );
  Json( char const *s ) : theType( Type::String ), theBool( false ),
    theText( s ) { }
  Json( std::string s ) : theType( Type::String ), theBool( false ),
    theText( std::move( s ) ) { }

  static Json array() { Json j; j.theType = Type::Array; return j; }
  static Json object() { Json j; j.theType = Type::Object; return j; }
  /** An object from (name, value) pairs. */
  static Json object( std::initializer_list<std::pair<std::string,Json> > m );
  /** A number from its JSON text; the caller guarantees the syntax. */
  static Json number_text( std::string text );

  Type type() const { return theType; }
  bool is_null() const { return theType == Type::Null; }
  bool is_bool() const { return theType == Type::Bool; }
  bool is_number() const { return theType == Type::Number; }
  bool is_string() const { return theType == Type::String; }
  bool is_array() const { return theType == Type::Array; }
  bool is_object() const { return theType == Type::Object; }
  /** A number without fraction or exponent. */
  bool is_integer() const;

  bool as_bool() const { return theBool; }
  /** The string's value, or a number's JSON text. */
  std::string const& text() const { return theText; }
  array_type const& items() const { return theArray; }
  object_type const& members() const { return theObject; }

  /** The member named \a name (the last one if repeated), or nullptr. */
  Json const* find( std::string const &name ) const;
  /** Appends to an array. */
  Json& push( Json v );
  /** Sets a member of an object, replacing an existing one. */
  Json& set( std::string const &name, Json v );

  /** Compact JSON text, without newlines (one stdio message per line). */
  std::string dump() const;

private:
  void dump( std::string &out ) const;

  Type        theType;
  bool        theBool;
  std::string theText;
  array_type  theArray;
  object_type theObject;
};

/** Limits that bound the parser's work on untrusted input. */
struct JsonLimits {
  std::size_t max_depth = 64;
};

/**
 * Parses one JSON text (surrounding whitespace allowed).
 *
 * @return \c true on success; otherwise \c false and \a error holds a short
 * reason with the byte offset. Never throws for bad input.
 */
bool parse_json( std::string const &text, Json *result, std::string *error,
                 JsonLimits const &limits = JsonLimits() );

/** Appends \a s to \a out as a JSON string literal. */
void append_json_string( std::string &out, std::string const &s );

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_JSON_H */
/* vim:set et sw=2 ts=2: */
