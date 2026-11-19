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

// Tests for XML trees as deep as memory allows: what walks over a tree
// recursively either works or raises zerr:ZXQP0072 when the machine stack
// runs out, and nothing crashes, at every optimization level. Every case runs
// in its own process (see CMakeLists.txt). The documents are generated here
// rather than stored as (huge) files.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
# include <pthread.h>
# include <sys/mman.h>
# include <unistd.h>
# define ZORBA_TEST_THREADS 1
#endif

#include <zorba/config.h>
#include <zorba/options.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/xquery_exception.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

const uint64_t LEVELS = 10000;
const uint64_t DEEP = 100000;

const char STACK_CODE[] = "ZXQP0072";

int theFailures = 0;
Zorba* theZorba = nullptr;

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
  unsigned line = 0;
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    return os << "ok: " << o.value.substr(0, 60);
  return os << o.code << " (line " << o.line << "): "
            << o.message.substr(0, 300);
}

void caught(ZorbaException const& e, Outcome& o)
{
  o.code = e.diagnostic().qname().localname();
  o.message = e.what();
  if (XQueryException const* xe = dynamic_cast<XQueryException const*>(&e))
    o.line = xe->source_line();
}

XQuery_t compile(const std::string& aQuery, Zorba_opt_level_t aLevel)
{
  StaticContext_t sctx = theZorba->createStaticContext();
  sctx->setSecurityPolicy(SecurityPolicy::unrestricted());
  Zorba_CompilerHints_t lHints;
  lHints.opt_level = aLevel;
  return theZorba->compileQuery(aQuery, sctx, lHints);
}

void execute(const XQuery_t& q, Outcome& o)
{
  try
  {
    Zorba_SerializerOptions_t lOptions;
    lOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
    std::ostringstream lOut;
    q->execute(lOut, &lOptions);
    o.value = lOut.str();
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    caught(e, o);
  }
}

Outcome run(const std::string& aQuery, Zorba_opt_level_t aLevel)
{
  Outcome o;
  try
  {
    execute(compile(aQuery, aLevel), o);
  }
  catch (ZorbaException const& e)
  {
    caught(e, o);
  }
  return o;
}

// An operation on the chain of nested <a> elements in $d, and on its twin in
// $e for comparisons. The chain's length is an external variable, so that
// nothing is evaluated at compile time. The expected value is for n levels.
struct Operation
{
  const char* name;
  const char* body;
  std::function<std::string(uint64_t)> expected;
};

std::string number(uint64_t n) { return std::to_string(n); }

const std::vector<Operation> theOperations = {
  { "load", "count($d//a)", number },
  { "construct", "count(<w>{ $d/a }</w>//a)", number },
  { "copy", "count(document { $d }//a)", number },
  { "update",
    "count((copy $c := $d\n"
    "       modify insert node <b/> into ($c//a)[last()]\n"
    "       return $c)//b)",
    [](uint64_t) { return std::string("1"); } },
  { "validate", "count((validate lax { $d })//a)", number },
  { "deep-equal", "deep-equal($d, $e)",
    [](uint64_t) { return std::string("true"); } },
  { "serialize", "string-length(serialize($d))",
    [](uint64_t n) { return number(7 * n + 1); } },
  { "string", "string-length(string($d))",
    [](uint64_t) { return std::string("1"); } },
  { "result", "$d",
    [](uint64_t n) {
      std::string r;
      r.reserve(7 * n + 1);
      for (uint64_t i = 0; i < n; ++i) r += "<a>";
      r += "x";
      for (uint64_t i = 0; i < n; ++i) r += "</a>";
      return r; } },
};

const std::string CHAIN =
  "declare function local:chain($n as xs:integer) as xs:string\n"
  "{\n"
  "  string-join((for $i in 1 to $n return \"<a>\", \"x\",\n"
  "               for $i in 1 to $n return \"</a>\"))\n"
  "};\n";

