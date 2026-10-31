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
#ifndef ZORBA_RUNTIME_UTIL_STACK_LIMIT_H
#define ZORBA_RUNTIME_UTIL_STACK_LIMIT_H

#include <cstddef>

namespace zorba {
namespace stack_limit {

/**
 * Returns the number of bytes of machine stack left below the caller's frame
 * on the current thread, or SIZE_MAX if it cannot be determined (unknown
 * platform, or the caller runs on a stack the thread library does not know
 * about, such as a coroutine stack).
 */
size_t remaining();

/**
 * Returns the number of bytes of stack that must stay free when a new
 * function call level is entered: a third of the thread's stack, at least
 * 64 KiB. Each call level of a query uses some stack between two checks, the
 * error must still be raised and unwound, and closing the plan recurses
 * through all call levels again.
 */
size_t reserve();

/**
 * Returns true if entering one more call level would leave less than
 * reserve() bytes of stack.
 */
bool exhausted();

} // namespace stack_limit
} // namespace zorba

#endif /* ZORBA_RUNTIME_UTIL_STACK_LIMIT_H */
/* vim:set et sw=2 ts=2: */
