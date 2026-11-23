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

// Cached functions of a shared static context, called by several threads that
// share each function's cache: %an:cache, %an:strictlydeterministic, and the
// automatic cache of a deterministic function with atomic arguments and
// result.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;
const unsigned CALLS = 50;

const char PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare namespace an = 'http://zorba.io/annotations';\n"
  "declare %an:cache function p:c($x as xs:integer) as xs:integer\n"
  "{ $x * 2 };\n"
  "declare %an:strictlydeterministic function p:s($x as xs:integer)\n"
  "  as xs:integer { $x * 3 };\n"
  "declare function p:a($x as xs:integer) as xs:integer { $x + 1 };\n";

// sum(for $i in 1 to CALLS return f($i mod aMod + aN)), f(x) = x * aMul + aAdd
std::string expected(unsigned aN, unsigned aMod, unsigned aMul, unsigned aAdd)
{
  unsigned lSum = 0;
  for (unsigned i = 1; i <= CALLS; ++i)
    lSum += (i % aMod + aN) * aMul + aAdd;
  return std::to_string(lSum);
}

std::string query(char const* aFunction, unsigned aMod, std::string const& aN)
{
  return "sum(for $i in 1 to " + std::to_string(CALLS) + " return " +
    aFunction + "($i mod " + std::to_string(aMod) + " + " + aN + "))";
}

} // namespace


int
mt_function_cache(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(PROLOG, lHints);

    run_threads([&](unsigned t)
    {
      // Overlapping arguments: the threads find each other's entries.
      unsigned const n = t % 3;
      std::string const lN = std::to_string(n);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("an:cache", t,
            run(lZorba, lShared, query("p:c", 7, lN)),
            expected(n, 7, 2, 0));

        lChecker.expect("an:strictlydeterministic", t,
            run(lZorba, lShared, query("p:s", 5, lN)),
            expected(n, 5, 3, 0));

        lChecker.expect("automatic cache", t,
            run(lZorba, lShared, query("p:a", 3, lN)),
            expected(n, 3, 1, 1));
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
