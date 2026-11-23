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

// The static context of a compiled query (XQuery::getStaticContext()) shared
// by threads that compile queries against it: they call functions that the
// query declares, also those that its own main expression does not call.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char QUERY[] =
  "declare namespace q = 'urn:q';\n"
  "declare function q:f($x) { $x * 3 };\n"
  "declare function q:g($x) { q:f($x) + 1 };\n"
  "declare function q:h($x) { for $i in 1 to $x return q:g($i) };\n"
  "q:f(1)";

} // namespace


int
mt_query_sctx(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    XQuery_t lQuery = lZorba->compileQuery(QUERY);
    lChecker.expect("the query", 0, drain(lQuery), "3");
    StaticContext_t lShared = lQuery->getStaticContext()->createChildContext();
    // The first compilation against a context gives it its type manager:
    // part of setting the context up.
    lChecker.expect("setup", 0, run(lZorba, lShared, "q:f(2)"), "6");

    run_threads([&](unsigned t)
    {
      unsigned const n = 3 + t;
      std::string const lN = std::to_string(n);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("called by the query", t,
            run(lZorba, lShared, "q:f(" + lN + ")"),
            std::to_string(3 * n));

        lChecker.expect("not called by the query", t,
            run(lZorba, lShared, "q:g(" + lN + ")"),
            std::to_string(3 * n + 1));

        lChecker.expect("calls one not called by the query", t,
            run(lZorba, lShared, "sum(q:h(" + lN + "))"),
            std::to_string(3 * n * (n + 1) / 2 + n));
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
