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

// StaticContext::invoke() on a function of a module imported with
// StaticContext::loadProlog(): the module's global variables (prolog
// "declare variable $x := ...") must be initialized before the function runs,
// exactly as for a query compiled against the same static context.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/item_sequence.h>
#include <zorba/iterator.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/security.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
namespace sfs = std::filesystem;

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

const char MODULE_NS[] = "http://zorba.io/test/invoke-module-globals";

const char MODULE_SRC[] =
  "module namespace m = 'http://zorba.io/test/invoke-module-globals';\n"
  "declare variable $m:base := 'hello';\n"
  "declare variable $m:derived := concat($m:base, '-world');\n"
  "declare variable $m:count := count((1, 2, 3));\n"
  "declare function m:base() { $m:base };\n"
  "declare function m:derived() { $m:derived };\n"
  "declare function m:with-arg($s) { concat($m:derived, ':', $s) };\n"
  "declare function m:count() { $m:count };\n";

struct ModuleFile
{
  sfs::path dir;
  sfs::path file;

  ModuleFile()
  {
    dir = sfs::temp_directory_path() /
      ("zorba-invoke-module-globals-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(dir);
    file = dir / "m.xq";
    std::ofstream(file) << MODULE_SRC;
  }

  ~ModuleFile()
  {
    std::error_code ec;
    sfs::remove_all(dir, ec);
  }

  std::string uri() const
  {
    std::string p = file.generic_string();
    if (p.empty() || p[0] != '/')
      p = "/" + p;
    return "file://" + p;
  }
};

std::string serialize(ItemSequence_t const& aSeq)
{
  if (!aSeq)
    return "<null sequence>";
  std::string lRes;
  Iterator_t lIter = aSeq->getIterator();
  lIter->open();
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

// Calls m:<aLocal>(aArgs...) through StaticContext::invoke() and returns the
// string values of the result, or "error: <code>" if it raised an error.
std::string invoke(Zorba* z, StaticContext_t const& aSctx,
                   char const* aLocal, std::vector<std::string> const& aArgs)
{
  try
  {
    ItemFactory* lFac = z->getItemFactory();
    Item lQName = lFac->createQName(MODULE_NS, aLocal);
    std::vector<ItemSequence_t> lArgs;
    for (std::string const& a : aArgs)
      lArgs.push_back(new SingletonItemSequence(lFac->createString(a)));
    return serialize(aSctx->invoke(lQName, lArgs));
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.diagnostic().qname().localname() +
      ": " + e.what();
  }
}

// Same calls through a regular query compiled against aSctx: the reference
// behavior invoke() must match.
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
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.diagnostic().qname().localname() +
      ": " + e.what();
  }
}

StaticContext_t load(Zorba* z, ModuleFile const& aModule)
{
  StaticContext_t lSctx = z->createStaticContext();
  Zorba_CompilerHints_t lHints;
  lSctx->loadProlog(
      std::string("import module namespace m = '") + MODULE_NS + "' at '" +
        aModule.uri() + "';\n",
      lHints);
  return lSctx;
}

void test_query_reference(Zorba* z, ModuleFile const& aModule)
{
  StaticContext_t lSctx = load(z, aModule);
  CHECK_EQ(query(z, lSctx, "m:base()"), "hello");
  CHECK_EQ(query(z, lSctx, "m:derived()"), "hello-world");
  CHECK_EQ(query(z, lSctx, "m:with-arg('x')"), "hello-world:x");
}

void test_invoke(Zorba* z, ModuleFile const& aModule)
{
  StaticContext_t lSctx = load(z, aModule);
  CHECK_EQ(invoke(z, lSctx, "base", {}), "hello");
  CHECK_EQ(invoke(z, lSctx, "derived", {}), "hello-world");
  CHECK_EQ(invoke(z, lSctx, "with-arg", {"x"}), "hello-world:x");
  CHECK_EQ(invoke(z, lSctx, "count", {}), "3");
}

// Repeated invocations on the same static context, and a query in between,
// keep seeing the initialized globals.
void test_invoke_repeated(Zorba* z, ModuleFile const& aModule)
{
  StaticContext_t lSctx = load(z, aModule);
  CHECK_EQ(invoke(z, lSctx, "with-arg", {"a"}), "hello-world:a");
  CHECK_EQ(query(z, lSctx, "m:derived()"), "hello-world");
  CHECK_EQ(invoke(z, lSctx, "with-arg", {"b"}), "hello-world:b");
  CHECK_EQ(invoke(z, lSctx, "base", {}), "hello");
}

} // namespace


int
invoke_module_globals(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  // The module is imported from a temporary file.
  lZorba->setSecurityPolicy(SecurityPolicy::secure().allow(Capability::FileRead));

  try
  {
    ModuleFile lModule;
    std::cout << "test_query_reference" << std::endl;
    test_query_reference(lZorba, lModule);
    std::cout << "test_invoke" << std::endl;
    test_invoke(lZorba, lModule);
    std::cout << "test_invoke_repeated" << std::endl;
    test_invoke_repeated(lZorba, lModule);
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
