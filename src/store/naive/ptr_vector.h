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

#ifndef ZORBA_SIMPLE_STORE_PTR_VECTOR
#define ZORBA_SIMPLE_STORE_PTR_VECTOR

#include <cassert>
#include <cstring>
#include <iterator>
#include <new>
#include <stdexcept>
#include <cstdint>

#include "store/api/shared_types.h"

#include "content_reader.h"

namespace zorba
{
namespace simplestore
{

/*******************************************************************************
  The attributes and children of an element or document node (see
  InternalNode). A subset of the std::vector<T*> interface, in 24 bytes
  instead of std::vector's 24 plus a heap block: up to INLINE_CAPACITY
  pointers are stored in the object itself, which covers the common elements
  with one text child, or one attribute and one text child, without any
  allocation. Larger contents live in a heap array, grown like libstdc++'s
  std::vector so that the heap sizes are the same as before.

  Iterators are plain pointers. As with std::vector, insert, erase, resize,
  reserve and shrink_to_fit invalidate them. Unlike std::vector, swap() does
  not carry the iterators of an inline vector to the other vector: they stay
  in this object and see its new contents. Every member function that
  changes the contents or the storage (including swap) first calls
  ContentReader::contentWillChange(this), so an iteration that registered
  this vector copies what it still needs before its iterators go stale (see
  ChildrenIteratorImpl). Writing through operator[], front(), back() or an
  iterator does not: only the loaders do that, on nodes they are building.

  The size and capacity are 32-bit: growing past them, which doubling reaches
  at about 2^31 elements, throws std::length_error.
********************************************************************************/
template <class T>
class PtrVector
{
public:
  typedef T*                                    value_type;
  typedef T**                                   iterator;
  typedef T* const*                             const_iterator;
  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef csize                                 size_type;

  static constexpr uint32_t INLINE_CAPACITY = 2;

private:
  union
  {
    T*   theInline[INLINE_CAPACITY];
    T**  theHeap;
  };
  uint32_t theSize;
  uint32_t theCapacity;   // <= INLINE_CAPACITY means inline storage

  T** data() { return isInline() ? theInline : theHeap; }

  T* const* data() const { return isInline() ? theInline : theHeap; }

  static uint32_t checkedSize(csize n)
  {
    if (n > static_cast<csize>(UINT32_MAX))
      throw std::length_error("PtrVector: too many elements");
    return static_cast<uint32_t>(n);
  }

  // Moves the contents to storage for exactly newCap elements
  // (newCap >= theSize). newCap <= INLINE_CAPACITY selects inline storage.
  void reallocate(csize newCap)
  {
    assert(newCap >= theSize);
    uint32_t cap = checkedSize(newCap);
    if (cap <= INLINE_CAPACITY)
    {
      if (isInline())
        return;
      T** old = theHeap;
      for (uint32_t i = 0; i < theSize; ++i)
        theInline[i] = old[i];
      for (uint32_t i = theSize; i < INLINE_CAPACITY; ++i)
        theInline[i] = NULL;
      delete [] old;
      theCapacity = INLINE_CAPACITY;
      return;
    }
    T** fresh = new T*[cap];
    T** old = data();
    if (theSize > 0)
      std::memcpy(fresh, old, theSize * sizeof(T*));
    if (!isInline())
      delete [] old;
    theHeap = fresh;
    theCapacity = cap;
  }

  // Room for add more elements, with libstdc++'s growth: the new capacity
  // is size + max(size, add).
  void grow(csize add)
  {
    csize need = static_cast<csize>(theSize) + add;
    if (need <= theCapacity)
      return;
    csize len = theSize + (theSize > add ? theSize : add);
    reallocate(len);
  }

public:
  bool isInline() const { return theCapacity <= INLINE_CAPACITY; }

  PtrVector() : theSize(0), theCapacity(INLINE_CAPACITY)
  {
    for (uint32_t i = 0; i < INLINE_CAPACITY; ++i)
      theInline[i] = NULL;
  }

  ~PtrVector()
  {
    if (!isInline())
      delete [] theHeap;
  }

