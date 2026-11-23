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

// ref:dereference of a node whose document another thread is freeing. One
// thread constructs a document, publishes the reference of its last element,
// waits until the other threads are dereferencing it, and drops the
// document; the other threads dereference the published reference in a
// loop. A reference must give the live node or the empty sequence, never a
// node whose tree is being destroyed.

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned DOCUMENTS = 60;

// The last element is among the last nodes the document's destruction
// unregisters.
const char* const PRODUCE =
  "import module namespace ref = 'http://zorba.io/modules/reference';\n"
  "let $d := document { <doc>{ for $i in 1 to 2000\n"
  "                            return <e n='{$i}'>text</e> }</doc> }\n"
  "return ($d, string(ref:reference($d/doc/e[last()])))\n";

const char* const CONSUME =
  "import module namespace ref = 'http://zorba.io/modules/reference';\n"
  "declare variable $r external;\n"
  "for $k in 1 to 200\n"
  "let $n := ref:dereference($r)\n"
  "return if (empty($n)) then 'gone' else string($n/@n)\n";

class Published
{
  std::mutex theMutex;
  std::string theReference;

public:
  std::atomic<unsigned> theLiveDereferences{0};
  std::atomic<bool> theDone{false};

  void set(std::string const& aReference)
  {
    std::lock_guard<std::mutex> lGuard(theMutex);
    theReference = aReference;
  }

  std::string get()
  {
    std::lock_guard<std::mutex> lGuard(theMutex);
    return theReference;
  }
};

void produce(Zorba* aZorba, Published& aPublished, Checker& aChecker)
{
  for (unsigned d = 0; d < DOCUMENTS; ++d)
  {
    Item lDoc;
    try
    {
      XQuery_t lQuery = aZorba->compileQuery(PRODUCE);
      Iterator_t lIter = lQuery->iterator();
      lIter->open();
      Item lReference;
      if (!lIter->next(lDoc) || !lIter->next(lReference) ||
          lReference.getStringValue().str().rfind("urn:uuid:", 0) != 0)
      {
        aChecker.fail("document " + std::to_string(d) + ": no reference");
        break;
      }
      lIter->close();
      aPublished.set(lReference.getStringValue().str());
    }
    catch (ZorbaException const& e)
    {
      aChecker.fail("document " + std::to_string(d) + ": " + e.what());
      break;
    }

    // Dropped once the other threads have found the node live.
    unsigned const lSeen = aPublished.theLiveDereferences.load();
    auto const lDeadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (aPublished.theLiveDereferences.load() < lSeen + THREADS &&
           std::chrono::steady_clock::now() < lDeadline)
      std::this_thread::yield();
  }
  aPublished.theDone = true;
}

void consume(Zorba* aZorba, Published& aPublished, Checker& aChecker,
             unsigned aThread)
{
  while (!aPublished.theDone.load())
  {
    std::string const lReference = aPublished.get();
    if (lReference.empty())
    {
      std::this_thread::yield();
      continue;
    }
    try
    {
      XQuery_t lQuery = aZorba->compileQuery(CONSUME);
      lQuery->getDynamicContext()->setVariable(
        "r", aZorba->getItemFactory()->createString(lReference));
      Iterator_t lIter = lQuery->iterator();
      lIter->open();
      Item lItem;
      while (lIter->next(lItem))
      {
        std::string const lValue = lItem.getStringValue().str();
        if (lValue == "2000")
          ++aPublished.theLiveDereferences;
        else if (lValue != "gone")
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
  Published lPublished;

  run_threads([&](unsigned t)
  {
    if (t == 0)
      produce(lZorba, lPublished, lChecker);
    else
      consume(lZorba, lPublished, lChecker, t);
  });

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
