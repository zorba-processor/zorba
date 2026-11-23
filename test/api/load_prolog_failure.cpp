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

// After a StaticContext::loadProlog() that fails, the static context and its
// children can still be used: the functions the prolog declared before the
// error are not declared, its other declarations are.

#include <iostream>
#include <string>

#include <zorba/item.h>
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

// The second function refers to an undeclared variable (XPST0008), after the
// namespace, the variable and the first function were declared.
const char FAILING_PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare variable $v := 'v';\n"
  "declare function local:f() { 'f' };\n"
  "declare function local:g() { $undeclared };\n";

std::string error_code(ZorbaException const& e)
{
  return std::string("error: ") + e.diagnostic().qname().localname();
}

// The string values of the result of aQuery compiled against aSctx, or
// "error: <code>".
std::string query(Zorba* z, StaticContext_t const& aSctx,
                  std::string const& aQuery)
{
  try
  {
    XQuery_t lQuery = z->compileQuery(aQuery, aSctx);
    Iterator_t lIter = lQuery->iterator();
    lIter->open();
    std::string lRes;
    Item lItem;
    while (lIter->next(lItem))
    {
      if (!lRes.empty())
        lRes += " ";
      lRes += lItem.getStringValue().str();
    }
    lIter->close();
    return lRes;
  }
  catch (ZorbaException const& e)
  {
    return error_code(e);
  }
}

std::string load(StaticContext_t const& aSctx, std::string const& aProlog)
{
  try
  {
    Zorba_CompilerHints_t lHints;
    aSctx->loadProlog(aProlog, lHints);
    return "loaded";
  }
  catch (ZorbaException const& e)
  {
    return error_code(e);
  }
}

// Queries compiled against a context after the failure: the functions of the
// failed prolog are not declared, its namespace is, and its variable is
// declared without a value.
void check_after_failure(Zorba* z, StaticContext_t const& aSctx)
{
  CHECK_EQ(query(z, aSctx, "local:f()"), "error: XPST0017");
  CHECK_EQ(query(z, aSctx, "local:g()"), "error: XPST0017");
  CHECK_EQ(query(z, aSctx, "namespace-uri(<p:e/>)"), "urn:p");
  CHECK_EQ(query(z, aSctx, "$v"), "error: XPDY0002");
  CHECK_EQ(query(z, aSctx, "declare variable $w := 'w'; $w"), "w");
}

void test_failed_prolog(Zorba* z)
{
  StaticContext_t lSctx = z->createStaticContext();
  CHECK_EQ(load(lSctx, FAILING_PROLOG), "error: XPST0008");
  check_after_failure(z, lSctx);
}

// A child created before the failure, and one created after it, see what the
// context sees.
void test_children(Zorba* z)
{
  StaticContext_t lParent = z->createStaticContext();
  StaticContext_t lEarlyChild = lParent->createChildContext();
  CHECK_EQ(load(lParent, FAILING_PROLOG), "error: XPST0008");
  check_after_failure(z, lEarlyChild);
  check_after_failure(z, lParent->createChildContext());
}

// A child created before the failure outlives the application's handle on
// its parent.
void test_child_outlives_parent(Zorba* z)
{
  StaticContext_t lChild;
  {
    StaticContext_t lParent = z->createStaticContext();
    lChild = lParent->createChildContext();
    CHECK_EQ(load(lParent, FAILING_PROLOG), "error: XPST0008");
  }
  check_after_failure(z, lChild);
}

// A namespace declared in the context before the failed prolog stays.
void test_earlier_declarations_stay(Zorba* z)
{
  StaticContext_t lSctx = z->createStaticContext();
  lSctx->addNamespace("q", "urn:api");
  CHECK_EQ(load(lSctx, FAILING_PROLOG), "error: XPST0008");
  CHECK_EQ(query(z, lSctx, "namespace-uri(<q:e/>)"), "urn:api");
}

} // namespace


int
load_prolog_failure(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    std::cout << "test_failed_prolog" << std::endl;
    test_failed_prolog(lZorba);
    std::cout << "test_children" << std::endl;
    test_children(lZorba);
    std::cout << "test_child_outlives_parent" << std::endl;
    test_child_outlives_parent(lZorba);
    std::cout << "test_earlier_declarations_stay" << std::endl;
    test_earlier_declarations_stay(lZorba);
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
