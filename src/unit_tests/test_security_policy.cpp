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
#include "stdafx.h"

#include <chrono>
#include <iostream>

#include <zorba/security.h>

#include "common/shared_types.h"
#include "context/static_context.h"
#include "system/globalenv.h"

using namespace std;
using std::chrono::seconds;

namespace zorba {
namespace UnitTests {

static int failures;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      cerr << __FILE__ << ':' << __LINE__ << ": check failed: "         \
           << #cond << endl;                                            \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

// Contexts the engine creates on a query's behalf use
// narrow_security_policy(): they can only narrow what they inherit.
static void test_engine_contexts_cannot_widen()
{
  static_context& root = GENV_ROOT_STATIC_CONTEXT;

  SecurityPolicy embedder = SecurityPolicy::secure();
  embedder.allow(Capability::Network)
          .addNetworkPrefix("https://api.example.com/v1/")
          .setTimeout(seconds(10));

  static_context_t user = root.create_child_context();
  user->set_security_policy(embedder);

  // No policy of its own: inherits.
  static_context_t query = user->create_child_context();
  CHECK(query->get_local_security_policy() == NULL);
  CHECK(query->get_security_policy() == embedder);

  // A narrowing unrestricted() policy changes nothing.
  static_context_t module = query->create_child_context();
  module->narrow_security_policy(SecurityPolicy::unrestricted());
  CHECK(module->get_security_policy() == embedder);
  CHECK(module->allows_capability(Capability::Network));
  CHECK(!module->allows_capability(Capability::Environment));

  // Wider entries are cut back to the inherited ones.
  static_context_t eval = query->create_child_context();
  eval->narrow_security_policy(SecurityPolicy::unrestricted()
                                 .addNetworkPrefix("https://api.example.com/")
                                 .setTimeout(seconds(60)));
  SecurityPolicy p = eval->get_security_policy();
  CHECK(p.isWithin(embedder));
  CHECK(p.getNetworkPrefixes().size() == 1 &&
        p.getNetworkPrefixes()[0] == "https://api.example.com/v1/");
  CHECK(p.getTimeout() == seconds(10));

  // A disjoint prefix leaves no network access.
  static_context_t other = query->create_child_context();
  other->narrow_security_policy(SecurityPolicy::unrestricted()
                                  .addNetworkPrefix("https://evil.example/"));
  CHECK(!other->get_security_policy().allows(Capability::Network));
  CHECK(!other->allows_capability(Capability::Network));

  // Narrowing really narrows, and stacks.
  static_context_t inner = module->create_child_context();
  inner->narrow_security_policy(SecurityPolicy::unrestricted()
                                  .setTimeout(seconds(5)));
  CHECK(inner->get_security_policy().getTimeout() == seconds(5));
  static_context_t innermost = inner->create_child_context();
  innermost->narrow_security_policy(SecurityPolicy::unrestricted()
                                      .deny(Capability::Network));
  CHECK(!innermost->allows_capability(Capability::Network));
  CHECK(innermost->get_security_policy().getTimeout() == seconds(5));

  // Narrowing follows changes of the inherited policy.
  user->set_security_policy(SecurityPolicy::secure());
  CHECK(module->get_security_policy() == SecurityPolicy::secure());
  CHECK(!module->allows_capability(Capability::Network));

  // Narrowing an explicit policy keeps it explicit (ancestors do not apply).
  static_context_t trusted = user->create_child_context();
  trusted->set_security_policy(SecurityPolicy::unrestricted());
  trusted->narrow_security_policy(SecurityPolicy::unrestricted()
                                    .deny(Capability::Environment));
  CHECK(trusted->allows_capability(Capability::Network));
  CHECK(!trusted->allows_capability(Capability::Environment));
}

int test_security_policy(int, char*[])
{
  failures = 0;
  test_engine_contexts_cannot_widen();
  if (failures)
    cerr << failures << " check(s) failed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
