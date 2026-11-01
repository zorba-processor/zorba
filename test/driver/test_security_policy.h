/*
 * Copyright 2006-2026 zorba.io
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
#ifndef ZORBA_TEST_SECURITY_POLICY_H
#define ZORBA_TEST_SECURITY_POLICY_H

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <zorba/security.h>
#include <zorba/zorba.h>

namespace zorba_test {

/**
 * Sets the security policy of the test drivers' Zorba instance.
 *
 * The library default is SecurityPolicy::secure(), but the test suites
 * exercise files, the HTTP test server, native modules and the environment,
 * so the drivers run with SecurityPolicy::unrestricted() unless the
 * environment variable ZORBA_TEST_SECURITY_POLICY says otherwise:
 *
 *   unset or "unrestricted"  SecurityPolicy::unrestricted()
 *   "secure"                 SecurityPolicy::secure() (the library default)
 *
 * "secure" runs the whole suite under the library default to check that
 * every capability a test needs is denied cleanly (with an error code) and
 * never by a crash, a hang or a wrong result.
 *
 * Returns false (after printing a message) for any other value, so that a
 * typo cannot silently run the suite with the wrong policy.
 */
inline bool applyTestSecurityPolicy(zorba::Zorba* aZorba)
{
  char const* const lValue = std::getenv("ZORBA_TEST_SECURITY_POLICY");
  if (lValue == nullptr || *lValue == '\0' ||
      std::strcmp(lValue, "unrestricted") == 0)
  {
    aZorba->setSecurityPolicy(zorba::SecurityPolicy::unrestricted());
    return true;
  }
  if (std::strcmp(lValue, "secure") == 0)
  {
    aZorba->setSecurityPolicy(zorba::SecurityPolicy::secure());
    return true;
  }
  std::cerr << "ZORBA_TEST_SECURITY_POLICY: unknown value \"" << lValue
            << "\" (expected \"secure\" or \"unrestricted\")" << std::endl;
  return false;
}

} // namespace zorba_test

#endif // ZORBA_TEST_SECURITY_POLICY_H
/* vim:set et sw=2 ts=2: */
