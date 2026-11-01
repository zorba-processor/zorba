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

// Enforcement of the SecurityPolicy (include/zorba/security.h) in modules:
// the import gate (file, http-client, ftp-client), NativeModules for native
// libraries, the file module's path checks, the HTTP client (URL, redirects,
// the address of every connection), the environment functions, the
// http-uri-resolution option, WordNet thesauri, the zorba-query module, and
// SecurityPolicy::checkConnection() / openFile().

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include <zorba/config.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
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
const char ENV_NAME[] = "ZORBA_MODULE_SECURITY_TEST_VAR";

// root/
//   data/in.txt
//   data/link.txt -> ../outside/secret.txt
//   data/ldir -> ../outside
//   outside/secret.txt
struct Files
{
  sfs::path root;

  Files()
  {
    root = sfs::temp_directory_path() /
      ("zorba-module-security-" + std::to_string(std::rand()) + "-" +
       std::to_string(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
    sfs::create_directories(root / "data");
    sfs::create_directories(root / "outside");
    write("data/in.txt", "inside-text");
    write("outside/secret.txt", SECRET);
    sfs::create_symlink(root / "outside" / "secret.txt",
                        root / "data" / "link.txt");
    sfs::create_directory_symlink(root / "outside", root / "data" / "ldir");
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

// Compiles and runs aQuery; "$uri" in the query is bound to aUri.
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

bool failed_with(const Outcome& o, const char* aCode,
                 const std::string& aText = std::string())
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

// A denial by the security policy is reported with the error code the
// function documents for a resource it cannot access (aCode); the message
// names the policy and the reason.
bool denied(const Outcome& o, const char* aCode,
            const std::string& aReason = std::string())
{
  return failed_with(o, aCode, aReason) &&
         failed_with(o, aCode, "access denied by security policy");
}

bool not_denied(const Outcome& o)
{
  if (!o.ok && (o.code == "ZXQP0029" || o.code == "ZXQP0067" ||
                o.message.find("access denied by security policy") !=
                  std::string::npos))
  {
    std::cerr << "  expected no denial, got " << o << std::endl;
    return false;
  }
  return true;
}

const char FILE_NS[] =
  "import module namespace file = \"http://expath.org/ns/file\";\n";
const char HTTP_NS[] =
  "import module namespace http = \"http://zorba.io/modules/http-client\";\n";
const char FTP_NS[] =
  "import module namespace ftp = \"http://zorba.io/modules/ftp-client\";\n";
const char ZQ_NS[] =
  "import module namespace zq = \"http://zorba.io/modules/zorba-query\";\n";
const char URI_VAR[] = "declare variable $uri as xs:string external;\n";


////////////////////////////////////////////////////////////////////////////////

// Modules that reach the OS need their capability to be imported, and
// native libraries need NativeModules.
void test_import_gate(Zorba* z)
{
  std::string const lFile = std::string(FILE_NS) + "file:directory-separator()";
  std::string const lHttp = std::string(HTTP_NS) + "1";
  std::string const lFtp = std::string(FTP_NS) + "1";

  StaticContext_t sec = context(z, SecurityPolicy::secure());
  CHECK(failed_with(run(z, sec, lFile), "ZXQP0067", "FileRead or FileWrite"));
  CHECK(failed_with(run(z, sec, lHttp), "ZXQP0067", "Network"));
  CHECK(failed_with(run(z, sec, lFtp), "ZXQP0067", "Network"));

  // The capability lets the import through; the native library still needs
  // NativeModules.
  CHECK(failed_with(run(z, context(z, SecurityPolicy::secure()
                                      .allow(Capability::FileRead)), lFile),
                    "ZXQP0067", "NativeModules"));
#ifdef ZORBA_HAVE_CURL
  CHECK(failed_with(run(z, context(z, SecurityPolicy::secure()
                                      .allow(Capability::Network)), lHttp),
                    "ZXQP0067", "NativeModules"));
#endif
  CHECK(failed_with(run(z, context(z, SecurityPolicy::secure()
                                      .allow(Capability::NativeModules)),
                        lFile), "ZXQP0067", "FileRead or FileWrite"));

  std::string const lSep(1, sfs::path::preferred_separator);
  CHECK(allowed(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::FileWrite)
                                  .allow(Capability::NativeModules)), lFile),
                lSep));
#ifdef ZORBA_HAVE_CURL
  CHECK(allowed(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::Network)
                                  .allow(Capability::NativeModules)), lHttp),
                "1"));
