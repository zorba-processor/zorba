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

// Tests for the SecurityPolicy API (include/zorba/security.h): defaults,
// inheritance through static contexts, embedder widening, copy semantics and
// plan serialization. That engine-created contexts cannot widen is tested in
// src/unit_tests/test_security_policy.cpp (needs the internal API).

#include <chrono>
#include <iostream>
#include <sstream>

#include <zorba/config.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
using std::chrono::milliseconds;
using std::chrono::seconds;

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

const Capability ALL_CAPS[] = {
  Capability::ExternalEntities,
  Capability::FileRead,
  Capability::FileWrite,
  Capability::Network,
  Capability::RemoteImport,
  Capability::Environment,
  Capability::NativeModules
};

[[maybe_unused]] bool isFileCap(Capability c)
{
  return c == Capability::FileRead || c == Capability::FileWrite;
}

void test_defaults()
{
  SecurityPolicy s = SecurityPolicy::secure();
  for (Capability c : ALL_CAPS)
    CHECK(!s.allows(c));
  CHECK(s.getReadPaths().empty());
  CHECK(s.getWritePaths().empty());
  CHECK(s.getNetworkPrefixes().empty());
  CHECK(s.getNativeModules().empty());
  CHECK(!s.allowsPrivateAddresses());
  CHECK(s.getTimeout() == milliseconds(0));
  CHECK(s.getMaxCallDepth() == 1024);
  CHECK(s.getMaxEntityAmplification() == 0);

  CHECK(SecurityPolicy() == SecurityPolicy::secure());

  SecurityPolicy u = SecurityPolicy::unrestricted();
  for (Capability c : ALL_CAPS)
  {
#ifdef ZORBA_WITH_FILE_ACCESS
    CHECK(u.allows(c));
#else
    CHECK(u.allows(c) == !isFileCap(c));
#endif
  }
  CHECK(u.getReadPaths().empty());
  CHECK(u.getWritePaths().empty());
  CHECK(u.getNetworkPrefixes().empty());
  CHECK(u.getNativeModules().empty());
  CHECK(u.allowsPrivateAddresses());
  CHECK(u.getTimeout() == milliseconds(0));
  CHECK(u.getMaxCallDepth() == 1024);
  CHECK(u.getMaxEntityAmplification() == 0);
  CHECK(u != s);
}

void test_modifiers()
{
  SecurityPolicy p;
  p.allow(Capability::Network).allow(Capability::Environment);
  CHECK(p.allows(Capability::Network));
  CHECK(p.allows(Capability::Environment));
  CHECK(!p.allows(Capability::RemoteImport));
  p.deny(Capability::Network);
  CHECK(!p.allows(Capability::Network));
  CHECK(p.allows(Capability::Environment));

  p.allow(Capability::FileRead);
#ifdef ZORBA_WITH_FILE_ACCESS
  CHECK(p.allows(Capability::FileRead));
#else
  CHECK(!p.allows(Capability::FileRead));
#endif

  p.setTimeout(milliseconds(-5));
  CHECK(p.getTimeout() == milliseconds(0));
  p.setTimeout(seconds(3));
  CHECK(p.getTimeout() == milliseconds(3000));
  p.setMaxCallDepth(0);
  CHECK(p.getMaxCallDepth() == 0);
  p.setMaxEntityAmplification(7);
  CHECK(p.getMaxEntityAmplification() == 7);
  p.setAllowPrivateAddresses(true);
  CHECK(p.allowsPrivateAddresses());

  p.addReadPath("/srv/data").addWritePath("/srv/out")
   .addNetworkPrefix("https://api.example.com/v1/");
  CHECK(p.getReadPaths().size() == 1 && p.getReadPaths()[0] == "/srv/data");
  CHECK(p.getWritePaths().size() == 1 && p.getWritePaths()[0] == "/srv/out");
  CHECK(p.getNetworkPrefixes().size() == 1 &&
        p.getNetworkPrefixes()[0] == "https://api.example.com/v1/");

  p.addNativeModule("http://expath.org/ns/file")
   .addNativeModule("http://zorba.io/modules/sleep");
  CHECK(p.getNativeModules().size() == 2 &&
        p.getNativeModules()[0] == "http://expath.org/ns/file" &&
        p.getNativeModules()[1] == "http://zorba.io/modules/sleep");
}

