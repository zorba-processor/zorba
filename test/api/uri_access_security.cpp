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

// Enforcement of the SecurityPolicy (include/zorba/security.h) at URI
// resolution: SecurityPolicy::checkURI() (path canonicalization, network
// prefixes, private addresses) and the built-in URL resolvers behind
// fn:doc, fn:doc-available, fn:unparsed-text*, module and schema import and
// the fetch module, including the fn:doc document cache and the
// ZORBA_URI_PATH environment variable.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <zorba/config.h>
#include <zorba/diagnostic_list.h>
#include <zorba/document_manager.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/uri_resolvers.h>
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

const char SECRET[] = "SECRET-VALUE-4c91";

// root/
//   data/in.xml, data/in.txt, data/m.xq, data/s.xsd, data/stop.txt, data/t.jsd
//   data/link.xml -> ../outside/secret.xml
//   data/ldir -> ../outside
//   data-evil/x.xml
//   datalink -> data
//   outside/secret.xml, outside/secret.txt
//   cache/cached.xml
//   env/test/example/envmod.module
struct Files
{
  sfs::path root;
  sfs::path data;
  sfs::path outside;

  Files()
  {
    root = sfs::temp_directory_path() /
      ("zorba-uri-access-security-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    data = root / "data";
    outside = root / "outside";
    for (const char* d : {"data", "data-evil", "outside", "cache",
                          "env/test/example"})
      sfs::create_directories(root / d);

    write("data/in.xml", "<in>inside</in>");
    write("data/in.txt", "inside-text");
    write("data/m.xq",
          "module namespace m = \"http://example.test/m\";\n"
          "declare function m:f() { 42 };\n");
    write("data/s.xsd",
          "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
          "targetNamespace=\"http://example.test/s\">"
          "<xs:element name=\"e\" type=\"xs:string\"/></xs:schema>");
    write("data/stop.txt", "foo");
    write("data/t.jsd",
          "{ \"$namespace\" : \"http://example.test/t\", \"$types\" : [ "
          "{ \"$name\" : \"t\", \"$kind\" : \"atomic\", "
          "\"$baseType\" : \"integer\" } ] }");
    write("data-evil/x.xml", std::string("<x>") + SECRET + "</x>");
    write("outside/secret.xml", std::string("<s>") + SECRET + "</s>");
    write("outside/secret.txt", SECRET);
    write("cache/cached.xml", std::string("<c>") + SECRET + "</c>");
    write("env/test/example/envmod.module",
          "module namespace e = \"http://example.test/envmod\";\n"
          "declare function e:f() { 43 };\n");

    sfs::create_symlink(outside / "secret.xml", data / "link.xml");
    sfs::create_directory_symlink(outside, data / "ldir");
    sfs::create_directory_symlink(data, root / "datalink");
  }

  ~Files()
  {
    std::error_code ec;
    sfs::remove_all(root, ec);
  }

  void write(const char* aName, const std::string& aContent) const
  {
    std::ofstream(root / aName) << aContent;
  }

  std::string path(const std::string& aRel) const
  {
    return (root / aRel).generic_string();
  }

  std::string uri(const std::string& aRel) const
  {
    return "file://" + path(aRel);
  }
};

std::string http_test_server()
{
  const char* const env = std::getenv("ZORBA_HTTP_TEST_SERVER");
  return env && *env ? env : ZORBA_HTTP_TEST_SERVER_DEFAULT;
}

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

Outcome run(Zorba* z, StaticContext_t sctx, const std::string& aQuery,
            const std::string& aUri = std::string())
{
  Outcome o;
  try
  {
    XQuery_t q = z->compileQuery(aQuery, sctx);
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

const char DOC[] =
  "declare variable $uri as xs:string external; fn:doc($uri)/string()";
const char DOC_AVAILABLE[] =
  "declare variable $uri as xs:string external; fn:doc-available($uri)";
const char TEXT[] =
  "declare variable $uri as xs:string external; fn:unparsed-text($uri)";
const char TEXT_LINES[] =
  "declare variable $uri as xs:string external; "
  "fn:unparsed-text-lines($uri)";
const char TEXT_AVAILABLE[] =
  "declare variable $uri as xs:string external; "
  "fn:unparsed-text-available($uri)";
const char FETCH[] =
  "import module namespace f = \"http://zorba.io/modules/fetch\";\n"
  "declare variable $uri as xs:string external; f:content($uri)";
const char FETCH_AS_MODULE[] =
  "import module namespace f = \"http://zorba.io/modules/fetch\";\n"
  "declare variable $uri as xs:string external; f:content($uri, \"MODULE\")";

StaticContext_t context(Zorba* z, const SecurityPolicy& p)
{
  StaticContext_t sctx = z->createStaticContext();
  sctx->setSecurityPolicy(p);
  return sctx;
}

bool allowed(const Outcome& o, const std::string& aExpected)
{
  if (!o.ok || o.value != aExpected)
  {
    std::cerr << "  expected \"" << aExpected << "\", got " << o << std::endl;
    return false;
  }
  return true;
}

// A denial by the security policy is reported with the error the
// specification prescribes for a resource that is not available (aCode); the
// message names the policy and the reason.
bool denied(const Outcome& o, const char* aCode,
            const std::string& aReason = std::string())
{
  if (o.ok || o.code != aCode ||
      o.message.find("access denied by security policy") ==
        std::string::npos ||
      o.message.find(aReason) == std::string::npos ||
      o.message.find(SECRET) != std::string::npos)
  {
    std::cerr << "  expected a " << aCode << " denial (" << aReason
              << "), got " << o << std::endl;
    return false;
  }
  return true;
}

bool not_denied(const Outcome& o)
{
  if (!o.ok &&
      (o.code == "ZXQP0029" ||
       o.message.find("access denied by security policy") !=
         std::string::npos))
  {
    std::cerr << "  expected no denial, got " << o << std::endl;
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////

void test_check_files(const Files& f)
{
  const EntityData::Kind lDoc = EntityData::DOCUMENT;

  CHECK(!SecurityPolicy::secure().checkURI(f.uri("data/in.xml"), lDoc));
  CHECK(SecurityPolicy::unrestricted().checkURI(f.uri("outside/secret.xml"),
                                                lDoc));
  CHECK(SecurityPolicy::secure().allow(Capability::FileRead)
        .checkURI(f.uri("outside/secret.xml"), lDoc));

  SecurityPolicy const p = SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.data.string());
  CHECK(p.checkURI(f.uri("data/in.xml"), lDoc));
  CHECK(p.checkURI(f.path("data/in.xml"), lDoc));
  CHECK(p.checkURI(f.uri("data/missing.xml"), lDoc));
  CHECK(p.checkURI(f.uri("data/nosuchdir/../in.xml"), lDoc));
  CHECK(p.checkURI(f.uri("datalink/in.xml"), lDoc));
  // ".." out of the directory
  CHECK(!p.checkURI(f.uri("data/../outside/secret.xml"), lDoc));
  CHECK(!p.checkURI(f.path("data/../outside/secret.xml"), lDoc));
  // a sibling directory that starts with the same name
  CHECK(!p.checkURI(f.uri("data-evil/x.xml"), lDoc));
  // symbolic links out of the directory, to a file and to a directory
  CHECK(!p.checkURI(f.uri("data/link.xml"), lDoc));
  CHECK(!p.checkURI(f.uri("data/ldir/secret.xml"), lDoc));
  CHECK(!p.checkURI(f.uri("data/ldir/missing.xml"), lDoc));
  // the parent of the directory
  CHECK(!p.checkURI(f.uri(""), lDoc));

  // A read path given with a trailing separator, or through a symbolic link.
  CHECK(SecurityPolicy::secure().allow(Capability::FileRead)
        .addReadPath(f.data.string() + "/")
        .checkURI(f.uri("data/in.xml"), lDoc));
  SecurityPolicy const viaLink = SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.path("datalink"));
  CHECK(viaLink.checkURI(f.uri("data/in.xml"), lDoc));
  CHECK(!viaLink.checkURI(f.uri("outside/secret.xml"), lDoc));

  // Write paths are separate from read paths.
  SecurityPolicy const w = SecurityPolicy::secure()
      .allow(Capability::FileWrite).addWritePath(f.data.string());
  CHECK(w.checkURI(f.uri("data/new.xml"), lDoc, true));
  CHECK(!w.checkURI(f.uri("outside/new.xml"), lDoc, true));
  CHECK(!w.checkURI(f.uri("data/ldir/new.xml"), lDoc, true));
  CHECK(!w.checkURI(f.uri("data/in.xml"), lDoc));
  CHECK(!p.checkURI(f.uri("data/new.xml"), lDoc, true));

  // Imports from files are FileRead too (trusted paths are known only to
  // the engine); collections live in the store.
  CHECK(!SecurityPolicy::secure().checkURI(f.uri("data/m.xq"),
                                           EntityData::MODULE));
  CHECK(p.checkURI(f.uri("data/m.xq"), EntityData::MODULE));
  CHECK(SecurityPolicy::secure().checkURI("http://example.test/coll",
                                          EntityData::COLLECTION));
}

void test_check_network()
{
  const EntityData::Kind lDoc = EntityData::DOCUMENT;

  CHECK(!SecurityPolicy::secure().checkURI("http://8.8.8.8/", lDoc));
  CHECK(SecurityPolicy::unrestricted().checkURI("http://127.0.0.1/",
                                                lDoc));

  // Network allowed: public addresses, but not loopback, private or
  // link-local ones (checked without any connection).
  SecurityPolicy const n = SecurityPolicy::secure().allow(Capability::Network);
  CHECK(n.checkURI("http://8.8.8.8/x", lDoc));
  CHECK(n.checkURI("https://[2001:4860:4860::8888]/x", lDoc));
  CHECK(n.checkURI("ftp://8.8.8.8/x", lDoc));
  for (const char* lPrivate : {
         "http://127.0.0.1/", "http://127.0.0.1:8080/x", "http://localhost/",
         "http://LOCALHOST:8080/", "http://[::1]/", "http://10.1.2.3/",
         "http://169.254.169.254/latest/meta-data/", "http://172.16.0.1/",
         "http://172.31.255.255/", "http://192.168.1.1/", "http://100.64.0.1/",
         "http://0.0.0.0/", "http://[::]/", "http://[fe80::1]/",
         "http://[fd00::1]/", "http://[::ffff:127.0.0.1]/",
         "http://[::ffff:169.254.169.254]/", "http://[64:ff9b::a00:1]/",
         "http://[2002:7f00:1::]/", "http://2130706433/", "http://127.1/",
         "http://0x7f.0.0.1/", "https://127.0.0.1/", "ftp://127.0.0.1/" })
  {
    if (n.checkURI(lPrivate, lDoc))
    {
      std::cerr << "  private address allowed: " << lPrivate << std::endl;
      ++theFailures;
    }
  }
  // ... unless allowed explicitly.
  SecurityPolicy const priv = SecurityPolicy::secure()
      .allow(Capability::Network).setAllowPrivateAddresses(true);
  CHECK(priv.checkURI("http://127.0.0.1/", lDoc));
  CHECK(priv.checkURI("http://169.254.169.254/", lDoc));
  CHECK(priv.checkURI("http://[::1]/", lDoc));

  // Module and schema import also need RemoteImport.
  CHECK(!n.checkURI("http://8.8.8.8/m.xq", EntityData::MODULE));
  CHECK(!n.checkURI("http://8.8.8.8/s.xsd", EntityData::SCHEMA));
  CHECK(SecurityPolicy::secure().allow(Capability::Network)
        .allow(Capability::RemoteImport)
        .checkURI("http://8.8.8.8/m.xq", EntityData::MODULE));
  CHECK(!SecurityPolicy::secure().allow(Capability::RemoteImport)
        .checkURI("http://8.8.8.8/m.xq", EntityData::MODULE));

  // Prefixes: scheme, host and port must be equal, the path must continue on
  // a segment boundary. (Private addresses allowed so that no name needs to
  // be resolved.)
  SecurityPolicy const pre = SecurityPolicy::secure()
      .allow(Capability::Network).setAllowPrivateAddresses(true)
      .addNetworkPrefix("http://allowed.example/v1/");
  for (const char* lOk : {
         "http://allowed.example/v1/", "http://allowed.example/v1/x",
         "http://ALLOWED.Example/v1/x", "http://allowed.example:80/v1/x",
         "http://allowed.example./v1/x", "http://user:pw@allowed.example/v1/x",
         "http://allowed.example/v1/a/../b", "http://allowed.example/v1/x?y#z" })
  {
    if (!pre.checkURI(lOk, lDoc))
    {
      std::cerr << "  under the prefix, denied: " << lOk << std::endl;
      ++theFailures;
    }
  }
  for (const char* lBad : {
         "http://allowed.example.evil.test/v1/x",
         "http://allowed.example@evil.test/v1/x",
         "http://evil.test/v1/?http://allowed.example/v1/",
         "http://evil.test/#http://allowed.example/v1/",
         "https://allowed.example/v1/x", "ftp://allowed.example/v1/x",
         "http://allowed.example:8080/v1/x", "http://allowed.example/v1",
         "http://allowed.example/v1x", "http://allowed.example/v2/x",
         "http://allowed.example/v1/../admin",
         "http://allowed.example/v1/%2e%2e/admin",
         "http://allowed.example/v1/%2E%2E/admin",
         "http://allowed.example/v1%2F..%2Fadmin",
         "http://a@b@allowed.example/v1/x",
         "http://allowed.example\\@evil.test/v1/x",
         "http://allowed.example /v1/x", "http://%61llowed.example/v1/x",
         "http:allowed.example/v1/x", "http://allowed.example:0/v1/x",
         "http://allowed.example:99999/v1/x" })
  {
    if (pre.checkURI(lBad, lDoc))
    {
      std::cerr << "  not under the prefix, allowed: " << lBad << std::endl;
      ++theFailures;
    }
  }

  // A prefix without a trailing slash covers the path itself and below.
  SecurityPolicy const pre2 = SecurityPolicy::secure()
      .allow(Capability::Network).setAllowPrivateAddresses(true)
      .addNetworkPrefix("http://allowed.example/v1");
  CHECK(pre2.checkURI("http://allowed.example/v1", lDoc));
  CHECK(pre2.checkURI("http://allowed.example/v1/x", lDoc));
  CHECK(!pre2.checkURI("http://allowed.example/v1x", lDoc));

  // A prefix that names a private address literally (or localhost) allows
  // it without setAllowPrivateAddresses(); other private hosts stay denied.
  SecurityPolicy const lit = SecurityPolicy::secure()
      .allow(Capability::Network)
      .addNetworkPrefix("http://127.0.0.1:8080/")
      .addNetworkPrefix("http://localhost:9090/");
  CHECK(lit.checkURI("http://127.0.0.1:8080/x", lDoc));
  CHECK(lit.checkURI("http://localhost:9090/x", lDoc));
  CHECK(!lit.checkURI("http://127.0.0.1:9090/x", lDoc));
  CHECK(!lit.checkURI("http://10.0.0.1:8080/x", lDoc));
}


////////////////////////////////////////////////////////////////////////////////

void test_documents(Zorba* z, const Files& f)
{
  StaticContext_t sec = context(z, SecurityPolicy::secure());
  CHECK(denied(run(z, sec, DOC, f.uri("data/in.xml")), "FODC0002",
               "requires capability FileRead"));
  CHECK(allowed(run(z, sec, DOC_AVAILABLE, f.uri("data/in.xml")), "false"));
  CHECK(denied(run(z, sec, TEXT, f.uri("data/in.txt")), "FOUT1170"));
  CHECK(denied(run(z, sec, TEXT_LINES, f.uri("data/in.txt")), "FOUT1170"));
  CHECK(allowed(run(z, sec, TEXT_AVAILABLE, f.uri("data/in.txt")), "false"));
  Outcome const lFetch = run(z, sec, FETCH, f.uri("data/in.txt"));
  CHECK(denied(lFetch, "URI_UNRESOLVED_OR_NOSTREAM",
               "requires capability FileRead"));
  // The denial is reported once, not wrapped in a second
  // URI_UNRESOLVED_OR_NOSTREAM message.
  CHECK(lFetch.message.find("stream is unresolved") ==
        lFetch.message.rfind("stream is unresolved"));
  // A child context inherits the policy.
  CHECK(denied(run(z, sec->createChildContext(), DOC, f.uri("data/in.xml")),
               "FODC0002"));

  StaticContext_t inData = context(z, SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.data.string()));
  CHECK(allowed(run(z, inData, DOC, f.uri("data/in.xml")), "inside"));
  CHECK(allowed(run(z, inData, DOC_AVAILABLE, f.uri("data/in.xml")),
                "true"));
  CHECK(allowed(run(z, inData, DOC_AVAILABLE, f.uri("data/missing.xml")),
                "false"));
  CHECK(allowed(run(z, inData, TEXT, f.uri("data/in.txt")), "inside-text"));
  CHECK(allowed(run(z, inData, TEXT_LINES, f.uri("data/in.txt")),
                "inside-text"));
  CHECK(allowed(run(z, inData, TEXT_AVAILABLE, f.uri("data/in.txt")),
                "true"));
  CHECK(allowed(run(z, inData, FETCH, f.uri("data/in.txt")), "inside-text"));
  CHECK(denied(run(z, inData, DOC, f.uri("outside/secret.xml")), "FODC0002",
               "outside the allowed read paths"));
  CHECK(denied(run(z, inData, DOC, f.uri("data/link.xml")), "FODC0002"));
  CHECK(denied(run(z, inData, DOC, f.uri("data/ldir/secret.xml")), "FODC0002"));
  CHECK(denied(run(z, inData, DOC, f.uri("data/../outside/secret.xml")),
               "FODC0002"));
  CHECK(denied(run(z, inData, DOC, f.uri("data-evil/x.xml")), "FODC0002"));
  CHECK(denied(run(z, inData, TEXT, f.uri("outside/secret.txt")), "FOUT1170"));
  CHECK(allowed(run(z, inData, TEXT_AVAILABLE, f.uri("outside/secret.txt")),
                "false"));
  CHECK(denied(run(z, inData, FETCH, f.uri("outside/secret.txt")),
               "URI_UNRESOLVED_OR_NOSTREAM"));
  // The entity kind chosen by the query does not matter: a file outside the
  // trusted paths is not a trusted module.
  CHECK(denied(run(z, inData, FETCH_AS_MODULE, f.uri("outside/secret.txt")),
               "URI_UNRESOLVED_OR_NOSTREAM"));

  // Under unrestricted(), nothing changes.
  StaticContext_t all = context(z, SecurityPolicy::unrestricted());
  CHECK(allowed(run(z, all, TEXT, f.uri("outside/secret.txt")), SECRET));
}

// fn:doc caches documents in the store, which all queries share. A document
// loaded under a wide policy must not be served to a narrower one.
void test_document_cache(Zorba* z, const Files& f)
{
  std::string const lUri = f.uri("cache/cached.xml");
  StaticContext_t all = context(z, SecurityPolicy::unrestricted());
  StaticContext_t sec = context(z, SecurityPolicy::secure());
  StaticContext_t inData = context(z, SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.data.string()));

  CHECK(allowed(run(z, all, DOC, lUri), SECRET));
  CHECK(denied(run(z, sec, DOC, lUri), "FODC0002"));
  CHECK(allowed(run(z, sec, DOC_AVAILABLE, lUri), "false"));
  CHECK(denied(run(z, inData, DOC, lUri), "FODC0002"));
  CHECK(allowed(run(z, all, DOC, lUri), SECRET));
  // The instance default is secure(): no cache hit for it either.
  CHECK(denied(run(z, z->createStaticContext(), DOC, lUri), "FODC0002"));

  // Documents the embedder puts into the store are not subject to the
  // policy: the embedder chose to make them available.
  std::string const lPutUri = "http://example.test/put.xml";
  std::istringstream lIn("<p>put</p>");
  Item lDoc = z->getXmlDataManager()->parseXML(lIn);
  z->getXmlDataManager()->getDocumentManager()->put(lPutUri, lDoc);
  CHECK(allowed(run(z, sec, DOC, lPutUri), "put"));
  z->getXmlDataManager()->getDocumentManager()->remove(lPutUri);
}

void test_network(Zorba* z)
{
  // The test server is on a private address (a container on the CI network,
  // or localhost); it may be absent (macOS, profile builds), so the allowed
  // cases only check that the policy does not refuse the access.
  std::string const lUrl =
    "http://" + http_test_server() + "/cgi-bin/test-xml?querystring";
  const char QUERY[] =
    "declare variable $uri as xs:string external; "
    "fn:doc($uri)/body/query/string()";

  CHECK(denied(run(z, context(z, SecurityPolicy::secure()), QUERY, lUrl),
               "FODC0002",
               "requires capability Network"));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()), TEXT, lUrl),
               "FOUT1170",
               "requires capability Network"));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()
                                 .allow(Capability::Network)), QUERY, lUrl),
               "FODC0002"));
  CHECK(not_denied(run(z, context(z, SecurityPolicy::secure()
                                     .allow(Capability::Network)
                                     .setAllowPrivateAddresses(true)),
                       QUERY, lUrl)));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()
                                 .allow(Capability::Network)
                                 .setAllowPrivateAddresses(true)
                                 .addNetworkPrefix("http://example.test/")),
                   QUERY, lUrl),
               "FODC0002", "not under an allowed network prefix"));
}

