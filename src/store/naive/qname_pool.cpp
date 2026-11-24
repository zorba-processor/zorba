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

#include <new>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>
#include "diagnostics/assert.h"
#include "system/globalenv.h"

#include "string_pool.h"
#include "qname_pool.h"
#include "atomic_items.h"
#include "simple_store.h"
#include "store_defs.h"
#include "node_items.h"
#include "store/api/collection.h"
#include "zorbatypes/datetime.h"
#include "util/string_util.h"


namespace zorba { namespace simplestore {


/*******************************************************************************

********************************************************************************/
QNamePool::QNamePool(ulong size, StringPool* nspool) 
  :
  theCache(new QNameItem[size]),
  theCacheSize(size),
  theFirstFree(1),
  theNumFree(size - 1),
  theHashSet(2 * size),
  theNamespacePool(nspool)
{
  // Put all the preallocated slots in the free list of the cahce.
  QNameItem* qn = &theCache[1];
  QNameItem* last = qn + size - 1;

  for (csize i = 1; qn < last; ++qn, ++i)
  {
    qn->theNextFree = i + 1;
    qn->thePrevFree = i - 1;
    qn->thePosition = i;
  }
  (--qn)->theNextFree = 0;

  qn = &theCache[0];
  qn->theNextFree =  qn->thePrevFree = qn->thePosition = 0;
}


/*******************************************************************************

********************************************************************************/
QNamePool::~QNamePool() 
{
  csize n = theHashSet.capacity();

#if 0
#ifndef NDEBUG
  csize numInPool = 0;
  for (csize i = 0; i < theCacheSize; ++i)
  {
    QNameItem* qn = &theCache[i];

    if (qn->getRefCount() != 0)
    {
      ++numInPool;
      std::cerr << "QName in pool: RC = " << qn->getRefCount() << " : "
                << qn->getStringValue() << " at pos : " << qn->thePosition
                << std::endl;
    }
  }

  std::cerr << std::endl << numInPool << "qnames in pool" << std::endl;
#endif
#endif

  for (csize i = 0; i < n; ++i)
  {
    if (!theHashSet.theHashTab[i].isFree() &&
        theHashSet.theHashTab[i].key()->isOverflow())
      delete theHashSet.theHashTab[i].key();
  }

  theCache.reset();
}


/*******************************************************************************

********************************************************************************/
void QNamePool::addInFreeList(QNameItem* qn)
{
  assert(qn->getRefCount() == 0);
  assert(theCache[theFirstFree].thePrevFree == 0);

  // Nothing to do if qn is already in the free list

  if (qn->thePrevFree != 0 || qn->theNextFree != 0)
  {
#ifndef NDEBUG
    QNameItem* curr = &theCache[theFirstFree];
    while (curr != NULL && curr != qn)
      curr = &theCache[curr->theNextFree];

    assert(curr != NULL);
#endif
    return;
  }

  if (theFirstFree == qn->thePosition)
    return;

  // add it in the list

  qn->theNextFree = (uint16_t)theFirstFree;

  if (theFirstFree != 0)
  {
    assert(theCache[theFirstFree].thePosition == theFirstFree);

    theCache[theFirstFree].thePrevFree = qn->thePosition;
  }

  theFirstFree = qn->thePosition;

  assert(theFirstFree > 0 && theFirstFree < theCacheSize);

  ++theNumFree;
}


/*******************************************************************************

********************************************************************************/
void QNamePool::removeFromFreeList(QNameItem* qn)
{
  assert(qn->isInCache());

  if (qn->theNextFree != 0)
  {
    assert(qn->getRefCount() == 0);
    assert(theFirstFree > 0 && theFirstFree < theCacheSize);
    assert(theCache[qn->theNextFree].thePrevFree == qn->thePosition);

    theCache[qn->theNextFree].thePrevFree = qn->thePrevFree;
  }

  if (qn->thePrevFree != 0)
  {
    assert(qn->getRefCount() == 0);
    assert(theFirstFree > 0 && theFirstFree < theCacheSize);
    assert(theCache[qn->thePrevFree].theNextFree == qn->thePosition);
    
    theCache[qn->thePrevFree].theNextFree = qn->theNextFree;
  }
  else if (qn->theNextFree == 0)
  {
    // Either qn does not belong to the free list, or is the only one in the
    // free list
    if (theFirstFree != qn->thePosition)
      return;

    assert(qn->getRefCount() == 0);
    assert(theFirstFree == qn->thePosition);
    assert(theNumFree == 1);

    theFirstFree = qn->theNextFree;
  }
  else
  {
    // qn is the 1st slot in the free list
    assert(qn->getRefCount() == 0);
    assert(theFirstFree == qn->thePosition);

    theFirstFree = qn->theNextFree;
  }

  qn->theNextFree = qn->thePrevFree = 0;

  --theNumFree;
}


/*******************************************************************************

********************************************************************************/
QNameItem* QNamePool::popFreeList()
{
  while (theFirstFree != 0)
  {
    assert(theNumFree > 0);

    QNameItem* qn = &theCache[theFirstFree];

    theFirstFree = qn->theNextFree;

    assert(theFirstFree == 0 ||
           theCache[theFirstFree].thePrevFree == qn->thePosition);

    theCache[theFirstFree].thePrevFree = 0;

    qn->theNextFree = qn->thePrevFree = 0;

    --theNumFree;

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
    // Revived by findShared(): remove() puts it back when it is released.
    if (qn->getRefCount() != 0)
      continue;
#endif

    assert(qn->getRefCount() == 0);
    return qn;
  }

  assert(theNumFree == 0);
  return NULL;
}


/*******************************************************************************
  Releases the last reference to qn (see QNameItem::free()). In the
  multithreaded build the count goes from 1 to 0 here, under the exclusive lock,
  while no lookup can add a reference: a name is removed once for each time its
  count reaches 0, never while a lookup revives it.
********************************************************************************/
void QNamePool::remove(QNameItem* qn)
{
  QNameItem* normVictim = NULL;

  SYNC_CODE(theLock.lock();)

  try 
  {
#ifdef ZORBA_FOR_ONE_THREAD_ONLY
    if (qn->getRefCount() > 0)
      return;
#else
    if (!qn->theRefCount.decrement())
    {
      theLock.unlock();
      return;
    }
#endif

    if (qn->isInCache())
    {
      addInFreeList(qn);
    }
    else
    {
      // If all the pointers to QNameItems were smart pointers, we could leave
      // qn in the pool, and let the pool garbage-collect it later (if it still
      // unused). If however QNameItems may be referenced by regular pointers
      // as well, then qn must be removed from the pool and really deleted.
      theHashSet.eraseNoSync(qn);
      qn->invalidate(true, &normVictim);
      delete qn;
    }

    // Releasing the lock here to avoid deadlock, because decrementing the 
    // normVictim counter might reenter QNamePool::remove.
    SYNC_CODE(theLock.unlock();)
  }
  catch(...)
  {
    SYNC_CODE(theLock.unlock();)
              
    ZORBA_FATAL(0, "Unexpected exception");
  }

  if (normVictim)
  {
    normVictim->removeReference();
  }
}


/*******************************************************************************
  If the pool does not already contain a qname with the given namespace, prefix,
  and local name, then create such a qname, insert it in the pool and return an
  rchandle to it. Otherwise, return an rchandle to the existing qname. 

  Note: The given namespace, prefix, and local name pointers may point to heap 
  or stack allocated strings. If a qname is inserted, then the strings are 
  copied internally into zstring objects. So, it's always the caller who is
  resposnible for freeing the given strings.
********************************************************************************/
void QNamePool::insert(
    store::Item_t& res,
    const char* ns,
    const char* pre,
    const char* ln)
{
  QNameItem* qn;
  QNameItem* fresh = NULL;  // not in the hash table yet
  QNameItem* normVictim = NULL;
  SYNC_CODE(bool haveLock = false;)
  store::Item_t normItem;
  QNameItem* normQName = NULL;

  res = NULL;

  bool normalized = (pre == NULL || *pre == '\0');

  if (ns == NULL) ns = "";
  if (pre == NULL) pre = "";
  ZORBA_FATAL(ln != NULL && *ln != '\0', "");

  csize const nslen = strlen(ns);
  csize const prelen = strlen(pre);
  csize const lnlen = strlen(ln);

  ulong hval = hashfun::h32(pre, hashfun::h32(ln, hashfun::h32(ns)));

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
  if (findShared(res, ns, pre, ln, nslen, prelen, lnlen, hval))
    return;
#endif

  zstring pooledNs;
  bool nsPooled = false;

  try
  {
retry:
    SYNC_CODE(theLock.lock();\
    haveLock = true;)

    QNHashEntry* entry = hashFind(ns, pre, ln, nslen, prelen, lnlen, hval);

    if (entry == 0)
    {
      if (normalized)
      {
        if (!nsPooled)
        {
          SYNC_CODE(theLock.unlock();\
          haveLock = false;)

          theNamespacePool->insertc(ns, pooledNs);
          nsPooled = true;
          goto retry;
        }
        // Build a new QName (either new object or in cache).
        fresh = qn = cacheInsert(normVictim);
        qn->initializeAsNormalizedQName(pooledNs, ln);
      }
      else
      {
        if (normQName == NULL)
        {
          SYNC_CODE(theLock.unlock();\
          haveLock = false;)

          insert(normItem, ns, NULL, ln);
          normQName = static_cast<QNameItem*>(normItem.getp());
          goto retry;
        }
        // Build a new QName (either new object or in cache).
        fresh = qn = cacheInsert(normVictim);
        qn->initializeAsUnnormalizedQName(normQName, pre);
      }

      bool found;
      entry = theHashSet.hashInsert(qn, hval, found);
      entry->key() = qn;
      fresh = NULL;
      ZORBA_FATAL(!found, "");
    }
    else
    {
      qn = entry->key();
#ifdef ZORBA_FOR_ONE_THREAD_ONLY
      cachePin(qn);
#endif
    }

#ifdef ZORBA_FOR_ONE_THREAD_ONLY
    assert(qn->theNextFree == 0);
#endif
    res = qn;

    SYNC_CODE(theLock.unlock();\
    haveLock = false;)
  }
  catch (std::bad_alloc const&)
  {
    QNameItem* freshVictim = NULL;
    undoCacheInsert(fresh, &freshVictim);

    SYNC_CODE(if (haveLock) \
      theLock.unlock();)

    if (normVictim != NULL)
      normVictim->removeReference();
    if (freshVictim != NULL)
      freshVictim->removeReference();
    throw;
  }
  catch (...)
  {
    SYNC_CODE(if (haveLock) \
      theLock.unlock();)

    ZORBA_FATAL(0, "Unexpected exception");
  }

  if (normVictim != NULL)
  {
    assert(normVictim->getRefCount() > 0 && normVictim->getRefCount() < 10000);
    normVictim->removeReference();
  }
}


/*******************************************************************************
  If the pool does not already contain a qname with the given namespace, prefix,
  and local name, then create such a qname, insert it in the pool and return an
  rchandle to it. Otherwise, return an rchandle to the existing qname. 
********************************************************************************/
void QNamePool::insert(
    store::Item_t& res,
    const zstring& ns,
    const zstring& pre,
    const zstring& ln)
{
  QNameItem* qn = NULL;
  QNameItem* fresh = NULL;  // not in the hash table yet
  QNameItem* normVictim = NULL;
  SYNC_CODE(bool haveLock = false;)
  store::Item_t normItem;
  QNameItem* normQName = NULL;

  res = NULL;

  bool normalized = pre.empty();

  ulong hval = hashfun::h32(pre.c_str(),
                            hashfun::h32(ln.c_str(),
                                         hashfun::h32(ns.c_str())));

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
  if (findShared(res, ns.c_str(), pre.c_str(), ln.c_str(),
                 ns.size(), pre.size(), ln.size(), hval))
    return;
#endif

  zstring pooledNs;
  bool nsPooled = false;

  try
  {
retry:
    SYNC_CODE(theLock.lock();\
    haveLock = true;)

    QNHashEntry* entry =
    hashFind(ns.c_str(), pre.c_str(), ln.c_str(),
             ns.size(), pre.size(), ln.size(),
             hval);

    if (entry == 0)
    {
      if (normalized)
      {
        if (!nsPooled)
        {
          SYNC_CODE(theLock.unlock();\
          haveLock = false;)

          theNamespacePool->insert(ns, pooledNs);
          nsPooled = true;
          goto retry;
        }
        // Build a new QName (either new object or in cache).
        fresh = qn = cacheInsert(normVictim);
        qn->initializeAsNormalizedQName(pooledNs, ln);
      }
      else
      {
        if (normQName == NULL)
        {
          SYNC_CODE(theLock.unlock();\
          haveLock = false;)

          // This call will need the lock.
          insert(normItem, ns, zstring(), ln);
          normQName = static_cast<QNameItem*>(normItem.getp());

          goto retry;
        }
        // Build a new QName (either new object or in cache).
        fresh = qn = cacheInsert(normVictim);
        qn->initializeAsUnnormalizedQName(normQName, pre);
      }

      bool found;
      entry = theHashSet.hashInsert(qn, hval, found);
      entry->key() = qn;
      fresh = NULL;
      ZORBA_FATAL(!found, "");
    }
    else
    {
      qn = entry->key();
#ifdef ZORBA_FOR_ONE_THREAD_ONLY
      cachePin(qn);
#endif
    }

#ifdef ZORBA_FOR_ONE_THREAD_ONLY
    assert(qn->theNextFree == 0);
#endif
    res = qn;

    SYNC_CODE(theLock.unlock();\
    haveLock = false;)
  }
  catch (std::bad_alloc const&)
  {
    QNameItem* freshVictim = NULL;
    undoCacheInsert(fresh, &freshVictim);

    SYNC_CODE(if (haveLock) theLock.unlock();)

    if (normVictim != NULL)
      normVictim->removeReference();
    if (freshVictim != NULL)
      freshVictim->removeReference();
    throw;
  }
  catch (...)
  {
    SYNC_CODE(if (haveLock) theLock.unlock();)

    ZORBA_FATAL(0, "Unexpected exception");
  }

  if (normVictim != NULL)
  {
    assert(normVictim->getRefCount() > 0 && normVictim->getRefCount() < 10000);
    normVictim->removeReference();
  }
}



/*******************************************************************************
  An allocation failed between cacheInsert() and the insertion in the hash table:
  the slot goes back. The caller holds the lock, and removes a reference from the
  victim after it releases it.
********************************************************************************/
void QNamePool::undoCacheInsert(QNameItem* qn, QNameItem** normVictim)
{
  if (qn == NULL)
    return;

  if (qn->isValid())
    qn->invalidate(true, normVictim);

  if (qn->isInCache())
    addInFreeList(qn);
  else
    delete qn;
}


/*******************************************************************************
  Return a qname slot for a new qname to be stored in. If the cache free list
  is not empty, we use the 1st slot from tha list; the qname currently in that
  slot (if any) is removed from the pool. If the cache free list is empty a new
  QNameItem is allocated from the heap.
********************************************************************************/
QNameItem* QNamePool::cacheInsert(QNameItem*& normVictim)
{
  assert(normVictim == NULL);

  QNameItem* qn = popFreeList();

  if (qn == NULL)
  {
    return new QNameItem();
  }

  if (qn->isValid())
  {
    ulong hval = CompareFunction::hash(qn);
    theHashSet.eraseNoSync(qn, hval);
    qn->invalidate(true, &normVictim);
  }
  
  return qn;
}


/*******************************************************************************
  If the given qname slot is in the free list of the cache, remove it from that
  list.
********************************************************************************/
void QNamePool::cachePin(QNameItem* qn)
{
  if (qn->isInCache())
  {
    removeFromFreeList(qn);
  }
}


#ifndef ZORBA_FOR_ONE_THREAD_ONLY
/*******************************************************************************
  The lookup of a name under the shared lock (see theLock). Returns false for a
  missing name: the caller then inserts it under the exclusive lock.
********************************************************************************/
bool QNamePool::findShared(
    store::Item_t& res,
    const char* ns,
    const char* pre,
    const char* ln,
    csize       nslen,
    csize       prelen,
    csize       lnlen,
    csize       hval)
{
  // Assigning res must not release a name: remove() would wait for this lock.
  assert(res.isNull());

  unsigned const slot = theLock.lockShared();

  QNHashEntry* entry = hashFind(ns, pre, ln, nslen, prelen, lnlen, hval);

  bool const found = (entry != NULL);
  if (found)
    res = entry->key();

  theLock.unlockShared(slot);
  return found;
}


unsigned ShardedSharedLock::threadSlot()
{
  std::uint64_t const id =
    std::hash<std::thread::id>()(std::this_thread::get_id());
  // Fibonacci hashing: thread ids can share their low bits.
  return static_cast<unsigned>(
    (id * 0x9E3779B97F4A7C15ull) >> (64 - SLOT_BITS));
}


void ShardedSharedLock::waitForWriter() const
{
  for (unsigned spins = 0; theWriter.load(std::memory_order_acquire); ++spins)
  {
    if (spins >= 64)
      std::this_thread::yield();
  }
}


void ShardedSharedLock::lock()
{
  theWriterMutex.lock();
  theWriter.store(true, std::memory_order_seq_cst);
  for (Slot& s : theSlots)
  {
    for (unsigned spins = 0;
         s.theReaders.load(std::memory_order_acquire) != 0;
         ++spins)
    {
      if (spins >= 64)
        std::this_thread::yield();
    }
  }
}
#endif


/*******************************************************************************

********************************************************************************/
QNamePool::QNHashEntry* QNamePool::hashFind(
    const char* ns,
    const char* pre,
    const char* ln,
    csize       nslen,
    csize       prelen,
    csize       lnlen,
    csize       hval)
{
  QNHashEntry* entry = theHashSet.bucket(hval);

  if (entry->isFree())
    return NULL;

  while (entry != NULL)
  {
    QNameItem* qn = entry->key();

    if (ztd::equals(qn->getLocalName2(), ln, lnlen) &&
        ztd::equals(qn->getNamespace2(), ns, nslen) &&
        ztd::equals(qn->getPrefix2(), pre, prelen))
      return entry;

    entry = entry->getNext();
  }

  return NULL;
}


} // namespace store
} // namespace zorba

