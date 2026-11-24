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

// Function items of a query that declares or assigns variables (scripting).
// Their calls write variables that the calls on other threads would share
// (the closure dynamic context, the prolog variables), so they can only be
// called on the thread that created them: elsewhere the call raises
// zerr:ZXQD0007. On their own thread they still work after their query is
// closed, while other threads release references to the function items of
// that query and so walk the graph whose variables the calls assign.

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <zorba/store_manager.h>

#include "mt_test.h"

namespace zorba {
// Library hook for tests (see QueryResources in src/compiler/api/compilercb.h).
ZORBA_DLL_PUBLIC long query_resources_alive();
}

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 10;
const unsigned CALLS = 200;
const unsigned COPIES = 2000;

// $g is stored in a prolog variable (a cycle through the query's dynamic
// context); the second item declares a variable in its closure dynamic
// context and assigns the prolog variable $n.
const char CREATOR[] =
  "declare variable $n := 0;\n"
  "declare variable $g := function($x) { $x + $n };\n"
  "let $k := 1\n"
  "return ($g, function($x) { variable $y := $x + $k; $n := $n + 1; $y })";

const char CALLER[] =
  "declare variable $f external;\n"
  "declare variable $x external;\n"
  "$f($x)";

std::string call(Zorba* aZorba, Item const& aF, unsigned aX)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(CALLER);
    DynamicContext* lDctx = lQuery->getDynamicContext();
    lDctx->setVariable("f", aF);
    lDctx->setVariable("x", aZorba->getItemFactory()->createInteger(aX));
    return drain(lQuery);
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.diagnostic().qname().localname();
  }
}

} // namespace


int
mt_function_item_thread(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    long const lAlive = query_resources_alive();

    for (unsigned r = 0; r < ROUNDS; ++r)
    {
      XQuery_t lCreator = lZorba->compileQuery(CREATOR);
      std::vector<Item> lItems;
      {
        Iterator_t lResult = lCreator->iterator();
        lResult->open();
        Item lItem;
        while (lResult->next(lItem))
          lItems.push_back(lItem);
        lResult->close();
      }

      if (lItems.size() != 2)
      {
        lChecker.fail("the creator returned " + std::to_string(lItems.size()) +
                      " items");
        break;
      }

      Item lG = lItems[0];
      Item lF = lItems[1];
      lItems.clear();

      // Other threads: every call raises, before and after the close.
      std::thread lOther([&]()
      {
        lChecker.expect("call of $g on another thread", 1,
                        call(lZorba, lG, 1), "error: ZXQD0007");
        lChecker.expect("call of $f on another thread", 1,
                        call(lZorba, lF, 1), "error: ZXQD0007");
      });
      lOther.join();

      lChecker.expect("call on the creating thread", 0, call(lZorba, lF, 1),
                      "2");

      lCreator->close();
      lCreator = NULL;

      // The creating thread calls $f, which assigns $n of the closed query,
      // while the other threads copy and release $g (each release may walk
      // the graph of the closed query, which holds $n).
      std::atomic<bool> lCalling(true);
      std::vector<std::thread> lReleasers;
      for (unsigned t = 1; t < THREADS; ++t)
      {
        lReleasers.emplace_back([&]()
        {
          Item lOwn = lG;
          for (unsigned k = 0; k < COPIES && lCalling.load(); ++k)
          {
            Item lCopy = lOwn;
            (void)lCopy;
          }
        });
      }

      for (unsigned c = 0; c < CALLS; ++c)
      {
        lChecker.expect("call after the close", 0, call(lZorba, lF, c),
                        std::to_string(c + 1));
      }
      lChecker.expect("$n after the calls", 0, call(lZorba, lG, 0),
                      std::to_string(CALLS + 1));

      lCalling.store(false);
      for (std::thread& lReleaser : lReleasers)
        lReleaser.join();
    }

    lChecker.expect("QueryResources alive after the rounds", 0,
                    std::to_string(query_resources_alive()),
                    std::to_string(lAlive));
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
