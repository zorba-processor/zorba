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

// Profile counters of an iterator state that is never initialized by
// PlanIteratorState::init() (an iterator that is not opened, e.g. the body of
// a cached function call) must read as zero, and the JSON plan printer must
// never write a number that is not valid JSON.

#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>

#include "runtime/base/plan_iterator.h"
#include "runtime/visitors/iterprinter.h"

using namespace std;

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

static void test_state_counters_start_at_zero()
{
  // Memory with a pattern that is not zero, as a reused heap block may have.
  alignas(PlanIteratorState) unsigned char lBuf[sizeof(PlanIteratorState)];
  memset(lBuf, 0xFF, sizeof lBuf);
  PlanIteratorState* const lState = new (lBuf) PlanIteratorState();
  profile_data const& pd = lState->get_profile_data();
  CHECK(pd.data_.call_count_ == 0);
  CHECK(pd.data_.next_count_ == 0);
  CHECK(pd.data_.cpu_time_ == 0);
  CHECK(pd.data_.wall_time_ == 0);
  lState->~PlanIteratorState();
}

static void test_json_decimal_is_valid()
{
  double const lBad[] = {
    numeric_limits<double>::quiet_NaN(),
    numeric_limits<double>::infinity(),
    -numeric_limits<double>::infinity()
  };
  for (double lValue : lBad)
  {
    ostringstream oss;
    {
      JSONIterPrinter lPrinter(oss, "", true);
      lPrinter.addDecAttribute("prof-cpu", lValue);
    }
    string const lOut = oss.str();
    CHECK(lOut.find("nan") == string::npos);
    CHECK(lOut.find("inf") == string::npos);
    CHECK(lOut.find("prof-cpu") != string::npos);
  }
}

int test_profile_data(int, char*[])
{
  failures = 0;
  test_state_counters_start_at_zero();
  test_json_decimal_is_valid();
  if (failures)
    cerr << failures << " check(s) failed" << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