#endif

  // A native module without a capability of its own (zorba-query).
  std::string const lZq = std::string(ZQ_NS) +
    "variable $q := zq:prepare-main-module(\"1 + 1\");\nzq:evaluate($q)";
  CHECK(failed_with(run(z, sec, lZq), "ZXQP0067", "NativeModules"));
  CHECK(allowed(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::NativeModules)), lZq),
                "2"));

  // Unrestricted: nothing changes.
  StaticContext_t all = context(z, SecurityPolicy::unrestricted());
  CHECK(allowed(run(z, all, lFile), lSep));
#ifdef ZORBA_HAVE_CURL
  CHECK(allowed(run(z, all, lHttp), "1"));
#endif
  CHECK(allowed(run(z, all, lZq), "2"));
}


////////////////////////////////////////////////////////////////////////////////

// A policy that lists native modules loads native libraries only for the
// modules in the list; an empty list allows every native module.
void test_native_module_list(Zorba* z)
{
  std::string const lFile = std::string(FILE_NS) + "file:directory-separator()";
  std::string const lZq = std::string(ZQ_NS) +
    "variable $q := zq:prepare-main-module(\"1 + 1\");\nzq:evaluate($q)";
  std::string const lSep(1, sfs::path::preferred_separator);

  SecurityPolicy const lFileOnly = SecurityPolicy::secure()
    .allow(Capability::FileRead)
    .allow(Capability::NativeModules)
    .addNativeModule("http://expath.org/ns/file");
  CHECK(allowed(run(z, context(z, lFileOnly), lFile), lSep));
  CHECK(failed_with(run(z, context(z, lFileOnly), lZq), "ZXQP0067",
                    "http://zorba.io/modules/zorba-query"));
  CHECK(failed_with(run(z, context(z, lFileOnly), lZq), "ZXQP0067",
                    "NativeModules"));

  // The list does not grant NativeModules by itself.
  CHECK(failed_with(run(z, context(z, SecurityPolicy::secure()
                                      .allow(Capability::FileRead)
                                      .addNativeModule(
                                          "http://expath.org/ns/file")),
                        lFile), "ZXQP0067", "NativeModules"));

  // Both modules listed: both load.
  StaticContext_t lBoth = context(z, SecurityPolicy(lFileOnly)
      .addNativeModule("http://zorba.io/modules/zorba-query"));
  CHECK(allowed(run(z, lBoth, lFile), lSep));
  CHECK(allowed(run(z, lBoth, lZq), "2"));

  // A query prepared through zorba-query inherits the list: the inner query
  // cannot load a native module that the caller's policy does not list.
  std::string const lInner = std::string(ZQ_NS) +
    "variable $q := zq:prepare-main-module("
    "\"import module namespace s = 'http://zorba.io/modules/sleep'; 1\");\n"
    "zq:evaluate($q)";
  CHECK(failed_with(run(z, lBoth, lInner), "ZXQP0067",
                    "http://zorba.io/modules/sleep"));

  // Empty list: every native module (backward compatible).
  StaticContext_t lAny = context(z, SecurityPolicy::secure()
                                      .allow(Capability::FileRead)
                                      .allow(Capability::NativeModules));
  CHECK(allowed(run(z, lAny, lFile), lSep));
  CHECK(allowed(run(z, lAny, lZq), "2"));
}


////////////////////////////////////////////////////////////////////////////////

