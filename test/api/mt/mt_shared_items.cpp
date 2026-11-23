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

// Items shared by all threads: an atomic string, a QName, a node, a JSON object
// and the items the store shares itself (booleans, small integers). The
// threads copy and release them through the Item API and pass them to their
// queries as external variables; the queries create and drop the same QNames
// in the pool concurrently, and the nodes they return are released by other
// threads. The queries do not iterate over the children of a node or the
// members of an array or object (ContentReader, P7).

#include <mutex>
#include <string>
#include <vector>

#include <zorba/dynamic_context.h>
#include <zorba/item_factory.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 50;

const unsigned COPIES = 200;

const char QUERY[] =
  "declare variable $s external;\n"
  "declare variable $q external;\n"
  "declare variable $n external;\n"
  "declare variable $o external;\n"
  "declare variable $t as xs:integer external;\n"
  "string-join((\n"
  "  $s,\n"
  "  local-name-from-QName($q),\n"
  "  string(node-name($n)),\n"
  "  string(($n, $n, $n)[2] is $n),\n"
  "  $o('d'),\n"
  "  string($o('a')(2)),\n"
  "  $o('a')(3)('b'),\n"
  "  string(count(distinct-values(\n"
  "    for $i in 1 to 200 return QName('urn:mt', 'q' || $i mod 7)))),\n"
  "  string(count(distinct-values(\n"
  "    for $i in 1 to 200 return QName('urn:mt', 'p:q' || $i mod 5 + $t)))),\n"
  "  string(sum(for $i in 1 to 100 return $i mod 10)),\n"
  "  string((1 to 3) = 2)),\n"
  "  ' ')";

const char WANT[] = "shared shared r true e 2 c 7 5 450 true";

// Returns the shared node and an element made on this thread.
const char NODES[] =
  "declare variable $n external;\n"
  "declare variable $t as xs:integer external;\n"
  "($n, <e>{$t}</e>)";

Item first(Zorba* aZorba, std::string const& aQuery)
{
  XQuery_t lQuery = aZorba->compileQuery(aQuery);
  Iterator_t lIter = lQuery->iterator();
  lIter->open();
  Item lItem;
  lIter->next(lItem);
  lIter->close();
  return lItem;
}

} // namespace


int
mt_shared_items(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    ItemFactory* lFactory = lZorba->getItemFactory();
    Item const lString = lFactory->createString("shared");
    Item const lQName = lFactory->createQName("urn:mt", "shared");
    Item const lTrue = lFactory->createBoolean(true);
    Item const lSeven = lFactory->createInteger(7LL);
    Item const lNode = first(lZorba, "<r a='1'><c>x</c><c>y</c></r>");
    Item const lObject =
      first(lZorba, "{ 'a' : [1, 2, { 'b' : 'c' }], 'd' : 'e' }");

    std::mutex lBinMutex;
    std::vector<Item> lBin;

    run_threads([&](unsigned t)
    {
      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        try
        {
          {
            std::vector<Item> lCopies;
            lCopies.reserve(6 * COPIES);
            for (unsigned i = 0; i < COPIES; ++i)
            {
              lCopies.push_back(lString);
              lCopies.push_back(lQName);
              lCopies.push_back(lTrue);
              lCopies.push_back(lSeven);
              lCopies.push_back(lNode);
              lCopies.push_back(lObject);
            }
            lChecker.expect("string copy", t,
                            lCopies[0].getStringValue().str(), "shared");
            lChecker.expect("QName copy", t,
                            lCopies[1].getLocalName().str(), "shared");
            lChecker.expect("boolean copy", t,
                            lCopies[2].getBooleanValue() ? "true" : "false",
                            "true");
            lChecker.expect("integer copy", t,
                            lCopies[3].getStringValue().str(), "7");
          }

          Item const lThread =
            lFactory->createInteger(static_cast<long long>(t));

          XQuery_t lQuery = lZorba->compileQuery(QUERY);
          DynamicContext* lDctx = lQuery->getDynamicContext();
          lDctx->setVariable("s", lString);
          lDctx->setVariable("q", lQName);
          lDctx->setVariable("n", lNode);
          lDctx->setVariable("o", lObject);
          lDctx->setVariable("t", lThread);
          lChecker.expect("query", t, drain(lQuery), WANT);

          XQuery_t lNodes = lZorba->compileQuery(NODES);
          lNodes->getDynamicContext()->setVariable("n", lNode);
          lNodes->getDynamicContext()->setVariable("t", lThread);
          Iterator_t lIter = lNodes->iterator();
          lIter->open();
          std::vector<Item> lMine;
          Item lItem;
          while (lIter->next(lItem))
            lMine.push_back(lItem);
          lIter->close();
          lChecker.expect("nodes", t, std::to_string(lMine.size()), "2");
          if (lMine.size() == 2)
            lChecker.expect("made node", t, lMine[1].getStringValue().str(),
                            std::to_string(t));

          // The nodes are released by whichever thread empties the bin.
          std::vector<Item> lTaken;
          {
            std::lock_guard<std::mutex> lGuard(lBinMutex);
            lBin.insert(lBin.end(), lMine.begin(), lMine.end());
            if (lBin.size() >= 4 * THREADS)
              lTaken.swap(lBin);
          }
        }
        catch (ZorbaException const& e)
        {
          lChecker.expect("round", t, std::string("error: ") + e.what(), WANT);
        }
      }
    });

    lBin.clear();
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
