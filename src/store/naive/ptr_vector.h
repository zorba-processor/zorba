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
#include <type_traits>
#include <cstddef>

#include <zorba/config.h>

#include "store/api/shared_types.h"
#ifdef ZORBA_CONTAINER_CHECKS
#include "diagnostics/assert.h"
#endif

#include "content_reader.h"

namespace zorba
{
namespace simplestore
{

#ifdef ZORBA_CONTAINER_CHECKS
/*******************************************************************************
  The iterator of a PtrVector with ZORBA_CONTAINER_CHECKS: a pointer into the
  vector's storage plus the vector's generation number when the iterator was
  made. The vector increments its generation at every change of its contents
  or storage (see PtrVector), and dereferencing, moving, comparing or
  subtracting an iterator whose generation is not the vector's current one
  fails with ZORBA_ASSERT.

  An iterator made from a plain pointer (or default-constructed) has no
  vector and is not checked: the node iterators point into their own copy of
  the contents once the node changed (see ChildrenIteratorImpl).

  The iterator reads its vector's generation, so the vector must outlive it;
  a use after the vector was destroyed is AddressSanitizer's to find.

  P is T** or T* const*.
********************************************************************************/
template <class P>
class PtrVectorIterator
{
  template <class Q> friend class PtrVectorIterator;

  P                theCur;
  const uint32_t*  theOwnerGen;   // the vector's generation, or NULL
  uint32_t         theGen;

  void check() const
  {
    ZORBA_ASSERT(theOwnerGen == NULL || *theOwnerGen == theGen);
  }

  template <class Q>
  void checkPair(const PtrVectorIterator<Q>& other) const
  {
    check();
    other.check();
  }

public:
  typedef std::random_access_iterator_tag                iterator_category;
  typedef typename std::iterator_traits<P>::value_type   value_type;
  typedef std::ptrdiff_t                                 difference_type;
  typedef P                                              pointer;
  typedef typename std::iterator_traits<P>::reference    reference;

  PtrVectorIterator() : theCur(NULL), theOwnerGen(NULL), theGen(0) {}

  // Not checked: a pointer into a copy of the contents.
  PtrVectorIterator(P p) : theCur(p), theOwnerGen(NULL), theGen(0) {}

  PtrVectorIterator(P p, const uint32_t* ownerGen)
    :
    theCur(p),
    theOwnerGen(ownerGen),
    theGen(*ownerGen)
  {
  }

  // iterator to const_iterator
  template <class Q,
            class = typename std::enable_if<std::is_convertible<Q, P>::value>::type>
  PtrVectorIterator(const PtrVectorIterator<Q>& other)
    :
    theCur(other.theCur),
    theOwnerGen(other.theOwnerGen),
    theGen(other.theGen)
  {
  }

  // The vector whose generation this iterator carries, or NULL.
  const uint32_t* owner() const { return theOwnerGen; }

  // The pointer, checked.
  P get() const { check(); return theCur; }

  reference operator*() const { check(); return *theCur; }

  pointer operator->() const { check(); return theCur; }

  reference operator[](difference_type n) const { check(); return theCur[n]; }

  PtrVectorIterator& operator++() { check(); ++theCur; return *this; }

  PtrVectorIterator operator++(int)
  {
    check();
    PtrVectorIterator old(*this);
    ++theCur;
    return old;
  }

  PtrVectorIterator& operator--() { check(); --theCur; return *this; }

  PtrVectorIterator operator--(int)
  {
    check();
    PtrVectorIterator old(*this);
    --theCur;
    return old;
  }

  PtrVectorIterator& operator+=(difference_type n)
  {
    check();
    theCur += n;
    return *this;
  }

  PtrVectorIterator& operator-=(difference_type n)
  {
    check();
    theCur -= n;
    return *this;
  }

  PtrVectorIterator operator+(difference_type n) const
  {
    PtrVectorIterator r(*this);
    return r += n;
  }

  friend PtrVectorIterator operator+(difference_type n,
                                     const PtrVectorIterator& i)
  {
    return i + n;
  }

  PtrVectorIterator operator-(difference_type n) const
  {
    PtrVectorIterator r(*this);
    return r -= n;
  }

  template <class Q>
  difference_type operator-(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur - other.theCur;
  }

  template <class Q>
  bool operator==(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur == other.theCur;
  }

  template <class Q>
  bool operator!=(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur != other.theCur;
  }

  template <class Q>
  bool operator<(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur < other.theCur;
  }

  template <class Q>
  bool operator>(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur > other.theCur;
  }

  template <class Q>
  bool operator<=(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur <= other.theCur;
  }