void test_imports(Zorba* z, const Files& f)
{
  StaticContext_t sec = context(z, SecurityPolicy::secure());
  StaticContext_t inData = context(z, SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.data.string()));

  // Modules from the trusted URI path need no capability.
  CHECK(allowed(run(z, sec,
      "import module namespace f = \"http://zorba.io/modules/fetch\";\n"
      "1"), "1"));

  // A module from a file elsewhere needs FileRead.
  std::string const lModule =
    "import module namespace m = \"http://example.test/m\" at \"" +
    f.uri("data/m.xq") + "\";\nm:f()";
  CHECK(denied(run(z, sec, lModule),
               "XQST0059", "requires capability FileRead"));
  CHECK(allowed(run(z, inData, lModule), "42"));

  // A module over the network needs RemoteImport (and Network).
  std::string const lRemote =
    "import module namespace r = \"http://example.test/r\" at "
    "\"http://8.8.8.8/r.xq\";\n1";
  CHECK(denied(run(z, sec, lRemote),
               "XQST0059", "requires capability RemoteImport"));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()
                                 .allow(Capability::Network)), lRemote),
               "XQST0059",
               "requires capability RemoteImport"));

#ifndef ZORBA_NO_XMLSCHEMA
  std::string const lSchema =
    "import schema namespace s = \"http://example.test/s\" at \"" +
    f.uri("data/s.xsd") + "\";\n1";
  CHECK(denied(run(z, sec, lSchema),
               "XQST0059", "requires capability FileRead"));
  CHECK(allowed(run(z, inData, lSchema), "1"));
