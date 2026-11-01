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

#include "json.h"

#include <cstdio>

namespace zorba {
namespace mcp {

///////////////////////////////////////////////////////////////////////////////
// Json

Json::Json( long long n ) : theType( Type::Number ), theBool( false ),
  theText( std::to_string( n ) ) {
}

Json::Json( unsigned long long n ) : theType( Type::Number ), theBool( false ),
  theText( std::to_string( n ) ) {
}

Json Json::object( std::initializer_list<std::pair<std::string,Json> > m ) {
  Json j( object() );
  for ( auto const &member : m )
    j.set( member.first, member.second );
  return j;
}

Json Json::number_text( std::string text ) {
  Json j;
  j.theType = Type::Number;
  j.theText = std::move( text );
  return j;
}

bool Json::is_integer() const {
  return theType == Type::Number &&
    theText.find_first_of( ".eE" ) == std::string::npos;
}

Json const* Json::find( std::string const &name ) const {
  if ( theType != Type::Object )
    return nullptr;
  for ( auto i = theObject.rbegin(); i != theObject.rend(); ++i )
    if ( i->first == name )
      return &i->second;
  return nullptr;
}

Json& Json::push( Json v ) {
  theArray.push_back( std::move( v ) );
  return theArray.back();
}

Json& Json::set( std::string const &name, Json v ) {
  for ( auto &member : theObject ) {
    if ( member.first == name ) {
      member.second = std::move( v );
      return member.second;
    }
  }
  theObject.emplace_back( name, std::move( v ) );
  return theObject.back().second;
}

std::string Json::dump() const {
  std::string out;
  dump( out );
  return out;
}

void Json::dump( std::string &out ) const {
  switch ( theType ) {
    case Type::Null:
      out += "null";
      break;
    case Type::Bool:
      out += theBool ? "true" : "false";
      break;
    case Type::Number:
      out += theText;
      break;
    case Type::String:
      append_json_string( out, theText );
      break;
    case Type::Array: {
      out += '[';
      bool first = true;
      for ( Json const &item : theArray ) {
        if ( !first )
          out += ',';
        first = false;
        item.dump( out );
      }
      out += ']';
      break;
    }
    case Type::Object: {
      out += '{';
      bool first = true;
      for ( auto const &member : theObject ) {
        if ( !first )
          out += ',';
        first = false;
        append_json_string( out, member.first );
        out += ':';
        member.second.dump( out );
      }
      out += '}';
      break;
    }
  }
}

void append_json_string( std::string &out, std::string const &s ) {
  out += '"';
  for ( char const c : s ) {
    unsigned char const u = static_cast<unsigned char>( c );
    switch ( c ) {
      case '"':  out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if ( u < 0x20 || u == 0x7F ) {
          char buf[8];
          std::snprintf( buf, sizeof buf, "\\u%04x", static_cast<unsigned>( u ) );
          out += buf;
        } else {
          out += c;
        }
    }
  }
  out += '"';
}

///////////////////////////////////////////////////////////////////////////////
// Parser

namespace {

class Parser {
public:
  Parser( std::string const &text, JsonLimits const &limits ) :
    s_( text ), pos_( 0 ), limits_( limits ) {
  }

  bool parse( Json *result, std::string *error ) {
    skip_ws();
    if ( !value( result, 0 ) ) {
      *error = error_ + " at offset " + std::to_string( pos_ );
      return false;
    }
    skip_ws();
    if ( pos_ != s_.size() ) {
      *error = "unexpected data after the JSON value at offset " +
        std::to_string( pos_ );
      return false;
    }
    return true;
  }

private:
  bool fail( char const *msg ) {
    error_ = msg;
    return false;
  }

  bool at_end() const { return pos_ >= s_.size(); }
  char peek() const { return s_[ pos_ ]; }

  void skip_ws() {
    while ( !at_end() ) {
      char const c = peek();
      if ( c != ' ' && c != '\t' && c != '\n' && c != '\r' )
        break;
      ++pos_;
    }
  }

  bool literal( char const *word ) {
    std::size_t i = 0;
    for ( ; word[i]; ++i ) {
      if ( pos_ + i >= s_.size() || s_[ pos_ + i ] != word[i] )
        return fail( "invalid literal" );
    }
    pos_ += i;
    return true;
  }

