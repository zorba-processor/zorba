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

// Minimal stand-ins for the Zorba types the zorba-owning-memory fixtures
// use, so that the fixtures compile without a Zorba build: the intrusive
// handles rchandle, const_rchandle and SmartPtr (same names and namespaces as
// src/zorbatypes/rchandle.h and include/zorba/util/smart_ptr.h) and the
// expression arena (src/compiler/expression/mem_manager.h).

#ifndef ZORBA_CLANG_TIDY_OWNING_MEMORY_MOCKS_H
#define ZORBA_CLANG_TIDY_OWNING_MEMORY_MOCKS_H

#include <cstddef>
#include <map>
#include <memory>
#include <new>
#include <vector>

namespace zorba {

struct SimpleRCObject {
  long theRefCount = 0;
  void addReference() { ++theRefCount; }
  void removeReference() { --theRefCount; }
};

template<class T> class rchandle {
protected:
  T* p;
public:
  rchandle(T* ptr = nullptr) : p(ptr) { if (p) p->addReference(); }
  rchandle(rchandle const& rhs) : p(rhs.p) { if (p) p->addReference(); }
  ~rchandle() { if (p) p->removeReference(); }
  rchandle& operator=(const T* rhs) { p = const_cast<T*>(rhs); return *this; }
  rchandle& operator=(rchandle const& rhs) { p = rhs.p; return *this; }
  T* getp() const { return p; }
  bool operator<(const rchandle& h) const { return p < h.p; }
};

template<class T> class const_rchandle : protected rchandle<T> {
public:
  const_rchandle(const T* ptr = nullptr) : rchandle<T>(const_cast<T*>(ptr)) {}
};

template<class T> class SmartPtr {
  T* p;
public:
  SmartPtr(T* ptr = nullptr) : p(ptr) {}
  SmartPtr& operator=(T* rhs) { p = rhs; return *this; }
};

namespace store {

// src/store/api/item_handle.h: the handle store::Item_t and the other item
// handles are made of.
template<class T> class ItemHandle {
protected:
  T* p;
public:
  ItemHandle() : p(nullptr) {}
  ItemHandle(T* ptr) : p(ptr) { if (p) p->addReference(); }
  ItemHandle(const ItemHandle& rhs) : p(rhs.p) { if (p) p->addReference(); }
  ~ItemHandle() { if (p) p->removeReference(); }
  ItemHandle& operator=(const T* rhs) { p = const_cast<T*>(rhs); return *this; }
  ItemHandle& operator=(const ItemHandle& rhs) { p = rhs.p; return *this; }
};

} // namespace store

class MemoryManager {
  char theBuffer[4096];
  std::size_t theUsed = 0;
public:
  void* allocate(std::size_t size) {
    void* r = theBuffer + theUsed;
    theUsed += size;
    return r;
  }
};

} // namespace zorba

inline void* operator new(std::size_t size, zorba::MemoryManager& mem)
{
  return mem.allocate(size);
}

inline void operator delete(void*, zorba::MemoryManager&)
{
}

#endif /* ZORBA_CLANG_TIDY_OWNING_MEMORY_MOCKS_H */
