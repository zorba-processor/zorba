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
#include "stdafx.h"

#include "runtime/util/stack_limit.h"

#include <cstdint>

#if defined(__EMSCRIPTEN__)
# include <emscripten/stack.h>
#elif defined(_WIN32)
# include <windows.h>
#elif defined(__APPLE__)
# include <pthread.h>
#elif defined(__unix__)
# include <pthread.h>
# include <sys/resource.h>
# include <sys/syscall.h>
# include <unistd.h>
#endif

namespace zorba {
namespace stack_limit {

namespace {

struct Bounds
{
  uintptr_t low = 0;   // lowest usable address (the stack grows down)
  uintptr_t high = 0;  // highest address; 0 = unknown
  size_t reserve = 0;
};

Bounds query_bounds()
{
  Bounds b;
#if defined(__EMSCRIPTEN__)
  b.low = emscripten_stack_get_end();
  b.high = emscripten_stack_get_base();
#elif defined(_WIN32)
  ULONG_PTR lLow, lHigh;
  GetCurrentThreadStackLimits(&lLow, &lHigh);
  b.low = lLow;
  b.high = lHigh;
#elif defined(__APPLE__)
  pthread_t const self = pthread_self();
  uintptr_t const top =
    reinterpret_cast<uintptr_t>(pthread_get_stackaddr_np(self));
  size_t const size = pthread_get_stacksize_np(self);
  b.low = top - size;
  b.high = top;
#elif defined(__unix__)
  pthread_attr_t attr;
  if (pthread_getattr_np(pthread_self(), &attr) == 0)
  {
    void* addr = nullptr;
    size_t size = 0;
    if (pthread_attr_getstack(&attr, &addr, &size) == 0 && size != 0)
    {
      b.low = reinterpret_cast<uintptr_t>(addr);
      b.high = b.low + size;

      // The main thread's stack grows on demand. glibc derives its size from
      // RLIMIT_STACK; with an unlimited limit (common in containers) it
      // reports the whole gap down to the next mapping, which the stack can
      // never actually use. Assume 8 MiB then, the usual default.
      if (getpid() == static_cast<pid_t>(syscall(SYS_gettid)))
      {
        size_t lLimit = 8 * 1024 * 1024;
        struct rlimit rl;
        if (getrlimit(RLIMIT_STACK, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY)
          lLimit = static_cast<size_t>(rl.rlim_cur);
        if (size > lLimit)
          b.low = b.high - lLimit;
      }
    }
    pthread_attr_destroy(&attr);
  }
#endif
  if (b.high != 0)
  {
    // A third of the stack, at least 64 KiB. Not only must the error be
    // raised and unwound: closing the plan afterwards recurses through every
    // call level again (UDF state -> body plan -> nested UDF state), starting
    // near the top of the stack, and its frames per level can be larger than
    // those of the calls (seen with GCC 16 -O2). Keeping a third of the stack
    // free lets the close use up to 1.5 times the stack of the calls.
    size_t const lThird = (b.high - b.low) / 3;
    b.reserve = lThird < 64 * 1024 ? 64 * 1024 : lThird;
  }
  return b;
}

const Bounds& bounds()
{
  // A thread's stack does not move; query it once per thread.
  thread_local Bounds const theBounds = query_bounds();
  return theBounds;
}

inline uintptr_t current_frame()
{
#if defined(__GNUC__) || defined(__clang__)
  // The real frame address, also under ASan's fake stacks.
  return reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
#else
  volatile char lMarker = 0;
  return reinterpret_cast<uintptr_t>(&lMarker);
#endif
}

} // anonymous namespace


size_t remaining()
{
  Bounds const& b = bounds();
  uintptr_t const sp = current_frame();
  if (b.high == 0 || sp <= b.low || sp > b.high)
    return SIZE_MAX;
  return sp - b.low;
}


size_t reserve()
{
  return bounds().reserve;
}


bool exhausted()
{
  Bounds const& b = bounds();
  uintptr_t const sp = current_frame();
  // Unknown bounds, or a stack the thread library does not know about.
  if (b.high == 0 || sp <= b.low || sp > b.high)
    return false;
  return sp - b.low < b.reserve;
}

} // namespace stack_limit
} // namespace zorba
/* vim:set et sw=2 ts=2: */
