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

// Tests for the limit on the nesting depth of JSON values (zerr:ZXQP0071).
// Every case runs in its own process (see CMakeLists.txt), so that each one
// reports its own result. The documents are generated here rather than
// stored as (huge) files.

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
# include <pthread.h>
# include <sys/mman.h>
# define ZORBA_TEST_THREADS 1
#endif

#include <zorba/config.h>
#include <zorba/item.h>
#include <zorba/iterator.h>
#include <zorba/options.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

// The limit, stated independently of the implementation.
const uint64_t LIMIT = 1500;

// Far beyond the limit.
const uint64_t DEEP = 200000;

const char LIMIT_CODE[] = "ZXQP0071";

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
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    return os << "ok: " << o.value.substr(0, 60);
  return os << o.code << ": " << o.message.substr(0, 300);
}

XQuery_t compile(const std::string& aQuery)
{
  StaticContext_t sctx = theZorba->createStaticContext();
  sctx->setSecurityPolicy(SecurityPolicy::unrestricted());
  return theZorba->compileQuery(aQuery, sctx);
}

void execute(const XQuery_t& q, Outcome& o, Zorba_serialization_method_t aMethod)
{
  try
  {
    Zorba_SerializerOptions_t lOptions;
    lOptions.ser_method = aMethod;
    std::ostringstream lOut;
    q->execute(lOut, &lOptions);
    o.value = lOut.str();
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
}

Outcome run(const std::string& aQuery,
            Zorba_serialization_method_t aMethod =
              ZORBA_SERIALIZATION_METHOD_JSON)
{
  Outcome o;
  try
  {
    XQuery_t q = compile(aQuery);
    execute(q, o, aMethod);
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
  return o;
}

std::string repeat(const std::string& s, uint64_t n)
{
  std::string r;
  r.reserve(s.size() * n);
  for (uint64_t i = 0; i < n; ++i)
    r += s;
  return r;
}

std::string squeeze(std::string s)
{
  s.erase(std::remove_if(s.begin(), s.end(),
                         [](char c) { return c == ' ' || c == '\n'; }),
          s.end());
  return s;
}

enum Shape { ARRAYS, OBJECTS, MIXED };

std::string json(Shape aShape, uint64_t n)
{
  std::string lOpen, lClose;
  lOpen.reserve(n * 5);
  lClose.reserve(n);
  for (uint64_t i = 0; i < n; ++i)
  {
    bool const lArray = aShape == ARRAYS || (aShape == MIXED && i % 2 == 0);
    lOpen += lArray ? "[" : "{\"a\":";
    lClose += lArray ? "]" : "}";
  }
  std::reverse(lClose.begin(), lClose.end());
  return lOpen + "1" + lClose;
}

const char IMPORT_JN[] =
  "import module namespace jn = \"http://jsoniq.org/functions\";\n";

// An expression that parses a JSON value nested n levels deep.
std::string value(Shape aShape, uint64_t n)
{
  return "jn:parse-json('" + json(aShape, n) + "')";
}

std::string with_jn(const std::string& aQuery)
{
  return IMPORT_JN + aQuery;
}

bool module_missing(const Outcome& o)
{
  if (o.code == "XQST0059")
  {
    std::cout << "  module not in this build: " << o << std::endl;
    return true;
  }
  return false;
}

// The value was refused: the limit error, with the limit in its message.
bool refused(const Outcome& o)
{
  if (!o.ok && o.code == LIMIT_CODE &&
      o.message.find(std::to_string(LIMIT)) != std::string::npos)
    return true;
  std::cerr << "  expected the " << LIMIT_CODE << " limit error, got " << o
            << std::endl;
  return false;
}

bool worked(const Outcome& o)
{
  if (o.ok)
    return true;
  std::cerr << "  expected success, got " << o << std::endl;
  return false;
}

void report(const char* aWhat, const Outcome& o)
{
  std::cout << "  " << aWhat << ": " << o << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// Parsing

void test_parse(Shape aShape)
{
  std::string const lAtLimit = json(aShape, LIMIT);

  Outcome o = run(with_jn(value(aShape, LIMIT)));
  CHECK(worked(o));
  CHECK(squeeze(o.value) == lAtLimit);

  o = run(with_jn(value(aShape, LIMIT + 1)));
  CHECK(refused(o));
  // The error says what to do and points into the document.
  CHECK(o.message.find("nest") != std::string::npos);

  CHECK(refused(run(with_jn(value(aShape, DEEP)))));

  // Siblings at the limit are fine; depth is what counts.
  std::string const lSibling = json(aShape, LIMIT - 1);
  o = run(with_jn("jn:parse-json('[" + lSibling + ",2,[]," + lSibling + "]')"));
  CHECK(worked(o));

  // One value's depth does not carry over to the next one.
  o = run(with_jn("count(jn:parse-json('" + lAtLimit + " " + lAtLimit + "', "
                  "{ \"jsoniq-multiple-top-level-items\" : true }))"));
  CHECK(worked(o));
  CHECK(squeeze(o.value) == "2");

  // Still working after a refusal, in the same process.
  CHECK(worked(run(with_jn("jn:parse-json('[[1]]')"))));
}

void test_parse_arrays() { test_parse(ARRAYS); }
void test_parse_objects() { test_parse(OBJECTS); }
void test_parse_mixed() { test_parse(MIXED); }

// A document that stops before the closing brackets is refused for its depth
// as soon as the parser gets there.
void test_parse_truncated()
{
  CHECK(refused(run(with_jn("jn:parse-json('" + repeat("[", DEEP) + "')"))));
  CHECK(refused(run(with_jn("jn:parse-json('" +
                            repeat("{\"a\":", DEEP) + "')"))));
}

///////////////////////////////////////////////////////////////////////////////
// Constructors

// One array or object constructor around aContent.
std::string wrap(Shape aShape, uint64_t aLevel, const std::string& aContent)
{
  bool const lArray = aShape == ARRAYS || (aShape == MIXED && aLevel % 2 == 0);
  return lArray ? "[ " + aContent + " ]"
                : "{ \"a\" : " + aContent + " }";
}

void test_constructors(Shape aShape)
{
  // Constructors around a value: exactly at the limit, then one more.
  Outcome o = run(with_jn(wrap(aShape, 0, wrap(aShape, 1,
                                               value(aShape, LIMIT - 2)))));
  CHECK(worked(o));
  CHECK(refused(run(with_jn(wrap(aShape, 0, wrap(aShape, 1, wrap(aShape, 2,
                                               value(aShape, LIMIT - 2))))))));

  // A loop that wraps a variable in constructors, 100 levels per iteration.
  std::string lWrapped = "$x";
  for (uint64_t i = 0; i < 100; ++i)
    lWrapped = wrap(aShape, i, lWrapped);

  std::string const lLoop = std::string(IMPORT_JN) +
    "variable $x := 1;\n"
    "variable $i := 0;\n"
    "while ($i lt $n) {\n"
    "  $x := " + lWrapped + ";\n"
    "  $i := $i + 1;\n"
    "}\n"
    "$x";
  auto lLoopOf = [&](uint64_t aIterations) {
    std::string lQuery = lLoop;
    lQuery.replace(lQuery.find("$n"), 2, std::to_string(aIterations));
    return lQuery;
  };
  o = run(lLoopOf(LIMIT / 100));
  CHECK(worked(o));
  CHECK(refused(run(lLoopOf(LIMIT / 100 + 1))));
  CHECK(refused(run(lLoopOf(DEEP / 100))));
}

void test_ctor_arrays() { test_constructors(ARRAYS); }
void test_ctor_objects() { test_constructors(OBJECTS); }
void test_ctor_mixed() { test_constructors(MIXED); }

// A recursive function that adds 100 levels per call.
void test_ctor_function()
{
  std::string lBody = "local:f($n - 1)";
  for (uint64_t i = 0; i < 100; ++i)
    lBody = wrap(MIXED, i, lBody);
  std::string const lFunction =
    "declare function local:f($n) {\n"
    "  if ($n le 0) then 1 else " + lBody + "\n"
    "};\n";
  CHECK(worked(run(lFunction + "local:f(" + std::to_string(LIMIT / 100) + ")")));
  CHECK(refused(run(lFunction + "local:f(" +
                    std::to_string(LIMIT / 100 + 1) + ")")));
}

// Object union and accumulation.
void test_ctor_union()
{
  auto lUnion = [](uint64_t n) {
    return with_jn("{| { \"a\" : " + value(ARRAYS, n) + " } |}");
  };
  CHECK(worked(run(lUnion(LIMIT - 1))));
  CHECK(refused(run(lUnion(LIMIT))));

  // A repeated key makes an array: object, then array, then the value.
  auto lAccumulate = [](uint64_t n) {
    return with_jn("{[ { \"a\" : " + value(OBJECTS, n) + " }, { \"a\" : 2 } ]}");
  };
  CHECK(worked(run(lAccumulate(LIMIT - 2))));
  CHECK(refused(run(lAccumulate(LIMIT - 1))));
}

///////////////////////////////////////////////////////////////////////////////
// Everything that walks over a value: works at the limit, and a value far
// beyond the limit never gets to it.

const Shape SHAPES[] = { ARRAYS, OBJECTS, MIXED };

void test_serialize()
{
  for (Shape s : SHAPES)
  {
    CHECK(worked(run(with_jn(value(s, LIMIT)),
                     ZORBA_SERIALIZATION_METHOD_JSON_XML_HYBRID)));
    report("fn:serialize", run(with_jn("serialize(" + value(s, LIMIT) + ")")));
    CHECK(refused(run(with_jn(value(s, DEEP)),
                      ZORBA_SERIALIZATION_METHOD_JSON_XML_HYBRID)));
    CHECK(refused(run(with_jn("serialize(" + value(s, DEEP) + ")"))));
  }
}

void test_deep_equal()
{
  for (Shape s : SHAPES)
  {
    std::string const lLimit = value(s, LIMIT);
    Outcome o = run(with_jn("deep-equal(" + lLimit + ", " + lLimit + ")"));
    CHECK(worked(o));
    CHECK(squeeze(o.value) == "true");
    std::string const lDeep = value(s, DEEP);
    CHECK(refused(run(with_jn("deep-equal(" + lDeep + ", " + lDeep + ")"))));
  }
}

void test_flatten()
{
  Outcome o = run(with_jn("count(jn:flatten(" + value(ARRAYS, LIMIT) + "))"));
  CHECK(worked(o));
  CHECK(squeeze(o.value) == "1");
  o = run(with_jn("count(jn:flatten(" + value(MIXED, LIMIT) + "))"));
  CHECK(worked(o));
  CHECK(refused(run(with_jn("count(jn:flatten(" + value(ARRAYS, DEEP) + "))"))));
}

void test_copy()
{
  for (Shape s : SHAPES)
  {
    // The constructor copies what it gets from a variable.
    std::string const lCopy =
      "let $x := " + value(s, LIMIT - 1) + " return [ $x, $x ]";
    CHECK(worked(run(with_jn(lCopy))));
    CHECK(refused(run(with_jn("let $x := " + value(s, DEEP) +
                              " return [ $x, $x ]"))));
  }
  CHECK(worked(run(with_jn("let $x := " + value(OBJECTS, LIMIT) +
                           " return {| $x |}"))));
}

void test_atomize()
{
  for (Shape s : SHAPES)
  {
    Outcome o = run(with_jn("data(" + value(s, LIMIT) + ")"));
    CHECK(!o.ok && o.code == "JNTY0004");
    report("data", o);
    o = run(with_jn("string(" + value(s, LIMIT) + ")"));
    report("string", o);
    CHECK(refused(run(with_jn("data(" + value(s, DEEP) + ")"))));
    CHECK(refused(run(with_jn("string(" + value(s, DEEP) + ")"))));
  }
}

// The arguments of a cached function are hashed.
void test_hash()
{
  std::string const lFunction =
    "declare namespace an = \"http://zorba.io/annotations\";\n"
    "declare %an:strictlydeterministic function local:f($x) { count($x) };\n";
  for (Shape s : SHAPES)
  {
    Outcome const o = run(std::string(IMPORT_JN) + lFunction +
                          "count((local:f(" + value(s, LIMIT) + "), local:f(" +
                          value(s, LIMIT) + ")))");
    CHECK(worked(o));
    CHECK(squeeze(o.value) == "2");
    CHECK(refused(run(std::string(IMPORT_JN) + lFunction +
                      "local:f(" + value(s, DEEP) + ")")));
  }
}

void test_item_size()
{
  std::string const lImport =
    "import module namespace item = \"http://zorba.io/modules/item\";\n";
  for (Shape s : SHAPES)
  {
    Outcome o = run(std::string(IMPORT_JN) + lImport +
                    "item:size(" + value(s, LIMIT) + ") gt 0");
    if (module_missing(o))
      return;
    CHECK(worked(o));
    CHECK(refused(run(std::string(IMPORT_JN) + lImport +
                      "item:size(" + value(s, DEEP) + ") gt 0")));
  }
}

std::string collection_prolog()
{
  return std::string(IMPORT_JN) +
    "import module namespace ddl = "
    "\"http://zorba.io/modules/store/dynamic/collections/ddl\";\n"
    "import module namespace dml = "
    "\"http://zorba.io/modules/store/dynamic/collections/dml\";\n";
}

void test_collection()
{
  int lCount = 0;
  for (Shape s : SHAPES)
  {
    std::string const lName = "c" + std::to_string(++lCount);
    std::string const lDeclare =
      "declare variable $c := xs:QName(\"" + lName + "\");\n";
    auto lQuery = [&](uint64_t n) {
      return collection_prolog() + lDeclare +
        "ddl:create($c, " + value(s, n) + ");\n"
        "variable $n := count(dml:collection($c));\n"
        "dml:delete-last($c, 1);\n"
        "ddl:delete($c);\n"
        "$n";
    };
    Outcome o = run(lQuery(LIMIT));
    if (module_missing(o))
      return;
    CHECK(worked(o));
    CHECK(squeeze(o.value) == "1");
    CHECK(refused(run(lQuery(DEEP))));
    // The refused value left no collection behind.
    Outcome const a = run(collection_prolog() + lDeclare +
                          "ddl:is-available-collection($c)");
    CHECK(worked(a));
    CHECK(squeeze(a.value) == "false");
  }
}

// XML conversions: the XML has limits of its own (zerr:ZSTR0030), a clean
// error like any other here.
void test_json_xml()
{
  std::string const lImport =
    "import module namespace jx = \"http://zorba.io/modules/json-xml\";\n";
  char const* const lFormats[] = { "Snelson", "JsonML-array", "JsonML-object" };
  for (char const* lFormat : lFormats)
    for (Shape s : SHAPES)
    {
      std::string const lCall = "jx:json-to-xml(";
      std::string const lOptions =
        ", { \"json-format\" : \"" + std::string(lFormat) + "\" })";
      Outcome o = run(std::string(IMPORT_JN) + lImport +
                      "count(" + lCall + value(s, LIMIT) + lOptions + ")");
      if (module_missing(o))
        return;
      report(lFormat, o);
      CHECK(refused(run(std::string(IMPORT_JN) + lImport +
                        "count(" + lCall + value(s, DEEP) + lOptions + ")")));
    }

  // Well-formed JsonML goes all the way down.
  auto lConvert = [&](const std::string& aJson, const char* aFormat) {
    return run(std::string(IMPORT_JN) + lImport +
               "count(jx:json-to-xml(jn:parse-json('" + aJson + "'), "
               "{ \"json-format\" : \"" + aFormat + "\" }))");
  };
  auto lJsonMLArray = [](uint64_t n) {
    return repeat("[\"a\",", n) + "\"t\"" + repeat("]", n);
  };
  auto lJsonMLObject = [](uint64_t n) {
    return repeat("{\"tagName\":\"a\",\"childNodes\":[", n) + "\"t\"" +
      repeat("]}", n);
  };
  report("JsonML-array at the limit",
         lConvert(lJsonMLArray(LIMIT), "JsonML-array"));
  report("JsonML-object at the limit",
         lConvert(lJsonMLObject(LIMIT / 2), "JsonML-object"));
  CHECK(refused(lConvert(lJsonMLArray(DEEP), "JsonML-array")));
  CHECK(refused(lConvert(lJsonMLObject(DEEP), "JsonML-object")));
}

void test_roundtrip()
{
  for (Shape s : SHAPES)
  {
    Outcome o = run(with_jn("jn:encode-for-roundtrip(" + value(s, LIMIT) + ")"));
    CHECK(worked(o));
    CHECK(squeeze(o.value) == json(s, LIMIT));
    o = run(with_jn("jn:decode-from-roundtrip(" + value(s, LIMIT) + ")"));
    CHECK(worked(o));
    CHECK(squeeze(o.value) == json(s, LIMIT));
    CHECK(refused(run(with_jn("jn:encode-for-roundtrip(" +
                              value(s, DEEP) + ")"))));
    CHECK(refused(run(with_jn("jn:decode-from-roundtrip(" +
                              value(s, DEEP) + ")"))));
  }
}

// JSound: a schema whose array types nest inline, and a value to match.
void test_jsound()
{
  auto lQuery = [](uint64_t n) {
    std::string const lType =
      repeat("{\"$kind\":\"array\",\"$content\":[", n) + "\"integer\"" +
      repeat("]}", n);
    return with_jn(
      "import module namespace jsd = \"http://jsound.io/modules/jsound\";\n"
      "jsd:validate(jn:parse-json('{\"$namespace\":\"http://example.com/s\","
      "\"$types\":[{\"$name\":\"t\",\"$kind\":\"array\",\"$content\":[" + lType +
      "]}]}'), \"t\", " + value(ARRAYS, n + 1) + ")");
  };
  // Each level of the type is an object and an array.
  Outcome o = run(lQuery(LIMIT / 2 - 2));
  if (module_missing(o))
    return;
  report("jsound at the limit", o);
  CHECK(refused(run(lQuery(DEEP / 2))));
}

// The C++ API takes the same path through the loader.
void test_api()
{
  // Item accessors hand out a value's members; walking down and dropping
  // the value afterwards must not use more stack than the value's depth.
  XQuery_t q = compile(with_jn(value(MIXED, LIMIT)));
  Iterator_t it = q->iterator();
  it->open();
  Item lItem;
  CHECK(it->next(lItem));
  CHECK(lItem.isJSONItem());
  it->close();
  lItem = Item();
}

///////////////////////////////////////////////////////////////////////////////
// Updates

void test_update()
{
  // Into the root of a value: exactly at the limit, then one more.
  CHECK(worked(run(with_jn(
    "variable $a := [ 1 ];\n"
    "replace value of json $a(1) with " + value(ARRAYS, LIMIT - 1) + ";\n"
    "$a"))));
  CHECK(refused(run(with_jn(
    "variable $a := [ 1 ];\n"
    "replace value of json $a(1) with " + value(ARRAYS, LIMIT) + ";\n"
    "$a"))));
  CHECK(worked(run(with_jn(
    "variable $a := [ ];\n"
    "insert json " + value(ARRAYS, LIMIT - 1) + " into $a at position 1;\n"
    "$a"))));
  CHECK(refused(run(with_jn(
    "variable $a := [ ];\n"
    "insert json " + value(ARRAYS, LIMIT) + " into $a at position 1;\n"
    "$a"))));
  CHECK(worked(run(with_jn(
    "variable $o := {||};\n"
    "insert json { \"a\" : " + value(ARRAYS, LIMIT - 1) + " } into $o;\n"
    "$o"))));
  CHECK(refused(run(with_jn(
    "variable $o := {||};\n"
    "insert json { \"a\" : " + value(ARRAYS, LIMIT) + " } into $o;\n"
    "$o"))));
  CHECK(worked(run(with_jn(
    "variable $o := { \"a\" : 1 };\n"
    "replace value of json $o(\"a\") with " + value(ARRAYS, LIMIT - 1) + ";\n"
    "$o"))));
  CHECK(refused(run(with_jn(
    "variable $o := { \"a\" : 1 };\n"
    "replace value of json $o(\"a\") with " + value(ARRAYS, LIMIT) + ";\n"
    "$o"))));

  // A deep member that was removed no longer counts.
  CHECK(worked(run(with_jn(
    "variable $a := [ " + value(ARRAYS, LIMIT - 1) + " ];\n"
    "delete json $a(1);\n"
    "[ [ $a ] ]"))));
}

// Updates that go down one level per step: each update only sees the member
// it changes, so these values are refused by what walks over them, and
// nothing may crash, including dropping them.
std::string update_chain(uint64_t n)
{
  return
    "variable $root := [ ];\n"
    "variable $cur := $root;\n"
    "variable $i := 0;\n"
    "while ($i lt " + std::to_string(n) + ") {\n"
    "  insert json [ [ ] ] into $cur at position 1;\n"
    "  $cur := $cur[[1]][[1]];\n"
    "  $i := $i + 1;\n"
    "}\n";
}

const uint64_t CHAIN = DEEP / 2;

void not_crashed(const char* aWhat, const Outcome& o)
{
  report(aWhat, o);
  CHECK(o.ok || o.code == LIMIT_CODE);
}

void test_chain_serialize()
{
  not_crashed("serialize", run(with_jn(update_chain(CHAIN) + "$root")));
}

void test_chain_copy()
{
  not_crashed("copy", run(with_jn(update_chain(CHAIN) +
                                  "let $x := $root return jn:size([ $x ])")));
}

void test_chain_deep_equal()
{
  // Two different values, so that the comparison has to go down.
  not_crashed("deep-equal", run(with_jn(
    update_chain(CHAIN) +
    "variable $root2 := [ ];\n"
    "variable $cur2 := $root2;\n"
    "$i := 0;\n"
    "while ($i lt " + std::to_string(CHAIN) + ") {\n"
    "  insert json [ [ ] ] into $cur2 at position 1;\n"
    "  $cur2 := $cur2[[1]][[1]];\n"
    "  $i := $i + 1;\n"
    "}\n"
    "deep-equal($root, $root2)")));
}

void test_chain_destroy()
{
  not_crashed("destroy", run(with_jn(update_chain(CHAIN) + "1")));
}

void test_chain_walkers()
{
  not_crashed("encode", run(with_jn(update_chain(CHAIN) +
                                    "jn:encode-for-roundtrip($root)")));
  not_crashed("decode", run(with_jn(update_chain(CHAIN) +
                                    "jn:decode-from-roundtrip($root)")));
  not_crashed("flatten", run(with_jn(update_chain(CHAIN) +
                                     "count(jn:flatten($root))")));
  Outcome o = run(
    "import module namespace item = \"http://zorba.io/modules/item\";\n" +
    with_jn(update_chain(CHAIN) + "item:size($root) gt 0"));
  if (!module_missing(o))
    not_crashed("item:size", o);
  std::string const lFunction =
    "declare namespace an = \"http://zorba.io/annotations\";\n"
    "declare %an:strictlydeterministic function local:f($x) { count($x) };\n";
  not_crashed("hash", run(std::string(IMPORT_JN) + lFunction +
                          update_chain(CHAIN) + "local:f($root)"));
  for (char const* lFormat : { "Snelson", "JsonML-array", "JsonML-object" })
  {
    o = run(
      "import module namespace jx = \"http://zorba.io/modules/json-xml\";\n" +
      with_jn(update_chain(CHAIN) +
              "count(jx:json-to-xml($root, { \"json-format\" : \"" +
              lFormat + "\" }))"));
    report(lFormat, o);
  }
}

// The same inside a collection, where updates also attach what they insert
// to the collection, and deleting the value detaches all of it.
void test_chain_collection()
{
  std::string const lQuery = collection_prolog() +
    "declare variable $c := xs:QName(\"chain\");\n"
    "ddl:create($c, [ ]);\n"
    "variable $cur := dml:collection($c)[1];\n"
    "variable $i := 0;\n"
    "while ($i lt " + std::to_string(CHAIN) + ") {\n"
    "  insert json [ [ ] ] into $cur at position 1;\n"
    "  $cur := $cur[[1]][[1]];\n"
    "  $i := $i + 1;\n"
    "}\n"
    "$cur := ();\n"
    "dml:delete-last($c, 1);\n"
    "ddl:delete($c);\n"
    "1";
  Outcome const o = run(lQuery);
  if (!module_missing(o))
    not_crashed("collection", o);
}

///////////////////////////////////////////////////////////////////////////////
// Stack

// An operation on the value in $v and, for comparisons, its twin in $w. They
// are external variables so that nothing is evaluated at compile time.
struct Operation
{
  const char* name;
  Shape shape;
  uint64_t levelsUnderLimit;
  const char* imports;
  const char* prolog;
  const char* body;
};

const char COLLECTION_IMPORTS[] =
  "import module namespace ddl = "
  "\"http://zorba.io/modules/store/dynamic/collections/ddl\";\n"
  "import module namespace dml = "
  "\"http://zorba.io/modules/store/dynamic/collections/dml\";\n";

const Operation theOperations[] = {
  { "parse, destroy", MIXED, 0, "", "", "count(jn:parse-json($v))" },
  { "parse, serialize, destroy", MIXED, 0, "", "", "jn:parse-json($v)" },
  { "deep-equal", MIXED, 0, "", "",
    "deep-equal(jn:parse-json($v), jn:parse-json($w))" },
  { "copy", MIXED, 1, "", "",
    "let $x := jn:parse-json($v) return jn:size([ $x, $x ])" },
  { "flatten", ARRAYS, 0, "", "", "count(jn:flatten(jn:parse-json($v)))" },
  { "encode-for-roundtrip", MIXED, 0, "", "",
    "count(jn:encode-for-roundtrip(jn:parse-json($v)))" },
  { "decode-from-roundtrip", MIXED, 0, "", "",
    "count(jn:decode-from-roundtrip(jn:parse-json($v)))" },
  { "hash", ARRAYS, 0,
    "declare namespace an = \"http://zorba.io/annotations\";\n",
    "declare %an:strictlydeterministic function local:f($x) { count($x) };\n",
    "local:f(jn:parse-json($v))" },
  { "collection", MIXED, 0, COLLECTION_IMPORTS, "",
    "ddl:create(xs:QName(\"m\"), jn:parse-json($v));\n"
    "dml:delete-last(xs:QName(\"m\"), 1);\n"
    "ddl:delete(xs:QName(\"m\"));\n"
    "1" },
};

std::string operation_query(const Operation& op, uint64_t n)
{
  std::string const lJson = json(op.shape, n - op.levelsUnderLimit);
  return std::string(IMPORT_JN) + op.imports +
    "declare variable $v external := '" + lJson + "';\n"
    "declare variable $w external := '" + lJson + "';\n" +
    op.prolog + op.body;
}

#ifdef ZORBA_TEST_THREADS
void* thread_main(void* aFn)
{
  (*static_cast<std::function<void()>*>(aFn))();
  return nullptr;
}

const unsigned char PAINT = 0xA5;
const size_t MEASURE_STACK_SIZE = 64u * 1024 * 1024;

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
  memset(lMem, PAINT, MEASURE_STACK_SIZE);
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

// The stack each level of nesting takes, for each operation, on a thread
// with stack to spare: what the choice of the limit rests on.
void test_measure()
{
  uint64_t const lSmall = 100;
  for (Operation const& op : theOperations)
  {
    Outcome lSmallOutcome, lLimitOutcome;
    std::string const lSmallQuery = operation_query(op, lSmall);
    std::string const lLimitQuery = operation_query(op, LIMIT);
    size_t const lSmallUsed = measure_stack([&] {
      lSmallOutcome = run(lSmallQuery); });
    size_t const lLimitUsed = measure_stack([&] {
      lLimitOutcome = run(lLimitQuery); });
    std::cout << "  stack: " << op.name << ": " << lLimitUsed / 1024
              << " KiB at " << LIMIT << " levels, "
              << (lLimitUsed > lSmallUsed
                  ? (lLimitUsed - lSmallUsed) / (LIMIT - lSmall) : 0)
              << " bytes per level (" << lSmallOutcome << " / "
              << lLimitOutcome.ok << ")" << std::endl;
    if (!module_missing(lLimitOutcome))
      CHECK(lLimitOutcome.ok);
  }
}

// On a secondary thread with a small stack (512 KiB is the default on
// macOS), everything at the limit either works or is refused for lack of
// stack, with the same error. The query is compiled before, on this thread:
// what is measured here is running it.
void test_second_thread()
{
  // The smaller first: glibc reuses a cached stack that is large enough.
  for (size_t const lSize : { size_t(256 * 1024), size_t(512 * 1024) })
    for (Operation const& op : theOperations)
    {
      Outcome o;
      try
      {
        XQuery_t q = compile(operation_query(op, LIMIT));
        std::function<void()> lFn = [&] {
          execute(q, o, ZORBA_SERIALIZATION_METHOD_JSON);
        };
        pthread_attr_t lAttr;
        pthread_attr_init(&lAttr);
        pthread_attr_setstacksize(&lAttr, lSize);
        pthread_t lThread;
        CHECK(pthread_create(&lThread, &lAttr, thread_main, &lFn) == 0);
        pthread_join(lThread, nullptr);
        pthread_attr_destroy(&lAttr);
      }
      catch (ZorbaException const& e)
      {
        o.code = e.diagnostic().qname().localname();
        o.message = e.what();
      }
      std::cout << "  " << lSize / 1024 << " KiB, " << op.name << ": " << o
                << std::endl;
      if (!module_missing(o))
        CHECK(o.ok || o.code == LIMIT_CODE);
    }
}
#else
void test_measure() { }
void test_second_thread() { }
#endif

struct Case
{
  const char* name;
  void (*fn)();
};

const Case theCases[] = {
  { "parse_arrays", test_parse_arrays },
  { "parse_objects", test_parse_objects },
  { "parse_mixed", test_parse_mixed },
  { "parse_truncated", test_parse_truncated },
  { "ctor_arrays", test_ctor_arrays },
  { "ctor_objects", test_ctor_objects },
  { "ctor_mixed", test_ctor_mixed },
  { "ctor_function", test_ctor_function },
  { "ctor_union", test_ctor_union },
  { "serialize", test_serialize },
  { "deep_equal", test_deep_equal },
  { "flatten", test_flatten },
  { "copy", test_copy },
  { "atomize", test_atomize },
  { "hash", test_hash },
  { "item_size", test_item_size },
  { "collection", test_collection },
  { "json_xml", test_json_xml },
  { "roundtrip", test_roundtrip },
  { "jsound", test_jsound },
  { "api", test_api },
  { "update", test_update },
  { "chain_serialize", test_chain_serialize },
  { "chain_copy", test_chain_copy },
  { "chain_deep_equal", test_chain_deep_equal },
  { "chain_destroy", test_chain_destroy },
  { "chain_walkers", test_chain_walkers },
  { "chain_collection", test_chain_collection },
  { "measure", test_measure },
  { "second_thread", test_second_thread },
};

} // anonymous namespace


int
json_depth_limit(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "usage: json_depth_limit <case>" << std::endl;
    return 2;
  }
  Case const* lCase = nullptr;
  for (Case const& c : theCases)
    if (std::strcmp(c.name, argv[1]) == 0)
      lCase = &c;
  if (!lCase)
  {
    std::cerr << "json_depth_limit: unknown case " << argv[1] << std::endl;
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
