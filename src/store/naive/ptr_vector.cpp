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

#ifdef ZORBA_CONTAINER_CHECKS

#include <cstdio>
#include <cstdlib>
#ifdef ZORBA_HAVE_EXECINFO_H
#include <execinfo.h>
#include <unistd.h>
#endif

#include "ptr_vector.h"

namespace zorba
{
namespace simplestore
{

bool PtrVectorCheck::theThrow = false;


void PtrVectorCheck::fail(const char* cond, const char* file, int line)
{
  if (theThrow)
    throw PtrVectorCheckFailure(cond);

  std::fprintf(stderr, "%s:%d: PtrVector iterator check failed: %s\n",
               file, line, cond);
#ifdef ZORBA_HAVE_EXECINFO_H
  void* frames[32];
  int const n = backtrace(frames, 32);
  backtrace_symbols_fd(frames, n, STDERR_FILENO);
#endif
  std::abort();
}

} // namespace simplestore
} // namespace zorba

#endif /* ZORBA_CONTAINER_CHECKS */
/* vim:set et sw=2 ts=2: */