#endif
}

// JSound schema imports and $constraints run under the policy of the query
// that calls jsd:validate()/jsd:annotate(), not under the instance policy.
std::string jsound_import(const std::string& aLocation)
{
  return
    "jsoniq version \"1.0\";\n"
    "import module namespace jsd = \"http://jsound.io/modules/jsound\";\n"
    "jsd:validate({ \"$namespace\" : \"http://example.test/main\",\n"
    "  \"$imports\" : [ { \"$namespace\" : \"http://example.test/t\",\n"
    "    \"$prefix\" : \"t\", \"$location\" : \"" + aLocation + "\" } ],\n"
    "  \"$types\" : [ { \"$name\" : \"u\", \"$kind\" : \"atomic\",\n"
    "    \"$baseType\" : \"t:t\" } ] }, \"u\", 2)";
}

std::string jsound_constraint(const std::string& aUri)
{
  return
    "jsoniq version \"1.0\";\n"
    "import module namespace jsd = \"http://jsound.io/modules/jsound\";\n"
    "jsd:validate({ \"$namespace\" : \"http://example.test/main\",\n"
    "  \"$types\" : [ { \"$name\" : \"u\", \"$kind\" : \"atomic\",\n"
    "    \"$baseType\" : \"integer\",\n"
    "    \"$constraints\" : [ \"fn:exists(fn:doc(\\\"" + aUri +
    "\\\"))\" ] } ] }, \"u\", 2)";
}

