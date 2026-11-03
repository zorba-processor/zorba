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
#pragma once
#ifndef ZORBA_ZORBAUTILS_NUMERIC_KEY_HASH_H
#define ZORBA_ZORBAUTILS_NUMERIC_KEY_HASH_H

#include <zorba/config.h>

#include "common/common.h"
#include "store/api/item.h"
#include "zorbatypes/zorbatypes_decl.h"

namespace zorba {

/**
 * The hash function of a hash container whose keys are compared with the eq
 * operator (fn:distinct-values, group by).
 *
 * Under eq (F&O 3.0 op:numeric-equal, XPath 3.0 B.1) an xs:integer or xs:decimal
 * compared with an xs:float is cast to xs:float: 16777217 eq xs:float(16777216).
 * For values equal to a key of type xs:float to be found, they must be hashed on
 * their xs:float rounding. Store::Item::hash() can not do that: all the integers
 * that round to one xs:float would share a hash code, up to 2^(e-23) of them
 * around 2^e. So this has three modes, and a container starts with the cheapest:
 *
 * - exact: Item::hash(). It is consistent with eq for every pair of numeric types
 *   except with xs:float. No cost for a container that never sees an xs:float.
 * - via_float: from the first key of type xs:float on, every numeric key hashes
 *   on its xs:float rounding. Only this container pays: its integers above 2^24
 *   share buckets, as above.
 * - linear: from the first key that rounds to different xs:float values directly
 *   and through xs:double. Such a value equals an xs:float and an xs:double that
 *   differ from each other, and rounding every such value to xs:float would join
 *   adjacent xs:float values, so no hash of the rounding can be consistent with
 *   eq: all numeric keys of this container hash alike, and lookups scan them.
 *
 * A container calls prepare() with each key before looking it up or inserting it.
 * fn:distinct-values and group by then satisfy the constraints F&O 3.0 and XQuery
 * 3.0 state for non-transitive equality: (a) no two items of the result compare
 * equal, (b) every other input item compares equal to an item of the result.
 */
class NumericKeyHash {
public:
  NumericKeyHash() : mode_( exact ), saw_double_rounding_( false ) { }

  /**
   * Gets the hash code of a key.
   *
   * @param key The key.
   * @param timezone The implicit timezone.
   * @param collator The collator for strings, if any.
   * @return Its hash code.
   */
  uint32_t hash( store::Item const *key, long timezone,
                 XQPCollator const *collator ) const {
    return mode_ == exact ?
      key->hash( timezone, collator ) : hash_numeric( key, timezone, collator );
  }

  /**
   * Prepares for the given key: changes mode if the key requires it, in which
   * case every key already in the given container is placed again.
   *
   * @tparam MapType The hash map type, whose compare function's hash() is based
   * on this object.
   * @param map The hash map.
   * @param key The key that is about to be looked up or inserted.
   */
  template<class MapType>
  void prepare( MapType &map, store::Item const *key ) {
    if ( mode_ == linear )
      return;
    if ( needs_change( key ) ) {
      map.rehash();
      if ( mode_ == via_float && saw_double_rounding_ ) {
        // A key already in the container rounds differently.
        mode_ = linear;
        map.rehash();
      }
    }
  }

  /**
   * Back to the initial mode, for a container that has been cleared.
   */
  void reset() {
    mode_ = exact;
    saw_double_rounding_ = false;
  }

private:
  enum mode { exact, via_float, linear };

  uint32_t hash_numeric( store::Item const*, long, XQPCollator const* ) const;
  bool needs_change( store::Item const* );

  mode mode_;
  mutable bool saw_double_rounding_;
};

/**
 * Gets whether a type is xs:decimal or a type derived from it.
 *
 * @param code The type.
 * @return Returns \c true only if it is.
 */
bool is_exact_numeric( store::SchemaTypeCode code );

} // namespace zorba

#endif /* ZORBA_ZORBAUTILS_NUMERIC_KEY_HASH_H */
/* vim:set et sw=2 ts=2: */
