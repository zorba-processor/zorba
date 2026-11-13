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

// A window reads its domain as it goes only when no side effect can run in the
// query that runs it. The body of a function loaded with
// StaticContext::loadProlog(), or of a function item made by another query, is
// compiled by that prolog or query, not by the query that runs it.

#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
# include <unistd.h>
#endif

#include <zorba/dynamic_context.h>
#include <zorba/item_sequence.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/vector_item_sequence.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;

void fail(const std::string& aWhat)
{
  std::cerr << "FAILED: " << aWhat << std::endl;
  ++theFailures;
}

void write_file(const std::string& aPath, const std::string& aText)
{
  std::ofstream out(aPath, std::ios::binary | std::ios::trunc);
  out << aText;
}

struct Read
{
  int theCount = 0;
  bool theSawChange = false;
  std::string theError;
};

// Reads the first window, changes the file, and reads the rest.
Read read_with_change(const std::function<XQuery_t()>& aCompile,
                      const std::string& aPath)
{
  Read r;
  write_file(aPath, "a");
  try
  {
    XQuery_t q = aCompile();
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
    {
      if (r.theCount == 0)
        write_file(aPath, "b");
      if (item.getStringValue().str()[0] == 'b')
        r.theSawChange = true;
      ++r.theCount;
    }
    it->close();
  }
  catch (ZorbaException const& e)
  {
    r.theError = e.diagnostic().qname().localname();
  }
  return r;
}

const int theWindows = 5;

const char* const theKinds[] = { "sliding", "tumbling" };

// One window per item of the domain, each item read from the file. The function
// is recursive, so that the optimizer does not inline it into the calling query.
std::string windows_function(const std::string& aPrefix, const std::string& aKind)
{
  std::string const name = aPrefix + ":" + aKind;
  return
    "declare %an:nondeterministic\n"
    "function " + name + "($p as xs:string, $n as xs:integer) {\n"
    "  if ($n gt 0) then " + name + "($p, $n - 1)\n"
    "  else\n"
    "    for " + aKind + " window $w in\n"
    "      (for $i in 1 to " + std::to_string(theWindows) + "\n"
    "       return concat(file:read-text($p), string($i)))\n"
    "      start when true\n"
    "      only end when true\n"
    "    return $w\n"
    "};\n";
}

// A statement that never writes, but lets the query run a side effect.
std::string never_writes(const std::string& aPath)
{
  return
    "declare variable $w external := false();\n"
    "if ($w) then file:write-text(\"" + aPath + "\", \"\"); else ();\n";
}

const char* const theImports =
  "import module namespace file = \"http://expath.org/ns/file\";\n"
  "declare namespace an = \"http://zorba.io/annotations\";\n";

void check(const std::string& aWhat, const Read& r, bool aLazy)
{
  if (!r.theError.empty() || r.theCount != theWindows || r.theSawChange != aLazy)
    fail(aWhat + ": got " + std::to_string(r.theCount) + " windows, error '" +
         r.theError + "', change seen: " + (r.theSawChange ? "yes" : "no"));
}

// The prolog has no query body and cannot run a side effect.
void check_prolog_function(Zorba* z, const std::string& aPath)
{
  StaticContext_t sctx = z->createStaticContext();
  sctx->setSecurityPolicy(SecurityPolicy::unrestricted());
  Zorba_CompilerHints_t hints;
  sctx->loadProlog(std::string(theImports) +
                   "declare namespace p = \"urn:window-running-query\";\n" +
                   windows_function("p", "sliding") +
                   windows_function("p", "tumbling"), hints);

  auto run = [&](const std::string& aBody) {
    return read_with_change(
      [&]() {
        return z->compileQuery(
          std::string(theImports) +
          "declare namespace p = \"urn:window-running-query\";\n" + aBody, sctx);
      },
      aPath);
  };

  for (std::string const kind : theKinds)
  {
    std::string const call = "p:" + kind + "(\"" + aPath + "\", 2)";
    check(kind + " function of a loaded prolog, query without side effects",
          run(call), true);
    check(kind + " function of a loaded prolog, query with a side effect",
          run(never_writes(aPath) + call), false);
  }
}

// The query that makes the function item cannot run a side effect.
void check_foreign_function_item(Zorba* z, const std::string& aPath,
                                 const std::string& aKind)
{
  StaticContext_t sctx = z->createStaticContext();
  sctx->setSecurityPolicy(SecurityPolicy::unrestricted());

  std::vector<Item> items;
  {
    XQuery_t q = z->compileQuery(std::string(theImports) +
                                 windows_function("local", aKind) +
                                 "local:" + aKind + "#2", sctx);
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
      items.push_back(item);
    it->close();
    q->close();
  }
  if (items.size() != 1)
  {
    fail(aKind + " function item: expected one item");
    return;
  }

  auto run = [&](const std::string& aBody) {
    return read_with_change(
      [&]() {
        XQuery_t q = z->compileQuery(
          std::string(theImports) + "declare variable $g external;\n" + aBody,
          sctx);
        ItemSequence_t seq = new VectorItemSequence(items);
        q->getDynamicContext()->setVariable("g", seq->getIterator());
        return q;
      },
      aPath);
  };

  check(aKind + " function item of another query, query without side effects",
        run("$g(\"" + aPath + "\", 2)"), true);
  check(aKind + " function item of another query, query with a side effect",
        run(never_writes(aPath) + "$g(\"" + aPath + "\", 2)"), false);
}

} // namespace


int
window_running_query(int, char*[])
{
  void* store = StoreManager::getStore();
  Zorba* z = Zorba::getInstance(store);

  std::string path;
  try
  {
    path = (std::filesystem::temp_directory_path() /
            ("window_running_query_" +
#if defined(__unix__) || defined(__APPLE__)
             std::to_string(getpid()) +
#endif
             ".txt")).string();
    check_prolog_function(z, path);
    for (std::string const kind : theKinds)
      check_foreign_function_item(z, path, kind);
  }
  catch (std::exception const& e)
  {
    fail(std::string("exception: ") + e.what());
  }
  std::remove(path.c_str());

  z->shutdown();
  StoreManager::shutdownStore(store);
  return theFailures == 0 ? 0 : 1;
}