std::string operation_query(const Operation& op, uint64_t n)
{
  return
    "declare variable $n as xs:integer external := " + number(n) + ";\n" +
    CHAIN +
    "let $d := parse-xml(local:chain($n))\n"
    "let $e := parse-xml(local:chain($n))\n"
    "return\n" + op.body;
}

const Zorba_opt_level_t theLevels[] = {
  ZORBA_OPT_LEVEL_O0, ZORBA_OPT_LEVEL_O1, ZORBA_OPT_LEVEL_O2
};

// What a deep tree may do: give the right value, or run out of stack with
// the stack error, which says how deep the node was and, for an expression
// of the query, where that expression is.
bool expected_outcome(const Operation& op, uint64_t n, const Outcome& o)
{
  if (o.ok)
  {
    std::string lValue = o.value;
    while (!lValue.empty() && (lValue.back() == '\n' || lValue.back() == ' '))
      lValue.pop_back();
    if (lValue == op.expected(n))
      return true;
    std::cerr << "  " << op.name << ": wrong value " << o << std::endl;
    return false;
  }
  if (o.code != STACK_CODE)
  {
    std::cerr << "  " << op.name << ": expected a value or " << STACK_CODE
              << ", got " << o << std::endl;
    return false;
  }
  if (o.message.find("levels deep") == std::string::npos)
  {
    std::cerr << "  " << op.name << ": the message does not give the depth: "
              << o << std::endl;
    return false;
  }
  if (std::strcmp(op.name, "result") != 0 && o.line == 0)
  {
    std::cerr << "  " << op.name << ": no query location: " << o << std::endl;
    return false;
  }
  return true;
}

// Each operation, at each optimization level: the expected outcome, and the
// same one (value or error) at every level.
void check_operation(const Operation& op, uint64_t n)
{
  bool lFirst = true;
  bool lFirstOk = false;
  for (Zorba_opt_level_t lLevel : theLevels)
  {
    Outcome const o = run(operation_query(op, n), lLevel);
    std::cout << "  " << n << " levels, O" << lLevel << ", " << op.name
              << ": " << o << std::endl;
    CHECK(expected_outcome(op, n, o));
    if (lFirst)
      lFirstOk = o.ok;
    else
      CHECK(o.ok == lFirstOk);
    lFirst = false;
  }
}

///////////////////////////////////////////////////////////////////////////////

// Ten thousand levels: within the stack of every supported platform for
// every operation.
void test_levels()
{
  for (Operation const& op : theOperations)
  {
    for (Zorba_opt_level_t lLevel : theLevels)
    {
      Outcome const o = run(operation_query(op, LEVELS), lLevel);
      std::cout << "  " << LEVELS << " levels, O" << lLevel << ", "
                << op.name << ": " << o << std::endl;
      CHECK(o.ok);
      CHECK(expected_outcome(op, LEVELS, o));
    }
  }
}

const Operation& operation(const char* aName)
{
  for (Operation const& op : theOperations)
    if (std::strcmp(op.name, aName) == 0)
      return op;
  std::cerr << "no operation " << aName << std::endl;
  std::abort();
}

void test_deep_load() { check_operation(operation("load"), DEEP); }
void test_deep_construct() { check_operation(operation("construct"), DEEP); }
void test_deep_copy() { check_operation(operation("copy"), DEEP); }
void test_deep_update() { check_operation(operation("update"), DEEP); }
void test_deep_validate() { check_operation(operation("validate"), DEEP); }
void test_deep_equal() { check_operation(operation("deep-equal"), DEEP); }
void test_deep_serialize() { check_operation(operation("serialize"), DEEP); }
void test_deep_string() { check_operation(operation("string"), DEEP); }
void test_deep_result() { check_operation(operation("result"), DEEP); }

#ifdef ZORBA_TEST_THREADS
void* thread_main(void* aFn);

