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

// The first compilations against a static context, on several threads at
// once. The context is set up through its setters only: no query is compiled
// against it, and no prolog is loaded, before the threads start. The threads
// compile against it and against child contexts they create from it; the
// queries use its namespace binding, base URI and default collation, and
// joins that the optimizer turns into temporary indexes. In debug builds,
// changing it once queries were compiled against it raises ZXQP0002.

#include <string>

#include <zorba/diagnostic_list.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char PRIMARY[] = "http://zorba.io/collations/PRIMARY/en/US";

std::string join_query(unsigned n)
{
  std::string const lN = std::to_string(n);
  return
    "count(for $a in (1 to " + lN + ") ! <a k='{.}'/>\n"
    "      for $b in (1 to 2 * " + lN + ") ! <b k='{. idiv 2}'/>\n"
    "      where $a/@k eq $b/@k\n"
    "      return $b)";
}

} // namespace


int
mt_first_compile(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    lShared->addNamespace("m", "urn:zorba:test:mt-first-compile");
    lShared->setBaseURI("http://zorba-mt.test/first-compile/");
    lShared->addCollation(PRIMARY);
    lShared->setDefaultCollation(PRIMARY);

    run_threads([&](unsigned t)
    {
      unsigned const n = 10 + t;
      std::string const lN = std::to_string(n);
      // Every value of $a/@k but 0 and n matches two $b; 1 to n leaves out 0
      // and keeps n, which matches one.
      std::string const lJoined = std::to_string(2 * n - 1);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        // The smart pointer of the shared context is not copied: its count
        // is not atomic.
        StaticContext_t lChild;
        if (r % 2 == 1)
          lChild = lShared->createChildContext();
        StaticContext_t const& lSctx = r % 2 == 0 ? lShared : lChild;

        lChecker.expect("namespace", t,
            run(lZorba, lSctx,
                "namespace-uri(<m:e" + lN + "/>) || ' ' || local-name(<m:e" +
                lN + "/>)"),
            "urn:zorba:test:mt-first-compile e" + lN);

        lChecker.expect("base uri", t,
            run(lZorba, lSctx, "string(resolve-uri('q" + lN + "'))"),
            "http://zorba-mt.test/first-compile/q" + lN);

        lChecker.expect("default collation", t,
            run(lZorba, lSctx,
                "string-join((string(compare('Stra" + lN + "', 'STRA" + lN +
                "')), default-collation()), ' ')"),
            std::string("0 ") + PRIMARY);

        lChecker.expect("join", t, run(lZorba, lSctx, join_query(n)), lJoined);
      }
    });

#ifndef NDEBUG
    try
    {
      lShared->addNamespace("late", "urn:zorba:test:late");
      lChecker.fail("a setter changed a static context queries were "
                    "compiled against");
    }
    catch (ZorbaException const& e)
    {
      if (e.diagnostic() != zerr::ZXQP0002_ASSERT_FAILED)
        lChecker.fail(std::string("late setter: unexpected error: ") +
                      e.what());
    }
#endif
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