void test_jsound(Zorba* z, const Files& f)
{
  StaticContext_t sec = context(z, SecurityPolicy::secure());
  StaticContext_t inData = context(z, SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.data.string()));
  StaticContext_t all = context(z, SecurityPolicy::unrestricted());
  std::string const lImport = jsound_import(f.uri("data/t.jsd"));
  std::string const lConstraint = jsound_constraint(f.uri("outside/secret.xml"));

  // Instance policy secure(): the query's policy decides, and a denial names
  // the capability the $location needs.
  CHECK(denied(run(z, sec, lImport), "SCHEMA_NOT_FOUND",
               "requires capability FileRead"));
  CHECK(allowed(run(z, inData, lImport), "true"));
  CHECK(allowed(run(z, all, lConstraint), "true"));

  // Instance policy unrestricted(): a secure() query cannot read files
  // through a JSound import or constraint.
  z->setSecurityPolicy(SecurityPolicy::unrestricted());
  CHECK(denied(run(z, sec, lImport), "SCHEMA_NOT_FOUND",
               "requires capability FileRead"));
  // A constraint that raises an error makes the instance invalid.
  CHECK(allowed(run(z, sec, lConstraint), "false"));
  CHECK(allowed(run(z, inData, lConstraint), "false"));
  CHECK(allowed(run(z, all, lConstraint), "true"));
  z->setSecurityPolicy(SecurityPolicy::secure());
}

