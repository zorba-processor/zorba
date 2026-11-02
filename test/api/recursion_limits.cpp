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

// Recursive functions under the evaluation limits: the timeout (ZXQP0068)
// must stop them, the memory of a completed call must be released, and
// releasing it must not change results.

#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
# include <sys/resource.h>
#endif

#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;

// AddressSanitizer needs far more address space than the process uses, and
// its quarantine keeps freed memory resident.
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
# define ZORBA_TEST_SANITIZER 1
#elif defined(__has_feature)
# if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#  define ZORBA_TEST_SANITIZER 1
# endif
#endif

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

// Bounds the address space while it lives, so that a query that retains
// memory fails with an out-of-memory error instead of exhausting the machine.
class AddressSpaceLimit
{
#if defined(__linux__) && !defined(ZORBA_TEST_SANITIZER)
  struct rlimit theOld;
  bool theSet;

public:
  explicit AddressSpaceLimit(uint64_t aBytes) : theSet(false)
  {
    if (getrlimit(RLIMIT_AS, &theOld) == 0)
    {
      struct rlimit lNew = theOld;
      lNew.rlim_cur = aBytes;
      theSet = setrlimit(RLIMIT_AS, &lNew) == 0;
    }
  }

  ~AddressSpaceLimit()
  {
    if (theSet)
      setrlimit(RLIMIT_AS, &theOld);
  }
#else
public:
  explicit AddressSpaceLimit(uint64_t) {}
#endif
};

long long peak_rss()
{
#if defined(__unix__) || defined(__APPLE__)
  struct rusage u;
  getrusage(RUSAGE_SELF, &u);
# ifdef __APPLE__
  return static_cast<long long>(u.ru_maxrss);
# else
  return static_cast<long long>(u.ru_maxrss) * 1024;
# endif
#else
  return 0;
#endif
}

// %N% is replaced by the argument of the outermost call.
const char UNDECLARED[] =
  "declare function local:f($n)\n"
  "{ if ($n le 0) then 1 else local:f($n - 1) + local:f($n - 1) };\n"
  "local:f(%N%)";

// With a declared single result, the consumer does not ask for a second item.
// (The sequence type of the parameter keeps the automatic cache out of it.)
const char DECLARED[] =
  "declare function local:f($n as xs:integer*) as xs:integer\n"
  "{ if ($n le 0) then 1 else local:f($n - 1) + local:f($n - 1) };\n"
  "local:f(%N%)";

const char MUTUAL[] =
  "declare function local:a($n)\n"
  "{ if ($n le 0) then 1 else local:b($n - 1) + local:b($n - 1) };\n"
  "declare function local:b($n) { local:a($n) };\n"
  "local:a(%N%)";

const char FUNCTION_ITEM[] =
  "declare function local:f($g, $n)\n"
  "{ if ($n le 0) then 1 else $g($g, $n - 1) + $g($g, $n - 1) };\n"
  "local:f(local:f#2, %N%)";

const char LET[] =
  "declare function local:f($n)\n"
  "{ if ($n le 0) then 1 else\n"
  "  let $a := local:f($n - 1), $b := local:f($n - 1) return $a + $b };\n"
  "local:f(%N%)";

const char FOR[] =
  "declare function local:f($n)\n"
  "{ if ($n le 0) then 1 else\n"
  "  sum(for $i in 1 to 2 return local:f($n - 1)) };\n"
  "local:f(%N%)";

const char* const SHAPES[] =
  { UNDECLARED, DECLARED, MUTUAL, LET, FOR, FUNCTION_ITEM };

std::string with_n(const char* aQuery, int n)
{
  std::string q(aQuery);
  q.replace(q.find("%N%"), 3, std::to_string(n));
  return q;
}

// Generous: sanitizer builds on shared CI runners are slow.
const milliseconds SLACK(15000);
const uint64_t ADDRESS_SPACE = 2000000000ULL;

