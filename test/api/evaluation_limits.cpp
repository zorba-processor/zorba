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

// Tests for the evaluation limits of the security policy: the timeout
// (ZXQP0068), XQuery::setTimeout() as a per-query override, and the call
// depth limit (ZXQP0069), including the machine-stack guard that applies
// when the depth is unlimited.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
# include <sys/resource.h>
#endif

// TEMPORARY DIAGNOSTICS
#if defined(__linux__)
# include <csignal>
# include <cstdlib>
# include <execinfo.h>
# include <unistd.h>
static void segv_handler(int)
{
  void* frames[64];
  int n = backtrace(frames, 64);
  static const char msg[] = "SIGSEGV backtrace (innermost 64):\n";
  ssize_t r = write(1, msg, sizeof msg - 1); (void)r;
  backtrace_symbols_fd(frames, n, 1);
  // Resolve the first 16 frames (unsafe in a signal handler; diagnostics).
  char** syms = backtrace_symbols(frames, n);
  for (int i = 0; syms && i < n && i < 16; ++i)
  {
    std::string sym(syms[i]);
    size_t lp = sym.find('('), plus = sym.find("(+"), rp = sym.find(')');
    if (lp == std::string::npos || plus == std::string::npos || rp == std::string::npos)
      continue;
    std::string cmd = "addr2line -C -f -e " + sym.substr(0, lp) + " " +
                      sym.substr(plus + 2, rp - plus - 2) + " | head -1";
    int rc = system(cmd.c_str()); (void)rc;
  }
  _exit(3);
}
static void install_segv_handler()
{
  static char altstack[1 << 16];
  stack_t ss;
  ss.ss_sp = altstack; ss.ss_size = sizeof altstack; ss.ss_flags = 0;
  sigaltstack(&ss, nullptr);
  struct sigaction sa = {};
  sa.sa_handler = segv_handler;
  sa.sa_flags = SA_ONSTACK;
  sigaction(SIGSEGV, &sa, nullptr);
}
#else
static void install_segv_handler() {}
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
  milliseconds elapsed{0};
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    return os << "ok: " << o.value << " (" << o.elapsed.count() << " ms)";
  return os << o.code << ": " << o.message << " (" << o.elapsed.count()
            << " ms)";
}

