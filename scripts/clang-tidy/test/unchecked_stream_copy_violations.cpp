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

// Fixture for zorba-unchecked-stream-copy: every line marked VIOLATION must
// be reported, and no other.

#include <fstream>
#include <istream>
#include <sstream>
#include <streambuf>
#include <string>

namespace fixture {

std::string discarded(const char* path) {
  std::ifstream in(path);
  std::ostringstream out;
  out << in.rdbuf(); // VIOLATION
  return out.str();
}

std::string chained(std::istream& in) {
  std::ostringstream out;
  out << "header: " << in.rdbuf() << '\n'; // VIOLATION
  return out.str();
}

std::string streambuf_pointer(std::streambuf* buf) {
  std::stringstream out;
  out << buf; // VIOLATION
  return out.str();
}

std::string examined_before_only(std::istream& in) {
  std::ostringstream out;
  if (!out)
    return "";
  out << in.rdbuf(); // VIOLATION
  return out.str();
}

std::string written_after(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf(); // VIOLATION
  out << "trailer";
  out.flush();
  return out.str();
}

std::string cast_to_void(std::istream& in) {
  std::ostringstream out;
  (void)(out << in.rdbuf()); // VIOLATION
  return out.str();
}

std::string cleared_unread(std::istream& in) {
  std::ostringstream out;
  out << in.rdbuf(); // VIOLATION
  out.clear(); // VIOLATION
  if (!out)
    return "";
  return out.str();
}

void file_copy(const char* from, const char* to) {
  std::ifstream in(from, std::ios::binary);
  std::ofstream out(to, std::ios::binary);
  out << in.rdbuf(); // VIOLATION
}

template<class Stream>
std::string in_template(std::istream& in) {
  Stream out;
  out << in.rdbuf(); // VIOLATION
  return out.str();
}

std::string instantiate(std::istream& in) {
  return in_template<std::ostringstream>(in);
}

} // namespace fixture
