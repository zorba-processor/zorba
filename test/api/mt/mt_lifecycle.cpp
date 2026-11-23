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

// Queries running while other threads create and drop static contexts,
// collections and indexes. Half of the threads, in every round, create a
// static context, their own collection and an index on it, fill and probe
// them, drop them and release the context. The other half read a collection
// and an index created before the threads start, and list the available
// collections and indexes while the first half adds and removes entries.

#include <memory>
#include <sstream>
#include <string>

#include <zorba/store_manager.h>
#include <zorba/uri_resolvers.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;
const unsigned BUILDERS = THREADS / 2;

const char MODULE_URL[] = "http://zorba-mt.test/lifecycle/store.xqlib";

std::string module_source()
{
  std::string lSrc =
    "module namespace t = 'urn:zorba:test:mt-lifecycle';\n"
    "import module namespace dml =\n"
    "  'http://zorba.io/modules/store/static/collections/dml';\n"
    "declare namespace ann = 'http://zorba.io/annotations';\n"
    "declare collection t:stable as node()*;\n"
    "declare %ann:value-equality %ann:automatic index t:stable-by-g\n"
    "  on nodes dml:collection(xs:QName('t:stable'))\n"
    "  by xs:integer(@g) as xs:integer;\n";
  for (unsigned t = 0; t < BUILDERS; ++t)
  {
    std::string const n = std::to_string(t);
    lSrc +=
      "declare collection t:c" + n + " as node()*;\n"
      "declare %ann:value-equality %ann:automatic index t:i" + n + "\n"
      "  on nodes dml:collection(xs:QName('t:c" + n + "'))\n"
      "  by xs:integer(@g) as xs:integer;\n";
  }
  return lSrc;
}

const char PROLOG[] =
  "import module namespace t = 'urn:zorba:test:mt-lifecycle'\n"
  "  at 'http://zorba-mt.test/lifecycle/store.xqlib';\n"
  "import module namespace ddl =\n"
  "  'http://zorba.io/modules/store/static/collections/ddl';\n"
  "import module namespace dml =\n"
  "  'http://zorba.io/modules/store/static/collections/dml';\n"
  "import module namespace iddl =\n"
  "  'http://zorba.io/modules/store/static/indexes/ddl';\n"
  "import module namespace idml =\n"
  "  'http://zorba.io/modules/store/static/indexes/dml';\n";

const char READ_QUERY[] =
  "string-join((\n"
  "  string(count(dml:collection(xs:QName('t:stable')))),\n"
  "  string(count(idml:probe-index-point-value(xs:QName('t:stable-by-g'), 1))),\n"
  "  string(ddl:available-collections() = xs:QName('t:stable')),\n"
  "  string(iddl:available-indexes() = xs:QName('t:stable-by-g'))), ' ')";

const char READ_EXPECTED[] = "3 2 true true";

void releaseStream(std::istream* aStream)
{
  std::unique_ptr<std::istream> lOwned(aStream);
}

class ModuleResolver : public URLResolver
{
public:
  Resource* resolveURL(String const& aUrl, EntityData const* aEntityData) override
  {
    if (aEntityData->getKind() != EntityData::MODULE || aUrl != MODULE_URL)
      return nullptr;
    return StreamResource::create(
      std::make_unique<std::istringstream>(module_source()).release(),
      &releaseStream);
  }
};

} // namespace


int
mt_lifecycle(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    ModuleResolver lResolver;
    auto lContext = [&]()
    {
      StaticContext_t lSctx = lZorba->createStaticContext();
      lSctx->registerURLResolver(&lResolver);
      Zorba_CompilerHints_t lHints;
      lSctx->loadProlog(PROLOG, lHints);
      return lSctx;
    };
    StaticContext_t lSetup = lContext();

    lZorba->compileQuery("ddl:create(xs:QName('t:stable'))", lSetup)->execute();
    lZorba->compileQuery("iddl:create(xs:QName('t:stable-by-g'))", lSetup)
      ->execute();
    lZorba->compileQuery(
      "dml:insert(xs:QName('t:stable'), (<x g='1'/>, <x g='2'/>, <x g='1'/>))",
      lSetup)->execute();

    run_threads([&](unsigned t)
    {
      try
      {
        if (t < BUILDERS)
        {
          std::string const c = "xs:QName('t:c" + std::to_string(t) + "')";
          std::string const i = "xs:QName('t:i" + std::to_string(t) + "')";
          for (unsigned r = 0; r < ROUNDS; ++r)
          {
            StaticContext_t lOwn = lContext();
            auto lExecute = [&](std::string const& aQuery)
            {
              lZorba->compileQuery(aQuery, lOwn)->execute();
            };
            lExecute("ddl:create(" + c + ")");
            lExecute("iddl:create(" + i + ")");
            lExecute("dml:insert(" + c + ", (<x g='1'/>, <x g='" +
                     std::to_string(r) + "'/>))");
            lChecker.expect("builder", t,
                            run(lZorba, lOwn,
                                "string-join((string(count(dml:collection(" +
                                c + "))), string(count(" +
                                "idml:probe-index-point-value(" + i +
                                ", 1)))), ' ')"),
                            r == 1 ? "2 2" : "2 1");
            lExecute("iddl:delete(" + i + ")");
            lExecute("ddl:delete(" + c + ")");
            lChecker.expect("builder after delete", t,
                            run(lZorba, lOwn,
                                "string-join((string(ddl:is-available-collection(" +
                                c + ")), string(iddl:is-available-index(" + i +
                                "))), ' ')"),
                            "false false");
          }
        }
        else
        {
          StaticContext_t lOwn = lContext();
          for (unsigned r = 0; r < 2 * ROUNDS; ++r)
            lChecker.expect("reader", t, run(lZorba, lOwn, READ_QUERY),
                            READ_EXPECTED);
        }
      }
      catch (ZorbaException const& e)
      {
        lChecker.fail("thread " + std::to_string(t) + ": " + e.what());
      }
    });

    lChecker.expect("final collections", 0,
                    run(lZorba, lSetup,
                        "string-join(for $c in ddl:available-collections()\n"
                        "  return local-name-from-QName($c), ' ')"),
                    "stable");
  }
  catch (ZorbaException const& e)
  {
    lChecker.fail(std::string("unexpected exception: ") + e.what());
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
