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

// The only reference from outside a cycle through the state of a query is a
// JSON object of a prolog variable that holds a function item of the cycle.
// One thread drops it while the main thread closes the query: the close
// examines the graph of the query, which a large array makes long, and may
// find the object referenced from outside. The cycle must be collected all
// the same: every round must free all the state it created.

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include <zorba/store_manager.h>

#include "mt_test.h"

namespace zorba {
// Library hook for tests (see QueryResources in src/compiler/api/compilercb.h).
ZORBA_DLL_PUBLIC long query_resources_alive();
}

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

// $m holds a function item that refers to the query's dynamic context, which
// holds $m.
const char CREATOR[] =
  "declare variable $m := { \"f\": function($x) { $x + 1 } };\n"
  "declare variable $big := [ 1 to 300000 ];\n"
  "$m";

} // namespace


int
mt_json_item_close(int, char*[])
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
      Item lObject;
      unsigned lCount = 0;
      {
        Iterator_t lResult = lCreator->iterator();
        lResult->open();
        Item lItem;
        while (lResult->next(lItem))
        {
          lObject = lItem;
          ++lCount;
        }
        lResult->close();
      }

      if (lCount != 1 || !lObject.isJSONItem())
      {
        lChecker.fail("the creator returned " + std::to_string(lCount) +
                      " items");
        break;
      }

      std::atomic<bool> lClosing(false);

      std::thread lDropper([&]()
      {
        while (!lClosing.load())
          std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        lObject = Item();
      });

      lClosing.store(true);
      lCreator->close();

      lDropper.join();
      lCreator = NULL;
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