void test_copy_semantics()
{
  SecurityPolicy a = SecurityPolicy::secure();
  a.allow(Capability::Network).addNetworkPrefix("https://a.example/");

  SecurityPolicy b(a);
  CHECK(b == a);
  b.deny(Capability::Network).addNetworkPrefix("https://b.example/");
  CHECK(a.allows(Capability::Network));
  CHECK(a.getNetworkPrefixes().size() == 1);
  CHECK(b != a);

  SecurityPolicy c = SecurityPolicy::unrestricted();
  c = a;
  CHECK(c == a);
  c.setTimeout(seconds(1));
  CHECK(a.getTimeout() == milliseconds(0));

  // The native module list is copied and compared like the other lists;
  // order and duplicates do not matter.
  SecurityPolicy m1 = SecurityPolicy::secure()
    .allow(Capability::NativeModules)
    .addNativeModule("http://expath.org/ns/file");
  SecurityPolicy m2(m1);
  CHECK(m2 == m1);
  CHECK(m2.getNativeModules().size() == 1);
  m2.addNativeModule("http://zorba.io/modules/sleep");
  CHECK(m1.getNativeModules().size() == 1);
  CHECK(m2 != m1);
  CHECK(m1 != SecurityPolicy::secure().allow(Capability::NativeModules));
  SecurityPolicy m3 = SecurityPolicy::secure()
    .allow(Capability::NativeModules)
    .addNativeModule("http://zorba.io/modules/sleep")
    .addNativeModule("http://expath.org/ns/file")
    .addNativeModule("http://zorba.io/modules/sleep");
  CHECK(m3 == m2);

  SecurityPolicy& cref = c;
  c = cref; // self-assignment
  CHECK(c.getTimeout() == seconds(1));
}

void test_narrowing()
{
  SecurityPolicy u = SecurityPolicy::unrestricted();
  SecurityPolicy s = SecurityPolicy::secure();
  CHECK(u.narrowedBy(s) == s);
  CHECK(s.narrowedBy(u) == s);
  CHECK(s.isWithin(u));
  CHECK(!u.isWithin(s));
  CHECK(u.isWithin(u));

  // Limits: the smaller non-zero value wins.
  SecurityPolicy t1 = SecurityPolicy::unrestricted().setTimeout(seconds(10));
  SecurityPolicy t2 = SecurityPolicy::unrestricted().setTimeout(seconds(60));
  CHECK(t1.narrowedBy(t2).getTimeout() == seconds(10));
  CHECK(t2.narrowedBy(t1).getTimeout() == seconds(10));
  CHECK(t1.narrowedBy(u).getTimeout() == seconds(10));
  SecurityPolicy d0 = SecurityPolicy::unrestricted().setMaxCallDepth(0);
  CHECK(d0.narrowedBy(u).getMaxCallDepth() == 1024);
  CHECK(d0.narrowedBy(d0).getMaxCallDepth() == 0);

  // Network prefixes: the inner prefix survives.
  SecurityPolicy n1 = SecurityPolicy::unrestricted()
    .addNetworkPrefix("https://api.example.com/");
  SecurityPolicy n2 = SecurityPolicy::unrestricted()
    .addNetworkPrefix("https://api.example.com/v1/");
  SecurityPolicy n12 = n1.narrowedBy(n2);
  CHECK(n12 == n2.narrowedBy(n1));
  CHECK(n12.allows(Capability::Network));
  CHECK(n12.getNetworkPrefixes().size() == 1 &&
        n12.getNetworkPrefixes()[0] == "https://api.example.com/v1/");
  CHECK(n2.isWithin(n1));
  CHECK(!n1.isWithin(n2));

  // Disjoint prefixes leave nothing: Network is denied.
  SecurityPolicy n3 = SecurityPolicy::unrestricted()
    .addNetworkPrefix("https://other.example/");
  CHECK(!n1.narrowedBy(n3).allows(Capability::Network));

  // Native modules: an empty list is taken from the other policy, two lists
  // keep the URIs in both (whole URIs, not prefixes), and no common URI
  // denies NativeModules.
  SecurityPolicy m1 = SecurityPolicy::unrestricted()
    .addNativeModule("http://expath.org/ns/file")
    .addNativeModule("http://zorba.io/modules/sleep");
  SecurityPolicy m2 = SecurityPolicy::unrestricted()
    .addNativeModule("http://zorba.io/modules/sleep")
    .addNativeModule("http://zorba.io/modules/zorba-query");
  SecurityPolicy m3 = SecurityPolicy::unrestricted()
    .addNativeModule("http://zorba.io/modules/sleep-more");
  CHECK(m1.narrowedBy(u) == m1);
  CHECK(u.narrowedBy(m1) == m1);
  CHECK(m1.narrowedBy(u).getNativeModules().size() == 2);
  SecurityPolicy m12 = m1.narrowedBy(m2);
  CHECK(m12 == m2.narrowedBy(m1));
  CHECK(m12.allows(Capability::NativeModules));
  CHECK(m12.getNativeModules().size() == 1 &&
        m12.getNativeModules()[0] == "http://zorba.io/modules/sleep");
  CHECK(m12.isWithin(m1));
  CHECK(m1.isWithin(u));
  CHECK(!u.isWithin(m1));
  CHECK(!m1.isWithin(m2));
  CHECK(!m1.narrowedBy(m3).allows(Capability::NativeModules));
  CHECK(m1.narrowedBy(m3).allows(Capability::Network));

#ifdef ZORBA_WITH_FILE_ACCESS
  // Directories match on path components, not on string prefixes.
  SecurityPolicy r1 = SecurityPolicy::unrestricted().addReadPath("/srv/data/");
  SecurityPolicy r2 = SecurityPolicy::unrestricted().addReadPath("/srv/data/x");
  SecurityPolicy r3 = SecurityPolicy::unrestricted().addReadPath("/srv/database");
  CHECK(r1.narrowedBy(r2).allows(Capability::FileRead));
  CHECK(r1.narrowedBy(r2).getReadPaths().size() == 1 &&
        r1.narrowedBy(r2).getReadPaths()[0] == "/srv/data/x");
  CHECK(!r1.narrowedBy(r3).allows(Capability::FileRead));
  CHECK(r2.isWithin(r1));
  CHECK(!r3.isWithin(r1));
  // The write list is independent of the read list.
  CHECK(r1.narrowedBy(r3).allows(Capability::FileWrite));
#endif
}