// Runs a compiled query on a thread with a stack of aSize bytes.
void execute_on_thread(const XQuery_t& q, Outcome& o, size_t aSize)
{
  std::function<void()> lFn = [&] { execute(q, o); };
  pthread_attr_t lAttr;
  pthread_attr_init(&lAttr);
  pthread_attr_setstacksize(&lAttr, aSize);
  pthread_t lThread;
  CHECK(pthread_create(&lThread, &lAttr, thread_main, &lFn) == 0);
  pthread_join(lThread, nullptr);
  pthread_attr_destroy(&lAttr);
}

// A document loaded with fn:doc: the stack error names it. Serializing runs
// on a 1 MiB stack, so that it surely runs out.
void test_doc_uri()
{
  char lPath[] = "/tmp/xml_depth_limit_XXXXXX";
  int const lFd = mkstemp(lPath);
  CHECK(lFd >= 0);
  if (lFd < 0)
    return;
  close(lFd);
  {
    std::ofstream lFile(lPath);
    for (uint64_t i = 0; i < DEEP; ++i) lFile << "<a>";
    lFile << "x";
    for (uint64_t i = 0; i < DEEP; ++i) lFile << "</a>";
  }
  std::string const lUri = std::string("file://") + lPath;
  for (Zorba_opt_level_t lLevel : theLevels)
  {
    Outcome o = run("count(doc('" + lUri + "')//a)", lLevel);
    std::cout << "  O" << lLevel << ", load: " << o << std::endl;
    CHECK(o.ok);
    o = Outcome();
    try
    {
      execute_on_thread(
        compile("string-length(serialize(doc('" + lUri + "')))", lLevel),
        o, 1024 * 1024);
    }
    catch (ZorbaException const& e)
    {
      caught(e, o);
    }
    std::cout << "  O" << lLevel << ", serialize: " << o << std::endl;
    CHECK(o.code == STACK_CODE);
    CHECK(o.line == 1);
    CHECK(o.message.find(lPath) != std::string::npos);
  }
  std::remove(lPath);
}

void* thread_main(void* aFn)
{
  (*static_cast<std::function<void()>*>(aFn))();
  return nullptr;
}

// On a secondary thread with a small stack (512 KiB is the default on
// macOS), every operation on ten thousand levels works or raises the stack
// error. The query is compiled before, on this thread.
void test_second_thread()
{
  for (size_t const lSize : { size_t(256 * 1024), size_t(512 * 1024) })
    for (Operation const& op : theOperations)
    {
      Outcome o;
      try
      {
        execute_on_thread(
          compile(operation_query(op, LEVELS), ZORBA_OPT_LEVEL_O1), o, lSize);
      }
      catch (ZorbaException const& e)
      {
        caught(e, o);
      }
      std::cout << "  " << lSize / 1024 << " KiB, " << op.name << ": " << o
                << std::endl;
      CHECK(expected_outcome(op, LEVELS, o));
    }
}

// Each level declares a namespace: freeing the tree releases a chain of
// namespace contexts as long as the tree is deep, here on a 512 KiB stack.
void test_namespaces()
{
  std::string const lQuery =
    "declare variable $n as xs:integer external := " + number(LEVELS) + ";\n"
    "let $d := parse-xml(string-join((\n"
    "  for $i in 1 to $n return concat('<a xmlns:p', $i, '=\"u\">'),\n"
    "  for $i in 1 to $n return '</a>')))\n"
    "return count($d//a)";
  for (Zorba_opt_level_t lLevel : theLevels)
  {
    Outcome o;
    try
    {
      execute_on_thread(compile(lQuery, lLevel), o, 512 * 1024);
    }
    catch (ZorbaException const& e)
    {
      caught(e, o);
    }
    std::cout << "  O" << lLevel << ": " << o << std::endl;
    CHECK(o.ok);
  }
}

const unsigned char PAINT = 0xA5;
const size_t MEASURE_STACK_SIZE = 256u * 1024 * 1024;

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

