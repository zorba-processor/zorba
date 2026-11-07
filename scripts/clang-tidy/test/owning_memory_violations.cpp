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

// Fixture for zorba-owning-memory: every line marked VIOLATION must be
// reported, and no other line. run_fixture.cmake compares the two sets.

#include "owning_memory_mocks.h"

namespace fixture {

struct Node : public zorba::SimpleRCObject { int value = 0; };
struct Plain { int value = 0; };
struct Base { virtual ~Base() = default; };
struct Derived : Base {};

Plain* make_raw() { return new Plain; } // VIOLATION

Base* make_base() { return new Derived; } // VIOLATION

struct Holder {
  Plain* theRaw;
  int* theArray;
  Holder() : theRaw(new Plain), theArray(new int[8]) {} // VIOLATION
  ~Holder() {
    delete theRaw; // VIOLATION
    delete[] theArray; // VIOLATION
  }
  void replace() { theRaw = new Plain; } // VIOLATION
};

void take_raw(Plain* p);

struct Wrapper {
  explicit Wrapper(Plain* p) : theP(p) {}
  Plain* theP;
};

void raw_owners(std::vector<Plain*>& v, bool flag) {
  Plain* a = new Plain; // VIOLATION
  Plain* b;
  b = new Plain; // VIOLATION
  auto* c = new Plain; // VIOLATION
  v.push_back(new Plain); // VIOLATION
  v.emplace_back(new Plain); // VIOLATION
  take_raw(new Plain); // VIOLATION
  Wrapper w(new Plain); // VIOLATION
  Plain* d = flag ? new Plain : nullptr; // VIOLATION
  Plain* e = new (std::nothrow) Plain; // VIOLATION
  Base* f = static_cast<Base*>(new Derived); // VIOLATION
  delete a; // VIOLATION
  delete b; // VIOLATION
  delete c; // VIOLATION
  delete d; // VIOLATION
  delete e; // VIOLATION
  delete f; // VIOLATION
  delete w.theP; // VIOLATION
}

template<class T> T* make_in_template() { return new T; } // VIOLATION

void instantiate() {
  Plain* p = make_in_template<Plain>();
  delete p; // VIOLATION
}

// emplace into a map: the new goes to the raw mapped value, even though the
// key type is a smart pointer.
void map_emplace(std::map<zorba::rchandle<Node>, Plain*>& m,
                 const zorba::rchandle<Node>& key) {
  m.emplace(key, new Plain); // VIOLATION
}

// The escape hatch: NOLINT with the check name is not reported.
void justified() {
  Plain* n = new Plain; // NOLINT(zorba-owning-memory): fixture
  delete n; // NOLINT(zorba-owning-memory): fixture
}

} // namespace fixture
