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

// Threads that share nothing but the engine: each compiles and runs its own
// queries against its own static context. What they do share is engine-global
// state: builtin functions, the QName and string pools, the shared items
// (booleans, small integers), the type manager and MAPM.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

struct Case
{
  char const* query;
  char const* expected;
};

const Case CASES[] = {
  { "for $i in 1 to 50 where $i mod 7 = 0 return $i * 2",
    "14 28 42 56 70 84 98" },
  { "string-join(for $w in tokenize('a b c b a', ' ') group by $w order by $w"
    " return concat($w, count($w)), ',')",
    "a1,b1,c1" },
  { "let $d := <r><x a='1'>p</x><x a='2'>q</x></r>"
    " return string-join($d//x[@a = '2']/text(), '')",
    "q" },
  { "sum(for $i in 1 to 100 return xs:decimal($i) div 8)", "631.25" },
  { "jn:parse-json('{\"a\":[1,2,{\"b\":\"c\"}]}')(\"a\")(3)(\"b\")", "c" },
  { "replace('2026-11-22', '(\\d+)-(\\d+)-(\\d+)', '$3/$2/$1')", "22/11/2026" },
  { "declare function local:f($n) { if ($n le 1) then 1 else $n * local:f($n - 1) };"
    " local:f(10)",
    "3628800" },
  { "let $f := function($x) { $x + 1 }"
    " return string-join(for-each(1 to 3, $f) ! string(.), '')",
    "234" },
  { "serialize(<a b='c'>{1 to 3}</a>)", "<a b=\"c\">1 2 3</a>" },
  { "count(distinct-values((1, 2, 2, 'x', 'x', xs:QName('fn:abc'))))", "4" },
  { "string(xs:date('2026-11-22') + xs:dayTimeDuration('P10D'))", "2026-12-02" },
  { "upper-case(normalize-unicode('caf\xC3\xA9'))"
    " || compare('a', 'b', 'http://www.w3.org/2005/xpath-functions/collation/codepoint')",
    "CAF\xC3\x89-1" },
  { "string-join(for $x in (3, 1, 2) order by $x descending return string($x), '')",
    "321" },
};

} // namespace


int
mt_independent(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  run_threads([&](unsigned t)
  {
    // Inputs that differ per thread, so that a mixed-up state shows as a
    // wrong result.
    unsigned const n = 10 + t;
    std::string const lSum = "sum(1 to " + std::to_string(n) + ")";
    std::string const lName =
      "local-name(element { QName('urn:mt', 'e" + std::to_string(n) + "') } {})";

    for (unsigned r = 0; r < ROUNDS; ++r)
    {
      for (Case const& c : CASES)
        lChecker.expect(c.query, t,
                        run(lZorba, lZorba->createStaticContext(), c.query),
                        c.expected);
      lChecker.expect(lSum, t, run(lZorba, lZorba->createStaticContext(), lSum),
                      std::to_string(n * (n + 1) / 2));
      lChecker.expect(lName, t, run(lZorba, lZorba->createStaticContext(), lName),
                      "e" + std::to_string(n));
    }
  });

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
