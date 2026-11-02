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

#ifndef ZORBA_RUNTIME_JSON_NESTING_H
#define ZORBA_RUNTIME_JSON_NESTING_H

#include <cstdint>

#include "runtime/util/stack_limit.h"

namespace zorba {
namespace json_nesting {

/**
 * How many levels of arrays and objects a JSON value may have: [ ] has one
 * level.
 *
 * The heaviest walkers take up to 248 bytes of stack per level in release
 * builds (encode-for-roundtrip; serializing takes up to 224; measured by the
 * "measure" case of test/api/json_depth_limit), so 1500 levels need at most
 * 372 KiB. That is within the two thirds of the stack the guards allow on an
 * 8 MiB stack many times over, also with the larger frames of sanitizer
 * builds, and within the 341 KiB they allow on the 512 KiB default of a
 * secondary thread on macOS, where the same operations take at most 184 bytes
 * per level. It is also what query text can nest: the expression limit
 * (SecurityPolicy::setMaxExprDepth(), 1500) allows arrays 1497 levels deep.
 *
 * The parser refuses deeper documents and the store refuses to make a deeper
 * value (see JSONItem). An update can still deepen a value it reaches through
 * a member, since an item does not know its parents; what walks over values
 * therefore also checks the stack, with check_stack().
 */
uint32_t const MAX_DEPTH = 1500;

/** Raises zerr:ZXQP0071 for a value deeper than MAX_DEPTH. */
[[noreturn]] void raise_limit();

/** Raises zerr:ZXQP0071 for a value too deep for the stack of this thread. */
[[noreturn]] void raise_stack();

/** To be called once per level by what walks over a JSON value recursively. */
inline void check_stack() {
  if ( stack_limit::exhausted() )
    raise_stack();
}

} // namespace json_nesting
} // namespace zorba

#endif /* ZORBA_RUNTIME_JSON_NESTING_H */
/* vim:set et sw=2 ts=2: */
