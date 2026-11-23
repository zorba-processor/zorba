/*
 * Copyright 2006-2011 2006-2016 zorba.io.
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

#ifndef ZORBA_JSON_LOADER_H
#define ZORBA_JSON_LOADER_H

// standard
#include <istream>
#include <memory>
#include <stack>
#include <string_view>
#include <utility>                          /* for pair */
#include <vector>

// Zorba
#include "store/api/item.h"
#include "util/hash/hash.h"
#include "util/json_parser.h"
#include "util/unordered_map.h"
#include "zorbatypes/zstring.h"

namespace zorba {
namespace json {

///////////////////////////////////////////////////////////////////////////////

/**
 * What a query reads of a parsed value: all of it, or some keys of an object,
 * each with what is read of its value.  Of an array where keys are read, no
 * member is read: looking up a key in an array gives nothing.
 */
struct projection {
  bool all;
  std::vector<zstring> keys;
  std::vector<projection> values;       // what is read of each key's value

  projection() : all( false ) { }

  /**
   * Notes that a query reads all of the value at the end of a path of keys.
   *
   * @param path The keys from this value.
   */
  void add( std::vector<zstring> const &path );

  /**
   * Gets what is read of a key's value.
   *
   * @param key The key.
   * @return Returns said projection or \c nullptr if nothing is read of it.
   */
  projection const* find( std::string_view key ) const;

  /**
   * Gets this %projection as text that parse() reads back.
   */
  zstring str() const;

  /**
   * Reads a %projection from the text that str() gives.
   *
   * @param text The text.
   * @return Returns \c false if \a text is not such text.
   */
  bool parse( std::string_view *text );
};

///////////////////////////////////////////////////////////////////////////////

class loader {
public:
  typedef parser::line_type line_type;
  typedef parser::column_type column_type;

  /**
   * Constructs a %loader.
   *
   * @param is The istream to read from.
   * @param allow_multiple If \c true, allow multiple top-level JSON items to
   * be returned.
   * @param strip_top_level_array If \c true, strips the top-level array, if
   * any.
   * @param skip_bom If \c true, skips a byte order mark at the current
   * position of \a is.  Two loaders that share a stream skip it only once.
   */
  loader( std::istream &is, bool allow_multiple = false,
          bool strip_top_level_array = false, bool skip_bom = true );

  /**
   * Destroys this %loader.
   */
  ~loader();

  /**
   * Resets this %loader to its initial state.
   */
  void clear();

  /**
   * Gets the next Item, if any.
   *
   * @param result A pointer to the Item to receive the Item.
   * @return Returns \c true only if an Item was gotten.
   */
  bool next( store::Item_t *result );

  /**
   * Makes next() return the members of each top-level array, one at a time and
   * without building the array, and then a null Item for the end of that array.
   * A top-level array counts for the nesting limit as it does when it is built.
   * Top-level objects are returned as usual: in_top_level_array() tells them
   * from members.
   */
  void unbox_top_level_arrays() {
    unbox_top_level_arrays_ = true;
  }

  /**
   * With unbox_top_level_arrays(), makes next() check the JSON without building
   * Items: it raises what building them raises, in the same order, and returns
   * a null Item for each member and for the end of each top-level value.
   * in_top_level_array() and last_value_was_array() tell them apart.
   */
  void check_only() {
    check_only_ = true;
  }

  /**
   * Makes next() build only what a query reads of each top-level value (see
   * projection): the rest is checked as check_only() does, so it raises what
   * building it raises, in the same order, and a value keeps its kind.
   *
   * @param p What is read.  It must outlive this %loader.
   */
  void project( projection const *p ) {
    projection_ = p;
  }

  /**
   * Gets whether the top-level value that check_only() parsing has just ended
   * was an array.
   */
  bool last_value_was_array() const {
    return last_value_was_array_;
  }

  /**
   * Gets whether the Item that next() returned is a member of a top-level array
   * (see unbox_top_level_arrays()).
   */
  bool in_top_level_array() const {
    return in_top_level_array_;
  }

  /**
   * Sets the file location.
   *
   * @param file The source file name.
   * @param line The source line number.
   * @param col  The source column number.
   */
  void set_loc( char const *file, line_type line, column_type col ) {
    parser_.set_loc( file, line, col );
  }

  /**
   * Gives back to the stream what was read from it but not parsed, so that
   * its position is that of the next character to parse (see lexer::sync()).
   */
  void sync() {
    parser_.sync();
  }

private:
  // What an open array or object has read: an array's members are values_.
  struct contents {
    std::vector<store::Item_t> keys_;
    std::vector<store::Item_t> values_;
  };

  struct stack_element {
    enum type {
      no_type,
      array_type,
      object_type,
      key_type
    };
    type const type_;
    union {
      contents *contents_;              // of an array or object, in scratch_
      store::Item *key_;
    };
    bool skipped_;                      // nothing of it is read
    projection const *read_;            // what is read of it; null: all

    stack_element( type = no_type, contents* = nullptr );
    void destroy();
  };

  struct destroyer {
    stack_element &e_;
    ~destroyer() { e_.destroy(); }
  };

  typedef std::stack<stack_element> stack_type;
  stack_type stack_;

  void check_depth( json::token const& );
  void add_value( store::Item_t const& );
  void check_duplicate_keys( std::vector<store::Item_t> const& ) const;
  void clear_stack();
  void make_key( std::string_view, store::Item_t* );
  static char const* number_of( token const&, char (&)[64] );
  void record_key( store::Item_t const& );
  void end_top_level_value();
  void read_of_value( bool *skipped, projection const **read ) const;
  bool building_value() const;
  void keep_read_pairs( contents* ) const;
  stack_element& push( stack_element::type );

  parser parser_;
  bool const strip_top_level_array_;
  bool stripped_top_level_array_;
  bool unbox_top_level_arrays_;
  bool in_top_level_array_;
  bool check_only_;
  bool last_value_was_array_;
  projection const *projection_;
  uint32_t depth_;                      // arrays and objects on stack_

  // The contents of the arrays and objects on stack_, by position: an element
  // reuses the vectors of the last one at its position, already grown.
  std::vector<std::unique_ptr<contents>> scratch_;

  //
  // Object keys of this parse: objects with the same key share one key item
  // (and its string). Key items are immutable, so an update of one object
  // (rename, delete, insert) never affects another object. The table holds a
  // reference to each item; it is bounded so that a long stream of objects
  // with ever-new keys does not keep them all alive.
  //
  static unsigned const max_shared_keys = 1024;
  static unsigned const max_shared_key_length = 256;
  typedef std::unordered_map<zstring,store::Item_t> keys_type;
  keys_type keys_;

  // Values of one stream tend to repeat their keys in order: match against the previous value's before hashing.
  static unsigned const key_lookahead = 4;
  std::vector<store::Item_t> prev_keys_;
  std::vector<store::Item_t> cur_keys_;
  size_t prev_key_pos_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace json
} // namespace zorba

#endif /* ZORBA_JSON_LOADER_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
