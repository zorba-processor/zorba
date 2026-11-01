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

// The library default security policy is SecurityPolicy::secure(): an
// embedder that does not call Zorba::setSecurityPolicy() gets an engine that
// cannot read files, read the environment, expand external entities or
// import modules that reach the OS. SecurityPolicy::unrestricted() restores
// the behavior of Zorba 4.x.
//
// This test must not set a policy before the checks of the default: it runs
// in its own process, so the instance is fresh.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <zorba/item.h>
#include <zorba/item_factory.h>
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

const char SECRET[] = "SECRET-VALUE-7d2e";
const char ENV_VAR[] = "ZORBA_SECURE_DEFAULT_TEST";

struct Files
{
  sfs::path root;

  Files()
  {
    root = sfs::temp_directory_path() /
      ("zorba-secure-default-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(root);
    std::ofstream(root / "secret.xml") << "<s>" << SECRET << "</s>";
    std::ofstream(root / "secret.txt") << SECRET;
  }

  ~Files()
  {
    std::error_code ec;
    sfs::remove_all(root, ec);
  }

  std::string uri(const char* aName) const
  {
    return "file://" + (root / aName).generic_string();
  }
};

struct Outcome
{
  bool ok = false;
  std::string value;
  std::string code;
  std::string message;
};

std::ostream& operator<<(std::ostream& os, const Outcome& o)
{
  if (o.ok)
    return os << "ok: \"" << o.value << "\"";
  return os << o.code << ": " << o.message;
}

// Compiles with the instance default (no static context of the caller).
Outcome run(Zorba* z, const std::string& aQuery,
            const std::string& aUri = std::string())
{
  Outcome o;
  try
  {
    XQuery_t q = z->compileQuery(aQuery);
    if (aQuery.find("$uri") != std::string::npos)
      q->getDynamicContext()->setVariable(
          "uri", z->getItemFactory()->createString(aUri));
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

bool returns(const Outcome& o, const std::string& aExpected)
{
  if (!o.ok || o.value != aExpected)
  {
    std::cerr << "  expected \"" << aExpected << "\", got " << o << std::endl;
    return false;
  }
  return true;
}

bool fails_with(const Outcome& o, const char* aCode, const char* aText)
{
  if (o.ok || o.code != aCode ||
      o.message.find(aText) == std::string::npos ||
      o.message.find(SECRET) != std::string::npos)
  {
    std::cerr << "  expected " << aCode << " (" << aText << "), got " << o
              << std::endl;
    return false;
  }
  return true;
}

const char DOC[] =
  "declare variable $uri as xs:string external; fn:doc($uri)/string()";
const char TEXT_AVAILABLE[] =
  "declare variable $uri as xs:string external; "
  "fn:unparsed-text-available($uri)";
const char FILE_IMPORT[] =
  "import module namespace file = \"http://expath.org/ns/file\";\n"
  "file:directory-separator()";

std::string env_query()
{
  return std::string("fn:environment-variable(\"") + ENV_VAR + "\")";
}

std::string env_listed_query()
{
  return std::string("fn:available-environment-variables() = \"") + ENV_VAR +
         "\"";
}


////////////////////////////////////////////////////////////////////////////////

void test_default_policy(Zorba* z)
{
  CHECK(z->getSecurityPolicy() == SecurityPolicy::secure());
  StaticContext_t sctx = z->createStaticContext();
  CHECK(sctx->getSecurityPolicy() == SecurityPolicy::secure());
  CHECK(sctx->createChildContext()->getSecurityPolicy() ==
        SecurityPolicy::secure());
  XQuery_t q = z->compileQuery("1");
  CHECK(q->getStaticContext()->getSecurityPolicy() ==
        SecurityPolicy::secure());
}

// What stays inside the process keeps working under the default.
void test_in_process(Zorba* z)
{
  CHECK(returns(run(z, "1 + 1"), "2"));
  CHECK(returns(run(z, "string(<a>{ 'x' }</a>)"), "x"));
  CHECK(returns(run(z,
      "import module namespace uri = \"http://zorba.io/modules/uri\";\n"
      "uri:decode('a%20b')"), "a b"));
}

void test_denied_by_default(Zorba* z, const Files& f)
{
  CHECK(fails_with(run(z, DOC, f.uri("secret.xml")), "FODC0002",
                   "requires capability FileRead"));
  CHECK(returns(run(z, TEXT_AVAILABLE, f.uri("secret.txt")), "false"));

  CHECK(returns(run(z, env_query()), ""));
  CHECK(returns(run(z, env_listed_query()), "false"));

  CHECK(fails_with(run(z, FILE_IMPORT), "ZXQP0067", "FileRead or FileWrite"));

  // External entities are not expanded in documents parsed by the
  // application either.
  std::string const lXml =
    "<!DOCTYPE r [<!ENTITY e SYSTEM \"" + f.uri("secret.txt") + "\">]>"
    "<r>&e;</r>";
  std::string lParsed;
  try
  {
    std::istringstream lIn(lXml);
    Item lDoc = z->getXmlDataManager()->parseXML(lIn);
    lParsed = lDoc.getStringValue().str();
  }
  catch (ZorbaException const&)
  {
  }
  CHECK(lParsed.find(SECRET) == std::string::npos);
}

// The embedder opts out with SecurityPolicy::unrestricted().
void test_unrestricted_opt_in(Zorba* z, const Files& f)
{
  z->setSecurityPolicy(SecurityPolicy::unrestricted());
  CHECK(z->getSecurityPolicy() == SecurityPolicy::unrestricted());
  CHECK(returns(run(z, DOC, f.uri("secret.xml")), SECRET));
  CHECK(returns(run(z, TEXT_AVAILABLE, f.uri("secret.txt")), "true"));
#ifndef WIN32
  CHECK(returns(run(z, env_query()), "1"));
  CHECK(returns(run(z, env_listed_query()), "true"));
#endif

  // Back to the default: denied again.
  z->setSecurityPolicy(SecurityPolicy::secure());
  CHECK(returns(run(z, TEXT_AVAILABLE, f.uri("secret.txt")), "false"));
  CHECK(returns(run(z, env_query()), ""));
}

// A single capability can be granted without the others.
void test_single_capability(Zorba* z, const Files& f)
{
  z->setSecurityPolicy(SecurityPolicy::secure()
                         .allow(Capability::FileRead)
                         .addReadPath(f.root.string()));
  CHECK(returns(run(z, TEXT_AVAILABLE, f.uri("secret.txt")), "true"));
  CHECK(returns(run(z, env_query()), ""));
  z->setSecurityPolicy(SecurityPolicy::secure());
}

} // anonymous namespace


int
secure_default(int argc, char* argv[])
{
#ifndef WIN32
  ::setenv(ENV_VAR, "1", 1);
#endif

  std::unique_ptr<Files> lFiles;
  try
  {
    lFiles.reset(new Files());
  }
  catch (std::exception const& e)
  {
    std::cerr << "cannot create the test files: " << e.what() << std::endl;
    return 1;
  }

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    test_default_policy(lZorba);
    test_in_process(lZorba);
    test_denied_by_default(lZorba, *lFiles);
    test_unrestricted_opt_in(lZorba, *lFiles);
    test_single_capability(lZorba, *lFiles);
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

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
