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

// The implicit timezone and the current dateTime a host sets through the
// DynamicContext are the ones every execution of the query sees; a value the
// host did not set is taken from the host clock at the start of each execution.

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;

void check(bool aCondition, const std::string& aWhat)
{
  if (aCondition)
    return;
  std::cerr << "FAILED: " << aWhat << std::endl;
  ++theFailures;
}

std::vector<std::string> run(const XQuery_t& aQuery)
{
  std::vector<std::string> lValues;
  Iterator_t lIter = aQuery->iterator();
  lIter->open();
  Item lItem;
  while (lIter->next(lItem))
    lValues.push_back(lItem.getStringValue().str());
  lIter->close();
  return lValues;
}

const char* const theQuery = "fn:implicit-timezone(), fn:current-dateTime()";

void both_set(Zorba* aZorba)
{
  XQuery_t lQuery = aZorba->compileQuery(theQuery);
  DynamicContext* lDctx = lQuery->getDynamicContext();
  lDctx->setImplicitTimezone(-360);
  lDctx->setCurrentDateTime(
      aZorba->getItemFactory()->createDateTime("2010-10-10T10:10:10-06:00"));

  for (int i = 1; i <= 2; ++i)
  {
    std::vector<std::string> lValues = run(lQuery);
    check(lValues.size() == 2 &&
          lValues[0] == "-PT6H" &&
          lValues[1] == "2010-10-10T10:10:10-06:00",
          "both_set: execution " + std::to_string(i) +
          " sees the values the host set");
  }

  lDctx->setImplicitTimezone(120);
  std::vector<std::string> lValues = run(lQuery);
  check(lValues.size() == 2 && lValues[0] == "PT2H",
        "both_set: a timezone the host changes between executions applies");
}

void timezone_set(Zorba* aZorba)
{
  XQuery_t lQuery = aZorba->compileQuery(theQuery);
  lQuery->getDynamicContext()->setImplicitTimezone(-360);

  std::vector<std::string> lFirst = run(lQuery);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  std::vector<std::string> lSecond = run(lQuery);

  check(lFirst.size() == 2 && lFirst[0] == "-PT6H",
        "timezone_set: execution 1 sees the timezone the host set");
  check(lSecond.size() == 2 && lSecond[0] == "-PT6H",
        "timezone_set: execution 2 sees the timezone the host set");
  check(lFirst.size() == 2 && lSecond.size() == 2 && lFirst[1] != lSecond[1],
        "timezone_set: the current dateTime is taken at each execution");
}

void date_time_set(Zorba* aZorba)
{
  XQuery_t lQuery = aZorba->compileQuery("fn:current-dateTime()");
  lQuery->getDynamicContext()->setCurrentDateTime(
      aZorba->getItemFactory()->createDateTime("2010-10-10T10:10:10Z"));

  for (int i = 1; i <= 2; ++i)
  {
    std::vector<std::string> lValues = run(lQuery);
    check(lValues.size() == 1 && lValues[0] == "2010-10-10T10:10:10Z",
          "date_time_set: execution " + std::to_string(i) +
          " sees the current dateTime the host set");
  }
}

void none_set(Zorba* aZorba)
{
  XQuery_t lQuery = aZorba->compileQuery("fn:current-dateTime()");

  std::vector<std::string> lFirst = run(lQuery);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  std::vector<std::string> lSecond = run(lQuery);

  check(lFirst.size() == 1 && lSecond.size() == 1 && lFirst[0] != lSecond[0],
        "none_set: the current dateTime is taken at each execution");
}

} // namespace

int host_date_time(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    both_set(lZorba);
    timezone_set(lZorba);
    date_time_set(lZorba);
    none_set(lZorba);
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
