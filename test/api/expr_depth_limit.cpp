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


// Every shape nested far beyond any limit fails cleanly.
void test_very_deep(Zorba* z)
{
  SecurityPolicy const p = SecurityPolicy::unrestricted();
  for (Shape const& s : shapes())
  {
    std::cout << "  " << s.name << std::endl;
    Outcome const o = run(z, p, s.make(100000));
    CHECK(failed_with(o, "ZXQP0070"));
  }
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


} // anonymous namespace


int
expr_depth_limit(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    std::cout << "test_very_deep" << std::endl;
    test_very_deep(lZorba);
    std::cout << "test_syntax_error" << std::endl;
    test_syntax_error(lZorba);
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
