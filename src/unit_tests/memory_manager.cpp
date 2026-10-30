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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "compiler/expression/mem_manager.h"

///////////////////////////////////////////////////////////////////////////////

static int failures;

static bool assert_true( char const *expr, int line, bool result ) {
  if ( !result ) {
    std::cout << "FAILED, line " << line << ": " << expr << std::endl;
    ++failures;
  }
  return result;
}

#define ASSERT_TRUE( EXPR ) assert_true( #EXPR, __LINE__, !!(EXPR) )

static bool is_aligned( void const *p ) {
  return reinterpret_cast<std::uintptr_t>( p ) %
         alignof( std::max_align_t ) == 0;
}

///////////////////////////////////////////////////////////////////////////////

using namespace zorba;

/**
 * Blocks of odd sizes must still come back aligned for any fundamental type
 * (they used to be packed back to back, so a 1-byte block misaligned every
 * block after it).
 */
static void test_alignment() {
  MemoryManager mm;
  for ( size_t size = 1; size <= 100; ++size ) {
    void *const p = mm.allocate( size );
    ASSERT_TRUE( is_aligned( p ) );
    std::memset( p, 0xA5, size );
  }
  // Enough small blocks to spill onto further pages.
  for ( int i = 0; i < 5000; ++i ) {
    void *const p = mm.allocate( 3 + i % 29 );
    ASSERT_TRUE( is_aligned( p ) );
  }
}

/**
 * A block larger than a page must be fully usable.
 */
static void test_large_block() {
  MemoryManager mm;
  size_t const big = 3 * 16384 + 1;
  char *const p = static_cast<char*>( mm.allocate( big ) );
  ASSERT_TRUE( is_aligned( p ) );
  std::memset( p, 0x5A, big );
  void *const q = mm.allocate( 8 );
  ASSERT_TRUE( is_aligned( q ) );
  ASSERT_TRUE( static_cast<char*>( q ) + 8 <= p ||
               static_cast<char*>( q ) >= p + big );
}

/**
 * Placement new through the manager.
 */
static void test_placement_new() {
  struct S { char c; };
  struct alignas( std::max_align_t ) W { long double d; };
  MemoryManager mm;
  S *const s = new (mm) S;
  W *const w = new (mm) W;
  ASSERT_TRUE( s != nullptr );
  ASSERT_TRUE( is_aligned( w ) );
  w->d = 1.0L;
  ASSERT_TRUE( w->d == 1.0L );
}

///////////////////////////////////////////////////////////////////////////////

namespace zorba {
namespace UnitTests {

int test_memory_manager( int, char*[] ) {
  test_alignment();
  test_large_block();
  test_placement_new();

  std::cout << failures << " test(s) failed\n";
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
