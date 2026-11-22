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
#ifndef ZORBA_TEST_API_MT_TEST_H
#define ZORBA_TEST_API_MT_TEST_H

// Shared by the multithreaded API tests (label "mt"): a fixed number of
// threads that start together, and result checks that count mismatches.

#include <atomic>
#include <iostream>
#include <latch>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <zorba/item.h>
#include <zorba/iterator.h>
#include <zorba/static_context.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

namespace zorba {
namespace mt_test {

const unsigned THREADS = 8;

class Checker
{
  std::atomic<unsigned> theFailures{0};
  std::mutex theOutputMutex;

public:
  void expect(std::string const& aWhat, unsigned aThread,
              std::string const& aGot, std::string const& aWant)
  {
    if (aGot == aWant)
      return;
    ++theFailures;
    std::lock_guard<std::mutex> lGuard(theOutputMutex);
    std::cerr << "thread " << aThread << ": " << aWhat << ": expected \""
              << aWant << "\", got \"" << aGot << "\"" << std::endl;
  }

  void fail(std::string const& aWhat)
  {
    ++theFailures;
    std::lock_guard<std::mutex> lGuard(theOutputMutex);
    std::cerr << aWhat << std::endl;
  }

  int result()
  {
    if (theFailures)
      std::cerr << theFailures << " check(s) failed" << std::endl;
    return theFailures == 0 ? 0 : 1;
  }
};

// The string values of the result items, separated by spaces.
inline std::string drain(XQuery_t const& aQuery)
{
  Iterator_t lIter = aQuery->iterator();
  lIter->open();
  std::string lResult;
  Item lItem;
  bool lFirst = true;
  while (lIter->next(lItem))
  {
    if (!lFirst)
      lResult += ' ';
    lFirst = false;
    lResult += lItem.getStringValue().str();
  }
  lIter->close();
  return lResult;
}

// Compiles and runs a query; an error becomes the result text.
inline std::string run(Zorba* aZorba, StaticContext_t const& aSctx,
                       std::string const& aQuery)
{
  try
  {
    return drain(aZorba->compileQuery(aQuery, aSctx));
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.what();
  }
}

// Runs aBody(thread) on THREADS threads released together.
template <class Body>
void run_threads(Body aBody)
{
  std::latch lStart(THREADS);
  std::vector<std::thread> lThreads;
  for (unsigned t = 0; t < THREADS; ++t)
  {
    lThreads.emplace_back([&lStart, &aBody, t]()
    {
      lStart.arrive_and_wait();
      aBody(t);
    });
  }
  for (std::thread& lThread : lThreads)
    lThread.join();
}

} // namespace mt_test
} // namespace zorba

#endif
/* vim:set et sw=2 ts=2: */
