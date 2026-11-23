/*
 * Copyright 2006-2026 zorba.io
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

// A child (StaticContext::createChildContext()) of a static context whose
// prolog was loaded with StaticContext::loadProlog() sees the variables of
// that prolog, initialized, as it sees its functions: a query compiled against
// the child gets the same values as one compiled against the parent.

#include <iostream>
#include <string>

#include <zorba/dynamic_context.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
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

const char PROLOG[] =
  "declare variable $a := 'a';\n"
  "declare variable $b := concat($a, 'b');\n"
  "declare variable $n := count((1, 2, 3));\n"
  "declare variable $e external;\n"
  "declare function local:f() { concat($b, '-', $n) };\n";

std::string run(XQuery_t const& aQuery)
{
  Iterator_t lIter = aQuery->iterator();
  lIter->open();
  std::string lRes;
  Item lItem;
  bool lFirst = true;
  while (lIter->next(lItem))
  {
    if (!lFirst)
      lRes += " ";
    lRes += lItem.getStringValue().str();
    lFirst = false;
  }
  lIter->close();
  return lRes;
}

// Runs aQuery compiled against aSctx, binding $e (declared external by the
// prolog) to "e"; returns the string values of the result, or
// "error: <code>: <message>".
std::string query(Zorba* z, StaticContext_t const& aSctx,
                  std::string const& aQuery)
{
  try
  {
    XQuery_t lQuery = z->compileQuery(aQuery, aSctx);
    lQuery->getDynamicContext()->setVariable(
        "e", z->getItemFactory()->createString("e"));
    return run(lQuery);
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.diagnostic().qname().localname() +
      ": " + e.what();
  }
}

StaticContext_t load(Zorba* z)
{
  StaticContext_t lSctx = z->createStaticContext();
  Zorba_CompilerHints_t lHints;
  lSctx->loadProlog(PROLOG, lHints);
  return lSctx;
}

// The values of the prolog, read directly, through a prolog function, and
// next to variables of the query itself (whose dynamic-context slots must not
// be those of the prolog variables).
void check_prolog_values(Zorba* z, StaticContext_t const& aSctx)
{
  CHECK_EQ(query(z, aSctx, "$a"), "a");
  CHECK_EQ(query(z, aSctx, "$b, $n"), "ab 3");
  CHECK_EQ(query(z, aSctx, "local:f()"), "ab-3");
  CHECK_EQ(query(z, aSctx, "$e"), "e");
  CHECK_EQ(query(z, aSctx,
                 "let $p := 'p' let $q := 'q' "
                 "return concat($a, $b, $p, $q, $n, $e)"),
           "aabpq3e");
  CHECK_EQ(query(z, aSctx,
                 "declare variable $own := 'own'; "
                 "for $i in 1 to 2 return concat($own, $i, $b)"),
           "own1ab own2ab");
}

void test_parent(Zorba* z)
{
  check_prolog_values(z, load(z));
}

void test_child(Zorba* z)
{
  StaticContext_t lParent = load(z);
  check_prolog_values(z, lParent->createChildContext());
}

void test_grandchild(Zorba* z)
{
  StaticContext_t lParent = load(z);
  check_prolog_values(
      z, lParent->createChildContext()->createChildContext());
}

// The child keeps the prolog of its parent when the application drops the
// parent.
void test_child_outlives_parent(Zorba* z)
{
  StaticContext_t lChild = load(z)->createChildContext();
  check_prolog_values(z, lChild);
}

// The parent loads its prolog after the child was created.
void test_prolog_loaded_after_child(Zorba* z)
{
  StaticContext_t lParent = z->createStaticContext();
  StaticContext_t lChild = lParent->createChildContext();
  Zorba_CompilerHints_t lHints;
  lParent->loadProlog(PROLOG, lHints);
  check_prolog_values(z, lChild);
}

// A child that loads a prolog of its own sees the variables of both prologs,
// and so do its children.
void test_child_with_own_prolog(Zorba* z)
{
  StaticContext_t lParent = load(z);
  StaticContext_t lChild = lParent->createChildContext();
  Zorba_CompilerHints_t lHints;
  lChild->loadProlog(
      "declare variable $c := concat($b, 'c');\n"
      "declare function local:g() { concat($c, '-', local:f()) };\n",
      lHints);
  check_prolog_values(z, lChild);
  CHECK_EQ(query(z, lChild, "$c, local:g()"), "abc abc-ab-3");
  StaticContext_t lGrandchild = lChild->createChildContext();
  check_prolog_values(z, lGrandchild);
  CHECK_EQ(query(z, lGrandchild, "$c, local:g()"), "abc abc-ab-3");
  // The parent does not see the prolog of its child.
  CHECK_EQ(query(z, lParent, "$a"), "a");
}

} // namespace


int
load_prolog_child_context(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    std::cout << "test_parent" << std::endl;
    test_parent(lZorba);
    std::cout << "test_child" << std::endl;
    test_child(lZorba);
    std::cout << "test_grandchild" << std::endl;
    test_grandchild(lZorba);
    std::cout << "test_child_outlives_parent" << std::endl;
    test_child_outlives_parent(lZorba);
    std::cout << "test_prolog_loaded_after_child" << std::endl;
    test_prolog_loaded_after_child(lZorba);
    std::cout << "test_child_with_own_prolog" << std::endl;
    test_child_with_own_prolog(lZorba);
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
