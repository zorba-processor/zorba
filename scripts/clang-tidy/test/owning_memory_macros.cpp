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

// Fixture for zorba-owning-memory with macros, checked with
// owning_memory_macros.diff: a new or delete is reported only when the line
// it is written on (the macro body, or the macro argument at the invocation)
// is changed. Invoking an unchanged macro whose body has a raw new or delete
// on a changed line is not reported.

#include "owning_memory_mocks.h"

namespace fixture {

struct MacroPlain { int value = 0; };

#define MAKE_AND_DROP(T) do { T* p = new T; delete p; } while (0)
#define PASS(x) x
#define CHANGED_BODY(T) do { T* q = new T; delete q; } while (0)
#define OUTER(T) PASS(new T)

void macros() {
  MAKE_AND_DROP(MacroPlain);
  MacroPlain* r = PASS(new MacroPlain);
  PASS(delete r);
  CHANGED_BODY(MacroPlain);
  MacroPlain* s = OUTER(MacroPlain);
  PASS(delete s);
}

} // namespace fixture
