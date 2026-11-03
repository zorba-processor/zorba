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

#include <stdexcept>

// Zorba
#include <store/api/item.h>
#include <store/api/store.h>
#include <zorba/store_consts.h>
#include <zorba/util/stream_util.h>

#include "context/static_context.h"
#include "diagnostics/xquery_diagnostics.h"
#include "store/api/item_factory.h"
#include "system/globalenv.h"
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

loader::stack_element::stack_element( type t ) : type_( t ) {
  switch ( type_ ) {
    case array_type:
      array_ = new json_array_type;
      break;
    case object_type:
      object_ = new json_object_type;
      break;
    default:
      key_ = nullptr;
      break;
  }
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
      delete array_;
      break;
    case object_type:
      delete object_;
      break;
    case key_type:
      key_->removeReference();
      break;
    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

loader::loader( istream &is, bool allow_multiple, bool strip_top_level_array ) :
  parser_( is, allow_multiple ),
  strip_top_level_array_( strip_top_level_array ),
  stripped_top_level_array_( false ),
  depth_( 0 )
{
  // F&O 3.1 fn:parse-json: "If the input starts with a byte order mark, this
  // function ignores it."  Not in json::lexer, which also checks single values.
  skip_utf8_bom( is, false );
}

loader::~loader() {
  clear_stack();
}

void loader::add_value( store::Item_t const &value ) {
  stack_element top( stack_.top() );
  switch ( top.type_ ) {
    case stack_element::array_type:
      top.array_->push_back( value );
      break;
    case stack_element::object_type:
      //
      // value must be a string that's the name of the object's next key/value
      // pair.
      //
      push( stack_element::key_type ).key_ = value.getp();
      value->addReference();
      break;
    case stack_element::key_type: {
      //
      // Otherwise, the top of the stack must be a string which means that the
      // second-to-top must be an object awaiting a value associated with this
      // name.
      //
      stack_.pop();
      stack_element &top2 = stack_.top();
      assert( top2.type_ == stack_element::object_type );
      top2.object_->keys_.push_back( top.key_ );
      top2.object_->values_.push_back( value );
      top.destroy();
      break;
    }
    default:
      assert( false );
  }
}

void loader::clear() {
  parser_.clear();
  clear_stack();
  keys_.clear();
  stripped_top_level_array_ = false;
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

void loader::make_key( zstring const &s, store::Item_t *result ) {
  keys_type::const_iterator const i( keys_.find( s ) );
  if ( i != keys_.end() ) {
    *result = i->second;
    return;
  }
  zstring copy( s );
  GENV_ITEMFACTORY->createString( *result, copy );
  if ( keys_.size() < max_shared_keys && s.size() <= max_shared_key_length )
    keys_.insert( keys_type::value_type( (*result)->getString(), *result ) );
}

void loader::clear_stack() {
  while ( !stack_.empty() ) {
    stack_.top().destroy();
    stack_.pop();
  }
}

bool loader::next( store::Item_t *result ) {
  store::Item_t item;
  zstring s;
  json::token t;

  try {
    while ( parser_.next( &t ) ) {
      switch( t.get_type() ) {
        case '[':
          if ( strip_top_level_array_ && !stripped_top_level_array_ )
            stripped_top_level_array_ = true;
          else {
            check_depth( t );
            push( stack_element::array_type );
          }
          continue;
        case '{':
          check_depth( t );
          push( stack_element::object_type );
          continue;
        case ']':
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
          struct destroyer {
            stack_element &e_;
            ~destroyer() { e_.destroy(); }
          } const destroy_top{ top };
          switch ( top.type_ ) {
            case stack_element::array_type:
              GENV_ITEMFACTORY->createJSONArray( item, *top.array_ );
              break;
            case stack_element::object_type:
              GENV_ITEMFACTORY->createJSONObject(
                item, top.object_->keys_, top.object_->values_
              );
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
          try {
            switch ( t.get_numeric_type() ) {
              case token::integer:
                try {
                  GENV_ITEMFACTORY->createInteger(
                    item, xs_integer( t.get_value() )
                  );
                }
                catch ( std::range_error const& ) {
                  // Without big integers, xs:integer is 64-bit: keep a larger
                  // integer (e.g. an ID) exactly as an xs:decimal rather than
                  // failing to parse the document.
                  GENV_ITEMFACTORY->createDecimal(
                    item, xs_decimal( t.get_value() )
                  );
                }
                break;
              case token::decimal:
                GENV_ITEMFACTORY->createDecimal(
                  item, xs_decimal( t.get_value() )
                );
                break;
              case token::floating_point:
                GENV_ITEMFACTORY->createDouble(
                  item, xs_double( t.get_value() )
                );
                break;
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
          s = t.get_value();
          if ( !stack_.empty() &&
               stack_.top().type_ == stack_element::object_type )
            make_key( s, &item );       // the string is an object's key
          else
            GENV_ITEMFACTORY->createString( item, s );
          break;
        case 'F':
        case 'T':
          GENV_ITEMFACTORY->createBoolean( item, t.get_type() == 'T' );
          break;
        case token::json_null:
          GENV_ITEMFACTORY->createJSONNull( item );
          break;
        default:
          assert( false );
      } // switch

      if ( stack_.empty() ) {
        *result = item;
        return true;
      }
      add_value( item );
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
