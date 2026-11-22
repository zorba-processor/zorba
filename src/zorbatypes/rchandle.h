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
#pragma once
#ifndef ZORBA_RCHANDLE_H
#define ZORBA_RCHANDLE_H

#include <atomic>
#include <cassert>
#include <errno.h>
#include <limits>
#include <string>
#include <type_traits>

#include <zorba/config.h>

#include "common/common.h"

#include "zorbaserialization/class_serializer.h"


namespace zorba
{

namespace serialization
{
  class archiver;
}


/*******************************************************************************
  A reference count. Plain when Zorba is built for one thread only, atomic
  otherwise.
********************************************************************************/
class RefCount
{
#ifdef ZORBA_FOR_ONE_THREAD_ONLY
  long              theValue;
#else
  std::atomic<long> theValue;
#endif

  static constexpr long IMMORTAL = std::numeric_limits<long>::min();

public:
  RefCount() : theValue(0) { }

  RefCount(const RefCount&) = delete;
  RefCount& operator=(const RefCount&) = delete;

#ifdef ZORBA_FOR_ONE_THREAD_ONLY
  long load() const { return theValue; }

  void reset() { theValue = 0; }

  void makeImmortal() { theValue = IMMORTAL; }

  void increment()
  {
    if (theValue != IMMORTAL)
      ++theValue;
  }

  bool decrement()
  {
    if (theValue == IMMORTAL)
      return false;
    assert(theValue > 0);
    return --theValue == 0;
  }

  void incrementUnshared() { ++theValue; }

  bool decrementUnshared()
  {
    assert(theValue > 0);
    return --theValue == 0;
  }
#else
  long load() const { return theValue.load(std::memory_order_acquire); }

  void reset() { theValue.store(0, std::memory_order_relaxed); }

  void makeImmortal() { theValue.store(IMMORTAL, std::memory_order_relaxed); }

  // A thread can only copy a reference it already holds, so an increment
  // needs no ordering.
  void increment()
  {
    if (theValue.load(std::memory_order_relaxed) != IMMORTAL)
      theValue.fetch_add(1, std::memory_order_relaxed);
  }

  bool decrement()
  {
    if (theValue.load(std::memory_order_relaxed) == IMMORTAL)
      return false;
    long const before = theValue.fetch_sub(1, std::memory_order_acq_rel);
    assert(before > 0);
    return before == 1;
  }

  // For a count that only one thread changes: no read-modify-write.
  void incrementUnshared()
  {
    theValue.store(theValue.load(std::memory_order_relaxed) + 1,
                   std::memory_order_relaxed);
  }

  bool decrementUnshared()
  {
    long const after = theValue.load(std::memory_order_relaxed) - 1;
    assert(after >= 0);
    theValue.store(after, std::memory_order_relaxed);
    return after == 0;
  }
#endif
};


/*******************************************************************************
  Base classes for reference counted objects

  Any class wishing to take advantage of automatic reference counting
  must inherit from one of them.

  SyncedRCObject and SimpleRCObject count with a RefCount: atomic unless
  Zorba is built for one thread only. An object shared process-wide calls
  theRefCount.makeImmortal() before it is published; its count then never
  changes and it is never freed by a release.

  LocalRCObject changes its count without read-modify-write operations. Only
  a class whose references are never copied or released concurrently may
  derive from it.
********************************************************************************/
class SyncedRCObject : public serialization::SerializeBaseClass
{
protected:
  mutable RefCount theRefCount;

public:
  SERIALIZABLE_CLASS(SyncedRCObject);

  SyncedRCObject(::zorba::serialization::Archiver& ar)  
    :
    ::zorba::serialization::SerializeBaseClass()
  {
  }

  void serialize(::zorba::serialization::Archiver& ar);

public:
  SyncedRCObject()
    :
    ::zorba::serialization::SerializeBaseClass()
  {
  }

  SyncedRCObject(const SyncedRCObject&) 
    :
    ::zorba::serialization::SerializeBaseClass()
  {
  }

  virtual ~SyncedRCObject() { }

  SyncedRCObject& operator=(const SyncedRCObject&) { return *this; }