void test_instance_default(Zorba* z)
{
  // The library default is secure().
  CHECK(z->getSecurityPolicy() == SecurityPolicy::secure());

  StaticContext_t inheriting = z->createStaticContext();
  CHECK(inheriting->getSecurityPolicy() == SecurityPolicy::secure());

  SecurityPolicy const lExplicit =
    SecurityPolicy::secure().allow(Capability::Environment);
  StaticContext_t explicitCtx = z->createStaticContext();
  explicitCtx->setSecurityPolicy(lExplicit);

  // Changing the instance default affects contexts that inherit it, not
  // contexts with an explicit policy.
  z->setSecurityPolicy(SecurityPolicy::unrestricted());
  CHECK(z->getSecurityPolicy() == SecurityPolicy::unrestricted());
  CHECK(inheriting->getSecurityPolicy() == SecurityPolicy::unrestricted());
  CHECK(inheriting->createChildContext()->getSecurityPolicy() ==
        SecurityPolicy::unrestricted());
  CHECK(z->createStaticContext()->getSecurityPolicy() ==
        SecurityPolicy::unrestricted());
  CHECK(explicitCtx->getSecurityPolicy() == lExplicit);

  z->setSecurityPolicy(SecurityPolicy::secure());
  CHECK(inheriting->getSecurityPolicy() == SecurityPolicy::secure());
  CHECK(explicitCtx->getSecurityPolicy() == lExplicit);
}

