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

// Fixture for zorba-unchecked-stream-copy: stream copies whose result is
// examined, and destinations whose state outlives the function. No line of
// this file may be reported.

#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace fixture {

void report(std::ostream& os);

// The result of the copy expression itself.
bool in_condition(std::istream& in) {
  std::ostringstream out;
  if (out << in.rdbuf())
    return true;
  return false;
}

bool negated(std::istream& in) {
  std::ostringstream out;
  if (!(out << in.rdbuf()))
    return false;
  return true;
}

bool member_of_result(std::istream& in) {
  std::ostringstream out;
  return !(out << in.rdbuf()).fail();
}

bool bound_to_reference(std::istream& in) {
  std::ostringstream out;
  std::ostream& result = out << in.rdbuf();
  return result.good();
}

std::ostream& returned_result(std::istream& in, std::ostringstream& out) {
  return out << in.rdbuf();
}

// The destination, later in the same function.
bool later_fail(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return !out.fail();
}

bool later_bad(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return !out.bad();
}

bool later_good(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return out.good();
}

bool later_rdstate(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return out.rdstate() == std::ios::goodbit;
}

bool later_negation(std::istream& in) {
  std::ostringstream out;
  out << "header: " << in.rdbuf();
  if (!out)
    return false;
  return true;
}

bool later_conversion(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  bool const ok = static_cast<bool>(out);
  return ok;
}

bool later_chain(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return static_cast<bool>(out << '\n');
}

bool later_flush(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return static_cast<bool>(out.flush());
}

void passed_on(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  report(out);
}

std::ostringstream returned_stream(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  return out;
}

bool cleared_after_examined(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  bool const ok = !out.fail();
  out.clear();
  return ok;
}

void exceptions_before(std::istream& in) {
  std::ostringstream out;
  out.exceptions(std::ios::failbit | std::ios::badbit);
  out << in.rdbuf();
}

void exceptions_after(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf();
  out.exceptions(std::ios::failbit | std::ios::badbit);
}

bool in_lambda(std::istream& in) {
  auto copy = [&in]() {
    std::ostringstream out;
    out << in.rdbuf();
    return out.good();
  };
  return copy();
}

// Destinations whose state the caller or other code can still examine.
void to_parameter(std::istream& in, std::ostream& os) {
  os << in.rdbuf();
}

void to_global(std::istream& in) {
  std::cout << in.rdbuf();
}

struct Holder {
  std::ostringstream theOut;
  void append(std::istream& in) { theOut << in.rdbuf(); }
};

void to_reference(std::istream& in, std::ostringstream& target) {
  std::ostream& os = target;
  os << in.rdbuf();
}

void to_static(std::istream& in) {
  static std::ostringstream log;
  log << in.rdbuf();
}

// Not a stream copy.
std::string text(const char* s) {
  std::ostringstream out;
  out << s << 42;
  return out.str();
}

} // namespace fixture
