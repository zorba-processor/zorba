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

#include <zorba/config.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <unordered_set>

// Zorba
#include <store/api/item.h>
#include <store/api/store.h>
#include <zorba/store_consts.h>
#include <zorba/util/stream_util.h>

#include "context/static_context.h"
#include "diagnostics/xquery_diagnostics.h"
#include "store/api/item_factory.h"
#include "system/globalenv.h"
#include "util/ascii_util.h"
#include "util/string_util.h"
#include "zorbatypes/decimal.h"
#include "zorbatypes/float.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/zstring.h"

// local
#include "common.h"
#include "json_loader.h"
#include "nesting.h"

using namespace std;

namespace zorba {
namespace json {

///////////////////////////////////////////////////////////////////////////////

loader::stack_element::stack_element( type t, contents *c ) :
  type_( t ), contents_( c ), skipped_( false ), read_( nullptr )
{
}

// Empties v for reuse; a large one gives its memory back rather than keep it for the rest of the parse.
static void release( vector<store::Item_t> &v ) {
  if ( v.capacity() > 4096 )
    vector<store::Item_t>().swap( v );
  else
    v.clear();
}

/**
 * Destroys a %stack_element.  We have this as an ordinary member function
 * rather than a destructor so that popping the stack and destroying an element
 * are distinct operations and can be called independently which comes in
 * handy.
 */
void loader::stack_element::destroy() {
  switch ( type_ ) {
    case array_type:
    case object_type:
      release( contents_->keys_ );
      release( contents_->values_ );
      break;
    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

loader::loader( istream &is, bool allow_multiple, bool strip_top_level_array,
                bool skip_bom ) :
  parser_( is, allow_multiple ),
  strip_top_level_array_( strip_top_level_array ),
  stripped_top_level_array_( false ),
  unbox_top_level_arrays_( false ),
  in_top_level_array_( false ),
  check_only_( false ),
  last_value_was_array_( false ),
  projection_( nullptr ),
  depth_( 0 ),
  prev_key_pos_( 0 )
{
  // F&O 3.1 fn:parse-json: "If the input starts with a byte order mark, this
  // function ignores it."  Not in json::lexer, which also checks single values.
  if ( skip_bom )
    skip_utf8_bom( is, false );
}

loader::~loader() {
  clear_stack();
}

void loader::add_value( store::Item_t &&value ) {
  stack_element top( stack_.top() );
  switch ( top.type_ ) {
    case stack_element::array_type:
      if ( !check_only_ && !top.skipped_ && !top.read_ )
        top.contents_->values_.push_back( std::move( value ) );
      break;
    case stack_element::object_type: {
      //
      // value must be a string that's the name of the object's next key/value
      // pair.
      //
      bool skipped = top.skipped_;
      projection const *read = nullptr;
      if ( !skipped && top.read_ ) {
        zstring const &name = value->getString();
        read = top.read_->find( std::string_view( name.data(), name.size() ) );
        skipped = !read;
        if ( read && read->all )
          read = nullptr;
      }
      top.contents_->keys_.push_back( std::move( value ) );
      stack_element &key = push( stack_element::key_type );
      key.skipped_ = skipped;
      key.read_ = read;
      break;
    }
    case stack_element::key_type: {
      //
      // Otherwise, the top of the stack is the key just added to the object
      // below it, which awaits the value associated with the key.
      //
      stack_.pop();
      stack_element &top2 = stack_.top();
      assert( top2.type_ == stack_element::object_type );
      // A value that is not read holds its key's place, for the duplicate check.
      if ( !check_only_ && !top2.skipped_ ) {
        if ( top.skipped_ )
          top2.contents_->values_.push_back( nullptr );
        else
          top2.contents_->values_.push_back( std::move( value ) );
      }
      break;
    }
    default:
      assert( false );
  }
}

/**
 * Raises what createJSONObject() raises for a duplicate key: the first key, in
 * order, that an earlier one equals.
 */
void loader::check_duplicate_keys( vector<store::Item_t> const &keys ) const {
  static size_t const max_linear = 16;
  std::unordered_set<zstring> seen;
  for ( size_t i = 0; i < keys.size(); ++i ) {
    bool duplicate = false;
    if ( keys.size() <= max_linear ) {
      for ( size_t j = 0; j < i && !duplicate; ++j )
        duplicate = keys[j] == keys[i] ||
                    keys[j]->getString() == keys[i]->getString();
    } else
      duplicate = !seen.insert( keys[i]->getString() ).second;
    if ( duplicate )
      throw XQUERY_EXCEPTION(
        jerr::JNDY0003, ERROR_PARAMS( keys[i]->getStringValue() )
      );
  }
}

void loader::clear() {
  parser_.clear();
  clear_stack();
  keys_.clear();
  prev_keys_.clear();
  cur_keys_.clear();
  prev_key_pos_ = 0;
  stripped_top_level_array_ = false;
  in_top_level_array_ = false;
  depth_ = 0;
}

void loader::check_depth( json::token const &t ) {
  if ( depth_ >= json_nesting::MAX_DEPTH )
    throw XQUERY_EXCEPTION(
      zerr::ZXQP0071_JSON_NESTING_EXCEEDED,
      ERROR_PARAMS( ZED( ZXQP0071_Limit_2 ), json_nesting::MAX_DEPTH ),
      ERROR_LOC( t.get_loc() )
    );
  ++depth_;
}

void loader::make_key( std::string_view s, store::Item_t *result ) {
  size_t const end = min( prev_keys_.size(), prev_key_pos_ + key_lookahead );
  for ( size_t i = prev_key_pos_; i < end; ++i ) {
    zstring const &key = prev_keys_[i]->getString();
    if ( std::string_view( key.data(), key.size() ) == s ) {
      *result = prev_keys_[i];
      prev_key_pos_ = i + 1;
      record_key( *result );
      return;
    }
  }
  zstring copy( s.data(), s.size() );
  keys_type::const_iterator const i( keys_.find( copy ) );
  if ( i != keys_.end() )
    *result = i->second;
  else {
    GENV_ITEMFACTORY->createString( *result, copy );
    if ( keys_.size() < max_shared_keys && s.size() <= max_shared_key_length )
      keys_.insert( keys_type::value_type( (*result)->getString(), *result ) );
  }
  record_key( *result );
}

void loader::record_key( store::Item_t const &key ) {
  if ( cur_keys_.size() < max_shared_keys )
    cur_keys_.push_back( key );
}

void loader::end_top_level_value() {
  prev_keys_.swap( cur_keys_ );
  cur_keys_.clear();
  prev_key_pos_ = 0;
}

// Numbers are short: parse them from a terminated copy on the stack, not from an allocated value.
char const* loader::number_of( token const &t, char (&buf)[64] ) {
  std::string_view const text( t.text() );
  if ( text.size() >= sizeof buf )
    return t.get_value().c_str();
  std::memcpy( buf, text.data(), text.size() );
  buf[ text.size() ] = '\0';
  return buf;
}

/**
 * Gets whether nothing is read of the value that starts now, and what is read
 * of it (null: all).
 */
void loader::read_of_value( bool *skipped, projection const **read ) const {
  if ( stack_.empty() ) {
    *skipped = false;
    *read = projection_;
    return;
  }
  stack_element const &top = stack_.top();
  if ( top.type_ == stack_element::array_type ) {
    *skipped = top.skipped_ || top.read_;
    *read = nullptr;
  } else {
    assert( top.type_ == stack_element::key_type );
    *skipped = top.skipped_;
    *read = top.read_;
  }
}

bool loader::building_value() const {
  if ( check_only_ )
    return false;
  bool skipped;
  projection const *read;
  read_of_value( &skipped, &read );
  return !skipped;
}

/**
 * Removes the pairs whose value is not read, once the keys of all of them are
 * checked for duplicates as createJSONObject() checks them.
 */
void loader::keep_read_pairs( contents *o ) const {
  check_duplicate_keys( o->keys_ );
  size_t n = 0;
  for ( size_t i = 0; i < o->values_.size(); ++i ) {
    if ( o->values_[i] ) {
      if ( n != i ) {
        o->keys_[n] = std::move( o->keys_[i] );
        o->values_[n] = std::move( o->values_[i] );
      }
      ++n;
    }
  }
  o->keys_.resize( n );
  o->values_.resize( n );
}

///////////////////////////////////////////////////////////////////////////////

void projection::add( std::vector<zstring> const &path ) {
  projection *p = this;
  for ( zstring const &key : path ) {
    if ( p->all )
      return;
    size_t i = 0;
    while ( i < p->keys.size() && p->keys[i] != key )
      ++i;
    if ( i == p->keys.size() ) {
      p->keys.push_back( key );
      p->values.push_back( projection() );
    }
    p = &p->values[i];
  }
  p->all = true;
  p->keys.clear();
  p->values.clear();
}

projection const* projection::find( std::string_view key ) const {
  for ( size_t i = 0; i < keys.size(); ++i )
    if ( std::string_view( keys[i].data(), keys[i].size() ) == key )
      return &values[i];
  return nullptr;
}

// "*" for all, else '{', then for each key its length, ':', the key and what is read of its value, then '}'.
zstring projection::str() const {
  if ( all )
    return "*";
  zstring result( "{" );
  for ( size_t i = 0; i < keys.size(); ++i ) {
    result += ztd::to_string( keys[i].size() );
    result += ':';
    result += keys[i];
    result += values[i].str();
  }
  result += '}';
  return result;
}

bool projection::parse( std::string_view *text ) {
  all = false;
  keys.clear();
  values.clear();
  if ( text->empty() )
    return false;
  char const c = text->front();
  text->remove_prefix( 1 );
  if ( c == '*' ) {
    all = true;
    return true;
  }
  if ( c != '{' )
    return false;
  while ( !text->empty() && text->front() != '}' ) {
    size_t len = 0;
    while ( !text->empty() && ascii::is_digit( text->front() ) ) {
      len = len * 10 + static_cast<size_t>( text->front() - '0' );
      text->remove_prefix( 1 );
    }
    if ( text->empty() || text->front() != ':' || text->size() < len + 1 )
      return false;
    keys.push_back( zstring( text->data() + 1, len ) );
    text->remove_prefix( len + 1 );
    values.push_back( projection() );
    if ( !values.back().parse( text ) )
      return false;
  }
  if ( text->empty() )
    return false;
  text->remove_prefix( 1 );
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void loader::clear_stack() {
  while ( !stack_.empty() ) {
    stack_.top().destroy();
    stack_.pop();
  }
}

loader::stack_element& loader::push( stack_element::type t ) {
  contents *c = nullptr;
  if ( t != stack_element::key_type ) {
    size_t const i = stack_.size();
    if ( scratch_.size() <= i )
      scratch_.resize( i + 1 );
    if ( !scratch_[i] )
      scratch_[i] = std::make_unique<contents>();
    c = scratch_[i].get();
  }
  stack_.push( stack_element( t, c ) );
  return stack_.top();
}

bool loader::next( store::Item_t *result ) {
  store::Item_t item;
  zstring s;
  json::token t;
  bool build;

  try {
    while ( parser_.next( &t ) ) {
      switch( t.get_type() ) {
        case '[':
          if ( unbox_top_level_arrays_ && stack_.empty() && !in_top_level_array_ ) {
            check_depth( t );
            in_top_level_array_ = true;
            continue;
          }
          if ( strip_top_level_array_ && !stripped_top_level_array_ &&
               stack_.empty() )
            stripped_top_level_array_ = true;
          else {
            check_depth( t );
            bool skipped;
            projection const *read;
            read_of_value( &skipped, &read );
            stack_element &e = push( stack_element::array_type );
            e.skipped_ = skipped;
            e.read_ = read;
          }
          continue;
        case '{': {
          check_depth( t );
          bool skipped;
          projection const *read;
          read_of_value( &skipped, &read );
          stack_element &e = push( stack_element::object_type );
          e.skipped_ = skipped;
          e.read_ = read;
          continue;
        }
        case ']':
          if ( unbox_top_level_arrays_ && stack_.empty() ) {
            --depth_;
            in_top_level_array_ = false;
            last_value_was_array_ = true;
            *result = nullptr;
            return true;
          }
          if ( stack_.empty() && strip_top_level_array_ ) {
            stripped_top_level_array_ = false;
            continue;
          }
          [[fallthrough]];
        case '}': {
          stack_element top( stack_.top() );
          stack_.pop();
          --depth_;
          // top is off the stack, so clear_stack() will not free it: free it
          // also when createJSONObject() raises JNDY0003 for a duplicate key.
          destroyer const destroy_top{ top };
          switch ( top.type_ ) {
            case stack_element::array_type:
              if ( !check_only_ && !top.skipped_ )
                GENV_ITEMFACTORY->createJSONArray(
                  item, std::move( top.contents_->values_ )
                );
              break;
            case stack_element::object_type:
              if ( check_only_ || top.skipped_ )
                check_duplicate_keys( top.contents_->keys_ );
              else {
                if ( top.read_ )
                  keep_read_pairs( top.contents_ );
                GENV_ITEMFACTORY->createJSONObject(
                  item, std::move( top.contents_->keys_ ),
                  std::move( top.contents_->values_ )
                );
              }
              break;
            default:
              assert( false );
          } // switch
          break;
        }
        case ':':
        case ',':
          continue;
        case token::number:
          build = building_value();
          try {
            char buf[64];
            char const *const n = number_of( t, buf );
            switch ( t.get_numeric_type() ) {
              case token::integer:
                try {
                  xs_integer const value( n );
                  if ( build )
                    GENV_ITEMFACTORY->createInteger( item, value );
                }
                catch ( std::range_error const& ) {
                  // Without big integers, xs:integer is 64-bit: keep a larger
                  // integer (e.g. an ID) exactly as an xs:decimal rather than
                  // failing to parse the document.
                  xs_decimal const value( n );
                  if ( build )
                    GENV_ITEMFACTORY->createDecimal( item, value );
                }
                break;
              case token::decimal: {
                xs_decimal const value( n );
                if ( build )
                  GENV_ITEMFACTORY->createDecimal( item, value );
                break;
              }
              case token::floating_point: {
                xs_double const value( n );
                if ( build )
                  GENV_ITEMFACTORY->createDouble( item, value );
                break;
              }
              default:
                assert( false );
            }
          }
          catch ( std::range_error const& ) {
            //
            // A number the lexer accepted does not get here: an xs:integer out
            // of range became an xs:decimal above, an xs:decimal has no range,
            // and XSD 1.1 floatingPointRound maps an xs:double out of range to
            // INF, -INF or zero (1e999 is INF, as in xs:double("1e999")).  If
            // one ever does, raise FOAR0002 rather than let std::range_error
            // escape as an internal error.
            //
            throw XQUERY_EXCEPTION(
              err::FOAR0002,
              ERROR_PARAMS( t.get_value() ),
              ERROR_LOC( t.get_loc() )
            );
          }
          break;
        case token::string:
          if ( !stack_.empty() &&
               stack_.top().type_ == stack_element::object_type ) {
            make_key( t.text(), &item );  // the string is an object's key
          } else if ( building_value() ) {
            t.get_value( &s );
            GENV_ITEMFACTORY->createString( item, s );
          }
          break;
        case 'F':
        case 'T':
          if ( building_value() )
            GENV_ITEMFACTORY->createBoolean( item, t.get_type() == 'T' );
          break;
        case token::json_null:
          if ( building_value() )
            GENV_ITEMFACTORY->createJSONNull( item );
          break;
        default:
          assert( false );
      } // switch

      if ( stack_.empty() ) {
        end_top_level_value();
        if ( check_only_ )
          last_value_was_array_ = false;
        *result = item;
        return true;
      }
      add_value( std::move( item ) );
    } // while
    return false;
  } // try
  catch ( json::illegal_character const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS(
          ZED( JNDY0021_IllegalCharacter_2 ),
          ascii::printable_char( e.get_char() )
        ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
  catch ( json::illegal_codepoint const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS(
          ZED( JNDY0021_IllegalCodepoint_2 ),
          e.get_codepoint()
        ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
  catch ( json::illegal_escape const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS(
          ZED( JNDY0021_IllegalEscape_2 ),
          ascii::printable_char( e.get_escape() )
        ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
  catch ( json::illegal_literal const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS( ZED( JNDY0021_IllegalLiteral ) ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
  catch ( json::illegal_number const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS( ZED( JNDY0021_IllegalNumber ) ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
  catch ( json::unexpected_token const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS(
          ZED( JNDY0021_UnexpectedToken_2 ),
          e.get_token()
        ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
  catch ( json::unterminated_string const &e ) {
    XQueryException xe(
      XQUERY_EXCEPTION(
        jerr::JNDY0021,
        ERROR_PARAMS( ZED( JNDY0021_UnterminatedString ) ),
        ERROR_LOC( e.get_loc() )
      )
    );
    set_data( &xe, e );
    throw xe;
  }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace json
} // namespace zorba

/* vim:set et sw=2 ts=2: */
