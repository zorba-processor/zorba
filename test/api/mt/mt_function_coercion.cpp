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

// Function coercion in functions of a shared static context, run by several
// threads: p:sq#1 is passed where a function(xs:integer) as xs:integer is
// expected, so it is coerced; the coerced function item is called, and it has
// the name of the function it coerces.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare function p:sq($x) { $x * $x };\n"
  "declare function p:ap($f as function(xs:integer) as xs:integer,\n"
  "                      $x as xs:integer) { $f($x) };\n"
  "declare function p:nm($f as function(xs:integer) as xs:integer)\n"
  "{ string(function-name($f)) };\n"
  "declare function p:use($x) { p:ap(p:sq#1, $x) };\n"
  "declare function p:name() { p:nm(p:sq#1) };\n";

} // namespace


int
mt_function_coercion(int, char*[])
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
      unsigned const n = 3 + t;
      std::string const lN = std::to_string(n);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("coerced call", t,
            run(lZorba, lShared, "p:use(" + lN + ")"),
            std::to_string(n * n));

        lChecker.expect("coerced name", t,
            run(lZorba, lShared, "p:name()"),
            "p:sq");

        lChecker.expect("coercion in the query", t,
            run(lZorba, lShared, "p:ap(p:sq#1, " + lN + ") + 1"),
            std::to_string(n * n + 1));
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
