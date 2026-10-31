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

// A function item taken from a query result and kept by the application must
// stay usable after the query that created it has been closed and destroyed:
// invoking it through StaticContext::invoke() or passing it to another query
// must work (and must not touch freed memory; run under ASan to check).
// Covers inline functions with and without closures, named UDF and builtin
// function references, nested closures, partial application, function items
// stored in prolog variables, and function items created by eval.

#include <iostream>
#include <string>
#include <vector>

#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/item_sequence.h>
#include <zorba/iterator.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/static_context.h>
#include <zorba/dynamic_context.h>
#include <zorba/store_manager.h>
#include <zorba/vector_item_sequence.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;

#define CHECK_EQ(actual, expected)                                       \
  do {                                                                   \
    std::string const lActual = (actual);                                \
    std::string const lExpected = (expected);                            \
    if (lActual != lExpected) {                                          \
      std::cerr << __FILE__ << ":" << __LINE__ << ": expected \""        \
                << lExpected << "\", got \"" << lActual << "\""          \
                << std::endl;                                            \
      ++theFailures;                                                     \
    }                                                                    \
  } while (0)

#define CHECK(cond)                                                      \
  do {                                                                   \
    if (!(cond)) {                                                       \
      std::cerr << __FILE__ << ":" << __LINE__ << ": check failed: "     \
                << #cond << std::endl;                                   \
      ++theFailures;                                                     \
    }                                                                    \
  } while (0)

std::string error_string(ZorbaException const& e)
{
  return std::string("error: ") + e.diagnostic().qname().localname() + ": " +
    e.what();
}

std::string to_string(std::vector<Item> const& aItems)
{
  std::string lRes;
  for (Item const& lItem : aItems)
  {
    if (!lRes.empty())
      lRes += " ";
    lRes += lItem.getStringValue().str();
  }
  return lRes;
}

// The public Item API has no function-item test; a function item is the only
// kind that is neither a node, an atomic value nor a JSON item.
bool is_function(Item const& aItem)
{
  return !aItem.isNull() && !aItem.isNode() && !aItem.isAtomic() &&
    !aItem.isJSONItem();
}

std::vector<Item> drain(Iterator_t const& aIter)
{
  std::vector<Item> lItems;
  aIter->open();
  Item lItem;
  while (aIter->next(lItem))
    lItems.push_back(lItem);
  aIter->close();
  return lItems;
}

// Compiles and runs aQuery, binding the external variable $f (if declared in
// aQuery) to aF. The query is closed and destroyed before returning, so the
// returned items outlive it.
std::vector<Item> run(Zorba* z, std::string const& aQuery,
                      std::vector<Item> const& aF = std::vector<Item>())
{
  std::vector<Item> lItems;
  XQuery_t lQuery = z->compileQuery(aQuery);
  if (!aF.empty())
  {
    ItemSequence_t lSeq = new VectorItemSequence(aF);
    Iterator_t lIter = lSeq->getIterator();
    lQuery->getDynamicContext()->setVariable("f", lIter);
  }
  {
    Iterator_t lIter = lQuery->iterator();
    lItems = drain(lIter);
  }
  lQuery->close();
  return lItems;
}

// Runs aQuery and returns the single function item it produces; the query is
// destroyed before this function returns.
Item function_item(Zorba* z, std::string const& aQuery,
                   std::vector<Item> const& aF = std::vector<Item>())
{
  std::vector<Item> lItems = run(z, aQuery, aF);
  CHECK(lItems.size() == 1);
  if (lItems.size() != 1)
    return Item();
  CHECK(is_function(lItems[0]));
  return lItems[0];
}

// Runs aQuery with $f bound to aF and returns the string values of the result.
std::string apply(Zorba* z, std::string const& aQuery,
                  std::vector<Item> const& aF)
{
  try
  {
    return to_string(run(z, "declare variable $f external;\n" + aQuery, aF));
  }
  catch (ZorbaException const& e)
  {
    return error_string(e);
  }
}

