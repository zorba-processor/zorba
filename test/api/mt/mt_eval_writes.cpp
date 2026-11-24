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

// Function items of a query that neither declares nor assigns variables are
// not bound to their thread, even when their calls run an eval query that
// assigns a prolog variable of their query, or leave their body with exit:
// the eval query assigns its own copy of the outer variables, so the outer
// query's variables keep their values, and the snapshot of the outer query,
// which every such call changes, is changed atomically.

#include <string>
#include <vector>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned CALLS = 200;

// local:bump assigns $n in an eval query; local:leave exits its body.
const char CREATOR[] =
  "import module namespace reflection = \"http://zorba.io/modules/reflection\";\n"
  "declare namespace an = \"http://zorba.io/annotations\";\n"
  "declare variable $n := 0;\n"
  "declare %an:sequential function local:bump()\n"
  "{ reflection:eval-s(\"$n := $n + 1; $n\") };\n"
  "declare function local:leave()\n"
  "{ exit returning $n + 2; };\n"
  "(function($x) { $x + $n }, local:bump#0, local:leave#0)";

const char CALLER[] =
  "declare variable $f external;\n"
  "$f()";

const char CALLER1[] =
  "declare variable $f external;\n"
  "declare variable $x external;\n"
  "$f($x)";

std::string call(Zorba* aZorba, Item const& aF)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(CALLER);
    lQuery->getDynamicContext()->setVariable("f", aF);
    return drain(lQuery);
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.diagnostic().qname().localname();
  }
}

std::string call(Zorba* aZorba, Item const& aF, unsigned aX)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(CALLER1);
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
mt_eval_writes(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
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

    if (lItems.size() != 3)
    {
      lChecker.fail("the creator returned " + std::to_string(lItems.size()) +
                    " items");
    }
    else
    {
      Item lG = lItems[0];
      Item lBump = lItems[1];
      Item lLeave = lItems[2];

      run_threads([&](unsigned aThread)
      {
        for (unsigned c = 0; c < CALLS; ++c)
        {
          lChecker.expect("eval assigning $n", aThread, call(lZorba, lBump),
                          "1");
          lChecker.expect("exit", aThread, call(lZorba, lLeave), "2");
          lChecker.expect("$n", aThread, call(lZorba, lG, c),
                          std::to_string(c));
        }
      });
    }

    lCreator->close();
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