  template <class Q>
  bool operator>=(const PtrVectorIterator<Q>& other) const
  {
    checkPair(other);
    return theCur >= other.theCur;
  }
};
#endif


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

  With ZORBA_CONTAINER_CHECKS (test builds), the iterators are checked
  (PtrVectorIterator): every member function that calls contentWillChange()
  makes all iterators taken before it stale, even where std::vector would
  keep some valid (push_back without reallocation, the elements before the
  position of insert or erase), and using a stale iterator fails with
  ZORBA_ASSERT. insert and erase also check that the iterator they get
  belongs to this vector. Without the option the iterators are plain
  pointers and nothing is checked.

  The size and capacity are 32-bit: growing past them, which doubling reaches
  at about 2^31 elements, throws std::length_error.
********************************************************************************/
template <class T>
class PtrVector
{
public:
  typedef T*                                    value_type;
#ifdef ZORBA_CONTAINER_CHECKS
  typedef PtrVectorIterator<T**>                iterator;
  typedef PtrVectorIterator<T* const*>          const_iterator;
#else
  typedef T**                                   iterator;
  typedef T* const*                             const_iterator;
#endif
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
#ifdef ZORBA_CONTAINER_CHECKS
  uint32_t theGeneration = 0;   // see PtrVectorIterator
#endif

  T** data() { return isInline() ? theInline : theHeap; }

  T* const* data() const { return isInline() ? theInline : theHeap; }

#ifdef ZORBA_CONTAINER_CHECKS
  iterator makeIter(T** p) { return iterator(p, &theGeneration); }

  const_iterator makeIter(T* const* p) const
  {
    return const_iterator(p, &theGeneration);
  }

  // The index of a valid iterator of this vector.
  csize indexOf(iterator pos)
  {
    ZORBA_ASSERT(pos.owner() == &theGeneration);
    return pos.get() - data();
  }
#else
  iterator makeIter(T** p) { return p; }

  const_iterator makeIter(T* const* p) const { return p; }

  csize indexOf(iterator pos) { return pos - data(); }
#endif

  // Called before every change of the contents or the storage.
  void willChange()
  {
    ContentReader::contentWillChange(this);
#ifdef ZORBA_CONTAINER_CHECKS
    ++theGeneration;
#endif
  }

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

  iterator begin() { return makeIter(data()); }
  const_iterator begin() const { return makeIter(data()); }

  iterator end() { return makeIter(data() + theSize); }
  const_iterator end() const { return makeIter(data() + theSize); }

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
      willChange();
      reallocate(n);
    }
  }

  // New elements are NULL. Shrinking keeps the capacity, like std::vector.
  void resize(csize n)
  {
    willChange();
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
    willChange();
    theSize = 0;
  }

  void shrink_to_fit()
  {
    if (!isInline() && theSize < theCapacity)
    {
      willChange();
      reallocate(theSize);
    }
  }

  void push_back(T* value)
  {
    willChange();
    grow(1);
    data()[theSize] = value;
    ++theSize;
  }

  iterator insert(iterator pos, T* value)
  {
    csize index = indexOf(pos);
    assert(index <= theSize);
    willChange();
    grow(1);
    T** d = data();
    std::memmove(d + index + 1, d + index, (theSize - index) * sizeof(T*));
    d[index] = value;
    ++theSize;
    return makeIter(d + index);
  }

  // [first, last) must not point into this vector.
  template <class InputIter>
  iterator insert(iterator pos, InputIter first, InputIter last)
  {
    csize index = indexOf(pos);
    assert(index <= theSize);
    csize count = static_cast<csize>(std::distance(first, last));
    willChange();
    grow(count);
    T** d = data();
    std::memmove(d + index + count, d + index, (theSize - index) * sizeof(T*));
    for (csize i = index; first != last; ++first, ++i)
      d[i] = *first;
    theSize = checkedSize(theSize + count);
    return makeIter(d + index);
  }

  iterator erase(iterator pos)
  {
    return erase(pos, pos + 1);
  }

  iterator erase(iterator first, iterator last)
  {
    csize index = indexOf(first);
    csize count = indexOf(last) - index;
    assert(index + count <= theSize);
    willChange();
    T** d = data();
    std::memmove(d + index, d + index + count,
                 (theSize - index - count) * sizeof(T*));
    theSize -= static_cast<uint32_t>(count);
    return makeIter(d + index);
  }

  // The union is plain data (pointers), so swapping it bytewise moves both
  // inline contents and heap ownership. The generations stay with their
  // objects: both vectors' iterators become stale.
  void swap(PtrVector& other)
  {
    willChange();
    other.willChange();
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
