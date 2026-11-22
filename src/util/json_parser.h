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

#ifndef ZORBA_JSON_PARSER_H
#define ZORBA_JSON_PARSER_H

#include <zorba/config.h>

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <zorba/internal/cxx_util.h>
#include <zorba/internal/diagnostic.h>

#include "zorbatypes/zstring.h"

#include "unicode_util.h"

namespace zorba {
namespace json {

///////////////////////////////////////////////////////////////////////////////

typedef internal::diagnostic::location location;

///////////////////////////////////////////////////////////////////////////////

/**
 * A JSON %type is the type of JSON data.  This isn't used by the lexer or
 * parser implementation at all, but it's handy.
 */
enum type {
  none,   // meaning "not set" as opposed to "null"
  array,
  boolean,
  null,
  number,
  object,
  string
};
extern char const *const type_string_of[];

inline std::ostream& operator<<( std::ostream &o, type t ) {
  return o << type_string_of[ t ];
}

/**
 * A JSON %token.  Tokens have a type, location at which they were found, and
 * sometimes a value.
 *
 * See: "RFC 4627: The application/json Media Type for JavaScript Object
 * Notation (JSON)."
 */
class token {
  // see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2333.html
  struct pointer_conversion { int valid; };
  typedef int pointer_conversion::*explicit_bool;
public:
  typedef zstring value_type;

  /**
   * The types of tokens in JSON.  The first 6 constants have values that
   * correspond to the actual structural characters used by JSON; the rest were
   * assigned non-standard, mnemonic values for convenience.
   */
  enum type {
    none,
    begin_array     = '[',
    begin_object    = '{',
    end_array       = ']',
    end_object      = '}',
    name_separator  = ':',
    value_separator = ',',
    string          = 'S',
    number          = 'N',
    json_false      = 'F',
    json_null       = '0',
    json_true       = 'T',
  };

  /**
   * The numeric types of \c number tokens in JSON.  Note that this is an
   * extension since JSON does not distinguish between numeric types.
   */
  enum numeric_type {
    non_numeric,
    integer         = 'i',
    decimal         = 'd',
    floating_point  = 'f',
  };

  /**
   * Default constructor.
   */
  token();

  /**
   * Copies a %token.  The copy holds its own value even when \a that only
   * refers to the lexer's buffer.
   */
  token( token const &that );
  token& operator=( token const &that );

  /**
   * Clears this %token.  Its location stays.
   */
  void clear() {
    type_ = none;
    numeric_type_ = non_numeric;
    value_.clear();
    view_ = nullptr;
    view_size_ = 0;
  }

  /**
   * Gets the location at which this %token was found.
   *
   * @return Returns said location.
   */
  location get_loc() const {
    return location( file_ ? file_ : "", line_, column_, line_end_,
                     column_end_ );
  }

  /**
   * Gets the type of this %token.
   *
   * @return Returns said type.
   */
  type get_type() const {
    return type_;
  }

  /**
   * Gets the numeric type of this %token if it's type is \c number.
   *
   * @return Returns said type or \c non_numeric if the type is not \c number.
   */
  numeric_type get_numeric_type() const {
    return numeric_type_;
  }

  /**
   * Gets the value of this %token, if any.  Only %token types string, number,
   * false, null, and true have a value.  A value that still refers to the
   * lexer's buffer (see text()) must be gotten before the lexer reads on.
   *
   * @return Returns said value or the empty string.
   */
  value_type const& get_value() const {
    if ( view_ ) {
      value_.assign( view_, view_size_ );
      view_ = nullptr;
      view_size_ = 0;
    }
    return value_;
  }

  /**
   * Gets the value of this %token without copying it: it can refer to the
   * lexer's buffer, so it is valid only until the lexer reads the next token.
   *
   * @return Returns said value or the empty string.
   */
  std::string_view text() const {
    return view_ ? std::string_view( view_, view_size_ ) :
                   std::string_view( value_.data(), value_.size() );
  }

  /**
   * Gets the value of this %token into a string, sharing it if it is held
   * rather than referred to.
   *
   * @param result The string to receive the value.
   */
  void get_value( value_type *result ) const {
    if ( view_ )
      result->assign( view_, view_size_ );
    else
      *result = value_;
  }

