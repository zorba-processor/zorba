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

// Schema import and validation with non-ASCII element/attribute names,
// enumeration facet values, namespace URI and schema location. Xerces hands
// Zorba UTF-16 strings; Zorba must convert them to UTF-8 explicitly. The
// CTest entry runs this with LC_ALL=C, and the test additionally switches
// LC_CTYPE to a non-UTF-8 locale when the system has one (as an embedding
// application may do with setlocale(LC_ALL, "")); under such a locale
// XMLString::transcode() mangles the characters.

#include <chrono>
#include <clocale>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <zorba/item.h>
#include <zorba/iterator.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
namespace sfs = std::filesystem;

namespace {

int theFailures = 0;

#define CHECK_EQ(actual, expected)                                       \
  do {                                                                   \
    std::string const lActual = (actual);                                \
    std::string const lExpected = (expected);                            \
    if (lActual != lExpected) {                                          \
      std::cerr << __FILE__ << ":" << __LINE__ << ": expected \""        \
                << lExpected << "\", got \"" << lActual << "\""          \
                << std::endl;                                            \
      ++theFailures;                                                     \
    }                                                                    \
  } while (0)

#define CHECK_CONTAINS(actual, needle)                                   \
  do {                                                                   \
    std::string const lActual = (actual);                                \
    if (lActual.find(needle) == std::string::npos) {                     \
      std::cerr << __FILE__ << ":" << __LINE__ << ": expected \""        \
                << lActual << "\" to contain \"" << (needle) << "\""     \
                << std::endl;                                            \
      ++theFailures;                                                     \
    }                                                                    \
  } while (0)

const char SCHEMA_NS[] = "urn:zorba:test:gr\xC3\xB6\xC3\x9F" "e";

const char SCHEMA_SRC[] =
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
  "  targetNamespace='urn:zorba:test:gr\xC3\xB6\xC3\x9F" "e'\n"
  "  xmlns='urn:zorba:test:gr\xC3\xB6\xC3\x9F" "e'\n"
  "  elementFormDefault='qualified' attributeFormDefault='qualified'>\n"
  "  <xs:simpleType name='Gr\xC3\xB6\xC3\x9F" "enTyp'>\n"
  "    <xs:restriction base='xs:string'>\n"
  "      <xs:enumeration value='gr\xC3\xB6\xC3\x9F" "er'/>\n"
  "      <xs:enumeration value='kleiner'/>\n"
  "    </xs:restriction>\n"
  "  </xs:simpleType>\n"
  "  <xs:element name='Gr\xC3\xB6\xC3\x9F" "e'>\n"
  "    <xs:complexType>\n"
  "      <xs:simpleContent>\n"
  "        <xs:extension base='Gr\xC3\xB6\xC3\x9F" "enTyp'>\n"
  "          <xs:attribute name='br\xC3\xA4u' type='xs:string'/>\n"
  "        </xs:extension>\n"
  "      </xs:simpleContent>\n"
  "    </xs:complexType>\n"
  "  </xs:element>\n"
  "</xs:schema>\n";

struct SchemaFile
{
  sfs::path dir;
  sfs::path file;

  SchemaFile()
  {
    dir = sfs::temp_directory_path() /
      ("zorba-schema-utf8-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(dir);
    file = dir / "gr\xC3\xB6\xC3\x9F" "e.xsd";
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

std::string run(Zorba* z, SchemaFile const& aSchema, std::string const& aBody)
{
  try
  {
    StaticContext_t lSctx = z->createStaticContext();
    std::string lQuery =
      std::string("import schema namespace s = '") + SCHEMA_NS + "' at '" +
      aSchema.uri() + "';\n" + aBody;
    XQuery_t lXQuery = z->compileQuery(lQuery, lSctx);
    Iterator_t lIter = lXQuery->iterator();
    lIter->open();
    std::string lRes;
    Item lItem;
    bool lFirst = true;
    while (lIter->next(lItem))
    {
      if (!lFirst)
        lRes += " ";
      lRes += lItem.getStringValue().str();
      lFirst = false;
    }
    lIter->close();
    return lRes;
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.diagnostic().qname().localname() +
      ": " + e.what();
  }
}

void test_valid(Zorba* z, SchemaFile const& aSchema)
{
  CHECK_EQ(run(z, aSchema,
    "let $e := validate { <s:Gr\xC3\xB6\xC3\x9F" "e s:br\xC3\xA4u='\xC3\xA4'>"
    "gr\xC3\xB6\xC3\x9F" "er</s:Gr\xC3\xB6\xC3\x9F" "e> }\n"
    "return (local-name($e), namespace-uri($e), string($e),\n"
    "        string($e/@s:br\xC3\xA4u),\n"
    "        string(node-name($e/@s:br\xC3\xA4u)))"),
    std::string("Gr\xC3\xB6\xC3\x9F" "e ") + SCHEMA_NS +
    " gr\xC3\xB6\xC3\x9F" "er \xC3\xA4 s:br\xC3\xA4u");
}

void test_type_names(Zorba* z, SchemaFile const& aSchema)
{
  // the type annotation of the validated element carries a non-ASCII name
  CHECK_EQ(run(z, aSchema,
    "let $e := validate { <s:Gr\xC3\xB6\xC3\x9F" "e>kleiner</s:Gr\xC3\xB6\xC3\x9F" "e> }\n"
    "return $e instance of element(s:Gr\xC3\xB6\xC3\x9F" "e, s:Gr\xC3\xB6\xC3\x9F" "enTyp)"),
    "true");
}

void test_invalid(Zorba* z, SchemaFile const& aSchema)
{
  // the validation error message echoes the offending non-ASCII value
  std::string lRes = run(z, aSchema,
    "validate { <s:Gr\xC3\xB6\xC3\x9F" "e>kl\xC3\xA4iner</s:Gr\xC3\xB6\xC3\x9F" "e> }");
  CHECK_CONTAINS(lRes, "error: XQDY0027");
  CHECK_CONTAINS(lRes, "kl\xC3\xA4iner");
}

} // namespace


int
schema_utf8(int argc, char* argv[])
{
  // An embedding application's locale must not matter: pick a single-byte
  // locale if one is installed, else stay in the C locale.
  static const char* const lLocales[] = {
    "de_DE.ISO-8859-1", "de_DE.ISO8859-1", "de_DE.iso88591", "en_US.ISO-8859-1",
    0
  };
  for (const char* const* l = lLocales; *l; ++l)
  {
    if (std::setlocale(LC_CTYPE, *l))
    {
      std::cout << "LC_CTYPE=" << *l << std::endl;
      break;
    }
  }

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    SchemaFile lSchema;
    std::cout << "test_valid" << std::endl;
    test_valid(lZorba, lSchema);
    std::cout << "test_type_names" << std::endl;
    test_type_names(lZorba, lSchema);
    std::cout << "test_invalid" << std::endl;
    test_invalid(lZorba, lSchema);
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "unexpected exception: " << e << std::endl;
    ++theFailures;
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