  // InternalNode never copies its vector; swap() moves contents.
  PtrVector(const PtrVector&) = delete;
  PtrVector& operator=(const PtrVector&) = delete;

  csize size() const { return theSize; }

  bool empty() const { return theSize == 0; }

  csize capacity() const { return theCapacity; }

  // Bytes allocated on the heap for the contents (0 when inline).
  csize heap_size() const { return isInline() ? 0 : theCapacity * sizeof(T*); }

  iterator begin() { return data(); }
  const_iterator begin() const { return data(); }

  iterator end() { return data() + theSize; }
  const_iterator end() const { return data() + theSize; }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

  T*& operator[](csize i) { assert(i < theSize); return data()[i]; }
  T* operator[](csize i) const { assert(i < theSize); return data()[i]; }

  T*& front() { assert(theSize > 0); return data()[0]; }
  T* front() const { assert(theSize > 0); return data()[0]; }

  T*& back() { assert(theSize > 0); return data()[theSize - 1]; }
  T* back() const { assert(theSize > 0); return data()[theSize - 1]; }

  void reserve(csize n)
  {
    if (n > theCapacity)
    {
      ContentReader::contentWillChange(this);
      reallocate(n);
    }
  }

  // New elements are NULL. Shrinking keeps the capacity, like std::vector.
  void resize(csize n)
  {
    ContentReader::contentWillChange(this);
    if (n > theSize)
    {
      grow(n - theSize);
      T** d = data();
      for (csize i = theSize; i < n; ++i)
        d[i] = NULL;
    }
    theSize = static_cast<uint32_t>(n);
  }

  void clear()
  {
    ContentReader::contentWillChange(this);
    theSize = 0;
  }

  void shrink_to_fit()
  {
    if (!isInline() && theSize < theCapacity)
    {
      ContentReader::contentWillChange(this);
      reallocate(theSize);
    }
  }

  void push_back(T* value)
  {
    ContentReader::contentWillChange(this);
    grow(1);
    data()[theSize] = value;
    ++theSize;
  }

  iterator insert(iterator pos, T* value)
  {
    csize index = pos - begin();
    assert(index <= theSize);
    ContentReader::contentWillChange(this);
    grow(1);
    T** d = data();
    std::memmove(d + index + 1, d + index, (theSize - index) * sizeof(T*));
    d[index] = value;
    ++theSize;
    return d + index;
  }

  // [first, last) must not point into this vector.
  template <class InputIter>
  iterator insert(iterator pos, InputIter first, InputIter last)
  {
    csize index = pos - begin();
    assert(index <= theSize);
    csize count = static_cast<csize>(std::distance(first, last));
    ContentReader::contentWillChange(this);
    grow(count);
    T** d = data();
    std::memmove(d + index + count, d + index, (theSize - index) * sizeof(T*));
    for (csize i = index; first != last; ++first, ++i)
      d[i] = *first;
    theSize = checkedSize(theSize + count);
    return d + index;
  }

  iterator erase(iterator pos)
  {
    return erase(pos, pos + 1);
  }

  iterator erase(iterator first, iterator last)
  {
    ContentReader::contentWillChange(this);
    T** d = data();
    csize index = first - d;
    csize count = last - first;
    assert(index + count <= theSize);
    std::memmove(d + index, d + index + count,
                 (theSize - index - count) * sizeof(T*));
    theSize -= static_cast<uint32_t>(count);
    return d + index;
  }

  // The union is plain data (pointers), so swapping it bytewise moves both
  // inline contents and heap ownership.
  void swap(PtrVector& other)
  {
    ContentReader::contentWillChange(this);
    ContentReader::contentWillChange(&other);
    unsigned char tmp[sizeof(theInline)];
    std::memcpy(tmp, theInline, sizeof(theInline));
    std::memcpy(theInline, other.theInline, sizeof(theInline));
    std::memcpy(other.theInline, tmp, sizeof(theInline));
    uint32_t s = theSize; theSize = other.theSize; other.theSize = s;
    uint32_t c = theCapacity; theCapacity = other.theCapacity; other.theCapacity = c;
  }
};


} // namespace simplestore
} // namespace zorba

#endif
/* vim:set et sw=2 ts=2: */