// Runs aFn on a thread with a large stack painted with a pattern and returns
// the bytes of stack it used.
size_t measure_stack(std::function<void()> aFn)
{
  void* lMem = mmap(nullptr, MEASURE_STACK_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANON, -1, 0);
  if (lMem == MAP_FAILED)
    return 0;
  std::memset(lMem, PAINT, MEASURE_STACK_SIZE);
  pthread_attr_t lAttr;
  pthread_attr_init(&lAttr);
  pthread_attr_setstack(&lAttr, lMem, MEASURE_STACK_SIZE);
  pthread_t lThread;
  size_t lUsed = 0;
  if (pthread_create(&lThread, &lAttr, thread_main, &aFn) == 0)
  {
    pthread_join(lThread, nullptr);
    lUsed = MEASURE_STACK_SIZE -
            untouched(static_cast<unsigned char*>(lMem), MEASURE_STACK_SIZE);
  }
  pthread_attr_destroy(&lAttr);
  munmap(lMem, MEASURE_STACK_SIZE);
  return lUsed;
}

// The stack each level takes, for each operation, on a thread with stack to
// spare.
void test_measure()
{
  uint64_t const lSmall = 1000;
  for (Operation const& op : theOperations)
  {
    Outcome lSmallOutcome, lLargeOutcome;
    XQuery_t const lSmallQuery =
      compile(operation_query(op, lSmall), ZORBA_OPT_LEVEL_O1);
    XQuery_t const lLargeQuery =
      compile(operation_query(op, LEVELS), ZORBA_OPT_LEVEL_O1);
    size_t const lSmallUsed = measure_stack([&] {
      execute(lSmallQuery, lSmallOutcome); });
    size_t const lLargeUsed = measure_stack([&] {
      execute(lLargeQuery, lLargeOutcome); });
    std::cout << "  stack: " << op.name << ": " << lLargeUsed / 1024
              << " KiB at " << LEVELS << " levels, "
              << (lLargeUsed > lSmallUsed
                  ? (lLargeUsed - lSmallUsed) / (LEVELS - lSmall) : 0)
              << " bytes per level" << std::endl;
    CHECK(lLargeOutcome.ok);
  }
}
#else
void test_doc_uri() { }
void test_second_thread() { }
void test_namespaces() { }
void test_measure() { }
#endif

struct Case
{
  const char* name;
  void (*fn)();
};

const Case theCases[] = {
  { "levels", test_levels },
  { "deep_load", test_deep_load },
  { "deep_construct", test_deep_construct },
  { "deep_copy", test_deep_copy },
  { "deep_update", test_deep_update },
  { "deep_validate", test_deep_validate },
  { "deep_equal", test_deep_equal },
  { "deep_serialize", test_deep_serialize },
  { "deep_string", test_deep_string },
  { "deep_result", test_deep_result },
  { "doc_uri", test_doc_uri },
  { "second_thread", test_second_thread },
  { "namespaces", test_namespaces },
  { "measure", test_measure },
};

} // anonymous namespace


int
xml_depth_limit(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "usage: xml_depth_limit <case>" << std::endl;
    return 2;
  }
  Case const* lCase = nullptr;
  for (Case const& c : theCases)
    if (std::strcmp(c.name, argv[1]) == 0)
      lCase = &c;
  if (!lCase)
  {
    std::cerr << "xml_depth_limit: unknown case " << argv[1] << std::endl;
    return 2;
  }

  void* lStore = StoreManager::getStore();
  theZorba = Zorba::getInstance(lStore);
  theZorba->setSecurityPolicy(SecurityPolicy::unrestricted());

  try
  {
    std::cout << argv[1] << std::endl;
    lCase->fn();
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "unexpected exception: " << e << std::endl;
    ++theFailures;
  }
  catch (std::exception const& e)
  {
    std::cerr << "unexpected exception: " << e.what() << std::endl;
    ++theFailures;
  }

  theZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
