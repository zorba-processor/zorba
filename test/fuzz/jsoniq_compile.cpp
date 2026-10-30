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

// Fuzz target: compile arbitrary text as a JSONiq main module.
// The static context is put in JSONiq 1.0 mode, which is what zorba(1) does
// for *.jq files; the query may still switch with a version declaration.

#include "fuzz_common.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  zorba_fuzz::engine().init();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  zorba_fuzz::compile(data, size, /*jsoniq=*/true);
  return 0;
}
