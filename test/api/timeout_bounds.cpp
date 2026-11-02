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

// The timeout (ZXQP0068) must bound the time of a query whose work is spread
// over few iterator calls: work the optimizer tries at compile time, and
// external functions that take long per call.

#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <string>

#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;

namespace {

int theFailures = 0;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      std::cerr << __FILE__ << ":" << __LINE__ << ": check failed: "    \
                << #cond << std::endl;                                  \
      ++theFailures;                                                    \
    }                                                                   \
  } while (0)

struct Outcome
{
  bool ok = false;
  std::string value;
  std::string code;
  std::string message;
  milliseconds compile{0};
  milliseconds elapsed{0};
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    os << "ok: " << o.value;
  else
    os << o.code << ": " << o.message;
  return os << " (compile " << o.compile.count() << " ms, total "
            << o.elapsed.count() << " ms)";
}

Outcome run(Zorba* z, const SecurityPolicy& p, const std::string& aQuery)
{
  Outcome o;
  steady_clock::time_point const start = steady_clock::now();
  try
  {
    StaticContext_t sctx = z->createStaticContext();
    sctx->setSecurityPolicy(p);
    XQuery_t q = z->compileQuery(aQuery, sctx);
    o.compile = std::chrono::duration_cast<milliseconds>(
        steady_clock::now() - start);
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
    {
      if (!o.value.empty())
        o.value += ' ';
      o.value += item.getStringValue().str();
    }
    it->close();
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
  catch (std::exception const& e)
  {
    o.code = "std::exception";
    o.message = e.what();
  }
  o.elapsed = std::chrono::duration_cast<milliseconds>(
      steady_clock::now() - start);
  return o;
}

bool failed_with(const Outcome& o, const char* aCode)
{
  if (!o.ok && o.code == aCode)
    return true;
  std::cerr << "expected " << aCode << ", got " << o << std::endl;
  return false;
}

bool within(const Outcome& o, milliseconds aLimit)
{
  if (o.elapsed < aLimit)
    return true;
  std::cerr << "expected under " << aLimit.count() << " ms, got " << o
            << std::endl;
  return false;
}

// The optimizer evaluates expressions without free variables at compile
// time. That must not use up the time limit of the query, or run the query
// for the limit once per nesting level before it even starts.
void test_constant_folding(Zorba* z)
{
  seconds const timeout(3);
  SecurityPolicy const p = SecurityPolicy::unrestricted().setTimeout(timeout);
  const char* const COUNT = "count((1 to 100000000000)[. lt 0])";

  Outcome o = run(z, p, COUNT);
  std::cout << "  one expression: " << o << std::endl;
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(o.compile < timeout);
  CHECK(within(o, 2 * timeout));

  o = run(z, p, std::string("1 + ") + COUNT + " + " + COUNT);
  std::cout << "  nested expressions: " << o << std::endl;
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(o.compile < timeout);
  CHECK(within(o, 2 * timeout));

  // Folding also ends with no time limit set, and the result stays right.
  o = run(z, SecurityPolicy::unrestricted(), "count((1 to 300000)[. lt 0])");
  CHECK(o.ok && o.value == "0");
}

// Sleeping is the workload here: 500 ms per call that the iterators hardly
// see. Checking only every so many iterator calls would run on for a long time
// after the limit.
void test_external_function(Zorba* z)
{
  seconds const timeout(1);
  SecurityPolicy const p = SecurityPolicy::unrestricted().setTimeout(timeout);

  Outcome const o = run(z, p,
    "import module namespace sleep = 'http://zorba.io/modules/sleep';\n"
    "declare variable $m external := 100000;\n"
    "sum(for $i in 1 to $m return (sleep:millis(500), 1))");
  std::cout << "  " << o << std::endl;
  if (o.code == "XQST0059")
  {
    std::cout << "sleep module not built: case skipped" << std::endl;
    return;
  }
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(within(o, timeout + seconds(8)));
}

// A sort makes no iterator calls, and the limit may run out while it is under
// way or while the tuples are read: which one depends on the machine. Either
// way the sort does not run far past the limit, and raising from within it
// leaves no memory behind (leak checks in the sanitizer builds).
void test_order_by(Zorba* z)
{
  for (long long ms : { 300, 1000, 2000 })
  {
    SecurityPolicy const p =
      SecurityPolicy::unrestricted().setTimeout(milliseconds(ms));
    Outcome const o = run(z, p,
      "declare variable $m external := 3000000;\n"
      "count(for $i in 1 to $m order by string($i) descending return $i)");
    std::cout << "  " << ms << " ms: " << o << std::endl;
    CHECK(o.ok || failed_with(o, "ZXQP0068"));
    CHECK(within(o, milliseconds(ms) + seconds(15)));
  }
}

// The limit may run out anywhere in a query, so try many: from early on to about
// the time the whole query takes. A limit that lands in a sort stops it with the
// tuples half moved. What is left must still release every sort key exactly once
// (checked by the sanitizer builds, and by the heap checks of the C library).
void test_order_by_deadlines(Zorba* z)
{
  const char* const QUERIES[] = {
    "declare variable $m external := 100000;\n"
    "count(for $i in 1 to $m order by string($i) descending return $i)",
    "declare variable $m external := 100000;\n"
    "count(for $i in 1 to $m stable order by string($i) descending return $i)"
  };
  int const STEPS = 32;
  seconds const budget(20);
  steady_clock::time_point const start = steady_clock::now();

  for (const char* query : QUERIES)
  {
    Outcome const full = run(z, SecurityPolicy::unrestricted(), query);
    CHECK(full.ok && full.value == "100000");

    int stopped = 0, completed = 0;
    for (int step = 1; step <= STEPS; ++step)
    {
      if (steady_clock::now() - start > budget)
        break;
      milliseconds const limit(
          std::max<long long>(1, full.elapsed.count() * step / STEPS));
      SecurityPolicy const p =
        SecurityPolicy::unrestricted().setTimeout(limit);
      Outcome const o = run(z, p, query);
      CHECK(o.ok || failed_with(o, "ZXQP0068"));
      (o.ok ? completed : stopped)++;
    }
    std::cout << "  full run " << full.elapsed.count() << " ms: " << stopped
              << " stopped, " << completed << " completed" << std::endl;
  }
}

// An error raised by a comparison stops the sort the same way. Here the keys
// are arranged so that the comparison that fails comes after the sort has
// written part of its result: the NaN keys compare with both of the other
// kinds, and the integers first meet the strings late.
void test_error_within_sort(Zorba* z)
{
  std::string const flwor =
    "for $i in 1 to $m\n"
    "let $k := if ($i le 14) then $i\n"
    "          else if ($i le 21) then number(concat('n', $i))\n"
    "          else concat('s', $i)\n"
    "stable order by $k empty least\n";
  std::string const prolog = "declare variable $m external := 56;\n";
  SecurityPolicy const p = SecurityPolicy::unrestricted();

  Outcome o = run(z, p, prolog + flwor + "return $i");
  std::cout << "  order by: " << o << std::endl;
  CHECK(failed_with(o, "XPTY0004"));

  o = run(z, p, prolog + flwor + "count $c return $c");
  std::cout << "  order by and count: " << o << std::endl;
  CHECK(failed_with(o, "XPTY0004"));

  // Caught within the query: the plan is reset, and goes on running.
  o = run(z, p, prolog +
    "try { " + flwor + "return $i } catch * { 'caught' }");
  std::cout << "  caught: " << o << std::endl;
  CHECK(o.ok && o.value == "caught");

  // The same keys, in an order that makes the integers meet the strings in
  // the first comparisons. And a sort that does not fail still sorts.
  o = run(z, p, prolog +
    "for $i in 1 to $m\n"
    "stable order by (if ($i mod 2 eq 0) then $i else concat('s', $i))\n"
    "return $i");
  CHECK(failed_with(o, "XPTY0004"));

  o = run(z, p,
    "string-join(for $i in (3, 1, 2) stable order by $i descending "
    "return string($i), ',')");
  CHECK(o.ok && o.value == "3,2,1");
}

// A string operation that takes long per call: the string is 50 MB, and each
// step reads all of it.
void test_big_strings(Zorba* z)
{
  seconds const timeout(3);
  SecurityPolicy const p = SecurityPolicy::unrestricted().setTimeout(timeout);
  Outcome const o = run(z, p,
    "declare variable $m external := 5000000;\n"
    "let $s := string-join(for $i in 1 to $m return 'abcdefghij')\n"
    "return sum(for $i in 1 to 1000000\n"
    "           return string-length(upper-case(concat($s, $i))))");
  std::cout << "  " << o << std::endl;
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(within(o, timeout + seconds(15)));
}

void section(Zorba* z, const char* aName, void (*aTest)(Zorba*))
{
  std::cout << aName << std::endl;
  aTest(z);
}

} // anonymous namespace


int
timeout_bounds(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    section(lZorba, "test_constant_folding", test_constant_folding);
    section(lZorba, "test_external_function", test_external_function);
    section(lZorba, "test_order_by", test_order_by);
    section(lZorba, "test_order_by_deadlines", test_order_by_deadlines);
    section(lZorba, "test_error_within_sort", test_error_within_sort);
    section(lZorba, "test_big_strings", test_big_strings);
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "unexpected exception: " << e << std::endl;
    ++theFailures;
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
