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

// Schema imports of queries compiled on several threads against shared
// static contexts. Each thread imports a schema of its own (a namespace per
// thread, served by a URL resolver) in queries compiled against:
// - a context with no schema, on which no query was compiled before the
//   threads start: the first imports create the schema of the context's type
//   manager at once;
// - a context whose loaded prolog imported a schema: the queries import
//   their schema next to it and use both.
// The imports of a query are added to the schema of the context it is
// compiled against (StaticContext::validate() relies on it), so the threads'
// schemas end up side by side in both contexts.

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

const char NS_PREFIX[] = "urn:zorba:test:mt-schema-import:";

std::string ns(std::string const& aName)
{
  return NS_PREFIX + aName;
}

// A schema whose type Small allows the integers up to aMax, and an element v
// of that type.
std::string schema(std::string const& aName, unsigned aMax)
{
  return
    "<?xml version='1.0' encoding='UTF-8'?>\n"
    "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
    "  targetNamespace='" + ns(aName) + "'\n"
    "  xmlns='" + ns(aName) + "' elementFormDefault='qualified'>\n"
    "  <xs:simpleType name='Small'>\n"
    "    <xs:restriction base='xs:integer'>\n"
    "      <xs:maxInclusive value='" + std::to_string(aMax) + "'/>\n"
    "    </xs:restriction>\n"
    "  </xs:simpleType>\n"
    "  <xs:element name='v' type='Small'/>\n"
    "</xs:schema>\n";
}

// The namespace of thread t is "t<t>", whose Small allows up to 10 * (t + 1);
// "base" allows up to 1000.
unsigned limit(unsigned aThread)
{
  return 10 * (aThread + 1);
}

void releaseStream(std::istream* aStream)
{
  std::unique_ptr<std::istream> lOwned(aStream);
}

class SchemaResolver : public URLResolver
{
public:
  Resource* resolveURL(String const& aUrl, EntityData const* aEntityData) override
  {
    std::string const lUrl = aUrl.str();
    if (aEntityData->getKind() != EntityData::SCHEMA ||
        lUrl.compare(0, sizeof(NS_PREFIX) - 1, NS_PREFIX) != 0)
      return nullptr;

    std::string const lName = lUrl.substr(sizeof(NS_PREFIX) - 1);
    unsigned lMax = 1000;
    if (lName != "base")
      lMax = limit(static_cast<unsigned>(std::stoul(lName.substr(1))));
    return StreamResource::create(
      std::make_unique<std::istringstream>(schema(lName, lMax)).release(),
      &releaseStream);
  }
};

std::string own_import(unsigned aThread)
{
  return "import schema namespace s = '" + ns("t" + std::to_string(aThread)) +
         "';\n";
}

// Uses the thread's schema: a cast at its limit, one above it, and validation.
std::string own_checks(unsigned aThread)
{
  std::string const lMax = std::to_string(limit(aThread));
  std::string const lOver = std::to_string(limit(aThread) + 1);
  return
    "string('" + lMax + "' castable as s:Small),\n"
    "string('" + lOver + "' castable as s:Small),\n"
    "string(data(validate { <s:v>" + lMax + "</s:v> }) instance of s:Small)";
}

} // namespace


int
mt_schema_import(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    SchemaResolver lResolver;

    StaticContext_t lPlain = lZorba->createStaticContext();
    lPlain->registerURLResolver(&lResolver);

    StaticContext_t lWithSchema = lZorba->createStaticContext();
    lWithSchema->registerURLResolver(&lResolver);
    Zorba_CompilerHints_t lHints;
    lWithSchema->loadProlog(
      "import schema namespace b = '" + ns("base") + "';\n", lHints);

    run_threads([&](unsigned t)
    {
      std::string const lPlainQuery = own_import(t) +
        "string-join((" + own_checks(t) + "), ' ')";
      std::string const lWithSchemaQuery = own_import(t) +
        "string-join((" + own_checks(t) + ",\n"
        "string('1000' castable as b:Small),\n"
        "string('1001' castable as b:Small)), ' ')";

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("plain context", t,
                        run(lZorba, lPlain, lPlainQuery), "true false true");

        lChecker.expect("context with a schema", t,
                        run(lZorba, lWithSchema, lWithSchemaQuery),
                        "true false true true false");
      }
    });
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
