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

// A function item that refers to the state of an open query, released on one
// thread while another thread closes that query. The query's state stays
// alive through a cycle (a function item stored in a prolog variable), and
// the released item holds the only reference to it from outside the cycle.
// The item is partially applied to a large sequence, so its destruction
// takes long enough for the query to be closed in between: the destruction
// finds the query open when it starts and releases its reference after the
// close has examined the cycle. The cycle must be collected all the same:
// every round must free all the state it created.

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

const char CREATOR[] =
  "declare variable $g := function($x) { $x };\n"
  "(function($b, $x) { count($b) + $g($x) })(1 to 1000000, ?)";

} // namespace


int
mt_function_item_close(int, char*[])
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
      Item lItem;
      {
        Iterator_t lResult = lCreator->iterator();
        lResult->open();
        // A function item is neither a node, an atomic value nor a JSON item.
        if (!lResult->next(lItem) || lItem.isNode() || lItem.isAtomic() ||
            lItem.isJSONItem())
        {
          lChecker.fail("the creator returned no function item");
          break;
        }
        lResult->close();
      }

      std::atomic<bool> lReleasing(false);

      std::thread lReleaser([&]()
      {
        lReleasing.store(true);
        lItem = Item();
      });

      while (!lReleasing.load())
        std::this_thread::yield();
      std::this_thread::sleep_for(std::chrono::microseconds(200));

      lCreator->close();
      lCreator = NULL;

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