  bool value( Json *out, std::size_t depth ) {
    if ( at_end() )
      return fail( "unexpected end of input" );
    switch ( peek() ) {
      case '{': return object( out, depth + 1 );
      case '[': return array( out, depth + 1 );
      case '"': {
        std::string str;
        if ( !string( &str ) )
          return false;
        *out = Json( std::move( str ) );
        return true;
      }
      case 't':
        if ( !literal( "true" ) ) return false;
        *out = Json( true );
        return true;
      case 'f':
        if ( !literal( "false" ) ) return false;
        *out = Json( false );
        return true;
      case 'n':
        if ( !literal( "null" ) ) return false;
        *out = Json();
        return true;
      default:
        return number( out );
    }
  }

  bool object( Json *out, std::size_t depth ) {
    if ( depth > limits_.max_depth )
      return fail( "nesting too deep" );
    ++pos_;                             // '{'
    Json obj( Json::object() );
    skip_ws();
    if ( !at_end() && peek() == '}' ) {
      ++pos_;
      *out = std::move( obj );
      return true;
    }
    for ( ;; ) {
      skip_ws();
      if ( at_end() || peek() != '"' )
        return fail( "expected a member name" );
      std::string name;
      if ( !string( &name ) )
        return false;
      skip_ws();
      if ( at_end() || peek() != ':' )
        return fail( "expected ':'" );
      ++pos_;
      skip_ws();
      Json v;
      if ( !value( &v, depth ) )
        return false;
      obj.set( name, std::move( v ) );
      skip_ws();
      if ( at_end() )
        return fail( "unterminated object" );
      if ( peek() == ',' ) {
        ++pos_;
        continue;
      }
      if ( peek() == '}' ) {
        ++pos_;
        *out = std::move( obj );
        return true;
      }
      return fail( "expected ',' or '}'" );
    }
  }

  bool array( Json *out, std::size_t depth ) {
    if ( depth > limits_.max_depth )
      return fail( "nesting too deep" );
    ++pos_;                             // '['
    Json arr( Json::array() );
    skip_ws();
    if ( !at_end() && peek() == ']' ) {
      ++pos_;
      *out = std::move( arr );
      return true;
    }
    for ( ;; ) {
      skip_ws();
      Json v;
      if ( !value( &v, depth ) )
        return false;
      arr.push( std::move( v ) );
      skip_ws();
      if ( at_end() )
        return fail( "unterminated array" );
      if ( peek() == ',' ) {
        ++pos_;
        continue;
      }
      if ( peek() == ']' ) {
        ++pos_;
        *out = std::move( arr );
        return true;
      }
      return fail( "expected ',' or ']'" );
    }
  }

  static bool is_digit( char c ) { return c >= '0' && c <= '9'; }

  bool number( Json *out ) {
    std::size_t const start = pos_;
    if ( !at_end() && peek() == '-' )
      ++pos_;
    if ( at_end() || !is_digit( peek() ) )
      return fail( "unexpected character" );
    if ( peek() == '0' )
      ++pos_;
    else
      while ( !at_end() && is_digit( peek() ) )
        ++pos_;
    if ( !at_end() && peek() == '.' ) {
      ++pos_;
      if ( at_end() || !is_digit( peek() ) )
        return fail( "expected a digit after '.'" );
      while ( !at_end() && is_digit( peek() ) )
        ++pos_;
    }
    if ( !at_end() && ( peek() == 'e' || peek() == 'E' ) ) {
      ++pos_;
      if ( !at_end() && ( peek() == '+' || peek() == '-' ) )
        ++pos_;
      if ( at_end() || !is_digit( peek() ) )
        return fail( "expected a digit in the exponent" );
      while ( !at_end() && is_digit( peek() ) )
        ++pos_;
    }
    *out = Json::number_text( s_.substr( start, pos_ - start ) );
    return true;
  }

  static void append_utf8( std::string *out, unsigned long cp ) {
    if ( cp < 0x80 ) {
      *out += static_cast<char>( cp );
    } else if ( cp < 0x800 ) {
      *out += static_cast<char>( 0xC0 | ( cp >> 6 ) );
      *out += static_cast<char>( 0x80 | ( cp & 0x3F ) );
    } else if ( cp < 0x10000 ) {
      *out += static_cast<char>( 0xE0 | ( cp >> 12 ) );
      *out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
      *out += static_cast<char>( 0x80 | ( cp & 0x3F ) );
    } else {
      *out += static_cast<char>( 0xF0 | ( cp >> 18 ) );
      *out += static_cast<char>( 0x80 | ( ( cp >> 12 ) & 0x3F ) );
      *out += static_cast<char>( 0x80 | ( ( cp >> 6 ) & 0x3F ) );
      *out += static_cast<char>( 0x80 | ( cp & 0x3F ) );
    }
  }