  virtual void free() { delete this; }

  long getRefCount() const { return theRefCount.load(); }

  void addReference() const { theRefCount.increment(); }

  void removeReference() 
  {
    if (theRefCount.decrement())
      free();
  }
};


/*******************************************************************************

********************************************************************************/
class ZORBA_DLL_PUBLIC SimpleRCObject : public serialization::SerializeBaseClass
{
protected:
  mutable RefCount theRefCount;

public:
  SERIALIZABLE_CLASS(SimpleRCObject);

  SimpleRCObject(serialization::Archiver& ar)  
    :
    serialization::SerializeBaseClass()
  {
  }

  void serialize(serialization::Archiver& ar);

public:
  SimpleRCObject()
    :
    serialization::SerializeBaseClass()
  {
  }

  SimpleRCObject(const SimpleRCObject&) 
    :
    serialization::SerializeBaseClass()
  {
  }

  virtual ~SimpleRCObject() { }

  virtual size_t alloc_size() const;
  virtual size_t dynamic_size() const;

  SimpleRCObject& operator=(const SimpleRCObject&) { return *this; }

  virtual void free() { delete this; }

  long getRefCount() const { return theRefCount.load(); }

  void addReference() const { theRefCount.increment(); }

  void removeReference() 
  {
    if (theRefCount.decrement())
      free();
  }
};


/*******************************************************************************

********************************************************************************/
class LocalRCObject : public SimpleRCObject
{
public:
  void addReference() const { theRefCount.incrementUnshared(); }