void test_file_module(Zorba* z, const Files& f)
{
  SecurityPolicy const lBase =
    SecurityPolicy::secure().allow(Capability::NativeModules);
  StaticContext_t rd = context(z, SecurityPolicy(lBase)
      .allow(Capability::FileRead).addReadPath(f.path("data")));
  StaticContext_t wr = context(z, SecurityPolicy(lBase)
      .allow(Capability::FileRead).addReadPath(f.path("data"))
      .allow(Capability::FileWrite).addWritePath(f.path("data")));

  std::string const READ = std::string(FILE_NS) + URI_VAR + "file:read-text($uri)";
  std::string const LINES =
    std::string(FILE_NS) + URI_VAR + "file:read-text-lines($uri)";
  std::string const BINARY = std::string(FILE_NS) + URI_VAR +
    "string(xs:hexBinary(file:read-binary($uri)))";
  std::string const EXISTS = std::string(FILE_NS) + URI_VAR + "file:exists($uri)";
  std::string const SIZE = std::string(FILE_NS) + URI_VAR + "file:size($uri)";
  std::string const LIST = std::string(FILE_NS) + URI_VAR +
    "string-join(for $f in file:list($uri) order by $f return $f, ',')";
  std::string const WRITE = std::string(FILE_NS) + URI_VAR +
    "file:write-text($uri, \"written\");\nfile:read-text($uri)";
  std::string const MKDIR = std::string(FILE_NS) + URI_VAR +
    "file:create-directory($uri);\nfile:is-directory($uri)";
  std::string const DELETE = std::string(FILE_NS) + URI_VAR +
    "file:delete($uri);\nfile:exists($uri)";

  // Reads inside the read path.
  CHECK(allowed(run(z, rd, READ, f.path("data/in.txt")), "inside-text"));
  CHECK(allowed(run(z, rd, LINES, f.path("data/in.txt")), "inside-text"));
  CHECK(allowed(run(z, rd, EXISTS, f.path("data/in.txt")), "true"));
  CHECK(allowed(run(z, rd, EXISTS, f.path("data/missing.txt")), "false"));
  CHECK(allowed(run(z, rd, SIZE, f.path("data/in.txt")), "11"));
  CHECK(allowed(run(z, rd, BINARY, f.path("data/in.txt")),
                "696E736964652D74657874"));
  CHECK(allowed(run(z, rd, LIST, f.path("data")), "in.txt,ldir,link.txt"));

  // Reads outside: directly, through "..", through symbolic links.
  for (const char* lOut : {"outside/secret.txt", "data/../outside/secret.txt",
                           "data/link.txt", "data/ldir/secret.txt"})
  {
    CHECK(denied(run(z, rd, READ, f.path(lOut)), "FOFL9999",
                 "outside the allowed read paths"));
    CHECK(denied(run(z, rd, LINES, f.path(lOut)), "FOFL9999"));
    CHECK(denied(run(z, rd, BINARY, f.path(lOut)), "FOFL9999"));
    CHECK(denied(run(z, rd, EXISTS, f.path(lOut)), "FOFL9999"));
    CHECK(denied(run(z, rd, SIZE, f.path(lOut)), "FOFL9999"));
  }
  CHECK(denied(run(z, rd, LIST, f.path("outside")), "FOFL9999"));
  CHECK(denied(run(z, rd, LIST, f.path("data/ldir")), "FOFL9999"));

  // Writes need FileWrite and a write path.
  CHECK(denied(run(z, rd, WRITE, f.path("data/new.txt")), "FOFL9999",
               "requires capability FileWrite"));
  CHECK(!sfs::exists(f.root / "data" / "new.txt"));
  CHECK(allowed(run(z, wr, WRITE, f.path("data/new.txt")), "written"));
  CHECK(denied(run(z, wr, WRITE, f.path("outside/new.txt")), "FOFL9999",
               "outside the allowed write paths"));
  CHECK(denied(run(z, wr, WRITE, f.path("data/ldir/new.txt")), "FOFL9999"));
  CHECK(!sfs::exists(f.root / "outside" / "new.txt"));
  CHECK(allowed(run(z, wr, MKDIR, f.path("data/sub")), "true"));
  CHECK(denied(run(z, wr, MKDIR, f.path("outside/sub")), "FOFL9999"));
  CHECK(denied(run(z, wr, DELETE, f.path("outside/secret.txt")), "FOFL9999"));
  CHECK(sfs::exists(f.root / "outside" / "secret.txt"));
  CHECK(allowed(run(z, wr, DELETE, f.path("data/new.txt")), "false"));

  // Copy: the source is read, the destination written.
  std::string const COPY = std::string(FILE_NS) +
    "declare variable $src as xs:string external;\n" + URI_VAR +
    "file:copy($src, $uri);\nfile:read-text($uri)";
  auto lCopy = [&](StaticContext_t aSctx, const std::string& aSrc,
                   const std::string& aDst) {
    Outcome o;
    try
    {
      XQuery_t q = z->compileQuery(COPY, aSctx);
      q->getDynamicContext()->setVariable(
          "src", z->getItemFactory()->createString(aSrc));
      q->getDynamicContext()->setVariable(
          "uri", z->getItemFactory()->createString(aDst));
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
  };
  CHECK(allowed(lCopy(wr, f.path("data/in.txt"), f.path("data/copy.txt")),
                "inside-text"));
  CHECK(denied(lCopy(wr, f.path("data/link.txt"), f.path("data/copy2.txt")),
               "FOFL9999"));
  CHECK(denied(lCopy(wr, f.path("data/in.txt"), f.path("outside/copy.txt")),
               "FOFL9999"));
  CHECK(!sfs::exists(f.root / "data" / "copy2.txt"));
  CHECK(!sfs::exists(f.root / "outside" / "copy.txt"));

  // Unrestricted: nothing changes.
  StaticContext_t all = context(z, SecurityPolicy::unrestricted());
  CHECK(allowed(run(z, all, READ, f.path("data/link.txt")), SECRET));
}

// SecurityPolicy::openFile() checks the file actually opened.
void test_open_file(const Files& f)
{
  SecurityPolicy const p = SecurityPolicy::secure()
      .allow(Capability::FileRead).addReadPath(f.path("data"));
  String lReason;
  std::unique_ptr<std::istream> lIn = p.openFile(f.path("data/in.txt"), true,
                                                 &lReason);
  CHECK(lIn && lReason.empty());
  if (lIn)
  {
    std::string lContent;
    std::getline(*lIn, lContent);
    CHECK(lContent == "inside-text");
    // seekable
    lIn->clear();
    lIn->seekg(2);
    std::getline(*lIn, lContent);
    CHECK(lContent == "side-text");
  }
  for (const char* lOut : {"outside/secret.txt", "data/link.txt",
                           "data/ldir/secret.txt"})
  {
    lReason = "";
    CHECK(!p.openFile(f.path(lOut), true, &lReason));
    CHECK(!lReason.empty());
  }
  lReason = "";
  CHECK(!p.openFile(f.path("data/missing.txt"), true, &lReason));
  CHECK(lReason.empty());
  lReason = "";
  CHECK(!SecurityPolicy::secure().openFile(f.path("data/in.txt"), true,
                                           &lReason));
  CHECK(lReason.str().find("FileRead") != std::string::npos);
  CHECK(SecurityPolicy::unrestricted().openFile(f.path("data/link.txt"), true));
}


////////////////////////////////////////////////////////////////////////////////

void test_check_connection()
{
  SecurityPolicy const n = SecurityPolicy::secure().allow(Capability::Network);
  CHECK(!SecurityPolicy::secure().checkConnection("http://example.test/",
                                                  "93.184.216.34"));
  CHECK(n.checkConnection("http://example.test/", "93.184.216.34"));
  CHECK(n.checkConnection("https://example.test/", "2606:2800:220:1::1"));
  CHECK(n.checkConnection("ftps://example.test/", "93.184.216.34"));
  // The name resolved to a public address when the URL was checked; the
  // connection goes to a private one (DNS rebinding).
  for (const char* lPrivate : {"127.0.0.1", "10.0.0.1", "169.254.169.254",
                               "192.168.0.1", "::1", "fe80::1", "fe80::1%2",
                               "::ffff:127.0.0.1", "fd00::1", "0.0.0.0",
                               "not-an-address", ""})
  {
    String lReason;
    if (n.checkConnection("http://example.test/", lPrivate, &lReason) ||
        lReason.empty())
    {
      std::cerr << "  connection to " << lPrivate << " allowed" << std::endl;
      ++theFailures;
    }
  }
  CHECK(SecurityPolicy(n).setAllowPrivateAddresses(true)
        .checkConnection("http://example.test/", "127.0.0.1"));
  CHECK(SecurityPolicy::unrestricted().checkConnection("http://x.test/",
                                                       "127.0.0.1"));

  // Prefixes apply to every URL fetched, e.g. a redirect target.
  SecurityPolicy const pre = SecurityPolicy(n)
      .addNetworkPrefix("https://api.example.test/v1/");
  CHECK(pre.checkConnection("https://api.example.test/v1/x", "93.184.216.34"));
  CHECK(!pre.checkConnection("https://api.example.test/v2/x",
                             "93.184.216.34"));
  CHECK(!pre.checkConnection("https://evil.test/v1/x", "93.184.216.34"));
  CHECK(!pre.checkConnection("http://api.example.test/v1/x", "93.184.216.34"));
  CHECK(!pre.checkConnection("file:///etc/passwd", "93.184.216.34"));
  CHECK(!pre.checkConnection("https://api.example.test/v1/x", "10.0.0.1"));

  // A prefix that names a private address allows connections for it, and
  // only for URLs under it.
  SecurityPolicy const lit = SecurityPolicy(n)
      .addNetworkPrefix("http://10.1.2.3:8080/")
      .addNetworkPrefix("http://name.test:8080/");
  CHECK(lit.checkConnection("http://10.1.2.3:8080/a", "10.1.2.3"));
  CHECK(!lit.checkConnection("http://name.test:8080/a", "10.1.2.3"));
  CHECK(lit.checkConnection("http://name.test:8080/a", "93.184.216.34"));
}


////////////////////////////////////////////////////////////////////////////////

void test_environment(Zorba* z)
{
  const char GET[] = "fn:environment-variable(\"ZORBA_MODULE_SECURITY_TEST_VAR\")";
  const char ALL[] =
    "count(fn:available-environment-variables()"
    "[. = \"ZORBA_MODULE_SECURITY_TEST_VAR\"])";

  StaticContext_t sec = context(z, SecurityPolicy::secure());
  CHECK(allowed(run(z, sec, GET), ""));
  CHECK(allowed(run(z, sec, std::string("empty(") + GET + ")"), "true"));
  CHECK(allowed(run(z, sec, ALL), "0"));
  CHECK(allowed(run(z, sec, "count(fn:available-environment-variables())"),
                "0"));

  StaticContext_t env = context(z, SecurityPolicy::secure()
                                       .allow(Capability::Environment));
  CHECK(allowed(run(z, env, GET), SECRET));
  CHECK(allowed(run(z, env, ALL), "1"));
  CHECK(allowed(run(z, context(z, SecurityPolicy::unrestricted()), GET),
                SECRET));
}


////////////////////////////////////////////////////////////////////////////////

// The http-uri-resolution option (disabled through the API) is
// Capability::RemoteImport.
void test_http_uri_resolution(Zorba* z)
{
  Item const lDisable = z->getItemFactory()->createQName(
      "http://zorba.io/options/features", "", "disable");
  Item const lEnable = z->getItemFactory()->createQName(
      "http://zorba.io/options/features", "", "enable");

  StaticContext_t sctx = context(z, SecurityPolicy::unrestricted());
  CHECK(sctx->getSecurityPolicy().allows(Capability::RemoteImport));
  sctx->declareOption(lDisable, "http-uri-resolution");
  SecurityPolicy const p = sctx->getSecurityPolicy();
  CHECK(!p.allows(Capability::RemoteImport));
  CHECK(p.allows(Capability::Network));
  CHECK(p.allows(Capability::FileRead));

  std::string const lRemote =
    "import module namespace r = \"http://example.test/r\" at "
    "\"http://8.8.8.8/r.xq\";\n1";
  CHECK(denied(run(z, sctx, lRemote),
               "XQST0059", "requires capability RemoteImport"));
  // A child context (and so every query compiled with it) inherits it.
  CHECK(denied(run(z, sctx->createChildContext(), lRemote), "XQST0059",
               "requires capability RemoteImport"));

  // Enabling the option cannot widen the policy.
  sctx->declareOption(lEnable, "http-uri-resolution");
  CHECK(!sctx->getSecurityPolicy().allows(Capability::RemoteImport));

  // Other contexts are not affected (the option used to switch the feature
  // off for the whole process).
  CHECK(z->createStaticContext()->getSecurityPolicy() ==
        z->getSecurityPolicy());
  CHECK(context(z, SecurityPolicy::unrestricted())->getSecurityPolicy()
        .allows(Capability::RemoteImport));

  // Queries cannot set it.
  CHECK(failed_with(run(z, context(z, SecurityPolicy::unrestricted()),
      "declare namespace op = \"http://zorba.io/options/features\";\n"
      "declare option op:disable \"http-uri-resolution\";\n1"), "ZXQP0061"));
}


////////////////////////////////////////////////////////////////////////////////

// The built-in thesaurus (WordNet data in the library path, "##default" or
// its URL) is engine data: it needs no capability. Other wordnet: URIs name a
// data file ("wordnet-en.zth") in the library path, not a native library: they
// need FileRead with no read-path list (the library path is not on a list),
// and a thesaurus the query names by file path needs FileRead.
void test_wordnet(Zorba* z, const Files& f)
{
  CHECK(SecurityPolicy::secure().checkURI("##default",
                                          EntityData::THESAURUS));
  CHECK(SecurityPolicy::secure().checkURI("wordnet://wordnet.princeton.edu/",
                                          EntityData::THESAURUS));
  CHECK(!SecurityPolicy::secure().checkURI("wordnet://other.example/",
                                           EntityData::THESAURUS));
  CHECK(!SecurityPolicy::secure().allow(Capability::NativeModules)
        .checkURI("wordnet://other.example/", EntityData::THESAURUS));
  CHECK(SecurityPolicy::secure().allow(Capability::FileRead)
        .checkURI("wordnet://other.example/", EntityData::THESAURUS));
  CHECK(!SecurityPolicy::secure().allow(Capability::FileRead)
        .addReadPath(f.path("data"))
        .checkURI("wordnet://other.example/", EntityData::THESAURUS));
  CHECK(SecurityPolicy::secure().allow(Capability::FileRead)
        .addReadPath(f.path("data"))
        .checkURI("##default", EntityData::THESAURUS));

#ifndef ZORBA_NO_FULL_TEXT
  const char QUERY[] =
    "<msg>affluent man</msg> contains text \"wealthy\" using thesaurus default";
  // No denial (the WordNet data may not be installed).
  CHECK(not_denied(run(z, context(z, SecurityPolicy::secure()), QUERY)));
  CHECK(not_denied(run(z, context(z, SecurityPolicy::secure()),
      "<msg>affluent man</msg> contains text \"wealthy\" using thesaurus at "
      "\"wordnet://wordnet.princeton.edu/\"")));

  std::string const lUserThesaurus =
    "<msg>affluent man</msg> contains text \"wealthy\" using thesaurus at \"" +
    f.uri("outside/secret.txt") + "\"";
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()), lUserThesaurus),
               "FTST0018", "requires capability FileRead"));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()),
      "<msg>affluent man</msg> contains text \"wealthy\" using thesaurus at "
      "\"wordnet://other.example/\""),
               "FTST0018", "requires capability FileRead"));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::NativeModules)),
      "<msg>affluent man</msg> contains text \"wealthy\" using thesaurus at "
      "\"wordnet://other.example/\""),
               "FTST0018", "requires capability FileRead"));
  CHECK(denied(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::FileRead)
                                  .addReadPath(f.path("data"))),
      "<msg>affluent man</msg> contains text \"wealthy\" using thesaurus at "
      "\"wordnet://other.example/\""),
               "FTST0018", "access denied by security policy"));
