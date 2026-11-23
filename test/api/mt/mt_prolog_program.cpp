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

// The program of a prolog loaded by loadProlog (its variable initializers),
// which every query compiled against the static context includes and
// generates: here the initializers hold inline functions, one of them closing
// over a local variable. Each query gets its own state for them.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare variable $p:k := 3;\n"
  "declare variable $p:add := function($x) { $x + $p:k };\n"
  "declare variable $p:mul := let $m := 10 return function($x) { $x * $m };\n"
  "declare function p:use($x) { $p:add($p:mul($x)) };\n";

} // namespace


int
mt_prolog_program(int, char*[])
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
        lChecker.expect("inline function", t,
            run(lZorba, lShared, "$p:add(" + lN + ")"),
            std::to_string(n + 3));

        lChecker.expect("closure", t,
            run(lZorba, lShared, "$p:mul(" + lN + ")"),
            std::to_string(n * 10));

        lChecker.expect("prolog function", t,
            run(lZorba, lShared, "p:use(" + lN + ")"),
            std::to_string(n * 10 + 3));

        lChecker.expect("both", t,
            run(lZorba, lShared, "$p:mul($p:add(" + lN + "))"),
            std::to_string((n + 3) * 10));
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
