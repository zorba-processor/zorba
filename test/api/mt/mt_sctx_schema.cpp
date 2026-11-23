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

// The schema introspection functions of the sctx module from several threads
// at once, against a schema imported by the loaded prolog of a shared static
// context, while other threads import new schemas into the same context: each
// import replaces the Xerces XSModel the introspection functions read.

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

const char NS_PREFIX[] = "urn:zorba:test:mt-sctx-schema:";

std::string ns(std::string const& aName)
{
  return NS_PREFIX + aName;
}

std::string shared_schema()
{
  return
    "<?xml version='1.0' encoding='UTF-8'?>\n"
    "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
    "  targetNamespace='" + ns("shared") + "'\n"
    "  xmlns='" + ns("shared") + "' elementFormDefault='qualified'>\n"
    "  <xs:simpleType name='Code'>\n"
    "    <xs:restriction base='xs:string'/>\n"
    "  </xs:simpleType>\n"
    "  <xs:complexType name='Order'>\n"
    "    <xs:sequence><xs:group ref='lines'/></xs:sequence>\n"
    "    <xs:attributeGroup ref='ids'/>\n"
    "  </xs:complexType>\n"
    "  <xs:group name='lines'>\n"
    "    <xs:sequence><xs:element ref='line' maxOccurs='unbounded'/></xs:sequence>\n"
    "  </xs:group>\n"
    "  <xs:attributeGroup name='ids'>\n"
    "    <xs:attribute ref='id'/>\n"
    "  </xs:attributeGroup>\n"
    "  <xs:attribute name='id' type='Code'/>\n"
    "  <xs:attribute name='rev' type='xs:integer'/>\n"
    "  <xs:element name='line' type='xs:string'/>\n"
    "  <xs:element name='order' type='Order'/>\n"
    "</xs:schema>\n";
}

// A schema of its own that a thread imports in each round.
std::string own_schema(std::string const& aName)
{
  return
    "<?xml version='1.0' encoding='UTF-8'?>\n"
    "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
    "  targetNamespace='" + ns(aName) + "'\n"
    "  xmlns='" + ns(aName) + "' elementFormDefault='qualified'>\n"
    "  <xs:simpleType name='Small'>\n"
    "    <xs:restriction base='xs:integer'/>\n"
    "  </xs:simpleType>\n"
    "  <xs:element name='v' type='Small'/>\n"
    "</xs:schema>\n";
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
    std::string const lSchema =
      lName == "shared" ? shared_schema() : own_schema(lName);
    return StreamResource::create(
      std::make_unique<std::istringstream>(lSchema).release(), &releaseStream);
  }
};

// The local names that a sctx function returns in a namespace, sorted.
std::string names(std::string const& aFunction, std::string const& aNs)
{
  return
    "  string-join(for $q in sctx:" + aFunction + "()\n"
    "              where namespace-uri-from-QName($q) eq '" + aNs + "'\n"
    "              order by local-name-from-QName($q)\n"
    "              return local-name-from-QName($q), ',')";
}

std::string introspection(std::string const& aNs)
{
  return
    "string-join((\n" +
    names("in-scope-element-declarations", aNs) + ",\n" +
    names("in-scope-attribute-declarations", aNs) + ",\n" +
    names("in-scope-element-groups", aNs) + ",\n" +
    names("in-scope-attribute-groups", aNs) + ",\n" +
    names("in-scope-schema-types", aNs) + "\n"
    "), ' ')";
}

const char MODULE_IMPORT[] =
  "import module namespace sctx = 'http://zorba.io/modules/sctx';\n";

} // namespace


int
mt_sctx_schema(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    SchemaResolver lResolver;

    StaticContext_t lShared = lZorba->createStaticContext();
    lShared->registerURLResolver(&lResolver);
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(
      "import schema namespace s = '" + ns("shared") + "';\n", lHints);

    std::string const lSharedQuery = MODULE_IMPORT + introspection(ns("shared"));
    std::string const lSharedWant = "line,order id,rev lines ids Code,Order";

    // Even threads read the shared schema's components; odd threads import a
    // new schema in each round and read its components.
    run_threads([&](unsigned t)
    {
      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        if (t % 2 == 0)
        {
          lChecker.expect("shared schema", t,
                          run(lZorba, lShared, lSharedQuery), lSharedWant);
          continue;
        }
        std::string const lOwn =
          ns("t" + std::to_string(t) + "r" + std::to_string(r));
        lChecker.expect("own schema", t,
                        run(lZorba, lShared,
                            MODULE_IMPORT +
                            ("import schema namespace o = '" + lOwn + "';\n") +
                            introspection(lOwn)),
                        "v    Small");
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
