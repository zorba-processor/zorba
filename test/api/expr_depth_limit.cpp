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

// Tests for the expression nesting limit of the security policy
// (SecurityPolicy::setMaxExprDepth(), zerr:ZXQP0070): deeply nested queries
// of every shape must fail to compile with a clean error instead of
// overflowing the stack, and queries just under the default limit must
// compile and run. The queries are generated here rather than stored as
// (huge) files.

#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
# include <pthread.h>
# include <sys/mman.h>
# define ZORBA_TEST_MEASURE_STACK 1
#endif

#include <zorba/item.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

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
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    return os << "ok: " << o.value.substr(0, 40);
  return os << o.code << ": " << o.message.substr(0, 200);
}

// Compiles the query and, if aExecute, runs it.
Outcome run(Zorba* z, const SecurityPolicy& p, const std::string& aQuery,
            bool aExecute = true)
{
  Outcome o;
  try
  {
    StaticContext_t sctx = z->createStaticContext();
    sctx->setSecurityPolicy(p);
    XQuery_t q = z->compileQuery(aQuery, sctx);
    if (aExecute)
    {
      Iterator_t it = q->iterator();
      it->open();
      Item item;
      while (it->next(item))
        if (!item.isJSONItem())
          o.value += item.getStringValue().str();
      it->close();
    }
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
  return o;
}

bool failed_with(const Outcome& o, const char* aCode)
{
  if (!o.ok && o.code == aCode)
    return true;
  std::cerr << "expected " << aCode << ", got " << o << std::endl;
  return false;
}

std::string repeat(const std::string& s, uint64_t n)
{
  std::string r;
  r.reserve(s.size() * n);
  for (uint64_t i = 0; i < n; ++i)
    r += s;
  return r;
}

// A way of nesting expressions n levels deep.
struct Shape
{
  const char* name;
  std::function<std::string(uint64_t)> make;
};

const char JSONIQ[] = "jsoniq version \"1.0\";\n";

const std::vector<Shape>& shapes()
{
  static const std::vector<Shape> theShapes = {
    // Left-deep chain of binary operators (folded at compile time).
    { "plus", [](uint64_t n) { return "1" + repeat(" + 1", n); } },
    // The same, not foldable: the plan is as deep as the query.
    { "plus-var", [](uint64_t n) {
        return "declare variable $x external := 1;\n$x" +
               repeat(" + $x", n); } },
    { "or", [](uint64_t n) {
        return "fn:false()" + repeat(" or fn:false()", n); } },
    { "parens", [](uint64_t n) {
        return repeat("(", n) + "1" + repeat(")", n); } },
    { "calls", [](uint64_t n) {
        return repeat("fn:abs(", n) + "-1" + repeat(")", n); } },
    { "elements", [](uint64_t n) {
        return repeat("<a>", n) + "x" + repeat("</a>", n); } },
    { "computed", [](uint64_t n) {
        return repeat("element a { ", n) + "1" + repeat(" }", n); } },
    { "if", [](uint64_t n) {
        return repeat("if (fn:true()) then ", n) + "1" +
               repeat(" else 0", n); } },
    { "flwor", [](uint64_t n) {
        return repeat("for $v in 1 return ", n) + "1"; } },
    { "predicates", [](uint64_t n) {
        return repeat("(1)[", n) + "1" + repeat("]", n); } },
    { "arrays", [](uint64_t n) {
        return JSONIQ + repeat("[", n) + "1" + repeat("]", n); } },
    { "objects", [](uint64_t n) {
        return JSONIQ + repeat("{ \"a\" : ", n) + "1" + repeat(" }", n); } },
  };
  return theShapes;
}


#ifdef ZORBA_TEST_MEASURE_STACK
// Runs aFn on a thread with a large stack painted with a pattern and
// returns the number of bytes of stack it used.
const unsigned char PAINT = 0xA5;
const size_t MEASURE_STACK_SIZE = 256u * 1024 * 1024;

void* measure_thread_main(void* aFn)
{
  (*static_cast<std::function<void()>*>(aFn))();
  return nullptr;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("address")))
#endif
size_t untouched(const unsigned char* aBase, size_t aSize)
{
  size_t i = 0;
  while (i < aSize && aBase[i] == PAINT)
    ++i;
  return i;
}

size_t measure_stack(std::function<void()> aFn)
{
  void* lMem = mmap(nullptr, MEASURE_STACK_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANON, -1, 0);
  if (lMem == MAP_FAILED)
    return 0;
  memset(lMem, PAINT, MEASURE_STACK_SIZE);

  pthread_attr_t lAttr;
  pthread_attr_init(&lAttr);
  pthread_attr_setstack(&lAttr, lMem, MEASURE_STACK_SIZE);
  pthread_t lThread;
  size_t lUsed = 0;
  if (pthread_create(&lThread, &lAttr, measure_thread_main, &aFn) == 0)
  {
    pthread_join(lThread, nullptr);
    lUsed = MEASURE_STACK_SIZE -
            untouched(static_cast<unsigned char*>(lMem), MEASURE_STACK_SIZE);
  }
  pthread_attr_destroy(&lAttr);
  munmap(lMem, MEASURE_STACK_SIZE);
  return lUsed;
}
#endif


