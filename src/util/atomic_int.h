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
#ifndef ATOMIC_INT_H
#define ATOMIC_INT_H

#include <atomic>
#include <cstdint>

namespace zorba {

/**
 * An %atomic_int behaves like an ordinary \c int except that all modifications
 * and accesses are guaranteed to be thread-safe.
 *
 * The API is a subset of the forthcoming C++ standard addition of atomic types;
 * see: http://www.open-std.org/JTC1/sc22/wg21/docs/papers/2007/n2427.html
 */
class atomic_int {
public:
  typedef std::int32_t value_type;

  /**
   * Constructs an %atomic_int.
   *
   * @param n The initial value.
   */
  explicit atomic_int( value_type n = 0 ) : value_( n ) {
  }

  /**
   * Atomically assigns a new value.
   *
   * @param n The new value.
   * @return Returns a reference to <code>this</code>.
   */
  atomic_int& operator=( value_type n ) {
    store( n );
    return *this;
  }

  /**
   * Atomically reads the current value.
   *
   * @return Returns the current value.
   */
  value_type load() const {
    return value_.load( std::memory_order_acquire );
  }

  /**
   * Atomically writes a new value.
   *
   * @param n The new value.
   */
  void store( value_type n ) {
    value_.store( n, std::memory_order_release );
  }

  /**
   * Atomically pre-increments the value.
   *
   * @return Returns the new value.
   */
  value_type operator++() {
    return value_.fetch_add( 1, std::memory_order_acq_rel ) + 1;
  }

  /**
   * Atomically post-increments the value.
   *
   * @return Returns the old value.
   */
  value_type operator++(int) {
    return value_.fetch_add( 1, std::memory_order_acq_rel );
  }

  /**
   * Atomically pre-decrements the value.
   *
   * @return Returns the new value.
   */
  value_type operator--() {
    return value_.fetch_sub( 1, std::memory_order_acq_rel ) - 1;
  }

  /**
   * Atomically post-decrements the value.
   *
   * @return Returns the old value.
   */
  value_type operator--(int) {
    return value_.fetch_sub( 1, std::memory_order_acq_rel );
  }

  /**
   * Atomically adds a value.
   *
   * @return Returns the new value.
   */
  value_type operator+=( value_type n ) {
    return value_.fetch_add( n, std::memory_order_acq_rel ) + n;
  }

  /**
   * Atomically subtracts a value.
   *
   * @return Returns the new value.
   */
  value_type operator-=( value_type n ) {
    return value_.fetch_sub( n, std::memory_order_acq_rel ) - n;
  }

private:
  std::atomic<value_type> value_;

  // forbid because they can't be done atomically
  atomic_int( atomic_int const& );
  atomic_int& operator=( atomic_int& );
};

} // namespace zorba

#endif /* ATOMIC_INT_H */
/* vim:set et sw=2 ts=2: */