Outcome execute(XQuery_t q)
{
  Outcome o;
  steady_clock::time_point const start = steady_clock::now();
  try
  {
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
      o.value += item.getStringValue().str();
    it->close();
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
  o.elapsed = std::chrono::duration_cast<milliseconds>(
      steady_clock::now() - start);
  return o;
}

StaticContext_t context(Zorba* z, const SecurityPolicy& p)
{
  StaticContext_t sctx = z->createStaticContext();
  sctx->setSecurityPolicy(p);
  return sctx;
}

Outcome run(Zorba* z, const SecurityPolicy& p, const std::string& aQuery)
{
  XQuery_t q;
  try
  {
    q = z->compileQuery(aQuery, context(z, p));
  }
  catch (ZorbaException const& e)
  {
    Outcome o;
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
    return o;
  }
  return execute(q);
}

bool failed_with(const Outcome& o, const char* aCode)
{
  if (!o.ok && o.code == aCode)
    return true;
  std::cerr << "expected " << aCode << ", got " << o << std::endl;
  return false;
}

bool succeeded_with(const Outcome& o, const std::string& aValue)
{
  if (o.ok && o.value == aValue)
    return true;
  std::cerr << "expected " << aValue << ", got " << o << std::endl;
  return false;
}

// Never terminates. The bound is an external variable so that the optimizer
// cannot fold the expression at compile time.
const char ENDLESS[] =
  "declare variable $max as xs:integer external := 9223372036854775807;\n"
  "(1 to $max)[. lt 0]";

// An endless loop whose body catches every error: the timeout must not be
// catchable, or the loop would go on.
const char ENDLESS_CATCH_ALL[] =
  "declare variable $max as xs:integer external := 9223372036854775807;\n"
  "for $i in 1 to $max\n"
  "return try { (1 to $max)[. lt 0] } catch * { () }";

const char ENDLESS_EVAL[] =
  "import module namespace r = \"http://zorba.io/modules/reflection\";\n"
  "r:eval(\"(1 to 9223372036854775807)[. lt 0]\")";

// A finite query that takes some time (well over 50 ms on any machine).
const char SLOW[] =
  "declare variable $max as xs:integer external := 5000000;\n"
  "count((1 to $max)[. mod 2 eq 0])";

const char DEPTH[] =
  "declare function local:depth($n as xs:integer) as xs:integer\n"
  "{ if ($n le 0) then 0 else 1 + local:depth($n - 1) };\n"
  "local:depth(%N%)";

std::string depth_query(uint64_t n)
{
  std::string q(DEPTH);
  q.replace(q.find("%N%"), 3, std::to_string(n));
  return q;
}

// Generous: sanitizer builds on shared CI runners are slow.
const milliseconds SLACK(15000);

void test_timeout(Zorba* z)
{
  SecurityPolicy p = SecurityPolicy::unrestricted().setTimeout(milliseconds(200));

  Outcome o = run(z, p, ENDLESS);
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(o.elapsed >= milliseconds(200));
  CHECK(o.elapsed < milliseconds(200) + SLACK);

  o = run(z, p, ENDLESS_CATCH_ALL);
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(o.elapsed < milliseconds(200) + SLACK);

  // eval shares the deadline of the query that runs it.
  o = run(z, p, ENDLESS_EVAL);
  if (o.code == "XQST0059")
    std::cout << "reflection module not built: eval case skipped" << std::endl;
  else
    CHECK(failed_with(o, "ZXQP0068"));
  CHECK(o.elapsed < milliseconds(200) + SLACK);

  // A query that finishes in time is unaffected.
  CHECK(succeeded_with(run(z, p, "sum(1 to 100)"), "5050"));

  // The deadline restarts with each execution of the same query.
  XQuery_t q = z->compileQuery(ENDLESS, context(z, p));
  CHECK(failed_with(execute(q), "ZXQP0068"));
  CHECK(failed_with(execute(q), "ZXQP0068"));
}

void test_set_timeout_override(Zorba* z)
{
  // setTimeout() (seconds) replaces a missing policy timeout ...
  XQuery_t q = z->createQuery();
  q->setTimeout(1);
  q->compile(ENDLESS);
  Outcome o = execute(q);
  CHECK(failed_with(o, "ZXQP0068"));
  CHECK(o.elapsed >= milliseconds(1000));
  CHECK(o.elapsed < milliseconds(1000) + SLACK);

  // ... can be set after compilation ...
  q = z->compileQuery(ENDLESS);
  q->setTimeout(1);
  CHECK(failed_with(execute(q), "ZXQP0068"));

  // ... and 0 disables the policy timeout for this query.
  SecurityPolicy p = SecurityPolicy::unrestricted().setTimeout(milliseconds(50));
  q = z->compileQuery(SLOW, context(z, p));
  q->setTimeout(0);
  CHECK(succeeded_with(execute(q), "2500000"));

  // -1 restores the policy timeout.
  q->setTimeout(-1);
  CHECK(failed_with(execute(q), "ZXQP0068"));
}

void test_no_timeout_by_default(Zorba* z)
{
  CHECK(SecurityPolicy::secure().getTimeout() == milliseconds(0));
  CHECK(SecurityPolicy::unrestricted().getTimeout() == milliseconds(0));
  CHECK(succeeded_with(run(z, SecurityPolicy::unrestricted(), SLOW),
                       "2500000"));
}

// Reports the cost of the deadline checks: the same query with no timeout
// and with a timeout that never fires (best of 3 each).
void report_timeout_overhead(Zorba* z)
{
  SecurityPolicy const none = SecurityPolicy::unrestricted();
  SecurityPolicy const hour =
    SecurityPolicy::unrestricted().setTimeout(seconds(3600));
  milliseconds best_none(INT64_MAX), best_hour(INT64_MAX);
  for (int i = 0; i < 3; ++i)
  {
    best_none = std::min(best_none, run(z, none, SLOW).elapsed);
    best_hour = std::min(best_hour, run(z, hour, SLOW).elapsed);
  }
  std::cout << "timeout overhead: " << best_none.count() << " ms without, "
            << best_hour.count() << " ms with a timeout" << std::endl;
}

void test_call_depth(Zorba* z)
{
  // Default limit (1024) in both presets.
  for (SecurityPolicy const& p :
       { SecurityPolicy::secure(), SecurityPolicy::unrestricted() })
  {
    CHECK(p.getMaxCallDepth() == 1024);
    // local:depth(n) nests n + 1 calls.
    CHECK(succeeded_with(run(z, p, depth_query(1023)), "1023"));
    CHECK(failed_with(run(z, p, depth_query(1024)), "ZXQP0069"));
    CHECK(failed_with(run(z, p, depth_query(100000)), "ZXQP0069"));
  }

  SecurityPolicy p = SecurityPolicy::unrestricted().setMaxCallDepth(100);
  CHECK(succeeded_with(run(z, p, depth_query(99)), "99"));
  Outcome o = run(z, p, depth_query(100));
  CHECK(failed_with(o, "ZXQP0069"));
  CHECK(o.message.find("100") != std::string::npos);

  std::cout << "  function items" << std::endl;
  // Recursion through function items counts too.
  CHECK(failed_with(run(z, p,
      "declare function local:f($g, $n) { $g($g, $n + 1) };\n"
      "local:f(local:f#2, 0)"), "ZXQP0069"));

  std::cout << "  higher limit" << std::endl;
  // A higher limit allows deeper recursion.
  p.setMaxCallDepth(5000);
  CHECK(succeeded_with(run(z, p, depth_query(2000)), "2000"));
}

// With no depth limit, deep recursion must still end with ZXQP0069 (the
// machine-stack guard) instead of a crash. Also reports how deep the
// recursion can go on this thread's stack.
void test_stack_guard(Zorba* z)
{
  SecurityPolicy p = SecurityPolicy::unrestricted().setMaxCallDepth(0);

#if defined(__unix__) || defined(__APPLE__)
  struct rlimit rl;
  if (getrlimit(RLIMIT_STACK, &rl) == 0)
  {
    if (rl.rlim_cur == RLIM_INFINITY)
      std::cout << "  RLIMIT_STACK: unlimited" << std::endl;
    else
      std::cout << "  RLIMIT_STACK: " << rl.rlim_cur << std::endl;
  }
#endif

  Outcome o = run(z, p, depth_query(100000000));
  CHECK(failed_with(o, "ZXQP0069"));
  CHECK(o.message.find("stack") != std::string::npos);

  std::cout << "  measuring" << std::endl;
  uint64_t lo = 1, hi = 100000000;
  while (lo + 1 < hi)
  {
    uint64_t const mid = lo + (hi - lo) / 2;
    if (run(z, p, depth_query(mid)).ok)
      lo = mid;
    else
      hi = mid;
  }
  std::cout << "stack guard: local:depth() reaches depth " << lo
            << " with an unlimited call depth" << std::endl;
  // The default limit must be reachable before the stack runs out.
  CHECK(lo >= 1024);
}

} // anonymous namespace


int
evaluation_limits(int argc, char* argv[])
{
  install_segv_handler();
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    std::cout << "test_timeout" << std::endl;
    test_timeout(lZorba);
    std::cout << "test_set_timeout_override" << std::endl;
    test_set_timeout_override(lZorba);
    std::cout << "test_no_timeout_by_default" << std::endl;
    test_no_timeout_by_default(lZorba);
    std::cout << "report_timeout_overhead" << std::endl;
    report_timeout_overhead(lZorba);
    std::cout << "test_call_depth" << std::endl;
    test_call_depth(lZorba);
    std::cout << "test_stack_guard" << std::endl;
    test_stack_guard(lZorba);
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
