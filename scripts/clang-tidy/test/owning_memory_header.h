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

// Fixture for zorba-owning-memory in a header, analyzed through
// owning_memory_header.cpp with HeaderFilterRegex '.*' as in CI: every line
// marked VIOLATION must be reported, and no other line.

#ifndef ZORBA_CLANG_TIDY_OWNING_MEMORY_HEADER_H
#define ZORBA_CLANG_TIDY_OWNING_MEMORY_HEADER_H

#include "owning_memory_mocks.h"

namespace fixture {

struct HeaderNode : public zorba::SimpleRCObject {};
struct HeaderPlain { int value = 0; };

inline HeaderPlain* header_make_raw() { return new HeaderPlain; } // VIOLATION

inline zorba::rchandle<HeaderNode> header_make_handle() { return new HeaderNode; }

inline void header_release(HeaderPlain* p) {
  delete p; // VIOLATION
}

} // namespace fixture

#endif /* ZORBA_CLANG_TIDY_OWNING_MEMORY_HEADER_H */