  void removeReference()
  {
    if (theRefCount.decrementUnshared())
      free();
  }
};


/*******************************************************************************

  rchandle = r(ef)c(ount)handle

  Template class for smart pointers-to-T objects. T must support the RCObject
  interface, typically by inheriting from RCObject

********************************************************************************/
template<class T> class rchandle
{
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef value_type& reference;
  typedef value_type const& const_reference;

protected:
  T  * p;

public:
  rchandle(T* ptr = 0) : p(ptr)
  {
    init(); 
  }

  rchandle(rchandle const& rhs) : p(rhs.getp())
  {
    init();
  }

  ~rchandle()
  {
    if (p)
      p->removeReference();
    p = 0;
  }

  bool isNull () const { return p == NULL; }

  void setNull() { p = NULL;}

  T* getp() const { return p; }
  T* get() const { return p; }          // synonym to match unique_ptr

  // rchandle const-ness is unclear. The implicit operators are more
  // restrictive than the explicit cast() and getp() methods.
  operator T* ()              { return getp(); }

  operator const T * () const { return getp(); }

  T* operator->() const       { return p; } 

  T& operator*() const        { return *p; } 

  bool operator!() const      { return !p; }

  bool operator==(rchandle const& h) const  { return p == h.p; }
  bool operator!=(rchandle const& h) const  { return p != h.p; }
  bool operator==(T const* pp) const        { return p == pp; } 
  bool operator!=(T const* pp) const        { return p != pp; }
  bool operator<(const rchandle& h) const   { return p < h.p; }


  template <class otherT> rchandle<otherT> cast() const 
  {
    return rchandle<otherT> (static_cast<otherT *> (p));
  }

  template <class otherT> rchandle<otherT> dyn_cast() const 
  {
    return rchandle<otherT> (dynamic_cast<otherT *> (p));
  }  

  // Implicit conversions only upcast (Derived -> Base); a downcast must name
  // cast<D>() or dyn_cast<D>().
  template <class otherT,
            class = typename std::enable_if<
                std::is_convertible<T*, otherT*>::value>::type>
  operator rchandle<otherT> ()
  {
    return cast<otherT> ();
  }

  template <class otherT,
            class = typename std::enable_if<
                std::is_convertible<T*, otherT*>::value>::type>
  operator const rchandle<otherT> () const
  {
    return cast<otherT> ();
  }

  rchandle& operator=(const T* rhs)
  {
    if (p != rhs)
    {
      if (p)
        p->removeReference();

      p = const_cast<T*>(rhs);

      init();
    }
    return *this;
  }

  template <class otherT,
            class = typename std::enable_if<
                std::is_convertible<const otherT*, const T*>::value>::type>
  rchandle& operator=(const otherT* rhs)
  {
    if (p != rhs)
    {
      if (p)
        p->removeReference();

      p = static_cast<T*>(const_cast<otherT*>(rhs));

      init();
    }
    return *this;
  }

  rchandle& operator=(rchandle const& rhs) 
  {
    return assign (rhs);
  }

  template <class otherT,
            class = typename std::enable_if<
                std::is_convertible<otherT*, T*>::value>::type>
  rchandle& operator=(rchandle<otherT> const& rhs)
  {
    return assign (rhs);
  }

  template <class otherT,
            class = typename std::enable_if<
                std::is_convertible<otherT*, T*>::value>::type>
  rchandle& transfer(rchandle<otherT>& rhs)
  {
    if (p != rhs.getp())
    {
      if (p)
        p->removeReference();

      p = static_cast<T*>(rhs.getp());

      rhs.setNull();
    }
    return *this;
  }

  rchandle& transfer(rchandle& rhs)
  {
    if (p != rhs.p)
    {
      if (p)
        p->removeReference();

      p = rhs.p;
      rhs.p = NULL;
    }
    return *this;
  }

  T* release()
  {
    T* tmp = p;
    p = NULL;
    return tmp;
  }

public:
  std::string debug() const
  {
    return "rchandle[refcount=" + std::to_string(p->getRefCount()) + ']';
  }

protected:
  void init()
  {
    if (p)
      p->addReference();
  }


  template <class otherT> rchandle& assign(const rchandle<otherT>& rhs)
  {
    if (p != rhs.getp())
    {
      if (p)
        p->removeReference();

      p = static_cast<T*>(rhs.getp());

      init();
    }
    return *this;
  }

};


namespace ztd 
{

template<typename T> inline
std::string to_string(const rchandle<T>& r)
{
  return !r ? "<null>" : to_string( *r );
}


template<typename T,class OutputStringType> inline
void to_string(const rchandle<T>& r, OutputStringType* out )
{
  if ( !r )
    *out = "<null>";
  else
    to_string( *r, out );
}

} // namespace ztd


/*******************************************************************************

********************************************************************************/
template<class T> class const_rchandle : protected rchandle<T> 
{
public:
  const_rchandle(const T* _p = 0) : rchandle<T>(const_cast<T *>(_p)) {}

  const_rchandle(const const_rchandle& rhs) : rchandle<T>(rhs) {}

  const_rchandle(rchandle<T>& rhs) : rchandle<T>(rhs) {}

  const_rchandle(::zorba::serialization::Archiver& ar) {}

  const_rchandle& operator= (const const_rchandle& rhs)
  {
    this->assign(rhs);
    return *this;
  }

public:
  bool isNull () const { return rchandle<T>::isNull(); }

  void setNull() { rchandle<T>::setNull();}

  const T* getp() const { return rchandle<T>::getp (); }

  operator const T* () const { return rchandle<T>::getp(); }

  const T* operator->() const { return getp(); } 
  const T& operator*() const  { return *getp(); }

  bool operator== (const_rchandle h) const  { return rchandle<T>::operator==(h); }
  bool operator!= (const_rchandle h) const  { return rchandle<T>::operator!=(h); }
  bool operator== (const T* pp) const       { return rchandle<T>::operator==(pp); } 
  bool operator!= (const T* pp) const       { return rchandle<T>::operator!=(pp); } 
  bool operator< (const_rchandle h) const   { return rchandle<T>::operator<(h); }
};



/*******************************************************************************

********************************************************************************/
namespace RCHelper 
{
  template<class T> 
  static void addReference(T *t)
  {
    t->addReference();
  }

  template<class T> 
  static void removeReference(T *t)
  {
    t->removeReference();
  }

  template<class T> 
  static void addReference(rchandle<T> &t)
  {
    addReference(t.getp());
  }
  
  template<class T> 
  static void removeReference(rchandle<T> &t)
  {
    removeReference(t.getp());
  }
};

} // namespace zorba

#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