  /**
   * Conversion to \c bool.
   *
   * @return Returns \c true only if this token's type is not \c none.
   */
  operator explicit_bool() const {
    return type_ ? &pointer_conversion::valid : nullptr;
  }

private:
  typedef location::line_type line_type;
  typedef location::column_type column_type;

  void set_loc( char const *file, line_type line, column_type column,
                line_type line_end, column_type column_end ) {
    file_ = file;
    line_ = line;
    column_ = column;
    line_end_ = line_end;
    column_end_ = column_end;
  }

  void take( token *that );

  char const *file_;                    // owned by the lexer
  line_type line_, line_end_;
  column_type column_, column_end_;
  type type_;
  numeric_type numeric_type_;
  mutable value_type value_;
  mutable char const *view_;            // the value in the lexer's buffer
  mutable size_t view_size_;

  friend class lexer;
  friend class parser;
};

/**
 * Map a token's type to a JSON type.
 *
 * @param tt The token::type to map.
 * @return Returns the corresponding JSON type or \c none if \a tt doesn't map.
 */
type map_type( token::type tt );

/**
 * Emits the given token type to an ostream.
 *
 * @param o The ostream to emit to.
 * @param tt The token type to emit.
 * @return Returns \a o.
 */
std::ostream& operator<<( std::ostream &o, token::type tt );

/**
 * Emits the given token numeric type to an ostream.
 *
 * @param o The ostream to emit to.
 * @param nt The token numeric type to emit.
 * @return Returns \a o.
 */
std::ostream& operator<<( std::ostream &o, token::numeric_type nt );

/**
 * Emits the given token to an ostream.
 *
 * @param o The ostream to emit to.
 * @param t The token to emit.
 * @return Returns \a o.
 */
std::ostream& operator<<( std::ostream &o, token const &t );

/**
 * Compares two tokens for equality.
 *
 * @param t1 The first token.
 * @param t2 The second token.
 * @return Returns \c true only if the two tokens' types and values are equal.
 */
inline bool operator==( token const &t1, token const &t2 ) {
  return t1.get_type() == t2.get_type() && t1.get_value() == t2.get_value();
}

/**
 * Compares a token's type to another type for equality.
 *
 * @param t The token whose type to compare.
 * @param tt The type to compare to.
 * @return Returns \c true only if the token's type equals \a tt.
 */
inline bool operator==( token const &t, token::type tt ) {
  return t.get_type() == tt;
}

/**
 * Compares a token's type to another type for equality.
 *
 * @param tt The type to compare.
 * @param t The token whose type to compare to.
 * @return Returns \c true only if \a tt equals the token's type.
 */
inline bool operator==( token::type tt, token const &t ) {
  return t == tt;
}

/**
 * Compares a token's value to a C string for equality.
 *
 * @param t The token whose value to compare.
 * @param value The value to compare to.
 * @return Returns \c true only if the token's value equals \a value.
 */
inline bool operator==( token const &t, char const *value ) {
  return t.get_value() == value;
}

/**
 * Compares a C string to a token's value for equality.
 *
 * @param value The value to compare.
 * @param t The token whose value to compare to.
 * @return Returns \c true only if \a value equals the token's value.
 */
inline bool operator==( char const *value, token const &t ) {
  return t == value;
}

/**
 * Compares two tokens for inequality.
 *
 * @param t1 The first token.
 * @param t2 The second token.
 * @return Returns \c true if either the two tokens' types or values are not
 * equal.
 */
inline bool operator!=( token const &t1, token const &t2 ) {
  return !(t1 == t2);
}

/**
 * Compares a token's type to another type for inequality.
 *
 * @param t The token whose type to compare.
 * @param tt The type to compare to.
 * @return Returns \c true only if the token's type is not equal to \a tt.
 */
inline bool operator!=( token const &t, token::type tt ) {
  return !(t == tt);
}

/**
 * Compares a token's type to another type for inequality.
 *
 * @param tt The type to compare.
 * @param t The token whose type to compare to.
 * @return Returns \c true only if \a tt is not equal to the token's type.
 */
inline bool operator!=( token::type tt, token const &t ) {
  return !(tt == t);
}

/**
 * Compares a token's value to a C string for inequality.
 *
 * @param t The token whose value to compare.
 * @param value The value to compare to.
 * @return Returns \c true only if the token's value is not equal to \a value.
 */
inline bool operator!=( token const &t, char const *value ) {
  return !(t == value);
}

/**
 * Compares a token's value to a C string for inequality.
 *
 * @param value The value to compare.
 * @param t The token whose value to compare to.
 * @return Returns \c true only if \a value is not equal to the token's value.
 */
inline bool operator!=( char const *value, token const &t ) {
  return !(value == t);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * An %exception is the root of the JSON %exception hierarchy.
 */
class exception : public std::exception {
public:
  ~exception() throw();

  /**
   * Gets the location in the JSON source whence this exception was thrown.
   */
  location const& get_loc() const {
    return loc_;
  }

  // inherited
  char const* what() const throw();

protected:
  exception( location const &loc, std::string const &message );

private:
  location loc_;
  std::string message_;
};

/**
 * This exception is thrown when an illegal character is encountered in a JSON
 * data stream.
 */
class illegal_character : public exception {
public:
  illegal_character( location const &loc, char c );
  ~illegal_character() throw();

  /**
   * Gets the illegal character.
   *
   * @return Returns said character.
   */
  char get_char() const {
    return c_;
  }

private:
  char c_;
};

/**
 * This exception is thrown when an illegal Unicode code-point escape sequence
 * (\uHHHH) is encountered.
 */
class illegal_codepoint : public exception {
public:
  illegal_codepoint( location const &loc, token::value_type const &cp );
  ~illegal_codepoint() throw();

  /**
   * Gets the illegal code-point.
   *
   * @return Returns said code-point.
   */
  token::value_type get_codepoint() const {
    return codepoint_;
  }

private:
  token::value_type codepoint_;
};

/**
 * This exception is thrown when an illegal character follows a backslash
 * (escape) within a string literal.  The legal escape characters are:
 * ["/\bfnrtu].
 */
class illegal_escape : public exception {
public:
  illegal_escape( location const &loc, char escape );
  ~illegal_escape() throw();

  /**
   * Gets the illegal escape character.
   *
   * @return Returns said character.
   */
  char get_escape() const {
    return esc_;
  }

private:
  char esc_;
};

/**
 * This exception is thrown when a literal other than \c false, \c null, or
 * \c true is encountered.
 */
class illegal_literal : public exception {
public:
  illegal_literal( location const &loc );
  ~illegal_literal() throw();
};

/**
 * This exception is thrown when an illegal number is encountered.
 */
class illegal_number : public exception {
public:
  illegal_number( location const &loc );
  ~illegal_number() throw();
};

/**
 * This exception is thrown when an unexpected token is encountered.
 */
class unexpected_token : public exception {
public:
  unexpected_token( token const &t );
  ~unexpected_token() throw();

  /**
   * Gets the unexpected token
   *
   * @return Returns said token.
   */
  token const& get_token() const {
    return token_;
  }

private:
  token token_;
};

/**
 * This exception is thrown when an EOF is encountered before a string's
 * terminating quote.
 */
class unterminated_string : public exception {
public:
  unterminated_string( location const &loc );
  ~unterminated_string() throw();
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A %lexer extracts JSON tokens from an istream.
 */
class lexer {
public:
  typedef location::line_type line_type;
  typedef location::column_type column_type;

  /**
   * Constructs a %lexer on the given istream.
   *
   * @param in The istream to read from.
   */
  lexer( std::istream &in );

  /**
   * Gets the next token, if any.
   *
   * @param result A pointer to the token to get into.
   * @param throw_exceptions If \c true and there is a lexical error, throws an
   * exception; if \c false, returns \c false.
   * @return Returns \c true only if there was a next token.
   * @throws exception upon error unless \a throw_exceptions is \c false.
   */
  bool next( token *result, bool throw_exceptions = true );

  /**
   * Sets the file location.
   *
   * @param file The source file name.
   * @param line The source line number.
   * @param col  The source column number.
   */
  void set_loc( char const *file, line_type line, column_type col );

  /**
   * Gives back to the stream what the lexer has read from it but not lexed,
   * so that the stream's position is that of the next character to lex.  Call
   * it before using the stream's position or moving it.
   */
  void sync();

  lexer( lexer const& ) = delete;
  lexer& operator=( lexer const& ) = delete;

private:
  typedef long long offset_type;

  bool fill( bool bump );
  void set_badbit_and_rethrow_if_masked();
  bool get_char( char* );
  bool peek_char( char* );
  void newline();
  bool parse_codepoint( unicode::code_point *cp, bool throw_exceptions );
  token::type parse_literal( char, bool throw_exceptions );
  token::numeric_type parse_number( char, bool throw_exceptions );
  bool parse_string( token*, line_type, column_type, bool throw_exceptions );
  bool parse_string_escapes( token*, line_type, column_type,
                             bool throw_exceptions );
  void end_capture( token* );

  offset_type pos() const {
    return buf_offset_ + (cur_ - buf_);
  }
  column_type col() const {
    return static_cast<column_type>( pos() - line_start_ + 1 );
  }
  line_type prev_line() const;
  column_type prev_col() const;
  void set_cur_loc();
  location& set_cur_loc_end( bool prev = true );
  void set_loc_range( token* );

  std::istream *in_;
  std::string file_;

  // What was read from the stream: [cur_,end_) is not lexed yet.
  static size_t const small_buf_size = 512;
  static size_t const max_buf_size = 64 * 1024;
  char small_buf_[ small_buf_size ];
  std::unique_ptr<char[]> big_buf_;
  char *buf_;
  size_t buf_size_;
  char const *cur_, *end_;
  offset_type buf_offset_;              // of buf_[0] in what was lexed

  // Columns are offsets from the start of the line, so that they are known
  // without counting each character.
  line_type line_;                      // of the next character
  offset_type line_start_;              // offset whose column is 1
  offset_type prev_line_start_;         // of the line before line_

  location cur_loc_;
  token::value_type value_;
  char const *capture_;                 // start of the value being lexed
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A %parser extracts JSON tokens from an istream while checking to ensure the
 * token sequence is valid.
 */
class parser {
public:
  typedef lexer::line_type line_type;
  typedef lexer::column_type column_type;

  /**
   * Constructs a %parser on the given istream.
   *
   * @param in The istream to read from.
   * @param allow_multiple If \c true, allow multiple top-level JSON items.
   */
  parser( std::istream &in, bool allow_multiple = false );

  /**
   * Resets this %parser to its almost-initial state.  (It does not, however,
   * reset the file location.)
   */
  void clear();

  /**
   * Gets the next token, if any.
   *
   * @param result A pointer to the token to receive the token.
   * @return Returns \c true only if there was a next token.
   * @throws exception upon error.
   */
  bool next( token *result );

  /**
   * Peeks at the next token, if any.
   *
   * @param result A pointer to the token to receive the token, if any.
   * @return Returns the type of the peeked token.
   * @throws exception upon error.
   */
  token::type peek( token *result = nullptr );

  /**
   * Sets the file location.
   *
   * @param file The source file name.
   * @param line The source line number.
   * @param col  The source column number.
   */
  void set_loc( char const *file, line_type line, column_type col ) {
    lexer_.set_loc( file, line, col );
  }

  /**
   * Gives back to the stream what was read from it but not lexed (see
   * lexer::sync()).
   */
  void sync() {
    lexer_.sync();
  }

private:
  enum state {
    A0, A1, A2, // Array
    E0, E1,     // Element
    J0, J1,     // JSON
    M0, M1,     // Member
    O0, O1, O2, // Object
    P0, P1,     // Pair
    V0          // Value
  };

  friend std::ostream& operator<<( std::ostream&, state );

  void init();
  bool get_token( token* );
  state pop_state() {
    state const s = state_stack_.back();
    state_stack_.pop_back();
    return s;
  }
  bool get_token_debug( int, token* );
  bool matches_token( token::type, token* );
  bool matches_token_debug( int, token::type, token* );
  token::type peek_token();
  token::type peek_token_debug( int );
  void require_token( token::type, token* );
  void require_token_debug( int, token::type, token* );

  bool allow_multiple_;
  lexer lexer_;
  token peeked_token_;
  std::vector<state> state_stack_;
  state state_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace json
} // namespace zorba

#endif /* ZORBA_JSON_PARSER_H */
/* vim:set et sw=2 ts=2: */
