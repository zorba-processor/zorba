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

// XML parser hardening under the SecurityPolicy (include/zorba/security.h):
// external entities (XXE through a SYSTEM entity, a parameter entity and an
// external DTD subset) and entity expansion (billion laughs), for policies
// set on a StaticContext and on the Zorba instance.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <zorba/config.h>
#include <zorba/diagnostic_list.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/item_sequence.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/xmldatamanager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
namespace sfs = std::filesystem;

namespace {

int theFailures = 0;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      std::cerr << __FILE__ << ":" << __LINE__ << ": check failed: "    \
                << #cond << std::endl;                                  \
      ++theFailures;                                                    \
    }                                                                   \
  } while (0)

const char SECRET[] = "SECRET-VALUE-7f3a";

// Files referenced by the test documents, in a fresh directory.
struct Files
{
  sfs::path dir;
  std::string secretURI;    // text file holding SECRET
  std::string entitiesURI;  // declares <!ENTITY e SECRET>
  std::string dtdURI;       // external DTD subset declaring r and e

  Files()
  {
    dir = sfs::temp_directory_path() /
      ("zorba-xml-parser-security-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(dir);
    secretURI = write("secret.txt", SECRET);
    entitiesURI = write("entities.ent",
                        std::string("<!ENTITY e \"") + SECRET + "\">");
    dtdURI = write("ext.dtd",
                   std::string("<!ELEMENT r (#PCDATA)>\n<!ENTITY e \"") +
                   SECRET + "\">\n");
  }

  ~Files()
  {
    std::error_code ec;
    sfs::remove_all(dir, ec);
  }

  std::string write(const char* aName, const std::string& aContent) const
  {
    sfs::path const p = dir / aName;
    std::ofstream(p) << aContent;
    return "file://" + p.generic_string();
  }
};

std::string xxe_system(const Files& f)
{
  return "<?xml version=\"1.0\"?>\n"
         "<!DOCTYPE r [<!ENTITY xxe SYSTEM \"" + f.secretURI + "\">]>\n"
         "<r>&xxe;</r>";
}

std::string xxe_parameter(const Files& f)
{
  return "<?xml version=\"1.0\"?>\n"
         "<!DOCTYPE r [<!ENTITY % pe SYSTEM \"" + f.entitiesURI + "\"> %pe;]>\n"
         "<r>&e;</r>";
}

std::string xxe_external_dtd(const Files& f)
{
  return "<?xml version=\"1.0\"?>\n"
         "<!DOCTYPE r SYSTEM \"" + f.dtdURI + "\">\n"
         "<r>&e;</r>";
}

const char INTERNAL_ENTITY[] =
  "<?xml version=\"1.0\"?>\n"
  "<!DOCTYPE r [<!ENTITY g \"hello\">]>\n"
  "<r>&g;</r>";

std::string billion_laughs()
{
  std::string s = "<?xml version=\"1.0\"?>\n<!DOCTYPE lolz [\n"
                  "<!ENTITY lol0 \"lol\">\n";
  for (int i = 1; i < 10; ++i)
  {
    s += "<!ENTITY lol" + std::to_string(i) + " \"";
    for (int j = 0; j < 10; ++j)
      s += "&lol" + std::to_string(i - 1) + ";";
    s += "\">\n";
  }
  s += "]>\n<lolz>&lol9;</lolz>";
  return s;
}

struct Outcome
{
  bool ok = false;
  std::string value;     // string value of the result, if ok
  std::string code;      // local name of the error code, if not ok
  std::string message;   // error message, if not ok
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    return os << "ok: \"" << o.value << "\"";
  return os << o.code << ": " << o.message;
}

Outcome run(Zorba* z, StaticContext_t sctx, const char* aQuery,
            const char* aVar, const std::string& aValue)
{
  Outcome o;
  try
  {
    XQuery_t q = z->compileQuery(aQuery, sctx);
    q->getDynamicContext()->setVariable(
        aVar, z->getItemFactory()->createString(aValue));
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
      o.value += item.getStringValue().str();
    it->close();
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
  return o;
}

Outcome parse_xml(Zorba* z, StaticContext_t sctx, const std::string& aXml)
{
  return run(z, sctx,
             "declare variable $input as xs:string external;\n"
             "fn:parse-xml($input)/string()",
             "input", aXml);
}

// x:canonicalize builds a libxml2 tree with the default SAX2 handler, the
// parse path where parameter entities are processed (the store's SAX loader
// behind fn:parse-xml ignores them).
[[maybe_unused]] Outcome canonicalize(Zorba* z, StaticContext_t sctx,
                                      const std::string& aXml)
{
  return run(z, sctx,
             "import module namespace x = \"http://zorba.io/modules/xml\";\n"
             "declare namespace opt = \"http://zorba.io/modules/xml-options\";\n"
             "declare variable $input as xs:string external;\n"
             "x:canonicalize($input, <opt:options><opt:substitute-entities/>"
             "</opt:options>)",
             "input", aXml);
}

Outcome doc(Zorba* z, StaticContext_t sctx, const std::string& aURI)
{
  return run(z, sctx,
             "declare variable $uri as xs:string external;\n"
             "fn:doc($uri)/string()",
             "uri", aURI);
}

StaticContext_t context(Zorba* z, const SecurityPolicy& p)
{
  StaticContext_t sctx = z->createStaticContext();
  sctx->setSecurityPolicy(p);
  return sctx;
}

bool expanded(const Outcome& o, const char* aExpected)
{
  if (!o.ok || o.value.find(aExpected) == std::string::npos)
  {
    std::cerr << "  expected \"" << aExpected << "\", got " << o << std::endl;
    return false;
  }
  return true;
}

bool denied(const Outcome& o, const char* aCode, const std::string& aURI)
{
  if (o.ok || o.code != aCode ||
      o.message.find("URI access not allowed") == std::string::npos ||
      o.message.find(aURI) == std::string::npos ||
      o.message.find(SECRET) != std::string::npos)
  {
    std::cerr << "  expected " << aCode << " denial of " << aURI
              << ", got " << o << std::endl;
    return false;
  }
  return true;
}

// Before 5.0.0 the instance default is unrestricted(): external entities are
// expanded as they always were.
void test_unrestricted(Zorba* z, const Files& f)
{
  StaticContext_t sctx = context(z, SecurityPolicy::unrestricted());
  CHECK(expanded(parse_xml(z, sctx, xxe_system(f)), SECRET));
#ifndef ZORBA_NO_XMLSCHEMA
  CHECK(expanded(canonicalize(z, sctx, xxe_parameter(f)), SECRET));
  CHECK(expanded(canonicalize(z, sctx, xxe_system(f)), SECRET));
#endif
  CHECK(expanded(parse_xml(z, sctx, INTERNAL_ENTITY), "hello"));

  // Also with the instance default.
  StaticContext_t dflt = z->createStaticContext();
  CHECK(expanded(parse_xml(z, dflt, xxe_system(f)), SECRET));
}

void test_secure_context(Zorba* z, const Files& f)
{
  StaticContext_t sctx = context(z, SecurityPolicy::secure());

  CHECK(denied(parse_xml(z, sctx, xxe_system(f)), "FODC0006", f.secretURI));
#ifndef ZORBA_NO_XMLSCHEMA
  CHECK(denied(canonicalize(z, sctx, xxe_parameter(f)), "FOCZ0001",
               f.entitiesURI));
  CHECK(denied(canonicalize(z, sctx, xxe_system(f)), "FOCZ0001",
               f.secretURI));
#endif

  // Internal entities do not need any capability.
  CHECK(expanded(parse_xml(z, sctx, INTERNAL_ENTITY), "hello"));

  // A child context without a policy of its own inherits the denial.
  StaticContext_t child = sctx->createChildContext();
  CHECK(denied(parse_xml(z, child, xxe_system(f)), "FODC0006", f.secretURI));

  // fn:doc of a document with an XXE (reading the document itself is not
  // restricted yet; its external entity is).
  std::string const lDocURI = f.write("doc-with-xxe.xml", xxe_system(f));
  CHECK(denied(doc(z, sctx, lDocURI), "FODC0002", f.secretURI));
}

void test_allow_lists(Zorba* z, const Files& f)
{
  // ExternalEntities alone is not enough for file: entities.
  StaticContext_t noFile = context(z, SecurityPolicy::secure()
      .allow(Capability::ExternalEntities));
  CHECK(denied(parse_xml(z, noFile, xxe_system(f)), "FODC0006", f.secretURI));

  // FileRead restricted to the directory of the entity.
  StaticContext_t inDir = context(z, SecurityPolicy::secure()
      .allow(Capability::ExternalEntities)
      .allow(Capability::FileRead)
      .addReadPath(f.dir.string()));
  CHECK(expanded(parse_xml(z, inDir, xxe_system(f)), SECRET));
#ifndef ZORBA_NO_XMLSCHEMA
  CHECK(expanded(canonicalize(z, inDir, xxe_parameter(f)), SECRET));
#endif

  // FileRead restricted to another directory.
  StaticContext_t otherDir = context(z, SecurityPolicy::secure()
      .allow(Capability::ExternalEntities)
      .allow(Capability::FileRead)
      .addReadPath((f.dir / "elsewhere").string()));
  CHECK(denied(parse_xml(z, otherDir, xxe_system(f)), "FODC0006",
               f.secretURI));

  // FileRead without ExternalEntities.
  StaticContext_t noEntities = context(z, SecurityPolicy::secure()
      .allow(Capability::FileRead));
  CHECK(denied(parse_xml(z, noEntities, xxe_system(f)), "FODC0006",
               f.secretURI));
}

void test_instance_default(Zorba* z, const Files& f)
{
  z->setSecurityPolicy(SecurityPolicy::secure());

  CHECK(denied(parse_xml(z, z->createStaticContext(), xxe_system(f)),
               "FODC0006", f.secretURI));

  XmlDataManager_t mgr = z->getXmlDataManager();
  {
    std::istringstream in(xxe_system(f));
    Outcome o;
    try
    {
      Item item = mgr->parseXML(in);
      o.ok = !item.isNull();
      if (o.ok)
        o.value = item.getStringValue().str();
    }
    catch (ZorbaException const& e)
    {
      o.code = e.diagnostic().qname().localname();
      o.message = e.what();
    }
    CHECK(denied(o, "FODC0006", f.secretURI));
  }

#ifndef ZORBA_NO_XMLSCHEMA
  // External DTD subset, loaded for DTD validation.
  for (int lSecure = 1; lSecure >= 0; --lSecure)
  {
    z->setSecurityPolicy(lSecure ? SecurityPolicy::secure()
                                 : SecurityPolicy::unrestricted());
    std::istringstream in(xxe_external_dtd(f));
    XmlDataManager::ParseOptions opts;
    opts.setDtdValidation(true);
    Outcome o;
    try
    {
      ItemSequence_t seq = mgr->parseXML(in, opts);
      Iterator_t it = seq->getIterator();
      it->open();
      Item item;
      while (it->next(item))
        o.value += item.getStringValue().str();
      it->close();
      o.ok = true;
    }
    catch (ZorbaException const& e)
    {
      o.code = e.diagnostic().qname().localname();
      o.message = e.what();
    }
    if (lSecure)
      CHECK(denied(o, "FODC0006", f.dtdURI));
    else
      CHECK(expanded(o, SECRET));
  }
#endif

  z->setSecurityPolicy(SecurityPolicy::unrestricted());
}

// Entity expansion must fail fast with an error under every policy; libxml2's
// amplification checks are never disabled.
void test_billion_laughs(Zorba* z)
{
  std::string const lDoc = billion_laughs();
  const SecurityPolicy lPolicies[] = {
    SecurityPolicy::secure(),
    SecurityPolicy::unrestricted(),
    SecurityPolicy::secure().setMaxEntityAmplification(5)
  };
  for (const SecurityPolicy& p : lPolicies)
  {
    auto const t0 = std::chrono::steady_clock::now();
    Outcome o = parse_xml(z, context(z, p), lDoc);
    auto const secs = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - t0).count();
    CHECK(!o.ok);
    CHECK(o.code == "FODC0006");
    CHECK(secs < 30);
    if (o.ok || secs >= 30)
      std::cerr << "  billion laughs: " << o << " after " << secs << " s"
                << std::endl;
  }
}

} // anonymous namespace


int
xml_parser_security(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    Files lFiles;
    test_secure_context(lZorba, lFiles);
    test_allow_lists(lZorba, lFiles);
    test_unrestricted(lZorba, lFiles);
    test_instance_default(lZorba, lFiles);
    test_billion_laughs(lZorba);
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

  lZorba->setSecurityPolicy(SecurityPolicy::unrestricted());
  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
