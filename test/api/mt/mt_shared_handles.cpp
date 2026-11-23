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

// Handles (StaticContext_t, XQuery_t) to one static context and one compiled
// query, copied and released on several threads: the threads compile against
// their copies of the context and run clones of their copies of the query.
// Afterwards the reference counts are what they were before. Then the threads
// release the last handles to new contexts and queries.

#include <string>
#include <vector>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 2000;
const unsigned COPIES = 16;
const unsigned USE_EVERY = 200;
const unsigned RELEASE_ROUNDS = 20;

const char PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare function p:sq($x) { $x * $x };\n";

const char QUERY[] =
  "string-join(for $i in 1 to 4 return string($i * $i), ' ')";

const char QUERY_RESULT[] = "1 4 9 16";

} // namespace


int
mt_shared_handles(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(PROLOG, lHints);
    XQuery_t lQuery = lZorba->compileQuery(QUERY);

    long const lSharedCount = lShared->getRefCount();
    long const lQueryCount = lQuery->getRefCount();

    run_threads([&](unsigned t)
    {
      unsigned const n = t + 2;
      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        std::vector<StaticContext_t> lContexts(COPIES, lShared);
        std::vector<XQuery_t> lQueries(COPIES, lQuery);
        StaticContext_t lContext;
        lContext = lContexts.back();
        XQuery_t lCopy;
        lCopy = lQueries.back();

        if (r % USE_EVERY != 0)
          continue;

        lChecker.expect("copied context", t,
            run(lZorba, lContext, "p:sq(" + std::to_string(n) + ")"),
            std::to_string(n * n));
        try
        {
          lChecker.expect("clone of copied query", t, drain(lCopy->clone()),
                          QUERY_RESULT);
        }
        catch (ZorbaException const& e)
        {
          lChecker.expect("clone of copied query", t,
                          std::string("error: ") + e.what(), QUERY_RESULT);
        }
      }
    });

    lChecker.expect("context count", 0, std::to_string(lShared->getRefCount()),
                    std::to_string(lSharedCount));
    lChecker.expect("query count", 0, std::to_string(lQuery->getRefCount()),
                    std::to_string(lQueryCount));

    for (unsigned r = 0; r < RELEASE_ROUNDS; ++r)
    {
      std::vector<StaticContext_t> lContexts(THREADS,
                                             lShared->createChildContext());
      std::vector<XQuery_t> lQueries(THREADS, lZorba->compileQuery(QUERY));

      run_threads([&](unsigned t)
      {
        lContexts[t] = StaticContext_t();
        lQueries[t] = XQuery_t();
      });
    }
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