std::string apply(Zorba* z, std::string const& aQuery, Item const& aF)
{
  return apply(z, aQuery, std::vector<Item>(1, aF));
}

// Invokes aF through StaticContext::invoke() with one integer argument.
std::string invoke(Zorba* z, Item const& aF, int aArg)
{
  try
  {
    StaticContext_t lSctx = z->createStaticContext();
    std::vector<ItemSequence_t> lArgs;
    lArgs.push_back(
        new SingletonItemSequence(z->getItemFactory()->createInteger(aArg)));
    ItemSequence_t lRes = lSctx->invoke(aF, lArgs);
    if (!lRes)
      return "<null sequence>";
    return to_string(drain(lRes->getIterator()));
  }
  catch (ZorbaException const& e)
  {
    return error_string(e);
  }
}

void test_inline(Zorba* z)
{
  Item lF = function_item(z, "function($x) { $x * 10 }");
  CHECK_EQ(apply(z, "$f(2)", lF), "20");
  CHECK_EQ(invoke(z, lF, 4), "40");
  CHECK_EQ(apply(z, "$f(3)", lF), "30");
}

void test_closure(Zorba* z)
{
  Item lF = function_item(z,
      "let $k := 7 return function($x) { $x + $k }");
  CHECK_EQ(apply(z, "$f(2)", lF), "9");
  CHECK_EQ(invoke(z, lF, 3), "10");
}

// The UDF is never called by the creating query, so its plan is generated
// lazily, by the first invocation in another query.
void test_named_udf(Zorba* z)
{
  Item lF = function_item(z,
      "declare function local:sq($x) { $x * $x };\n"
      "local:sq#1");
  CHECK_EQ(apply(z, "$f(4)", lF), "16");
  CHECK_EQ(invoke(z, lF, 5), "25");
}

void test_builtin_ref(Zorba* z)
{
  Item lF = function_item(z, "fn:upper-case#1");
  CHECK_EQ(apply(z, "$f('abc')", lF), "ABC");
}

// The inner closure is created while the outer function item is invoked by a
// second query; it is then returned from that query and invoked by a third.
void test_nested_closure(Zorba* z)
{
  Item lF = function_item(z,
      "let $a := 1\n"
      "return function($x) {\n"
      "  let $b := $x + $a\n"
      "  return function($y) { $y + $b + $a }\n"
      "}");
  CHECK_EQ(apply(z, "$f(10)(100)", lF), "112");

  Item lInner = function_item(z, "declare variable $f external; $f(10)",
                              std::vector<Item>(1, lF));
  lF = Item();
  CHECK_EQ(apply(z, "$f(100)", lInner), "112");
  CHECK_EQ(invoke(z, lInner, 200), "212");
}

// The creating query returns an inner closure produced by its own call.
void test_returned_inner_closure(Zorba* z)
{
  Item lF = function_item(z,
      "let $g := function($x) {\n"
      "  let $b := $x * 2\n"
      "  return function($y) { $y + $b }\n"
      "}\n"
      "return $g(5)");
  CHECK_EQ(apply(z, "$f(100)", lF), "110");
}

// A closure created by a second query that captures the first query's
// function item outlives both queries. The item is captured through a local
// variable: a function body reads prolog variables from the dynamic context
// of the query that invokes it, not from that of the query that created it.
void test_closure_over_function_item(Zorba* z)
{
  Item lF = function_item(z, "function($x) { $x * 10 }");
  Item lG = function_item(z,
      "declare variable $f external;\n"
      "let $g := $f return function($y) { $g($y) * 2 }",
      std::vector<Item>(1, lF));
  lF = Item();
  CHECK_EQ(apply(z, "$f(3)", lG), "60");
}