#else
  (void)z;
  (void)f;
#endif
}


////////////////////////////////////////////////////////////////////////////////

// Queries prepared with the zorba-query module run under the caller's
// policy, not the instance default.
void test_zorba_query(Zorba* z, const Files& f)
{
  StaticContext_t zqOnly = context(z, SecurityPolicy::secure()
                                          .allow(Capability::NativeModules));
  std::string const lRead = std::string(ZQ_NS) + URI_VAR +
    "variable $q := zq:prepare-main-module("
    "concat(\"fn:unparsed-text('\", $uri, \"')\"));\n"
    "zq:evaluate($q)";
  CHECK(denied(run(z, zqOnly, lRead, f.uri("data/in.txt")), "FOUT1170"));
  CHECK(allowed(run(z, context(z, SecurityPolicy::secure()
                                  .allow(Capability::NativeModules)
                                  .allow(Capability::FileRead)
                                  .addReadPath(f.path("data"))),
                    lRead, f.uri("data/in.txt")), "inside-text"));

  std::string const lImport = std::string(ZQ_NS) +
    "variable $q := zq:prepare-main-module("
    "\"import module namespace file = 'http://expath.org/ns/file'; 1\");\n"
    "zq:evaluate($q)";
  Outcome const o = run(z, zqOnly, lImport);
  CHECK(!o.ok && o.code == "ZXQP0067");
  if (o.ok || o.code != "ZXQP0067")
    std::cerr << "  expected ZXQP0067, got " << o << std::endl;

  // A serialized plan carries its own policy: loading one needs a caller
  // that may do everything anyway.
  std::string const lLoad = std::string(ZQ_NS) +
    "zq:load-from-query-plan(xs:base64Binary(\"AAAA\"))";
  CHECK(failed_with(run(z, zqOnly, lLoad), "ZXQP0067",
                    "zq:load-from-query-plan"));
  Outcome const lAll = run(z, context(z, SecurityPolicy::unrestricted()), lLoad);
  CHECK(lAll.code != "ZXQP0067");
}


