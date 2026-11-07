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

// Fixture for zorba-owning-memory: the allowed idioms. No line of this file
// may be reported.

#include "owning_memory_mocks.h"

namespace fixture {

struct Node : public zorba::SimpleRCObject { int value = 0; };
struct DerivedNode : Node {};
struct Plain { int value = 0; };

// A new expression handed straight to a smart pointer: construction,
// copy-initialization, assignment, reset, return and argument conversion.
zorba::rchandle<Node> make_handle() { return new Node; }
zorba::SmartPtr<Plain> make_smart() { return new Plain; }
std::unique_ptr<Plain> make_unique_ptr() { return std::unique_ptr<Plain>(new Plain); }

void take_handle(zorba::rchandle<Node> h);
void take_const_handle(const zorba::const_rchandle<Node>& h);

struct Holder {
  zorba::rchandle<Node> theHandle;
  std::unique_ptr<Plain> theOwned;
  std::unique_ptr<int[]> theArray;
  Holder() : theHandle(new Node), theOwned(new Plain), theArray(new int[8]) {}
  void replace() {
    theHandle = new Node;
    theOwned.reset(new Plain);
  }
};

void smart_owners(std::vector<zorba::rchandle<Node>>& v, bool flag) {
  zorba::rchandle<Node> a(new Node);
  zorba::rchandle<Node> b = new Node;
  zorba::rchandle<Node> c;
  c = new Node;
  zorba::rchandle<Node> d = new DerivedNode;
  zorba::rchandle<Node> e = flag ? new Node : nullptr;
  zorba::rchandle<Node> f(static_cast<Node*>(new DerivedNode));
  zorba::const_rchandle<Node> g(new Node);
  zorba::SmartPtr<Plain> h(new Plain);
  h = new Plain;
  std::shared_ptr<Plain> i(new Plain);
  std::unique_ptr<Plain> j = std::make_unique<Plain>();
  v.push_back(new Node);
  v.emplace_back(new Node);
  take_handle(new Node);
  take_const_handle(new Node);
  (void)a; (void)b; (void)d; (void)e; (void)f; (void)g; (void)i; (void)j;
}

// The store's item handle (store::Item_t is an ItemHandle<Item>).
zorba::store::ItemHandle<Node> make_item() { return new Node; }

void item_handles() {
  zorba::store::ItemHandle<Node> a(new Node);
  zorba::store::ItemHandle<Node> b = new Node;
  b = new Node;
  std::vector<zorba::store::ItemHandle<Node>> v;
  v.push_back(new Node);
  v.emplace_back(new Node);
}

// A class derived from a smart pointer.
struct NodeHandle : zorba::rchandle<Node> {
  using zorba::rchandle<Node>::rchandle;
};

void derived_handle() {
  NodeHandle h(new Node);
  NodeHandle k = NodeHandle(new DerivedNode);
}

// Ownership passed to a legacy interface through release().
void take_raw(Plain* p);

void legacy_interface() {
  auto p = std::make_unique<Plain>();
  take_raw(p.release());
}

// Arena and placement new: the memory is owned by the arena or the buffer.
Plain* arena_new(zorba::MemoryManager& mem) {
  return new (mem) Plain;
}

void placement_new(void* buffer) {
  Plain* p = new (buffer) Plain;
  p->~Plain();
}

// A template with a dependent new expression into a smart pointer, and its
// instantiation.
template<class T> zorba::rchandle<T> make_in_template() { return new T; }

void instantiate() {
  zorba::rchandle<Node> h = make_in_template<Node>();
  (void)h;
}

// Not a delete expression: a deleted function.
struct NoCopy {
  NoCopy() = default;
  NoCopy(const NoCopy&) = delete;
};

} // namespace fixture
