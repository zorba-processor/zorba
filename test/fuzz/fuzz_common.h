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

// Shared setup for the libFuzzer targets in this directory.
//
// Each target initializes one Zorba instance in LLVMFuzzerInitialize and
// reuses it for every input. Errors that Zorba reports for malformed input
// (static/dynamic errors, parse errors) are the expected outcome and are
// swallowed. Internal failures are turned into crashes so the fuzzer records
// them:
//   - ZXQP0002 (ZORBA_ASSERT failed)
//   - ZXQP0003 (internal error, which is also what an escaped
//     std::exception becomes at the API boundary)

#ifndef ZORBA_TEST_FUZZ_COMMON_H
#define ZORBA_TEST_FUZZ_COMMON_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <zorba/diagnostic_handler.h>
#include <zorba/dynamic_context.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/diagnostic_list.h>
#include <zorba/options.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/uri_resolvers.h>
#include <zorba/xquery.h>
#include <zorba/xquery_exception.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

namespace zorba_fuzz {

// Abort on internal errors so libFuzzer reports them; ignore everything else.
inline void check_internal(zorba::ZorbaException const& e)
{
  zorba::Diagnostic const& d = e.diagnostic();
  if (d == zorba::zerr::ZXQP0002_ASSERT_FAILED ||
      d == zorba::zerr::ZXQP0003_INTERNAL_ERROR)
  {
    std::fprintf(stderr, "zorba internal error: %s: %s (%s:%lu)\n",
                 d.qname().localname(), e.what(), e.raise_file(),
                 static_cast<unsigned long>(e.raise_line()));
    std::abort();
  }
}

// Zorba's API takes UTF-8 text; invalid UTF-8 is rejected at the API
// boundary with an internal error (ZXQP0003) that says nothing about the
// engine. Such inputs are skipped. Accepts the UTF-8 of RFC 3629.
inline bool is_valid_utf8(const uint8_t* p, size_t n)
{
  size_t i = 0;
  while (i < n)
  {
    uint8_t c = p[i];
    size_t len;
    uint32_t cp;
    if (c < 0x80) { ++i; continue; }
    else if ((c & 0xE0) == 0xC0) { len = 2; cp = c & 0x1F; }
    else if ((c & 0xF0) == 0xE0) { len = 3; cp = c & 0x0F; }
    else if ((c & 0xF8) == 0xF0) { len = 4; cp = c & 0x07; }
    else return false;
    if (i + len > n)
      return false;
    for (size_t k = 1; k < len; ++k)
    {
      if ((p[i + k] & 0xC0) != 0x80)
        return false;
      cp = (cp << 6) | (p[i + k] & 0x3F);
    }
    if ((len == 2 && cp < 0x80) || (len == 3 && cp < 0x800) ||
        (len == 4 && cp < 0x10000) || cp > 0x10FFFF ||
        (cp >= 0xD800 && cp <= 0xDFFF))
      return false;
    i += len;
  }
  return true;
}

// Collects errors instead of throwing; warnings are dropped.
class QuietHandler : public zorba::DiagnosticHandler
{
public:
  void error(zorba::ZorbaException const& e) override { check_internal(e); }
  void warning(zorba::XQueryWarning const&) override {}
};

// Denies file: URIs (schema and module imports "at" a local path, fn:doc,
// ...): a fuzzer must not read the local filesystem. http(s) URIs are left
// alone so that Zorba's builtin modules can still be imported; the fuzz build
// has no curl, so nothing can actually be fetched over the network.
class DenyFileMapper : public zorba::URIMapper
{
public:
  void mapURI(const zorba::String aUri, zorba::EntityData const*,
              std::vector<zorba::String>& oUris) override
  {
    if (aUri.compare(0, 5, "file:") == 0 || aUri.compare(0, 1, "/") == 0)
      oUris.push_back(zorba::URIMapper::DENY_ACCESS);
  }
};

struct Engine
{
  void* store = nullptr;
  zorba::Zorba* zorba = nullptr;
  QuietHandler handler;
  DenyFileMapper mapper;

  void init()
  {
    store = zorba::StoreManager::getStore();
    zorba = zorba::Zorba::getInstance(store);
  }
};

inline Engine& engine()
{
  // Leaked on purpose: shutting Zorba down from a static destructor after
  // libFuzzer exits is not worth the ordering problems.
  static Engine* e = new Engine();
  return *e;
}

// Compile (not execute) the input as a main module.
inline void compile(const uint8_t* data, size_t size, bool jsoniq)
{
  if (!is_valid_utf8(data, size))
    return;
  Engine& eng = engine();
  std::string text(reinterpret_cast<const char*>(data), size);
  try
  {
    zorba::StaticContext_t sctx = eng.zorba->createStaticContext(&eng.handler);
    sctx->registerURIMapper(&eng.mapper);
    if (jsoniq)
      sctx->setJSONiqVersion(zorba::jsoniq_version_1_0);

    Zorba_CompilerHints_t hints;
    zorba::XQuery_t query = eng.zorba->createQuery(&eng.handler);
    query->compile(text, sctx, hints);
    query->close();
  }
  catch (zorba::ZorbaException const& e)
  {
    check_internal(e);
  }
}

// A query compiled once, with one external xs:string variable $input, that
// is re-executed for every fuzz input. Used to reach the XML and JSON parsers
// through fn:parse-xml / jn:parse-json, which is the same path that
// XmlDataManager::parseXML and JsonDataManager::parseJSON take.
class ParseQuery
{
public:
  void init(const char* text)
  {
    Engine& eng = engine();
    zorba::StaticContext_t sctx = eng.zorba->createStaticContext(&eng.handler);
    sctx->registerURIMapper(&eng.mapper);
    Zorba_CompilerHints_t hints;
    theQuery = eng.zorba->createQuery();
    theQuery->compile(text, sctx, hints);  // throws: a harness bug
  }

  void run(const uint8_t* data, size_t size)
  {
    if (!is_valid_utf8(data, size))
      return;
    Engine& eng = engine();
    try
    {
      zorba::Item input = eng.zorba->getItemFactory()->createString(
          std::string(reinterpret_cast<const char*>(data), size));
      theQuery->getDynamicContext()->setVariable("input", input);
      zorba::Iterator_t it = theQuery->iterator();
      it->open();
      zorba::Item item;
      while (it->next(item))
      {
      }
      it->close();
    }
    catch (zorba::ZorbaException const& e)
    {
      check_internal(e);
    }
  }

private:
  zorba::XQuery_t theQuery;
};

} // namespace zorba_fuzz

#endif /* ZORBA_TEST_FUZZ_COMMON_H */