void test_inheritance(Zorba* z)
{
  SecurityPolicy p = SecurityPolicy::secure()
    .allow(Capability::Network)
    .addNetworkPrefix("https://api.example.com/")
    .setTimeout(seconds(10));

  StaticContext_t parent = z->createStaticContext();
  parent->setSecurityPolicy(p);
  CHECK(parent->getSecurityPolicy() == p);

  // Children created before and after inherit the parent's policy.
  StaticContext_t child = parent->createChildContext();
  CHECK(child->getSecurityPolicy() == p);
  StaticContext_t grandchild = child->createChildContext();
  CHECK(grandchild->getSecurityPolicy() == p);

  // A child can narrow.
  SecurityPolicy narrower = SecurityPolicy::secure()
    .allow(Capability::Network)
    .addNetworkPrefix("https://api.example.com/v1/")
    .setTimeout(seconds(5));
  child->setSecurityPolicy(narrower);
  CHECK(child->getSecurityPolicy() == narrower);
  CHECK(grandchild->getSecurityPolicy() == narrower);
  CHECK(parent->getSecurityPolicy() == p);

  // Modifying the local objects does not affect the contexts.
  narrower.deny(Capability::Network);
  CHECK(child->getSecurityPolicy().allows(Capability::Network));
  SecurityPolicy copy = child->getSecurityPolicy();
  copy.allow(Capability::Environment);
  CHECK(!child->getSecurityPolicy().allows(Capability::Environment));

  // A compiled query runs under the policy of its static context.
  XQuery_t q = z->compileQuery("1", grandchild);
  CHECK(q->getStaticContext()->getSecurityPolicy() ==
        grandchild->getSecurityPolicy());
}

void test_embedder_widening(Zorba* z)
{
  // Typical use: secure process default, one trusted context that may read
  // a directory.
  z->setSecurityPolicy(SecurityPolicy::secure());

  SecurityPolicy trusted = SecurityPolicy::secure();
  trusted.allow(Capability::FileRead).addReadPath("/srv/data");

  StaticContext_t sctx = z->createStaticContext();
  CHECK(sctx->getSecurityPolicy() == SecurityPolicy::secure());
  sctx->setSecurityPolicy(trusted);
  CHECK(sctx->getSecurityPolicy() == trusted);
  CHECK(!trusted.isWithin(SecurityPolicy::secure())); // really wider

  // Children without a policy of their own inherit it.
  StaticContext_t child = sctx->createChildContext();
  CHECK(child->getSecurityPolicy() == trusted);

  // The embedder may also set a wider policy on a child of a narrower one.
  child->setSecurityPolicy(SecurityPolicy::unrestricted());
  CHECK(child->getSecurityPolicy() == SecurityPolicy::unrestricted());
  CHECK(sctx->getSecurityPolicy() == trusted);

  // Queries compiled with a context run under its policy.
  XQuery_t q = z->compileQuery("1", sctx);
  CHECK(q->getStaticContext()->getSecurityPolicy() == trusted);

  // Other contexts still get the instance default.
  CHECK(z->createStaticContext()->getSecurityPolicy() ==
        SecurityPolicy::secure());

  z->setSecurityPolicy(SecurityPolicy::unrestricted());
  CHECK(sctx->getSecurityPolicy() == trusted);
}

void test_plan_serialization(Zorba* z)
{
  SecurityPolicy p = SecurityPolicy::secure()
    .allow(Capability::Network)
    .allow(Capability::Environment)
    .addNetworkPrefix("https://api.example.com/v1/")
    .allow(Capability::NativeModules)
    .addNativeModule("http://zorba.io/modules/http-client")
    .setAllowPrivateAddresses(true)
    .setTimeout(seconds(7))
    .setMaxCallDepth(99)
    .setMaxEntityAmplification(3);

  std::ostringstream lOut;
  {
    StaticContext_t sctx = z->createStaticContext();
    sctx->setSecurityPolicy(p);
    XQuery_t q = z->compileQuery("1 + 1", sctx);
    q->saveExecutionPlan(lOut);
  }

  std::istringstream lIn(lOut.str());
  XQuery_t q = z->createQuery();
  q->loadExecutionPlan(lIn);
  CHECK(q->getStaticContext()->getSecurityPolicy() == p);
  CHECK(q->getStaticContext()->getSecurityPolicy().getNativeModules().size()
        == 1);
}

} // anonymous namespace


int
security_policy(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    test_defaults();
    test_modifiers();
    test_copy_semantics();
    test_narrowing();
    test_instance_default(lZorba);
    test_inheritance(lZorba);
    test_embedder_widening(lZorba);
    test_plan_serialization(lZorba);
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
