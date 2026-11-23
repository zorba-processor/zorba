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

// ref:dereference of nodes that other threads are freeing. Half of the
// threads construct documents, take the reference of the last element of
// each, publish it and drop the document; the other half dereference the
// published references. A reference must give the live node or the empty
// sequence, never a node whose tree is being destroyed.

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned DOCUMENTS = 150;
const unsigned SLOTS = 32;

// The document is freed when the query is: the last element is among the
// last nodes its destruction unregisters.
const char* const PRODUCE =
  "import module namespace ref = 'http://zorba.io/modules/reference';\n"
  "let $d := document { <doc>{ for $i in 1 to 200\n"
  "                            return <e n='{$i}'>text</e> }</doc> }\n"
  "return string(ref:reference($d/doc/e[last()]))\n";

const char* const CONSUME =
  "import module namespace ref = 'http://zorba.io/modules/reference';\n"
  "declare variable $refs external;\n"
  "for $k in 1 to 20, $r in tokenize($refs, ' ')\n"
  "let $n := ref:dereference($r)\n"
  "return if (empty($n)) then 'gone' else string($n/@n)\n";

class References
{
  std::mutex theMutex;
  std::vector<std::string> theSlots;
  unsigned theNext = 0;

public:
  References() : theSlots(SLOTS) { }

  void publish(std::string const& aReference)
  {
    std::lock_guard<std::mutex> lGuard(theMutex);
    theSlots[theNext++ % SLOTS] = aReference;
  }

  std::string all()
  {
    std::lock_guard<std::mutex> lGuard(theMutex);
    std::string lAll;
    for (std::string const& lSlot : theSlots)
      if (!lSlot.empty())
        lAll += lSlot + ' ';
    return lAll;
  }
};

void produce(Zorba* aZorba, References& aReferences, Checker& aChecker,
             unsigned aThread)
{
  for (unsigned d = 0; d < DOCUMENTS; ++d)
  {
    try
    {
      std::string const lReference = drain(aZorba->compileQuery(PRODUCE));
      if (lReference.rfind("urn:uuid:", 0) != 0)
      {
        aChecker.fail("thread " + std::to_string(aThread) +
                      ": reference: " + lReference);
        return;
      }
      aReferences.publish(lReference);
    }
    catch (ZorbaException const& e)
    {
      aChecker.fail("thread " + std::to_string(aThread) + ": " + e.what());
      return;
    }
  }
}

void consume(Zorba* aZorba, References& aReferences, Checker& aChecker,
             unsigned aThread, std::atomic<unsigned> const& aProducing)
{
  while (aProducing.load() != 0)
  {
    std::string const lReferences = aReferences.all();
    if (lReferences.empty())
      continue;
    try
    {
      XQuery_t lQuery = aZorba->compileQuery(CONSUME);
      lQuery->getDynamicContext()->setVariable(
        "refs", aZorba->getItemFactory()->createString(lReferences));
      Iterator_t lIter = lQuery->iterator();
      lIter->open();
      Item lItem;
      while (lIter->next(lItem))
      {
        std::string const lValue = lItem.getStringValue().str();
        if (lValue != "gone" && lValue != "200")
          aChecker.fail("thread " + std::to_string(aThread) +
                        ": dereferenced: " + lValue);
      }
      lIter->close();
    }
    catch (ZorbaException const& e)
    {
      aChecker.fail("thread " + std::to_string(aThread) + ": " + e.what());
      return;
    }
  }
}

} // namespace


int
mt_node_dereference(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;
  References lReferences;
  std::atomic<unsigned> lProducing(THREADS / 2);

  run_threads([&](unsigned t)
  {
    if (t < THREADS / 2)
    {
      produce(lZorba, lReferences, lChecker, t);
      --lProducing;
    }
    else
    {
      consume(lZorba, lReferences, lChecker, t, lProducing);
    }
  });

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
