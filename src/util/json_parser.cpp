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

#include "diagnostics/assert.h"

#include "ascii_util.h"
#include "stl_util.h"
#include "string_util.h"
#include "utf8_util.h"

#define DEBUG_JSON_PARSER 0
#if DEBUG_JSON_PARSER
# include "indent.h"
#endif /* DEBUG_JSON_PARSER */

#include <utility>

#include "json_parser.h"

using namespace std;

namespace zorba {
namespace json {

///////////////////////////////////////////////////////////////////////////////

char const *const type_string_of[] = {
  "none",
  "array",
  "boolean",
  "null",
  "number",
  "object",
  "string"
};

type map_type( token::type tt ) {
  switch ( tt ) {
    case token::string:
      return string;
    case token::number:
      return number;
    case token::json_false:
    case token::json_true:
      return boolean;
    case token::json_null:
      return null;
    default:
      return none;
  }
}

///////////////////////////////////////////////////////////////////////////////

exception::exception( location const &loc, std::string const &message ) :
  loc_( loc ), message_( message )
{
}

exception::~exception() throw() {
  // out-of-line since it's virtual
}

char const* exception::what() const throw() {
  return message_.c_str();
}

illegal_character::illegal_character( location const &loc, char c ) :
  exception(
    loc,
    BUILD_STRING( '\'', ascii::printable_char( c ), "': illegal character" )
  ),
  c_( c )
{
}

illegal_character::~illegal_character() throw() {
  // out-of-line since it's virtual
}

illegal_codepoint::illegal_codepoint( location const &loc,
                                      token::value_type const &cp ) :
  exception( loc, BUILD_STRING( '"', cp, "\": illegal codepoint" ) ),
  codepoint_( cp )
{
}

illegal_codepoint::~illegal_codepoint() throw() {
  // out-of-line since it's virtual
}

illegal_escape::illegal_escape( location const &loc, char c ) :
  exception(
    loc,
    BUILD_STRING(
      "\"\\", ascii::printable_char( c ), "\": illegal character escape"
    )
  ),
  esc_( c )
{
}

illegal_escape::~illegal_escape() throw() {
  // out-of-line since it's virtual
}

illegal_literal::illegal_literal( location const &loc ) :
  exception( loc, "illegal literal" )
{
}

illegal_literal::~illegal_literal() throw() {
  // out-of-line since it's virtual
}

illegal_number::illegal_number( location const &loc ) :
  exception( loc, "illegal number" )
{
}

illegal_number::~illegal_number() throw() {
  // out-of-line since it's virtual
}

unexpected_token::unexpected_token( token const &t ) :
  exception( t.get_loc(), BUILD_STRING( '"', t, "\": unexpected token" ) ),
  token_( t )
{
}

unexpected_token::~unexpected_token() throw() {
  // out-of-line since it's virtual
}

unterminated_string::unterminated_string( location const &loc ) :
  exception( loc, "unterminated string" )
{
}

unterminated_string::~unterminated_string() throw() {
  // out-of-line since it's virtual
}

///////////////////////////////////////////////////////////////////////////////

token::token() :
  file_( nullptr ), line_( 0 ), line_end_( 0 ), column_( 0 ), column_end_( 0 ),
  type_( none ), numeric_type_( non_numeric ), view_( nullptr ), view_size_( 0 )
{
}

token::token( token const &that ) :
  file_( that.file_ ), line_( that.line_ ), line_end_( that.line_end_ ),
  column_( that.column_ ), column_end_( that.column_end_ ),
  type_( that.type_ ), numeric_type_( that.numeric_type_ ),
  value_( that.get_value() ), view_( nullptr ), view_size_( 0 )
{
}

token& token::operator=( token const &that ) {
  if ( &that != this ) {
    set_loc(
      that.file_, that.line_, that.column_, that.line_end_, that.column_end_
    );
    type_ = that.type_;
    numeric_type_ = that.numeric_type_;
    value_ = that.get_value();
    view_ = nullptr;
    view_size_ = 0;
  }
  return *this;
}

// As *this = *that; that->clear(), without copying the value.
void token::take( token *that ) {
  set_loc(
    that->file_, that->line_, that->column_, that->line_end_, that->column_end_
  );
  type_ = that->type_;
  numeric_type_ = that->numeric_type_;
  value_.swap( that->value_ );
  view_ = that->view_;
  view_size_ = that->view_size_;
  that->clear();
}

ostream& operator<<( ostream &o, token::type tt ) {
  switch ( tt ) {
    case token::string    : o << "string"; break;
    case token::number    : o << "number"; break;
    case token::json_false: o << "false" ; break;
    case token::json_null : o << "null"  ; break;
    case token::json_true : o << "true"  ; break;
    case token::none      : o << "<none>"; break;
    default               : o << static_cast<char>( tt );
  }
  return o;
}

std::ostream& operator<<( ostream &o, token::numeric_type nt ) {
  switch ( nt ) {
    case token::non_numeric   : o << "<non_numeric>" ; break;
    case token::integer       : o << "integer"       ; break;
    case token::decimal       : o << "decimal"       ; break;
    case token::floating_point: o << "floating_point"; break;
  }
  return o;
}

ostream& operator<<( ostream &o, token const &t ) {
  switch ( t.get_type() ) {
    case token::string: o << '"' << t.get_value() << '"'; break;
    case token::number: o <<        t.get_value()       ; break;
    default           : o <<        t.get_type()        ;
  }
  return o;
}

///////////////////////////////////////////////////////////////////////////////

lexer::lexer( istream &in ) :
  in_( &in ),
  buf_( small_buf_ ),
  buf_size_( small_buf_size ),
  cur_( small_buf_ ),
  end_( small_buf_ ),
  buf_offset_( 0 ),
  line_( 1 ),
  line_start_( 0 ),
  prev_line_start_( 0 ),
  capture_( nullptr )
{
}

// As istream does when its buffer throws: rethrow only if badbit is in the exception mask.
void lexer::set_badbit_and_rethrow_if_masked() {
  ios::iostate const mask = in_->exceptions();
  if ( !(mask & ios::badbit) ) {
    in_->setstate( ios::badbit );
    return;
  }
  in_->exceptions( ios::goodbit );
  in_->setstate( ios::badbit );
  try {
    in_->exceptions( mask );
  }
  catch ( ios::failure const& ) {
    // restoring the mask throws because badbit is set; the buffer's exception is the one to raise
  }
  throw;
}

/**
 * Once the buffer is all lexed, reads into it what the stream's buffer holds,
 * at most max_buf_size bytes.  At the end of the stream, or when its buffer
 * throws, it sets the stream state as istream::get() (if \a bump) or
 * istream::peek() would.
 */
bool lexer::fill( bool bump ) {
  typedef istream::traits_type traits;
  if ( capture_ )
    value_.append( capture_, static_cast<size_t>( end_ - capture_ ) );
  buf_offset_ += end_ - buf_;
  cur_ = end_ = buf_;
  if ( capture_ )
    capture_ = buf_;

  if ( buffer_error_ ) {
    try {
      std::rethrow_exception( std::exchange( buffer_error_, nullptr ) );
    }
    catch ( ... ) {
      set_badbit_and_rethrow_if_masked();
    }
    if ( bump )
      in_->setstate( ios::failbit );
    return false;
  }
  if ( !in_->good() ) {
    in_->setstate( ios::failbit );
    return false;
  }
  std::streambuf *const sb = in_->rdbuf();
  streamsize avail;
  try {
    if ( traits::eq_int_type( sb->sgetc(), traits::eof() ) ) {
      in_->setstate( bump ? ios::eofbit | ios::failbit : ios::eofbit );
      return false;
    }
    // after sgetc(), what the stream's buffer holds: reading that cannot fail
    avail = sb->in_avail();
  }
  catch ( ... ) {
    set_badbit_and_rethrow_if_masked();
    if ( bump )
      in_->setstate( ios::failbit );
    return false;
  }

  size_t const want = avail < 1 ? 1 :
    static_cast<size_t>( std::min<streamsize>( avail, max_buf_size ) );
  if ( want > buf_size_ ) {
    size_t const size =
      std::min( max_buf_size, std::max( want, 2 * buf_size_ ) );
    big_buf_ = std::make_unique_for_overwrite<char[]>( size );
    buf_ = big_buf_.get();
    buf_size_ = size;
    cur_ = end_ = buf_;
    if ( capture_ )
      capture_ = buf_;
  }

  // One character at a time, not sgetn(): a buffer without a get area that
  // throws mid-copy would lose the characters already copied.
  char *const buf = buf_;
  size_t got = 0;
  try {
    while ( got < want ) {
      traits::int_type const c = sb->sgetc();
      if ( traits::eq_int_type( c, traits::eof() ) )
        break;
      buf[ got++ ] = traits::to_char_type( c );
      sb->sbumpc();
    }
  }
  catch ( ... ) {
    if ( got > 0 ) {
      // lex what was read first; the next fill() reports the failure
      buffer_error_ = std::current_exception();
      end_ = buf_ + got;
      return true;
    }
    set_badbit_and_rethrow_if_masked();
    if ( bump )
      in_->setstate( ios::failbit );
    return false;
  }
  if ( got < 1 ) {
    // sgetc() saw a character that the loop does not give: the end after all
    in_->setstate( bump ? ios::eofbit | ios::failbit : ios::eofbit );
    return false;
  }
  end_ = buf_ + got;
  return true;
}

inline void lexer::newline() {
  prev_line_start_ = line_start_;
  line_start_ = pos();
  ++line_;
}

inline bool lexer::get_char( char *c ) {
  if ( cur_ == end_ && !fill( true ) )
    return false;
  char const temp = *cur_++;
  if ( temp == '\n' )
    newline();
  *c = temp;
  return true;
}

inline bool lexer::peek_char( char *c ) {
  if ( cur_ == end_ && !fill( false ) )
    return false;
  *c = *cur_;
  return true;
}

// The position of the character read last: on the line before if it was a newline.
lexer::line_type lexer::prev_line() const {
  return line_start_ == pos() ? line_ - 1 : line_;
}

lexer::column_type lexer::prev_col() const {
  offset_type const start =
    line_start_ == pos() ? prev_line_start_ : line_start_;
  return static_cast<column_type>( pos() - start );
}

inline void lexer::set_cur_loc() {
  column_type const c = col();
  cur_loc_.set( line_, c, line_, c );
}

inline location& set_loc_end( location *loc, location::line_type line_end,
                              location::column_type col_end ) {
  loc->set( loc->line(), loc->column(), line_end, col_end );
  return *loc;
}

inline void lexer::set_loc_range( token *t ) {
  t->set_loc(
    file_.c_str(), cur_loc_.line(), cur_loc_.column(), prev_line(), prev_col()
  );
}

// The value lexed since capture_: in the buffer if it is all there, else in value_.
void lexer::end_capture( token *t ) {
  size_t const n = static_cast<size_t>( cur_ - capture_ );
  if ( value_.empty() ) {
    t->view_ = capture_;
    t->view_size_ = n;
  } else {
    value_.append( capture_, n );
    t->value_ = value_;
    t->view_ = nullptr;
    t->view_size_ = 0;
  }
  capture_ = nullptr;
}

bool lexer::next( token *t, bool throw_exceptions ) {
  capture_ = nullptr;
  while ( true ) {
    set_cur_loc();
    char c;
    if ( !get_char( &c ) )
      return false;
    switch ( c ) {
      case ' ':
      case '\n':
      case '\r':
      case '\t':
        continue;
      case '"': {
        //
        // We need to remember the opening quote location here because
        // parse_string() calls set_cur_loc().
        //
        line_type const quote_line = cur_loc_.line();
        column_type const quote_col = cur_loc_.column();
        if ( !parse_string( t, quote_line, quote_col, throw_exceptions ) )
          return false;
        t->type_ = token::string;
        t->set_loc(
          file_.c_str(), quote_line, quote_col, prev_line(), prev_col()
        );
        return true;
      }
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        capture_ = cur_ - 1;
        value_.clear();
        token::numeric_type nt;
        if ( !(nt = parse_number( c, throw_exceptions )) )
          return false;
        end_capture( t );
        t->numeric_type_ = nt;
        t->type_ = token::number;
        set_loc_range( t );
        return true;
      }
      case 'f':   // false
      case 'n':   // null
      case 't': { // true
        token::type tt;
        if ( !(tt = parse_literal( c, throw_exceptions )) )
          return false;
        t->value_ = value_;
        t->view_ = nullptr;
        t->view_size_ = 0;
        t->type_ = tt;
        set_loc_range( t );
        return true;
      }
      case '[':
      case '{':
      case ']':
      case '}':
      case ':':
      case ',':
        t->type_ = static_cast<token::type>( c );
        t->set_loc(
          file_.c_str(), cur_loc_.line(), cur_loc_.column(),
          cur_loc_.line_end(), cur_loc_.column_end()
        );
        return true;
      default:
        if ( throw_exceptions )
          throw illegal_character( cur_loc_, c );
        return false;
    }
  } // while
}

bool lexer::parse_codepoint( unicode::code_point *result,
                             bool throw_exceptions ) {
  static char const hex_digits[] = "0123456789ABCDEF";

  char c;
  zstring cp_string( "\\u" );           // needed only for error message
  unicode::code_point high_surrogate = 0;

  while ( true ) {
    unicode::code_point cp = 0;
    for ( int i = 1; i <= 4; ++i ) {
      if ( !get_char( &c ) )
        goto error_set_cur_loc_end_false;
      cp_string += c;
      if ( !ascii::is_xdigit( c ) )
        goto error_set_cur_loc_end;
      c = ascii::to_upper( c );
      char const *const p = std::strchr( hex_digits, c );
      assert( p );
      cp = (cp << 4) | (p - hex_digits);
    }

    if ( unicode::is_high_surrogate( cp ) ) {
      if ( high_surrogate )
        goto error_set_cur_loc_end;
      //
      // It's easier to parse the \u for the low surrogate here rather than
      // trying to manage state in parse_string().
      //
      if ( !get_char( &c ) )
        goto error_set_cur_loc_end_false;
      cp_string += c;
      if ( c != '\\' || !get_char( &c ) )
        goto error_set_cur_loc_end;
      cp_string += c;
      if ( c != 'u' )
        goto error_set_cur_loc_end;

      high_surrogate = cp;
      continue;
    }
    if ( unicode::is_low_surrogate( cp ) ) {
      if ( !high_surrogate )
        goto error_set_cur_loc_end;
      *result = unicode::convert_surrogate( high_surrogate, cp );
      return true;
    }
    if ( high_surrogate )
      goto error_set_cur_loc_end;

    *result = cp;
    return true;
  } // while

error_set_cur_loc_end:
  if ( throw_exceptions )
    throw illegal_codepoint( set_cur_loc_end(), cp_string );
  return false;

error_set_cur_loc_end_false:
  if ( throw_exceptions )
    throw illegal_codepoint( set_cur_loc_end( false ), cp_string );
  return false;
}

token::type lexer::parse_literal( char first_c, bool throw_exceptions ) {
  static token::value_type const false_value( "false" );
  static token::value_type const null_value ( "null"  );
  static token::value_type const true_value ( "true"  );

  token::type tt = token::none;
  switch ( first_c ) {
    case 'f': value_ = false_value; tt = token::json_false; break;
    case 'n': value_ = null_value ; tt = token::json_null ; break;
    case 't': value_ = true_value ; tt = token::json_true ; break;
    default : assert( false );
  }

  char c;
  for ( char const *s = value_.c_str(); *++s; ) {
    if ( !get_char( &c ) )
      goto error_set_cur_loc_end_false;
    if ( c != *s )
      goto error_set_cur_loc_end;
  }
  if ( peek_char( &c ) && (ascii::is_alnum( c ) || c == '_') )
    goto error_set_cur_loc_end_false;

  return tt;

error_set_cur_loc_end:
  if ( throw_exceptions )
    throw illegal_literal( set_cur_loc_end() );
  return token::none;

error_set_cur_loc_end_false:
  if ( throw_exceptions )
    throw illegal_literal( set_cur_loc_end( false ) );
  return token::none;
}

token::numeric_type lexer::parse_number( char first_c, bool throw_exceptions ) {
  token::numeric_type numeric_type;

  // <number> ::= [-] <int> [<frac>] [<exp>]
  char c = first_c;
  if ( c == '-' ) {
    if ( !get_char( &c ) )
      goto error_set_cur_loc_end_false;
  }

  // <int> := '0' | <1-9> <digit>*
  if ( !ascii::is_digit( c ) )
    goto error_set_cur_loc_end;
  numeric_type = token::integer;
  if ( c == '0' ) {
    if ( !peek_char( &c ) )
      goto done;
    if ( ascii::is_alnum( c ) )
      goto error_set_cur_loc_end_false;
  } else {
    while ( true ) {
      if ( !peek_char( &c ) )
        goto done;
      if ( ascii::is_alpha( c ) && c != 'e' && c != 'E' )
        goto error_set_cur_loc_end_false;
      if ( !ascii::is_digit( c ) )
        break;
      get_char( &c );
    }
  }

  // <frac> ::= '.' <digit>+
  if ( c == '.' ) {
    get_char( &c );
    if ( !get_char( &c ) )
      goto error_set_cur_loc_end_false;
    if ( !ascii::is_digit( c ) )
      goto error_set_cur_loc_end;
    numeric_type = token::decimal;
    while ( true ) {
      if ( !peek_char( &c ) )
        goto done;
      if ( ascii::is_alpha( c ) && c != 'e' && c != 'E' )
        goto error_set_cur_loc_end_false;
      if ( !ascii::is_digit( c ) )
        break;
      get_char( &c );
    }
  }

  // <exp>  ::= <e> [<sign>] <digit>+
  // <e>    ::= 'e' | 'E'
  // <sign> ::= '-' | '+'
  if ( c == 'e' || c == 'E' ) {
    get_char( &c );
    if ( !get_char( &c ) )
      goto error_set_cur_loc_end_false;
    if ( c == '+' || c == '-' ) {
      if ( !get_char( &c ) )
        goto error_set_cur_loc_end_false;
    }
    if ( !ascii::is_digit( c ) )
      goto error_set_cur_loc_end;
    numeric_type = token::floating_point;
    while ( true ) {
      if ( !peek_char( &c ) )
        goto done;
      if ( ascii::is_alpha( c ) )
        goto error_set_cur_loc_end_false;
      if ( !ascii::is_digit( c ) )
        break;
      get_char( &c );
    }
  }

done:
  return numeric_type;

error_set_cur_loc_end:
  if ( throw_exceptions )
    throw illegal_number( set_cur_loc_end() );
  return token::non_numeric;

error_set_cur_loc_end_false:
  if ( throw_exceptions )
    throw illegal_number( set_cur_loc_end( false ) );
  return token::non_numeric;
}

/**
 * Lexes a string up to its closing quote.  A string without escapes is found
 * by scanning bytes and is not copied; one with escapes is built in value_.
 */
bool lexer::parse_string( token *t, line_type quote_line,
                          column_type quote_col, bool throw_exceptions ) {
  capture_ = cur_;
  value_.clear();
  while ( true ) {
    char const *p = cur_;
    while ( p != end_ && *p != '"' && *p != '\\' && *p != '\n' )
      ++p;
    cur_ = p;
    if ( p == end_ ) {
      if ( fill( true ) )
        continue;
      capture_ = nullptr;
      if ( throw_exceptions )
        throw unterminated_string(
          location( file_, quote_line, quote_col, line_, col() )
        );
      return false;
    }
    switch ( *p ) {
      case '"':
        end_capture( t );
        ++cur_;
        return true;
      case '\n':
        ++cur_;
        newline();
        continue;
      default:                          // '\\'
        value_.append( capture_, static_cast<size_t>( p - capture_ ) );
        capture_ = nullptr;
        return parse_string_escapes( t, quote_line, quote_col,
                                     throw_exceptions );
    }
  }
}

bool lexer::parse_string_escapes( token *t, line_type quote_line,
                                  column_type quote_col,
                                  bool throw_exceptions ) {
  bool got_backslash = false;
  ztd::string_appender<token::value_type,1024> value( &value_ );

  while ( true ) {
    //
    // We need to call set_cur_loc() here since strings can have invalid
    // code-points or escapes and we need to report the exact error location of
    // those not just the start of the string.
    //
    set_cur_loc();

    char c;
    if ( !get_char( &c ) ) {
      if ( throw_exceptions )
        throw unterminated_string(
          location( file_, quote_line, quote_col, line_, col() )
        );
      return false;
    }
    if ( got_backslash ) {
      got_backslash = false;
      switch ( c ) {
        case '"':
        case '/':
        case '\\':
          value += c;
          break;
        case 'b':
          value += '\b';
          break;
        case 'f':
          value += '\f';
          break;
        case 'n':
          value += '\n';
          break;
        case 'r':
          value += '\r';
          break;
        case 't':
          value += '\t';
          break;
        case 'u': {
          unicode::code_point cp;
          if ( !parse_codepoint( &cp, throw_exceptions ) )
            return false;
          value.flush();
          utf8::encode( cp, &value_ );
          break;
        }
        default:
          if ( throw_exceptions )
            throw illegal_escape( set_cur_loc_end(), c );
          return false;
      }
      continue;
    }

    switch ( c ) {
      case '\\':
        got_backslash = true;
        break;
      case '"':
        value.flush();
        t->value_ = value_;
        t->view_ = nullptr;
        t->view_size_ = 0;
        return true;
      default:
        value += c;
    }
  } // while
}

location& lexer::set_cur_loc_end( bool prev ) {
  return prev ?
    set_loc_end( &cur_loc_, prev_line(), prev_col() )
  :
    set_loc_end( &cur_loc_, line_, col() );
}

void lexer::set_loc( char const *file, line_type line, column_type col ) {
  line_ = line;
  line_start_ = pos() - (col - 1);
  if ( file ) {
    file_ = file;
    cur_loc_.set( file, line_, col, line_, col );
  }
}

void lexer::sync() {
  if ( cur_ == end_ )
    return;
  std::streambuf *const sb = in_->rdbuf();
  std::streampos const at = sb->pubseekoff( 0, ios::cur, ios::in );
  if ( at == std::streampos( std::streamoff( -1 ) ) ||
       sb->pubseekpos( at - std::streamoff( end_ - cur_ ), ios::in ) ==
         std::streampos( std::streamoff( -1 ) ) )
    return;                             // the stream cannot go back: keep the buffer
  buf_offset_ = pos();
  cur_ = end_ = buf_;
}

///////////////////////////////////////////////////////////////////////////////

#if DEBUG_JSON_PARSER

ostream& operator<<( ostream &o, parser::state s ) {
  static char const *const string_of[] = {
    "A0", "A1", "A2",
    "E0", "E1",
    "J0", "J1",
    "M0", "M1",
    "O0", "O1", "O2",
    "P0", "P1",
    "V0"
  };
  return o << string_of[ s ];
}

static void throw_unexpected_token( int line, token const &t ) {
  try {
    throw unexpected_token( t );
  }
  catch ( exception const &e ) {
    cerr << line << ": " << e.what() << endl;
    throw;
  }
}

bool parser::get_token_debug( int line, token *t ) {
  bool const got_token = get_token( t );
  cout << line << ": get_token => " << *t << endl;
  return got_token;
}

bool parser::matches_token_debug( int line, token::type tt, token *t ) {
  bool const matched = matches_token( tt, t );
  cout << line << ": token " << *t << " matches " << tt << " => " << (matched ? 'T' : 'F') << endl;
  return matched;
}

token::type parser::peek_token_debug( int line ) {
  token::type const tt = peek_token();
  cout << line << ": peek_token => " << peeked_token_ << endl;
  return tt;
}

void parser::require_token_debug( int line, token::type tt, token *t ) {
  if ( !get_token_debug( line, t ) || t->get_type() != tt )
    throw_unexpected_token( line, *t );
}

# define GET_TOKEN(T)               get_token_debug( __LINE__, T )
# define MATCHES_TOKEN(TT,T)        matches_token_debug( __LINE__, TT, T )
# define PEEK_TOKEN()               peek_token_debug( __LINE__ )
# define REQUIRE_TOKEN(TT,T)        require_token_debug( __LINE__, TT, T )
# define THROW_UNEXPECTED_TOKEN(T)  throw_unexpected_token( __LINE__, T )

# define GOTO_STATE(S)  \
    if (0) ; else {     \
      state_ = (S);     \
      cout << __LINE__ << ':' << indent << "GOTO_STATE( " << state_ << " )" << endl;                    \
      continue;         \
    }

# define PUSH_STATE(S)      \
    if (0) ; else {         \
      state_stack_.push_back(S); \
      cout << __LINE__ << ':' << indent << "PUSH_STATE( " << (S) << " )" << endl << inc_indent;             \
    }

# define POP_STATE()                            \
    if (0) ; else {                             \
      state_ = pop_state();                     \
      cout << __LINE__ << ':' << indent << "POP_STATE() => " << state_ << endl << dec_indent;                                   \
    }

#else

# define GET_TOKEN(T)               get_token( T )
# define MATCHES_TOKEN(TT,T)        matches_token( TT, T )
# define PEEK_TOKEN()               peek_token()
# define REQUIRE_TOKEN(TT,T)        require_token( TT, T )
# define THROW_UNEXPECTED_TOKEN(T)  throw unexpected_token( T )

# define GOTO_STATE(S)              { state_ = (S); continue; }
# define PUSH_STATE(S)              state_stack_.push_back(S)
# define POP_STATE()                state_ = pop_state()

#endif /* DEBUG_JSON_PARSER */

///////////////////////////////////////////////////////////////////////////////

parser::parser( std::istream &in, bool allow_multiple ) :
  allow_multiple_( allow_multiple ),
  lexer_( in )
{
  state_stack_.reserve( 32 );
  init();
}

void parser::clear() {
  peeked_token_.clear();
  state_stack_.clear();
  init();
}

void parser::init() {
#if DEBUG_JSON_PARSER
  get_indent( cout ) = 0;
#endif /* DEBUG_JSON_PARSER */
  PUSH_STATE( J0 );
}

bool parser::get_token( token *t ) {
  if ( peeked_token_ ) {
    t->take( &peeked_token_ );
    return true;
  }
  t->clear();
  return lexer_.next( t );
}

bool parser::matches_token( token::type tt, token *t ) {
  if ( peek_token() == tt )
    return get_token( t );
  // Before t is read, get_token() replaces it, but for the location when there is no token.
  t->set_loc(
    peeked_token_.file_, peeked_token_.line_, peeked_token_.column_,
    peeked_token_.line_end_, peeked_token_.column_end_
  );
  t->type_ = peeked_token_.type_;
  t->numeric_type_ = peeked_token_.numeric_type_;
  return false;
}

token::type parser::peek_token() {
  if ( !peeked_token_ )
    lexer_.next( &peeked_token_ );
  return peeked_token_.get_type();
}

#if ! DEBUG_JSON_PARSER
void parser::require_token( token::type tt, token *t ) {
  if ( !get_token( t ) || t->get_type() != tt )
    THROW_UNEXPECTED_TOKEN( *t );
}
#endif /* DEBUG_JSON_PARSER */

bool parser::next( token *t ) {
  if ( state_stack_.empty() )
    return false;
  POP_STATE();
  while ( true ) {
    switch ( state_ ) {

      // <JSON> ::= <Array> | <Object>
      case J0:  PUSH_STATE( allow_multiple_ ? J0 : J1 );
                switch ( PEEK_TOKEN() ) {
                  case token::begin_array : GOTO_STATE( A0 );
                  case token::begin_object: GOTO_STATE( O0 );
                  case token::none        : break;
                  default: THROW_UNEXPECTED_TOKEN( peeked_token_ );
                }
                [[fallthrough]];
      case J1:  return false;

      // <Array> ::= '[' <Element>* ']'
      case A0:  REQUIRE_TOKEN( token::begin_array, t );
                PUSH_STATE( A1 );
                return true;
      case A1:  if ( MATCHES_TOKEN( token::end_array, t ) )
                  return true;
                PUSH_STATE( A2 );
                GOTO_STATE( E0 );
      case A2:  REQUIRE_TOKEN( token::end_array, t );
                return true;

      // <Element> ::= <Value> [ ',' <Element> ]
      case E0:  PUSH_STATE( E1 );
                GOTO_STATE( V0 );
      case E1:  if ( MATCHES_TOKEN( token::value_separator, t ) ) {
                  PUSH_STATE( E0 );
                  return true;
                }
                POP_STATE();
                continue;

      // <Object> ::= '{' <Member>* '}'
      case O0:  REQUIRE_TOKEN( token::begin_object, t );
                PUSH_STATE( O1 );
                return true;
      case O1:  if ( MATCHES_TOKEN( token::end_object, t ) )
                  return true;
                PUSH_STATE( O2 );
                GOTO_STATE( M0 );
      case O2:  REQUIRE_TOKEN( token::end_object, t );
                return true;

      // <Member> ::= <Pair> [ ',' <Member> ]
      case M0:  PUSH_STATE( M1 );
                GOTO_STATE( P0 );
      case M1:  if ( MATCHES_TOKEN( token::value_separator, t ) ) {
                  PUSH_STATE( M0 );
                  return true;
                }
                POP_STATE();
                continue;

      // <Pair> ::= <String> ':' <Value>
      case P0:  REQUIRE_TOKEN( token::string, t );
                PUSH_STATE( P1 );
                return true;
      case P1:  REQUIRE_TOKEN( token::name_separator, t );
                PUSH_STATE( V0 );
                return true;

      // <Value> ::= <Array> | <Object> | <String> | <Number>
      //           | false | null | true
      case V0:  switch ( PEEK_TOKEN() ) {
                  case token::begin_array:
                    GOTO_STATE( A0 );
                  case token::begin_object:
                    GOTO_STATE( O0 );
                  case token::string:
                  case token::number:
                  case token::json_false:
                  case token::json_null:
                  case token::json_true:
                    GET_TOKEN( t );
                    return true;
                  default:
                    THROW_UNEXPECTED_TOKEN( peeked_token_ );
                }
    } // switch ( state_ )
  } // while
}

token::type parser::peek( token *t ) {
  if ( token::type const tt = PEEK_TOKEN() ) {
    if ( t )
      *t = peeked_token_;
    return tt;
  }
  return token::none;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace json
} // namespace zorba

/* vim:set et sw=2 ts=2: */
