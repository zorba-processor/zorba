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

// The last reference from outside a cycle through the state of a closed query
// dropped on one thread while another thread drops a reference to a JSON
// object of that state that is not part of the cycle. The first release
// examines the graph of the closed query, which a large array makes long;
// the second changes the reference count of the JSON object meanwhile. The
// cycle must be collected all the same: every round must free all the state
// it created.

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <zorba/store_manager.h>

#include "mt_test.h"

namespace zorba {
// Library hooks for tests (see QueryResources in src/compiler/api/compilercb.h).
ZORBA_DLL_PUBLIC long query_resources_alive();
ZORBA_DLL_PUBLIC unsigned long query_resources_walks();
}

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

// $g is in a cycle through the query's dynamic context; $j only hangs off it.
const char CREATOR[] =
  "declare variable $j := { \"a\": 1 };\n"
  "declare variable $big := [ 1 to 300000 ];\n"
  "declare variable $g := function($x) { $x };\n"
  "($g, $j)";

} // namespace


int
mt_function_item_walk(int, char*[])
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

      if (lItems.size() != 2 || !lItems[1].isJSONItem())
      {
        lChecker.fail("the creator returned " + std::to_string(lItems.size()) +
                      " items");
        break;
      }

      lCreator->close();
      lCreator = NULL;

      Item lFunction = lItems[0];
      Item lObject = lItems[1];
      lItems.clear();

      unsigned long const lWalks = query_resources_walks();
      std::atomic<bool> lWalking(false);

      std::thread lWalker([&]()
      {
        lWalking.store(true);
        lFunction = Item();
      });

      while (!lWalking.load())
        std::this_thread::yield();
      std::this_thread::sleep_for(std::chrono::microseconds(500));
      lObject = Item();

      lWalker.join();

      if (query_resources_walks() == lWalks)
        lChecker.fail("round " + std::to_string(r) + ": no walk");
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