// 2^40 calls: only the timeout can end these. The call tree stays 40 deep.
void test_timeout(Zorba* z)
{
  SecurityPolicy const p =
    SecurityPolicy::unrestricted().setTimeout(seconds(1));
  AddressSpaceLimit const limit(ADDRESS_SPACE);

  for (const char* shape : SHAPES)
  {
    Outcome const o = run(z, p, with_n(shape, 40));
    std::cout << "  n=40: " << o << std::endl;
    CHECK(failed_with(o, "ZXQP0068"));
    CHECK(o.elapsed >= seconds(1));
    CHECK(within(o, seconds(1) + SLACK));
  }

  // The error cannot be caught and ignored.
  Outcome const o = run(z, p,
    "declare function local:f($n)\n"
    "{ if ($n le 0) then 1 else local:f($n - 1) + local:f($n - 1) };\n"
    "declare variable $m external := 9223372036854775807;\n"
    "count(for $i in 1 to $m return try { local:f(40) } catch * { 0 })");
  std::cout << "  catch-all: " << o << std::endl;
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(within(o, seconds(1) + SLACK));
}

// The peak must follow the depth of the recursion, not the number of calls:
// at n = 17, about 11 KB per call would be 1.4 GB.
void test_memory(Zorba* z)
{
#ifdef ZORBA_TEST_SANITIZER
  int const n = 15;
  long long const allowed = 640LL << 20;
#else
  int const n = 17;
  long long const allowed = 128LL << 20;
#endif
  SecurityPolicy const p = SecurityPolicy::unrestricted();
  AddressSpaceLimit const limit(ADDRESS_SPACE);

  // Warm up the process: the first query loads what every query needs.
  CHECK(run(z, p, with_n(UNDECLARED, 8)).value == "256");

  long long const base = peak_rss();
  for (const char* shape : SHAPES)
  {
    Outcome const o = run(z, p, with_n(shape, n));
    long long const grown = peak_rss() - base;
    std::cout << "  n=" << n << ": peak RSS grew by " << (grown >> 20)
              << " MiB; " << o << std::endl;
    CHECK(o.ok && o.value == std::to_string(1 << n));
    if (grown >= allowed)
    {
      std::cerr << "peak RSS grew by " << (grown >> 20) << " MiB, allowed "
                << (allowed >> 20) << " MiB:\n" << shape << std::endl;
      ++theFailures;
    }
  }

#ifndef ZORBA_TEST_SANITIZER
  // Each dynamic call also used to leave a small expression behind: 400 bytes
  // per call, which only shows with more calls.
  long long const before = peak_rss();
  Outcome const o = run(z, p, with_n(FUNCTION_ITEM, 19));
  long long const grown = peak_rss() - before;
  std::cout << "  function items, n=19: peak RSS grew by " << (grown >> 20)
            << " MiB; " << o << std::endl;
  CHECK(o.ok && o.value == std::to_string(1 << 19));
  if (grown >= allowed)
  {
    std::cerr << "function items: peak RSS grew by " << (grown >> 20)
              << " MiB" << std::endl;
    ++theFailures;
  }
#endif
}

struct Case
{
  const char* query;
  const char* expected;
};

