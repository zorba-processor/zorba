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

// Collects errors instead of throwing; warnings are dropped.
class QuietHandler : public zorba::DiagnosticHandler
{
public:
  void error(zorba::ZorbaException const& e) override { check_internal(e); }
  void warning(zorba::XQueryWarning const&) override {}
};

// Denies every URI the query tries to resolve (module and schema imports,
// fn:doc, ...): a fuzzer must not reach the network or the filesystem.
class DenyAllMapper : public zorba::URIMapper
{
public:
  void mapURI(const zorba::String, zorba::EntityData const*,
              std::vector<zorba::String>& oUris) override
  {
    oUris.push_back(zorba::URIMapper::DENY_ACCESS);
  }
};

struct Engine
{
  void* store = nullptr;
  zorba::Zorba* zorba = nullptr;
  QuietHandler handler;
  DenyAllMapper mapper;

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

} // namespace zorba_fuzz

#endif /* ZORBA_TEST_FUZZ_COMMON_H */
