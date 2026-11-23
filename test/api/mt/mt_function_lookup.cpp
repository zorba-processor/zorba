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

// fn:function-lookup from several threads: in functions of a shared static
// context and in the queries, of user functions and of builtin functions; and
// function items that one query looked up, called by all the threads.

#include <string>
#include <vector>

#include <zorba/dynamic_context.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare function p:sq($x) { $x * $x };\n"
  "declare function p:lk($x) { function-lookup(xs:QName('p:sq'), 1)($x) };\n"
  "declare function p:abs($x) {\n"
  "  function-lookup(xs:QName('fn:abs'), 1)($x) };\n";

// Runs "$f($x)" with $f bound to aF and $x to aX.
std::string call(Zorba* aZorba, StaticContext_t const& aSctx, Item const& aF,
                 long long aX)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(
        "declare variable $f external; declare variable $x external; $f($x)",
        aSctx);
    DynamicContext* lDctx = lQuery->getDynamicContext();
    lDctx->setVariable("f", aF);
    lDctx->setVariable("x", aZorba->getItemFactory()->createInteger(aX));
    return drain(lQuery);
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.what();
  }
}

// The function item that aQuery returns; aQuery stays open in aQueries.
Item lookup(Zorba* aZorba, StaticContext_t const& aSctx, char const* aQuery,
            std::vector<XQuery_t>& aQueries)
{
  XQuery_t lQuery = aZorba->compileQuery(aQuery, aSctx);
  aQueries.push_back(lQuery);
  Iterator_t lIter = lQuery->iterator();
  Item lItem;
  lIter->open();
  lIter->next(lItem);
  lIter->close();
  return lItem;
}

} // namespace


int
mt_function_lookup(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(PROLOG, lHints);

    std::vector<XQuery_t> lLookups;
    Item const lAbs = lookup(lZorba, lShared,
        "function-lookup(xs:QName('fn:abs'), 1)", lLookups);
    Item const lSq = lookup(lZorba, lShared,
        "function-lookup(xs:QName('p:sq'), 1)", lLookups);

    run_threads([&](unsigned t)
    {
      unsigned const n = 3 + t;
      std::string const lN = std::to_string(n);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("user function, in a prolog function", t,
            run(lZorba, lShared, "p:lk(" + lN + ")"),
            std::to_string(n * n));

        lChecker.expect("builtin function, in a prolog function", t,
            run(lZorba, lShared, "p:abs(-" + lN + ")"),
            lN);

        lChecker.expect("builtin function, in the query", t,
            run(lZorba, lShared,
                "function-lookup(xs:QName('fn:string-length'), 1)('" +
                std::string(n, 'x') + "')"),
            lN);

        lChecker.expect("builtin function looked up once", t,
            call(lZorba, lShared, lAbs, -static_cast<long long>(n)),
            lN);

        lChecker.expect("user function looked up once", t,
            call(lZorba, lShared, lSq, n),
            std::to_string(n * n));
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