void test_policy()
{
  CHECK(SecurityPolicy::secure().getMaxExprDepth() == 5000);
  CHECK(SecurityPolicy::unrestricted().getMaxExprDepth() == 5000);
  CHECK(SecurityPolicy().setMaxExprDepth(0).getMaxExprDepth() == 0);

  // Narrowing keeps the smaller non-zero limit.
  SecurityPolicy a = SecurityPolicy::unrestricted().setMaxExprDepth(100);
  SecurityPolicy b = SecurityPolicy::unrestricted().setMaxExprDepth(0);
  CHECK(a.narrowedBy(b).getMaxExprDepth() == 100);
  CHECK(b.narrowedBy(a).getMaxExprDepth() == 100);
  CHECK(a.narrowedBy(SecurityPolicy::unrestricted()).getMaxExprDepth() == 100);
  CHECK(!(a == b));
  CHECK(a.isWithin(b));
}


// Every shape nested far beyond any limit fails cleanly.
void test_very_deep(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted();
  for (Shape const& s : shapes())
  {
    std::cout << "  " << s.name << std::endl;
    Outcome const o = run(z, p, s.make(100000));
    CHECK(failed_with(o, "ZXQP0070"));
    CHECK(o.message.find("5000") != std::string::npos);
  }
}


// A small limit is enforced exactly for every shape: nesting depth n
// compiles, n + 1 does not.
void test_small_limit(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted().setMaxExprDepth(50);
  for (Shape const& s : shapes())
  {
    uint64_t n = 0;
    while (n < 60 && run(z, p, s.make(n + 1), false).ok)
      ++n;
    std::cout << "  " << s.name << ": " << n << " levels" << std::endl;
    CHECK(n >= 10);
    CHECK(n < 50);
    Outcome const o = run(z, p, s.make(n + 1));
    CHECK(failed_with(o, "ZXQP0070"));
    CHECK(o.message.find("50") != std::string::npos);
  }
}


// Finds, for each shape, the deepest nesting that the default limit allows,
// and checks that it compiles and runs (also in sanitizer builds), and that
// one more level fails with the limit error, not the stack check. Reports
// the stack that compiling and running it takes.
void test_default_limit(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted();
  for (Shape const& s : shapes())
  {
    uint64_t lo = 1, hi = 5001;  // run(lo) compiles, run(hi) does not
    CHECK(run(z, p, s.make(lo), false).ok);
    CHECK(!run(z, p, s.make(hi), false).ok);
    while (lo + 1 < hi)
    {
      uint64_t const mid = lo + (hi - lo) / 2;
      if (run(z, p, s.make(mid), false).ok)
        lo = mid;
      else
        hi = mid;
    }

    Outcome const ok = run(z, p, s.make(lo));
    if (!ok.ok)
      std::cerr << s.name << " at " << lo << " levels: " << ok << std::endl;
    CHECK(ok.ok);
    Outcome const over = run(z, p, s.make(lo + 1));
    CHECK(failed_with(over, "ZXQP0070"));
    CHECK(over.message.find("5000") != std::string::npos);

    std::cout << "  " << s.name << ": " << lo << " levels";
#ifdef ZORBA_TEST_MEASURE_STACK
    std::string const q = s.make(lo);
    size_t const lUsed = measure_stack([&] { run(z, p, q); });
    std::cout << ", " << lUsed / 1024 << " KiB of stack";
#endif
    std::cout << std::endl;
  }
}


// Without a limit, translating the parse tree stops before the stack runs
// out; parse trees too deep to translate are destroyed without recursion.
void test_unlimited(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted().setMaxExprDepth(0);
  Outcome const o = run(z, p, shapes()[0].make(300000));
  CHECK(failed_with(o, "ZXQP0070"));
  CHECK(o.message.find("stack") != std::string::npos);
}


// Syntax errors deep inside a deep parse tree: the partial tree on the
// parser's stack is destroyed without recursion.
void test_syntax_error(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted();
  CHECK(failed_with(run(z, p, "1" + repeat(" + 1", 100000) + " +)"),
                    "XPST0003"));
  CHECK(failed_with(run(z, p, repeat("(", 100000) + "1"), "XPST0003"));
}


// eval'ed queries are compiled under the policy of the query that runs them.
void test_eval(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted().setMaxExprDepth(50);
  std::string const q =
    "import module namespace r = \"http://zorba.io/modules/reflection\";\n"
    "r:eval(\"" + repeat("(", 60) + "1" + repeat(")", 60) + "\")";
  Outcome const o = run(z, p, q);
  if (o.code == "XQST0059")
    std::cout << "reflection module not built: eval case skipped" << std::endl;
  else
    CHECK(failed_with(o, "ZXQP0070"));
}

} // anonymous namespace


int
expr_depth_limit(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    std::cout << "test_policy" << std::endl;
    test_policy();
    std::cout << "test_very_deep" << std::endl;
    test_very_deep(lZorba);
    std::cout << "test_small_limit" << std::endl;
    test_small_limit(lZorba);
    std::cout << "test_default_limit" << std::endl;
    test_default_limit(lZorba);
    std::cout << "test_unlimited" << std::endl;
    test_unlimited(lZorba);
    std::cout << "test_syntax_error" << std::endl;
    test_syntax_error(lZorba);
    std::cout << "test_eval" << std::endl;
    test_eval(lZorba);
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
