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

// An environment variable that is not valid UTF-8 makes every call of the
// environment functions raise err:FOCH0001, also a call after one whose error
// was caught; none of them sees the variables read before the invalid one.

#include <cstdlib>
#include <iostream>
#include <string>

#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;

void check(bool aCondition, const std::string& aWhat, const std::string& aGot)
{
  if (aCondition)
    return;
  std::cerr << "FAILED: " << aWhat << ": " << aGot << std::endl;
  ++theFailures;
}

std::string run(Zorba* aZorba)
{
  XQuery_t lQuery = aZorba->compileQuery(
    "declare namespace err = 'http://www.w3.org/2005/xqt-errors';\n"
    "string-join((\n"
    "  try { fn:environment-variable('ZORBA_TEST_ENV_VALID') }\n"
    "  catch err:FOCH0001 { 'FOCH0001' },\n"
    "  try { fn:environment-variable('ZORBA_TEST_ENV_VALID') }\n"
    "  catch err:FOCH0001 { 'FOCH0001' },\n"
    "  try { string(fn:available-environment-variables() ="
    " 'ZORBA_TEST_ENV_VALID') }\n"
    "  catch err:FOCH0001 { 'FOCH0001' }), ' ')");
  Iterator_t lIter = lQuery->iterator();
  lIter->open();
  Item lItem;
  std::string lResult;
  if (lIter->next(lItem))
    lResult = lItem.getStringValue().str();
  lIter->close();
  return lResult;
}

} // anonymous namespace


int environment_variables(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  lZorba->setSecurityPolicy(SecurityPolicy::unrestricted());

  try
  {
    // Set first: environ lists it before the invalid one.
    setenv("ZORBA_TEST_ENV_VALID", "valid", 1);
    setenv("ZORBA_TEST_ENV_INVALID", "\xff", 1);
    std::string lGot = run(lZorba);
    check(lGot == "FOCH0001 FOCH0001 FOCH0001",
          "every call fails while a variable is invalid", lGot);

    unsetenv("ZORBA_TEST_ENV_INVALID");
    lGot = run(lZorba);
    check(lGot == "valid valid true",
          "the variables are read once all are valid", lGot);
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "FAILED: unexpected exception: " << e << std::endl;
    ++theFailures;
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return theFailures == 0 ? 0 : 1;
}