#ifndef ZORBA_NO_FULL_TEXT
// Stop-word lists and thesauri the policy denies are not available: FTST0008
// and FTST0018.
void test_full_text(Zorba* z, const Files& f)
{
  StaticContext_t sec = context(z, SecurityPolicy::secure());
  StaticContext_t inData = context(z, SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.data.string()));

  std::string const lStop =
    "\"inside text\" contains text \"inside\" using stop words at \"" +
    f.uri("data/stop.txt") + "\"";
  CHECK(denied(run(z, sec, lStop), "FTST0008",
               "requires capability FileRead"));
  CHECK(allowed(run(z, inData, lStop), "true"));
  std::string const lOutsideStop =
    "\"inside text\" contains text \"inside\" using stop words at \"" +
    f.uri("outside/secret.txt") + "\"";
  CHECK(denied(run(z, inData, lOutsideStop), "FTST0008",
               "outside the allowed read paths"));

  std::string const lThesaurus =
    "\"inside\" contains text \"inside\" using thesaurus at \"" +
    f.uri("outside/secret.xml") + "\"";
  CHECK(denied(run(z, inData, lThesaurus), "FTST0018"));
}
#endif

// ZORBA_URI_PATH (set before the instance is created) adds to the trusted
// URI path only where the policy allows Environment.
void test_env_uri_path(Zorba* z)
{
  const char QUERY[] =
    "import module namespace e = \"http://example.test/envmod\";\ne:f()";
  Outcome const lSecure = run(z, context(z, SecurityPolicy::secure()), QUERY);
  CHECK(!lSecure.ok);
  if (lSecure.ok)
    std::cerr << "  module from ZORBA_URI_PATH without Environment: "
              << lSecure << std::endl;
  CHECK(allowed(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::Environment)), QUERY),
                "43"));
  CHECK(allowed(run(z, context(z, SecurityPolicy::unrestricted()), QUERY),
                "43"));
}

} // anonymous namespace


int
uri_access_security(int argc, char* argv[])
{
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

  // Read by the root static context when the instance is created.
  ::setenv("ZORBA_URI_PATH", lFiles->path("env").c_str(), 1);

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    test_check_files(*lFiles);
    test_check_network();
    test_documents(lZorba, *lFiles);
    test_document_cache(lZorba, *lFiles);
    test_network(lZorba);
    test_imports(lZorba, *lFiles);
    test_jsound(lZorba, *lFiles);
#ifndef ZORBA_NO_FULL_TEXT
    test_full_text(lZorba, *lFiles);
#endif
    test_env_uri_path(lZorba);
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