void test_partial_application(Zorba* z)
{
  // The partially applied item shares the closure of an item that is a
  // temporary of the creating query.
  Item lF = function_item(z,
      "let $k := 3\n"
      "return (function($a, $b) { $a * $b + $k })(2, ?)");
  CHECK_EQ(apply(z, "$f(5)", lF), "13");
  CHECK_EQ(invoke(z, lF, 6), "15");

  Item lG = function_item(z,
      "let $k := 1\n"
      "let $p := (function($a, $b, $c) { $a + $b + $c + $k })(1, ?, ?)\n"
      "return $p(2, ?)");
  CHECK_EQ(apply(z, "$f(3)", lG), "7");

  Item lH = function_item(z, "fn:concat('a', ?, 'c')");
  CHECK_EQ(apply(z, "$f('b')", lH), "abc");

  // Partial application by a second query of the first query's item.
  Item lI = function_item(z,
      "let $k := 100 return function($a, $b) { $a - $b + $k }");
  Item lJ = function_item(z, "declare variable $f external; $f(?, 1)",
                          std::vector<Item>(1, lI));
  lI = Item();
  CHECK_EQ(apply(z, "$f(10)", lJ), "109");
}

void test_several_items(Zorba* z)
{
  std::vector<Item> lItems = run(z,
      "declare function local:f() { 3 };\n"
      "let $k := 5\n"
      "return (function() { 1 }, function() { $k }, local:f#0)");
  CHECK(lItems.size() == 3);
  CHECK_EQ(apply(z, "for $g in $f return $g()", lItems), "1 5 3");
}

// A function item held in a prolog variable of the creating query.
void test_prolog_variable(Zorba* z)
{
  Item lF = function_item(z,
      "declare variable $g := function($x) { $x + 1 };\n"
      "$g");
  CHECK_EQ(apply(z, "$f(1)", lF), "2");

  // Not escaping: the query must still release everything when closed.
  CHECK_EQ(to_string(run(z,
      "declare variable $g := function($x) { $x + 1 };\n"
      "$g(41)")), "42");
}

// Each iteration re-runs eval, which recompiles; the function items created
// by the earlier iterations must survive that.
void test_eval(Zorba* z)
{
  std::vector<Item> lItems = run(z,
      "import module namespace refl = 'http://zorba.io/modules/reflection';\n"
      "for $i in 1 to 3\n"
      "return refl:eval(concat('function($x) { $x + ', $i, ' }'))");
  CHECK(lItems.size() == 3);
  CHECK_EQ(apply(z, "for $g in $f return $g(10)", lItems), "11 12 13");
}

// A function item that is never used again is simply released.
void test_drop_without_use(Zorba* z)
{
  Item lF = function_item(z,
      "let $k := 2 return function($x) { $x * $k }");
  lF = Item();
  Item lG = function_item(z, "declare function local:f() { 3 };\n"
                             "local:f#0");
  (void)lG;
}

typedef void (*test_fn)(Zorba*);

struct test_case
{
  char const* name;
  test_fn fn;
};

} // namespace


int
function_item_lifetime(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  test_case const lTests[] = {
    { "test_inline", test_inline },
    { "test_closure", test_closure },
    { "test_named_udf", test_named_udf },
    { "test_builtin_ref", test_builtin_ref },
    { "test_nested_closure", test_nested_closure },
    { "test_returned_inner_closure", test_returned_inner_closure },
    { "test_closure_over_function_item", test_closure_over_function_item },
    { "test_partial_application", test_partial_application },
    { "test_several_items", test_several_items },
    { "test_prolog_variable", test_prolog_variable },
    { "test_eval", test_eval },
    { "test_drop_without_use", test_drop_without_use },
  };

  for (test_case const& lTest : lTests)
  {
    std::cout << lTest.name << std::endl;
    try
    {
      lTest.fn(lZorba);
    }
    catch (ZorbaException const& e)
    {
      std::cerr << lTest.name << ": unexpected exception: " << e << std::endl;
      ++theFailures;
    }
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