  bool hex4( unsigned long *cp ) {
    if ( s_.size() - pos_ < 4 )
      return fail( "truncated \\u escape" );
    unsigned long v = 0;
    for ( int i = 0; i < 4; ++i ) {
      char const c = s_[ pos_++ ];
      v <<= 4;
      if ( c >= '0' && c <= '9' )
        v |= static_cast<unsigned long>( c - '0' );
      else if ( c >= 'a' && c <= 'f' )
        v |= static_cast<unsigned long>( c - 'a' + 10 );
      else if ( c >= 'A' && c <= 'F' )
        v |= static_cast<unsigned long>( c - 'A' + 10 );
      else
        return fail( "invalid \\u escape" );
    }
    *cp = v;
    return true;
  }

  // Copies one UTF-8 encoded character starting at pos_, validating it
  // (shortest form, no surrogates, at most U+10FFFF).
  bool utf8_char( std::string *out ) {
    unsigned char const b0 = static_cast<unsigned char>( s_[ pos_ ] );
    std::size_t len;
    unsigned long cp;
    if ( b0 < 0x80 ) {
      len = 1; cp = b0;
    } else if ( ( b0 & 0xE0 ) == 0xC0 ) {
      len = 2; cp = b0 & 0x1F;
    } else if ( ( b0 & 0xF0 ) == 0xE0 ) {
      len = 3; cp = b0 & 0x0F;
    } else if ( ( b0 & 0xF8 ) == 0xF0 ) {
      len = 4; cp = b0 & 0x07;
    } else {
      return fail( "invalid UTF-8" );
    }
    if ( s_.size() - pos_ < len )
      return fail( "invalid UTF-8" );
    for ( std::size_t i = 1; i < len; ++i ) {
      unsigned char const b = static_cast<unsigned char>( s_[ pos_ + i ] );
      if ( ( b & 0xC0 ) != 0x80 )
        return fail( "invalid UTF-8" );
      cp = ( cp << 6 ) | ( b & 0x3F );
    }
    static unsigned long const min_cp[] = { 0, 0, 0x80, 0x800, 0x10000 };
    if ( cp < min_cp[ len ] || cp > 0x10FFFF ||
         ( cp >= 0xD800 && cp <= 0xDFFF ) )
      return fail( "invalid UTF-8" );
    out->append( s_, pos_, len );
    pos_ += len;
    return true;
  }

  bool string( std::string *out ) {
    ++pos_;                             // '"'
    for ( ;; ) {
      if ( at_end() )
        return fail( "unterminated string" );
      char const c = peek();
      if ( c == '"' ) {
        ++pos_;
        return true;
      }
      if ( static_cast<unsigned char>( c ) < 0x20 )
        return fail( "control character in string" );
      if ( c != '\\' ) {
        if ( !utf8_char( out ) )
          return false;
        continue;
      }
      ++pos_;                           // '\\'
      if ( at_end() )
        return fail( "unterminated string" );
      char const e = s_[ pos_++ ];
      switch ( e ) {
        case '"':  *out += '"'; break;
        case '\\': *out += '\\'; break;
        case '/':  *out += '/'; break;
        case 'b':  *out += '\b'; break;
        case 'f':  *out += '\f'; break;
        case 'n':  *out += '\n'; break;
        case 'r':  *out += '\r'; break;
        case 't':  *out += '\t'; break;
        case 'u': {
          unsigned long cp;
          if ( !hex4( &cp ) )
            return false;
          if ( cp >= 0xD800 && cp <= 0xDBFF ) {
            unsigned long lo;
            if ( s_.size() - pos_ < 2 || s_[ pos_ ] != '\\' ||
                 s_[ pos_ + 1 ] != 'u' )
              return fail( "unpaired surrogate" );
            pos_ += 2;
            if ( !hex4( &lo ) )
              return false;
            if ( lo < 0xDC00 || lo > 0xDFFF )
              return fail( "unpaired surrogate" );
            cp = 0x10000 + ( ( cp - 0xD800 ) << 10 ) + ( lo - 0xDC00 );
          } else if ( cp >= 0xDC00 && cp <= 0xDFFF ) {
            return fail( "unpaired surrogate" );
          }
          append_utf8( out, cp );
          break;
        }
        default:
          return fail( "invalid escape" );
      }
    }
  }

  std::string const &s_;
  std::size_t pos_;
  JsonLimits const &limits_;
  std::string error_;
};

} // namespace

bool parse_json( std::string const &text, Json *result, std::string *error,
                 JsonLimits const &limits ) {
  Parser p( text, limits );
  Json v;
  if ( !p.parse( &v, error ) )
    return false;
  *result = std::move( v );
  return true;
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
