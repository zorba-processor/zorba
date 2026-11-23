/*
 * Copyright 2006-2026 zorba.io
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
#ifndef ZORBA_TEST_OPTIMIZATION_LEVEL_H
#define ZORBA_TEST_OPTIMIZATION_LEVEL_H

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <zorba/options.h>

namespace zorba_test {

/**
 * The optimization level the test drivers compile queries at, from the
 * environment variable ZORBA_OPTLEVEL:
 *
 *   unset or ""  O1
 *   "O0"         O0
 *   "O1"         O1
 *   "O2"         O2
 *
 * Any other value ends the driver with status 1 (after printing a message),
 * so that a typo cannot silently run the tests at another level.
 */
inline Zorba_opt_level_t testOptimizationLevel()
{
  char const* const lValue = std::getenv("ZORBA_OPTLEVEL");
  if (lValue == nullptr || *lValue == '\0' || std::strcmp(lValue, "O1") == 0)
    return ZORBA_OPT_LEVEL_O1;
  if (std::strcmp(lValue, "O0") == 0)
    return ZORBA_OPT_LEVEL_O0;
  if (std::strcmp(lValue, "O2") == 0)
    return ZORBA_OPT_LEVEL_O2;
  std::cerr << "ZORBA_OPTLEVEL: unknown value \"" << lValue
            << "\" (expected \"O0\", \"O1\" or \"O2\")" << std::endl;
  std::exit(1);
}

inline char const* optimizationLevelName(Zorba_opt_level_t aLevel)
{
  switch (aLevel)
  {
  case ZORBA_OPT_LEVEL_O0: return "O0";
  case ZORBA_OPT_LEVEL_O1: return "O1";
  case ZORBA_OPT_LEVEL_O2: return "O2";
  }
  return "unknown";
}

} // namespace zorba_test

#endif // ZORBA_TEST_OPTIMIZATION_LEVEL_H
/* vim:set et sw=2 ts=2: */
