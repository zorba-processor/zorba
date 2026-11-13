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

// Two threads use the schema grammar pool of one static context at the same
// time: one compiles and runs a validation, the other runs the sctx module
// functions that list the in-scope schema components. Run under
// ThreadSanitizer, it shows whether the pool is accessed without a lock.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <latch>
#include <string>
#include <thread>

#include <zorba/item.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
namespace sfs = std::filesystem;

namespace {

const char SCHEMA_NS[] = "urn:zorba:test:schema-threads";

const char SCHEMA_SRC[] =
  "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
  "  targetNamespace='urn:zorba:test:schema-threads'\n"
  "  xmlns='urn:zorba:test:schema-threads' elementFormDefault='qualified'>\n"
  "  <xs:attribute name='a' type='xs:int'/>\n"
  "  <xs:attributeGroup name='ag'><xs:attribute ref='a'/></xs:attributeGroup>\n"
  "  <xs:group name='g'><xs:sequence><xs:element ref='v'/></xs:sequence></xs:group>\n"
  "  <xs:simpleType name='t'><xs:restriction base='xs:int'/></xs:simpleType>\n"
  "  <xs:element name='v' type='t'/>\n"
  "  <xs:element name='e'>\n"
  "    <xs:complexType><xs:group ref='g'/><xs:attributeGroup ref='ag'/></xs:complexType>\n"
  "  </xs:element>\n"
  "</xs:schema>\n";

const char VALIDATE_QUERY[] =
  "declare namespace s = 'urn:zorba:test:schema-threads';\n"
  "data(validate { <s:e s:a='1'><s:v>2</s:v></s:e> }/s:v)";

const char SCTX_QUERY[] =
  "import module namespace sctx = 'http://zorba.io/modules/sctx';\n"
  "count(sctx:in-scope-element-declarations()),\n"
  "count(sctx:in-scope-attribute-declarations()),\n"
  "count(sctx:in-scope-element-groups()),\n"
  "count(sctx:in-scope-attribute-groups()),\n"
  "count(sctx:in-scope-schema-types()[namespace-uri-from-QName(.) eq\n"
  "  'urn:zorba:test:schema-threads'])";

struct SchemaFile
{
  sfs::path dir;
  sfs::path file;

  SchemaFile()
  {
    dir = sfs::temp_directory_path() /
      ("zorba-schema-threads-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(dir);
    file = dir / "schema-threads.xsd";
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

std::string run(XQuery_t const& aQuery)
{
  try
  {
    Iterator_t lIter = aQuery->iterator();
    lIter->open();
    std::string lRes;
    Item lItem;
    while (lIter->next(lItem))
    {
      if (!lRes.empty())
        lRes += " ";
      lRes += lItem.getStringValue().str();
    }
    lIter->close();
    return lRes;
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.what();
  }
}

std::string compileAndRun(Zorba* z, char const* aQuery,
                          StaticContext_t const& aSctx)
{
  try
  {
    return run(z->compileQuery(aQuery, aSctx));
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.what();
  }
}

} // namespace


int
schema_threads(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  // The schema is imported from a temporary file.
  lZorba->setSecurityPolicy(SecurityPolicy::secure().allow(Capability::FileRead));

  int lFailures = 0;
  try
  {
    SchemaFile lSchema;
    for (int i = 0; i < 8; ++i)
    {
      // A new static context per round: the pool builds its schema model
      // lazily, on the first use after a schema was loaded.
      StaticContext_t lSctx = lZorba->createStaticContext();
      Zorba_CompilerHints_t lHints;
      lSctx->loadProlog(std::string("import schema namespace s = '") +
                        SCHEMA_NS + "' at '" + lSchema.uri() + "';", lHints);
      XQuery_t lSctxQuery = lZorba->compileQuery(SCTX_QUERY, lSctx);

      std::string lValidated, lListed;
      std::latch lStart(2);
      std::thread lValidator([&] {
        lStart.arrive_and_wait();
        lValidated = compileAndRun(lZorba, VALIDATE_QUERY, lSctx);
      });
      std::thread lLister([&] {
        lStart.arrive_and_wait();
        lListed = run(lSctxQuery);
      });
      lValidator.join();
      lLister.join();

      if (lValidated != "2")
      {
        std::cerr << "round " << i << ": validation returned \""
                  << lValidated << "\"" << std::endl;
        ++lFailures;
      }
      if (lListed != "2 1 1 1 1")
      {
        std::cerr << "round " << i << ": sctx functions returned \""
                  << lListed << "\"" << std::endl;
        ++lFailures;
      }
    }
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "unexpected exception: " << e << std::endl;
    ++lFailures;
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
