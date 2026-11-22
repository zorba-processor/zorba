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

// Clones of one compiled query run in parallel: they share its plan, the plans
// of its user functions and its global variables. Each thread gives its clone
// its own external variable and its own implicit timezone, so that a clone
// that runs with another clone's state shows a wrong result.
//
// By default the query runs once before it is cloned, so the plans of its user
// functions exist before the threads start (the clones share the plan as
// built). With the argument "cold" it does not: the first calls on the threads
// generate those plans concurrently (lazy code generation of user functions).

#include <cstring>
#include <string>

#include <zorba/dynamic_context.h>
#include <zorba/item_factory.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

// $h is the implicit timezone in hours: the local 12:00 equals 12:00Z minus
// $h hours only under that timezone.
const char QUERY[] =
  "declare variable $n external := 12;\n"
  "declare variable $h as xs:integer external := 0;\n"
  "declare variable $g := <g>{for $i in 1 to 5 return <i>{$i}</i>}</g>;\n"
  "declare function local:fib($k) {\n"
  "  if ($k lt 2) then $k else local:fib($k - 1) + local:fib($k - 2) };\n"
  "declare function local:h($xs) { sum($xs ! (. * 2)) };\n"
  "string-join((string(local:fib($n)), string(local:h($g/i)),\n"
  "  string(count($g//i)), string(implicit-timezone()),\n"
  "  string(xs:dateTime('2026-01-01T12:00:00') eq\n"
  "         xs:dateTime('2026-01-01T12:00:00Z') - $h * xs:dayTimeDuration('PT1H'))),\n"
  "  ' ')";

unsigned fib(unsigned k)
{
  return k < 2 ? k : fib(k - 1) + fib(k - 2);
}

std::string expected(unsigned n, unsigned h)
{
  std::string const lTimezone = h == 0 ? "PT0S" : "PT" + std::to_string(h) + "H";
  return std::to_string(fib(n)) + " 30 5 " + lTimezone + " true";
}

} // namespace


int
mt_clone(int argc, char* argv[])
{
  bool const lCold = argc > 1 && std::strcmp(argv[1], "cold") == 0;
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    XQuery_t lMaster = lZorba->compileQuery(QUERY);
    ItemFactory* lFactory = lZorba->getItemFactory();
    if (!lCold)
    {
      XQuery_t lFirst = lMaster->clone();
      lFirst->getDynamicContext()->setImplicitTimezone(0);
      lChecker.expect("first run", 0, drain(lFirst), expected(12, 0));
    }

    run_threads([&](unsigned t)
    {
      unsigned const n = 8 + t;
      unsigned const h = t;
      std::string const lWant = expected(n, h);
      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        try
        {
          XQuery_t lQuery = lMaster->clone();
          DynamicContext* lDctx = lQuery->getDynamicContext();
          lDctx->setVariable("n", lFactory->createInteger(static_cast<long long>(n)));
          lDctx->setVariable("h", lFactory->createInteger(static_cast<long long>(h)));
          lDctx->setImplicitTimezone(static_cast<int>(h) * 60);
          lChecker.expect("clone", t, drain(lQuery), lWant);
        }
        catch (ZorbaException const& e)
        {
          lChecker.expect("clone", t, std::string("error: ") + e.what(), lWant);
        }
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