// Queries for which a body that is closed and opened again could be wrong: a
// call that is run several times, abandoned, or that returned something that
// outlives it.
const Case RESULTS[] =
{
  // Several results.
  { "declare function local:g($n)\n"
    "{ $n, if ($n gt 0) then local:g($n - 1) else () };\n"
    "local:g(3)", "3 2 1 0" },
  // An infinite sequence that is only read in part stays lazy.
  { "declare function local:g($n) { $n, local:g($n + 1) };\n"
    "head(local:g(0)), subsequence(local:g(5), 2, 2)", "0 6 7" },
  // The same call site runs again.
  { "declare function local:c($n)\n"
    "{ if ($n le 0) then 1 else local:c($n - 1) + local:c($n - 1) };\n"
    "for $i in 1 to 3 return local:c($i)", "2 4 8" },
  { "declare function local:c($n as xs:integer) as xs:integer\n"
    "{ if ($n le 0) then 1 else local:c($n - 1) + local:c($n - 1) };\n"
    "for $i in 1 to 3 let $x := local:c($i) return ($x, $x)", "2 2 4 4 8 8" },
  // A call abandoned after the first result and then run again.
  { "declare function local:g($n) { $n, if ($n gt 0) then local:g($n - 1) };\n"
    "for $i in 1 to 3 return (head(local:g($i)), count(local:g($i)))",
    "1 2 2 3 3 4" },
  // Empty results, optional results.
  { "declare function local:e($n) as xs:integer?\n"
    "{ if ($n le 0) then () else local:e($n - 1) };\n"
    "empty(local:e(3)), count((local:e(2), local:e(1)))", "true 0" },
  // A declared single result must still be checked.
  { "declare function local:bad($n) as xs:integer?\n"
    "{ if ($n le 0) then (1, 2) else local:bad($n - 1) };\n"
    "try { local:bad(2) } catch * { 'err' }", "err" },
  // Arguments that are read lazily from the caller.
  { "declare function local:s($seq, $n)\n"
    "{ if ($n le 0) then count($seq) else local:s(($seq, $n), $n - 1) };\n"
    "local:s((), 5)", "5" },
  // Constructed nodes outlive the call that built them.
  { "declare function local:t($n)\n"
    "{ if ($n le 0) then <leaf/> else <n>{local:t($n - 1), local:t($n - 1)}</n> };\n"
    "count(local:t(4)//leaf), local-name(local:t(2)/n[1]/leaf[2])",
    "16 leaf" },
  // A function item built in a call outlives that call.
  { "declare function local:mk($n)\n"
    "{ if ($n le 0) then function($x) { $x } else\n"
    "  let $g := local:mk($n - 1) return function($x) { $g($x) + $n } };\n"
    "local:mk(3)(1)", "7" },
  // A cached function.
  { "declare namespace an = 'http://zorba.io/annotations';\n"
    "declare %an:cache function local:fib($n as xs:integer) as xs:integer\n"
    "{ if ($n lt 2) then $n else local:fib($n - 1) + local:fib($n - 2) };\n"
    "local:fib(30), local:fib(30)", "832040 832040" },
  // Mutual recursion with several results.
  { "declare function local:ev($n)\n"
    "{ if ($n le 0) then 'e' else ($n, local:od($n - 1)) };\n"
    "declare function local:od($n)\n"
    "{ if ($n le 0) then 'o' else ($n, local:ev($n - 1)) };\n"
    "local:ev(3)", "3 2 1 o" },
  // An error in a nested call, caught and run again.
  { "declare function local:r($n)\n"
    "{ if ($n le 0) then error() else local:r($n - 1) };\n"
    "for $i in 1 to 2 return try { local:r($i) } catch * { $i }", "1 2" },
  // A function that is not recursive, called from a recursive one.
  { "declare function local:leaf($x) { $x * 2 };\n"
    "declare function local:r($n)\n"
    "{ if ($n le 0) then 0 else local:leaf($n) + local:r($n - 1) };\n"
    "for $i in 1 to 2 return local:r($i)", "2 6" },
};

void test_results(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted();
  for (const Case& c : RESULTS)
  {
    Outcome const o = run(z, p, c.query);
    if (!o.ok || o.value != c.expected)
    {
      std::cerr << "expected " << c.expected << ", got " << o << ":\n"
                << c.query << std::endl;
      ++theFailures;
    }
  }
}

void section(Zorba* z, const char* aName, void (*aTest)(Zorba*))
{
  std::cout << aName << std::endl;
  steady_clock::time_point const start = steady_clock::now();
  aTest(z);
  std::cout << aName << ": " << std::chrono::duration_cast<milliseconds>(
      steady_clock::now() - start).count() << " ms" << std::endl;
}

} // anonymous namespace


int
recursion_limits(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    section(lZorba, "test_results", test_results);
    section(lZorba, "test_memory", test_memory);
    section(lZorba, "test_timeout", test_timeout);
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
