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

// Schema import, validation and casts to user-defined types from several
// threads at once. Half of the threads share one imported schema (one Xerces
// grammar pool and one type cache) through a common static context; the others
// import the schema themselves.

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include <zorba/security.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;
namespace sfs = std::filesystem;

namespace {

const unsigned ROUNDS = 20;

const char SCHEMA_NS[] = "urn:zorba:test:mt-schema";

const char SCHEMA_SRC[] =
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
  "  targetNamespace='urn:zorba:test:mt-schema'\n"
  "  xmlns='urn:zorba:test:mt-schema' elementFormDefault='qualified'>\n"
  "  <xs:simpleType name='Code'>\n"
  "    <xs:restriction base='xs:string'>\n"
  "      <xs:pattern value='[A-Z]{3}-[0-9]+'/>\n"
  "    </xs:restriction>\n"
  "  </xs:simpleType>\n"
  "  <xs:simpleType name='Small'>\n"
  "    <xs:restriction base='xs:integer'>\n"
  "      <xs:maxInclusive value='100'/>\n"
  "    </xs:restriction>\n"
  "  </xs:simpleType>\n"
  "  <xs:element name='order'>\n"
  "    <xs:complexType>\n"
  "      <xs:sequence>\n"
  "        <xs:element name='code' type='Code'/>\n"
  "        <xs:element name='qty' type='Small' maxOccurs='unbounded'/>\n"
  "      </xs:sequence>\n"
  "      <xs:attribute name='id' type='xs:ID'/>\n"
  "    </xs:complexType>\n"
  "  </xs:element>\n"
  "</xs:schema>\n";

// The second quantity is the thread number, so each thread expects its own sum.
std::string query(unsigned aThread)
{
  return
    "let $o := validate { <s:order id='o1'><s:code>ABC-12</s:code>"
    "<s:qty>3</s:qty><s:qty>" + std::to_string(aThread) + "</s:qty></s:order> }\n"
    "return string-join((\n"
    "  string(sum(data($o/s:qty))),\n"
    "  string($o instance of element(s:order)),\n"
    "  string(s:Code('XYZ-7') instance of s:Code),\n"
    "  string('100' castable as s:Small),\n"
    "  string('101' castable as s:Small),\n"
    "  string(data($o/s:code) instance of s:Code)), ' ')";
}

std::string expected(unsigned aThread)
{
  return std::to_string(3 + aThread) + " true true true false true";
}

struct SchemaFile
{
  sfs::path dir;
  sfs::path file;

  SchemaFile()
  {
    dir = sfs::temp_directory_path() /
      ("zorba-mt-schema-" + std::to_string(std::random_device()()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(dir);
    file = dir / "order.xsd";
    std::ofstream(file, std::ios::binary) << SCHEMA_SRC;
  }

  ~SchemaFile()
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

} // namespace


int
mt_schema(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  // The schema is imported from a temporary file.
  lZorba->setSecurityPolicy(SecurityPolicy::secure().allow(Capability::FileRead));
  Checker lChecker;

  try
  {
    SchemaFile lSchema;
    std::string const lImport = std::string("import schema namespace s = '") +
      SCHEMA_NS + "' at '" + lSchema.uri() + "';\n";

    StaticContext_t lShared = lZorba->createStaticContext();
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(lImport, lHints);

    run_threads([&](unsigned t)
    {
      std::string const lQuery = query(t);
      std::string const lWant = expected(t);
      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        if (t % 2 == 0)
          lChecker.expect("shared schema", t, run(lZorba, lShared, lQuery), lWant);
        else
          lChecker.expect("own schema", t,
                          run(lZorba, lZorba->createStaticContext(), lImport + lQuery),
                          lWant);
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
