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

// Self-test of the fuzz harness: is_internal_error() must flag an internal
// error reported directly and one carried inside the error that wraps it.

#include <cstdio>

#include "fuzz_common.h"

namespace {

struct TestException : zorba::ZorbaException
{
  TestException(zorba::Diagnostic const& d, char const* message)
    : zorba::ZorbaException(d, __FILE__, __LINE__, message) {}
};

int failures = 0;

void expect(bool flagged, bool expected, char const* what)
{
  if (flagged != expected)
  {
    std::fprintf(stderr, "FAIL: %s: flagged=%d, expected=%d\n", what, flagged,
                 expected);
    ++failures;
  }
}

bool flagged(zorba::Diagnostic const& d, char const* message)
{
  return zorba_fuzz::is_internal_error(TestException(d, message));
}

} // namespace

int main()
{
  using namespace zorba;

  expect(flagged(zerr::ZXQP0003_INTERNAL_ERROR, "internal error: x"), true,
         "top-level ZXQP0003");
  expect(flagged(zerr::ZXQP0002_ASSERT_FAILED,
                 "\"p != nullptr\": assertion failed"), true,
         "top-level ZXQP0002");

  expect(flagged(err::FODC0006,
                 "invalid content passed to fn:parse-xml(): "
                 "[zerr:ZXQP0003]: internal error"),
         true, "ZXQP0003 wrapped in FODC0006");
  expect(flagged(err::FODC0006,
                 "invalid content passed to fn:parse-xml-fragment(): "
                 "[zerr:ZXQP0002]: \"p != nullptr\": assertion failed"),
         true, "ZXQP0002 wrapped in FODC0006");

  expect(flagged(err::FODC0006,
                 "invalid content passed to fn:parse-xml(): "
                 "[zerr:ZSTR0021]: \"\":1,7: loader parsing error: mismatched tag"),
         false, "ordinary parse error in FODC0006");
  expect(flagged(err::FODC0006,
                 "invalid content passed to fn:parse-xml(): "
                 "[zerr:ZSTR0021]: internal error"),
         false, "FODC0006 of a parse error whose text reads internal error");
  expect(flagged(err::FODC0006,
                 "invalid content passed to fn:parse-xml(): "
                 "[zerr:ZSTR0021]: an error [zerr:ZXQP0003]: in the input"),
         false, "FODC0006 mentioning an internal code in the middle");
  expect(flagged(err::FODC0002, "internal error: x"), false,
         "internal error text under another code");

  return failures == 0 ? 0 : 1;
}
