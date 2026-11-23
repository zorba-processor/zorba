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

// Validation from several threads at once against one schema, imported by the
// loaded prolog of a shared static context: the threads validate with one
// Xerces grammar pool. Even and odd threads validate different values of the
// same list type with an enumeration, valid and invalid ones, in elements,
// attributes and a union, values of a type with a pattern, and documents that
// break the content model. While
// they validate, each thread's first query imports a schema of its own into
// the same context, which adds a grammar to the pool the others validate with.

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

const char NS_PREFIX[] = "urn:zorba:test:mt-validate-shared:";

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
    "  <xs:simpleType name='Pair'>\n"
    "    <xs:restriction>\n"
    "      <xs:simpleType><xs:list itemType='xs:token'/></xs:simpleType>\n"
    "      <xs:enumeration value='a b'/>\n"
    "      <xs:enumeration value='c d'/>\n"
    "    </xs:restriction>\n"
    "  </xs:simpleType>\n"
    "  <xs:simpleType name='Code'>\n"
    "    <xs:restriction base='xs:string'>\n"
    "      <xs:pattern value='[A-Z]{3}-[0-9]+'/>\n"
    "    </xs:restriction>\n"
    "  </xs:simpleType>\n"
    "  <xs:simpleType name='NumberOrPair'>\n"
    "    <xs:union memberTypes='xs:integer Pair'/>\n"
    "  </xs:simpleType>\n"
    "  <xs:element name='doc'>\n"
    "    <xs:complexType>\n"
    "      <xs:sequence>\n"
    "        <xs:element name='pair' type='Pair'/>\n"
    "        <xs:element name='n' type='NumberOrPair' maxOccurs='unbounded'/>\n"
    "      </xs:sequence>\n"
    "      <xs:attribute name='p' type='Pair'/>\n"
    "      <xs:attribute name='c' type='Code'/>\n"
    "    </xs:complexType>\n"
    "  </xs:element>\n"
    "</xs:schema>\n";
}

// The schema a thread imports into the shared context while the others
// validate: an element v of an integer type up to aMax.
std::string own_schema(std::string const& aName, unsigned aMax)
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
    std::string const lSchema = lName == "shared" ? shared_schema() :
      own_schema(lName, limit(static_cast<unsigned>(std::stoul(lName.substr(1)))));
    return StreamResource::create(
      std::make_unique<std::istringstream>(lSchema).release(), &releaseStream);
  }
};

// Even threads validate "a b", odd threads "c d"; each one's invalid value
// mixes its tokens with the other's. The codes match a pattern, or not.
std::string valid(unsigned aThread)
{
  return aThread % 2 == 0 ? "a b" : "c d";
}

std::string invalid(unsigned aThread)
{
  return aThread % 2 == 0 ? "a d" : "c b";
}

std::string query(unsigned aThread)
{
  std::string const lValid = valid(aThread);
  std::string const lInvalid = invalid(aThread);
  std::string const lMax = std::to_string(limit(aThread));
  return
    "declare namespace err = 'http://www.w3.org/2005/xqt-errors';\n"
    "import schema namespace o = '" + ns("t" + std::to_string(aThread)) + "';\n"
    "let $d := validate { <s:doc p='" + lValid + "' c='ABC-" + lMax +
    "'><s:pair>" + lValid +
    "</s:pair><s:n>" + std::to_string(aThread) + "</s:n><s:n>" + lValid +
    "</s:n></s:doc> }\n"
    "return string-join((\n"
    "  string-join(data($d/s:pair), '-'),\n"
    "  string-join(data($d/@p), '-'),\n"
    "  string-join(for $n in data($d/s:n) return string($n), '-'),\n"
    "  try { validate { <s:doc><s:pair>" + lInvalid +
    "</s:pair><s:n>1</s:n></s:doc> }, 'valid' }\n"
    "  catch err:XQDY0027 { 'invalid' },\n"
    "  try { validate { <s:doc p='" + lInvalid + "'><s:pair>" + lValid +
    "</s:pair><s:n>1</s:n></s:doc> }, 'valid' }\n"
    "  catch err:XQDY0027 { 'invalid' },\n"
    "  try { validate { <s:doc><s:pair>" + lValid + "</s:pair><s:n>" + lInvalid +
    "</s:n></s:doc> }, 'valid' }\n"
    "  catch err:XQDY0027 { 'invalid' },\n"
    "  try { validate { <s:doc><s:n>1</s:n></s:doc> }, 'valid' }\n"
    "  catch err:XQDY0027 { 'invalid' },\n"
    "  try { validate { <s:doc c='AB-" + lMax + "'><s:pair>" + lValid +
    "</s:pair><s:n>1</s:n></s:doc> }, 'valid' }\n"
    "  catch err:XQDY0027 { 'invalid' },\n"
    "  string(data(validate { <o:v>" + lMax + "</o:v> }) instance of o:Small)\n"
    "), ' ')";
}

std::string expected(unsigned aThread)
{
  std::string lTokens = valid(aThread);
  lTokens[1] = '-';
  return lTokens + " " + lTokens + " " + std::to_string(aThread) + "-" +
         lTokens + " invalid invalid invalid invalid invalid true";
}

} // namespace


int
mt_validate_shared(int, char*[])
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

    run_threads([&](unsigned t)
    {
      std::string const lQuery = query(t);
      std::string const lWant = expected(t);
      for (unsigned r = 0; r < ROUNDS; ++r)
        lChecker.expect("shared schema", t, run(lZorba, lShared, lQuery), lWant);
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
