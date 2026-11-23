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

// The dynamic loader used by several threads at once: the threads, released
// together, compile the first queries that need the test-only external module
// mt-loader (a shared library in TEST_LIB_PATH), so the library is opened and
// its module created concurrently; then they keep importing it, and declaring
// its function in the main module (with the module's version option), which
// loads it through the library path too.

#include <string>
#include <vector>

#include <zorba/security.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char* const DIGITS[] = {
  "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};

std::string imported(unsigned aDigit)
{
  return "import module namespace m = "
         "'http://zorba-tests.example.org/modules/mt-loader';\n"
         "m:digit('" + std::string(DIGITS[aDigit]) + "')";
}

std::string declared(unsigned aDigit)
{
  return "declare namespace m = "
         "'http://zorba-tests.example.org/modules/mt-loader';\n"
         "declare namespace ver = 'http://zorba.io/options/versioning';\n"
         "declare option ver:module-version '1.0';\n"
         "declare function m:digit($name as xs:string) as xs:integer?"
         " external;\n"
         "m:digit('" + std::string(DIGITS[aDigit]) + "')";
}

} // namespace


// argv[1]: the build directory, which holds TEST_URI_PATH and TEST_LIB_PATH.
int
mt_dynamic_loader(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "usage: mt_dynamic_loader <build directory>" << std::endl;
    return 1;
  }
  std::string const lBuildDir = argv[1];

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    lShared->setSecurityPolicy(SecurityPolicy::unrestricted());
    lShared->setURIPath(std::vector<String>{lBuildDir + "/TEST_URI_PATH"});
    lShared->setLibPath(std::vector<String>{lBuildDir + "/TEST_LIB_PATH"});

    run_threads([&](unsigned t)
    {
      unsigned const lDigit = t % 10;
      std::string const lWant = std::to_string(lDigit);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("imported module", t,
            run(lZorba, lShared, imported(lDigit)), lWant);

        lChecker.expect("function declared by the query", t,
            run(lZorba, lShared, declared(lDigit)), lWant);

        lChecker.expect("imported module, own static context", t,
            run(lZorba, lShared->createChildContext(), imported(lDigit)),
            lWant);
      }
    });
  }
  catch (ZorbaException const& e)
  {
    lChecker.fail(std::string("unexpected exception: ") + e.what());
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
