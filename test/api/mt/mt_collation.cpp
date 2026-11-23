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

// Collations from several threads. One static context holds ICU collators
// (added through the API, one of them the default collation) that every
// thread's queries use at once: comparisons, substring searches (an ICU
// StringSearch over the shared collator), ordering, grouping, distinct values
// and deep equality. Half of the threads also create, in every round, a
// static context with collators of their own.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char PRIMARY[] = "http://zorba.io/collations/PRIMARY/en/US";
const char SECONDARY[] = "http://zorba.io/collations/SECONDARY/de/DE";

struct Case
{
  char const* query;
  char const* expected;
};

// $p is the primary collation, $s the secondary one; the default collation is
// the primary one.
const Case CASES[] = {
  { "compare('Stra\xC3\x9F" "e', 'STRASSE')", "0" },
  { "compare('caf\xC3\xA9', 'cafe', $s)", "1" },
  { "contains('Gr\xC3\xBC\xC3\x9F" "e aus K\xC3\xB6ln', 'KOLN', $p)", "true" },
  { "substring-after('Abc-Def-ghi', 'DEF', $p)", "-ghi" },
  { "starts-with('\xC3\x84pfel', 'apf', $p) || ends-with('Strasse', 'SSE', $p)",
    "truetrue" },
  { "string-join(for $w in ('delta', 'Bravo', '\xC3\xA1lpha', 'Charlie')\n"
    "  order by $w collation 'http://zorba.io/collations/PRIMARY/en/US'\n"
    "  return $w, ' ')",
    "\xC3\xA1lpha Bravo Charlie delta" },
  { "string-join(for $w in ('a', 'A', '\xC3\xA1', 'b', 'B')\n"
    "  group by $k := $w collation 'http://zorba.io/collations/PRIMARY/en/US'\n"
    "  order by count($w)\n"
    "  return string(count($w)), ' ')",
    "2 3" },
  { "count(distinct-values(('x', 'X', 'y', '\xC3\xBD', 'z')))", "3" },
  { "deep-equal(<a>Hello</a>/text(), <b>HELLO</b>/text(), $p)", "true" },
  { "string-join(index-of(('k', 'K', 'l'), 'k', $p) ! string(.), ' ')", "1 2" },
  { "max(('b', 'C', 'a'))", "C" },
};

std::string prolog()
{
  return std::string("declare variable $p := '") + PRIMARY + "';\n"
         "declare variable $s := '" + SECONDARY + "';\n";
}

} // namespace


int
mt_collation(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  auto lContext = [&]()
  {
    StaticContext_t lSctx = lZorba->createStaticContext();
    lSctx->addCollation(PRIMARY);
    lSctx->addCollation(SECONDARY);
    lSctx->setDefaultCollation(PRIMARY);
    return lSctx;
  };

  try
  {
    StaticContext_t lShared = lContext();
    std::string const lProlog = prolog();

    run_threads([&](unsigned t)
    {
      std::string const lT = std::to_string(t);
      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        StaticContext_t lSctx =
          t % 2 == 1 && r % 2 == 1 ? lContext() : lShared;

        for (Case const& c : CASES)
          lChecker.expect(c.query, t, run(lZorba, lSctx, lProlog + c.query),
                          c.expected);

        // An input of this thread's own, so that a mixed-up state shows.
        lChecker.expect("thread input", t,
            run(lZorba, lSctx,
                lProlog + "string-join((string(compare('W" + lT + "x', 'w" +
                lT + "X')), substring-before('aaa" + lT + "BBB', 'bbb', $p)), ' ')"),
            "0 aaa" + lT);
      }
    });
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
