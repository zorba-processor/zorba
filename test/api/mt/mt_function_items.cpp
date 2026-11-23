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

// Function items created by one query and called, copied and released by
// other threads (T6 of the thread-safety design). The items close over prolog
// variables of the query that created them, and one of them is stored in a
// prolog variable, so the state of that query stays alive through a cycle
// after it is closed. One thread closes the query while the others call the
// items; the threads then drop the last references, which must collect the
// cycle (QueryResources): every round must free all the state it created.

#include <atomic>
#include <string>
#include <vector>

#include <zorba/dynamic_context.h>
#include <zorba/store_manager.h>
#include <zorba/vector_item_sequence.h>

#include "mt_test.h"

namespace zorba {
// Library hook for tests (see QueryResources in src/compiler/api/compilercb.h).
ZORBA_DLL_PUBLIC long query_resources_alive();
}

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;
const unsigned CALLS = 5;
const unsigned COPIES = 200;

// $g is a function item stored in a prolog variable (a cycle through the
// query's dynamic context); each $h closes over $i and calls $g.
const char CREATOR[] =
  "declare variable $off := 100;\n"
  "declare variable $base := <b v='7'/>;\n"
  "declare variable $g := function($x) { $x + $off };\n"
  "($g, for $i in 1 to 8 return\n"
  "  function($x) { $g($x) * $i + xs:integer($base/@v) })";

const char CALLER[] =
  "declare variable $g external;\n"
  "declare variable $h external;\n"
  "declare variable $x external;\n"
  "($g($x), $h($x))";

std::string call(Zorba* aZorba, Item const& aG, Item const& aH, unsigned aX)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(CALLER);
    DynamicContext* lDctx = lQuery->getDynamicContext();
    lDctx->setVariable("g", aG);
    lDctx->setVariable("h", aH);
    lDctx->setVariable("x", aZorba->getItemFactory()->createInteger(aX));
    return drain(lQuery);
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.what();
  }
}

} // namespace


int
mt_function_items(int, char*[])
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
      Iterator_t lResult = lCreator->iterator();
      std::vector<Item> lItems;
      lResult->open();
      Item lItem;
      while (lResult->next(lItem))
        lItems.push_back(lItem);
      lResult->close();
      lResult = NULL;

      if (lItems.size() != THREADS + 1)
      {
        lChecker.fail("the creator returned " + std::to_string(lItems.size()) +
                      " items");
        break;
      }

      // The threads hold the only references from here on; thread 0 closes
      // the creator while the others are calling the items.
      std::vector<Item> lShared(THREADS, lItems[0]);
      std::vector<Item> lOwn(lItems.begin() + 1, lItems.end());
      lItems.clear();
      lItem = Item();

      run_threads([&](unsigned t)
      {
        unsigned const x = t + 1;
        std::string const lWant =
          std::to_string(x + 100) + " " +
          std::to_string((x + 100) * (t + 1) + 7);

        for (unsigned c = 0; c < CALLS; ++c)
        {
          lChecker.expect("call", t, call(lZorba, lShared[t], lOwn[t], x),
                          lWant);

          if (t == 0 && c == 0)
          {
            lCreator->close();
            lCreator = NULL;
          }

          // Copies and releases of items of the closed query: each release
          // may start a collection of its cycle.
          for (unsigned k = 0; k < COPIES; ++k)
          {
            Item lCopy = lShared[t];
            Item lOwnCopy = lOwn[t];
            (void)lCopy;
            (void)lOwnCopy;
          }
        }

        lOwn[t] = Item();
        lShared[t] = Item();
      });
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