////////////////////////////////////////////////////////////////////////////////

// The HTTP client checks the URL, every redirect target and the address of
// every connection. Needs the HTTP test server; skipped without it.
void test_http_client(Zorba* z)
{
#ifndef ZORBA_HAVE_CURL
  (void)z;
  std::cerr << "  built without cURL, skipping HTTP client tests" << std::endl;
}
#else
  std::string const lServer = http_test_server();
  std::string const lBase = "http://" + lServer;
  std::string const GET = std::string(HTTP_NS) + URI_VAR +
    "http:get-text($uri)(\"status\")";
  std::string const TEXT = std::string(URI_VAR) +
    "contains(fn:unparsed-text($uri), \"simpletest\")";
  std::string const XGET =
    "import module namespace xh = "
    "\"http://www.zorba-xquery.com/modules/http-client\";\n" +
    std::string(URI_VAR) + "string(xh:get-text($uri)[1]/@status)";

  StaticContext_t all = context(z, SecurityPolicy::unrestricted());
  Outcome const lProbe =
    run(z, all, GET, lBase + "/http-test-data/http1.xml");
  if (!lProbe.ok || lProbe.value != "200")
  {
    std::cerr << "  HTTP test server not available at " << lServer
              << ", skipping HTTP client tests (" << lProbe << ")"
              << std::endl;
    return;
  }

  std::string const lRedirect = lBase + "/http-test-data/redirect.php?to=";
  std::string const lInside = lBase + "/http-test-data/http1.xml";
  std::string const lOutside = lBase + "/cgi-bin/test-text";
  CHECK(allowed(run(z, all, GET, lRedirect + lOutside), "200"));

  SecurityPolicy const lNet = SecurityPolicy::secure()
      .allow(Capability::Network).allow(Capability::NativeModules);

  // The server is on a private address.
  CHECK(denied(run(z, context(z, lNet), GET, lInside), "HTTP"));

  // A redirect out of the allowed prefix is refused, for the http-client
  // module and for fn:doc (which uses it).
  StaticContext_t pre = context(z, SecurityPolicy(lNet)
      .setAllowPrivateAddresses(true)
      .addNetworkPrefix(lBase + "/http-test-data/"));
  CHECK(allowed(run(z, pre, GET, lInside), "200"));
  CHECK(allowed(run(z, pre, GET, lRedirect + lInside), "200"));
  CHECK(denied(run(z, pre, GET, lRedirect + lOutside), "HTTP",
               "not under an allowed network prefix"));
  CHECK(denied(run(z, pre, GET, lOutside), "HTTP"));
  CHECK(allowed(run(z, pre, TEXT, lInside), "true"));
  // Through the EXPath interface, the denial is error:HC001.
  CHECK(denied(run(z, pre, XGET, lRedirect + lOutside), "HC001",
               "not under an allowed network prefix"));
  CHECK(denied(run(z, pre, TEXT, lRedirect + lOutside), "FOUT1170"));

  // Other protocols are refused, also as redirect targets (curl refuses
  // those: an error, but not necessarily a denial by the policy).
  CHECK(denied(run(z, pre, GET, "file:///etc/hostname"), "HTTP"));
  Outcome const lToFile = run(z, pre, GET, lRedirect + "file:///etc/hostname");
  CHECK(!lToFile.ok);
  if (lToFile.ok)
    std::cerr << "  redirect to file: followed: " << lToFile << std::endl;

  // The address of each connection is checked: a prefix naming the server's
  // address literally allows it; a redirect to the same server under its
  // host name (which is not allowed to be private) is refused when curl
  // connects, without any DNS check by the policy.
  std::string const lHost = lServer.substr(0, lServer.rfind(':'));
  std::string const lPort = lServer.substr(lServer.rfind(':'));
  if (lHost != "localhost" && lHost.find_first_not_of("0123456789.") !=
                                std::string::npos)
  {
    addrinfo lHints;
    std::memset(&lHints, 0, sizeof lHints);
    lHints.ai_family = AF_INET;
    lHints.ai_socktype = SOCK_STREAM;
    addrinfo* lRes = nullptr;
    if (::getaddrinfo(lHost.c_str(), nullptr, &lHints, &lRes) == 0 && lRes)
    {
      char lIp[INET_ADDRSTRLEN] = "";
      ::inet_ntop(AF_INET,
                  &reinterpret_cast<sockaddr_in*>(lRes->ai_addr)->sin_addr,
                  lIp, sizeof lIp);
      ::freeaddrinfo(lRes);
      std::string const lByIp = "http://" + std::string(lIp) + lPort;
      StaticContext_t lit = context(z, SecurityPolicy(lNet)
          .addNetworkPrefix(lByIp + "/")
          .addNetworkPrefix(lBase + "/"));
      CHECK(allowed(run(z, lit, GET, lByIp + "/http-test-data/http1.xml"),
                    "200"));
      CHECK(denied(run(z, lit, GET,
                       lByIp + "/http-test-data/redirect.php?to=" + lInside),
                   "HTTP",
                   "private or link-local address"));
      CHECK(allowed(run(z, context(z, SecurityPolicy(lNet)
                                      .setAllowPrivateAddresses(true)),
                        GET,
                        lByIp + "/http-test-data/redirect.php?to=" + lInside),
                    "200"));
    }
    else
      std::cerr << "  cannot resolve " << lHost << std::endl;
  }
}
#endif /* ZORBA_HAVE_CURL */

} // anonymous namespace


int
module_security(int argc, char* argv[])
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

  ::setenv(ENV_NAME, SECRET, 1);

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    test_check_connection();
    test_open_file(*lFiles);
    test_import_gate(lZorba);
    test_native_module_list(lZorba);
    test_file_module(lZorba, *lFiles);
    test_environment(lZorba);
    test_http_uri_resolution(lZorba);
    test_wordnet(lZorba, *lFiles);
    test_zorba_query(lZorba, *lFiles);
    test_http_client(lZorba);
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