/* vim:set et sw=2 ts=2: */

namespace zorba { namespace simplestore {

std::size_t QNamePool::tableBytes() const
{
  return theHashSet.theHashTab.capacity() * sizeof(QNHashEntry);
}

void QNamePool::flushUnused()
{
  // An unnormalized name holds its normalized one: that one is free only after,
  // so this runs until a pass drops nothing. As in remove(), the normalized
  // names are released after the lock, since that can come back to remove().
  std::vector<QNameItem*> victims;
  victims.reserve(theCacheSize);   // push_back() cannot throw under the lock
  for (bool dropped = true; dropped; )
  {
    dropped = false;
    victims.clear();
    SYNC_CODE(theLock.lock();)
    for (csize i = 1; i < theCacheSize; ++i)
    {
      QNameItem* qn = &theCache[i];
      if (!qn->isValid() || qn->getRefCount() != 0)
        continue;
      QNameItem* victim = NULL;
      theHashSet.eraseNoSync(qn, CompareFunction::hash(qn));
      qn->invalidate(true, &victim);
      if (victim != NULL)
        victims.push_back(victim);
      dropped = true;
    }
    SYNC_CODE(theLock.unlock();)
    for (QNameItem* victim : victims)
      victim->removeReference();
  }
}

} // namespace simplestore
} // namespace zorba

/*******************************************************************************
  A test hook: the bytes of the QName pool's hash table. Its size follows the
  number of names it holds, cached ones included, by halving and doubling, so a
  test that compares the memory two calls leave behind subtracts it.
********************************************************************************/
extern "C" ZORBA_DLL_PUBLIC std::size_t zorba_test_qname_table_bytes()
{
  return GET_STORE().getQNamePool().tableBytes();
}

/*******************************************************************************
  A test hook: drops the names no one holds from the QName pool (see
  QNamePool::flushUnused()).
********************************************************************************/
extern "C" ZORBA_DLL_PUBLIC void zorba_test_qname_pool_flush()
{
  GET_STORE().getQNamePool().flushUnused();
}
