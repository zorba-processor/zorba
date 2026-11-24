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
#ifndef ZORBA_SIMPLE_STORE_QNAME_POOL
#define ZORBA_SIMPLE_STORE_QNAME_POOL

#include <memory>
#include <vector>

#include "zorbautils/hashfun.h"
#include "zorbautils/hashset.h"

#include "common/shared_types.h"
#include "common/common.h"

#include "atomic_items.h"

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
#include <atomic>
#include <mutex>
#endif

namespace zorba { namespace simplestore {

class StringPool;

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
/*******************************************************************************
  A reader/writer lock whose readers count in one of several cache lines,
  chosen by thread, so that readers on different threads mostly write to
  different lines. Not recursive. A reader that finds a writer waits until it
  is done; a writer waits for the readers that came before it.
********************************************************************************/
class ShardedSharedLock
{
  static constexpr unsigned SLOT_BITS = 4;
  static constexpr unsigned NUM_SLOTS = 1u << SLOT_BITS;

  struct alignas(64) Slot
  {
    std::atomic<unsigned long> theReaders{0};
  };

  Slot               theSlots[NUM_SLOTS];
  std::atomic<bool>  theWriter{false};
  std::mutex         theWriterMutex;

  static unsigned threadSlot();

  void waitForWriter() const;

public:
  unsigned lockShared()
  {
    unsigned const slot = threadSlot();
    for (;;)
    {
      theSlots[slot].theReaders.fetch_add(1, std::memory_order_seq_cst);
      if (!theWriter.load(std::memory_order_seq_cst))
        return slot;
      theSlots[slot].theReaders.fetch_sub(1, std::memory_order_release);
      waitForWriter();
    }
  }

  void unlockShared(unsigned slot)
  {
    theSlots[slot].theReaders.fetch_sub(1, std::memory_order_release);
  }

  void lock();

  void unlock()
  {
    theWriter.store(false, std::memory_order_release);
    theWriterMutex.unlock();
  }
};
#endif

/*******************************************************************************

  theCache :
  ----------
  An array of QName slots that is managed as a cache. This means that slots that
  are not used are placed in a free list. When a new qname is inserted in the
  cache, a slot is taken from the free list and the qname currently in that slot
  is replaced with the new slot.

  theCacheSize :
  --------------
  The size of theCache (number of slots). This size is given as a param to the
  QNamePool constructor, and it never changes afterwards.
 
  theFirstFree :
  --------------
  The position in theCache of the 1st free slot. NOTE: the 1st slot of theCache
  (at position 0) is reserved (i.e., never used) so that position 0 can be used
  to indicate the end of the free list.

  theNumFree :
  ------------
  Number of free slots in theCache.

  theHashSet :
  ------------
  A hash set mapping qnames (i.e. triplets of strings) to QName slots.

  theLock :
  ---------
  Guards the hash set and the cache. In the multithreaded build a lookup takes
  it shared, and adds the reference to the name it finds even if the name had
  none: such a name stays in the free list, and popFreeList() skips it while it
  has references. Everything else takes it exclusive, including remove(), where
  a count goes from 1 to 0.

********************************************************************************/
class QNamePool
{
protected:

  class CompareFunction
  {
  public:
    static bool equal(const QNameItem* t1, const QNameItem* t2)
    {
      return (t1->getLocalName2() == t2->getLocalName2() &&
              t1->getNamespace2() == t2->getNamespace2() &&
              t1->getPrefix2() == t2->getPrefix2());
    }

    static uint32_t hash(const QNameItem* t)
    {
      return  hashfun::h32(t->getPrefix2().c_str(),
                           hashfun::h32(t->getLocalName2().c_str(),
                                        hashfun::h32(t->getNamespace2().c_str())));
    }
  };


  class QNamePoolHashSet : public HashSet<QNameItem*, CompareFunction>
  {
    friend class QNamePool;

  public:
    QNamePoolHashSet(ulong size) 
      :
      HashSet<QNameItem*, CompareFunction>(size, false)
    {
    }
  };


 typedef HashEntry<QNameItem*, DummyHashValue> QNHashEntry;

public:
  static const ulong MAX_CACHE_SIZE = 32768;

protected:
  std::unique_ptr<QNameItem[]> theCache;
  ulong               theCacheSize;
	ulong               theFirstFree;
  ulong               theNumFree;

  QNamePoolHashSet    theHashSet;

  StringPool        * theNamespacePool;

  SYNC_CODE(ShardedSharedLock theLock;)

public:
  QNamePool(ulong size, StringPool* nspool);

  ~QNamePool();

  // The bytes of the hash table (its capacity in entries times their size), for
  // tests that measure the memory a call leaves: see
  // zorba_test_qname_table_bytes().
  std::size_t tableBytes() const;

  // Drops the names no one holds (cached for reuse) from the pool, for the same
  // tests: the names a call leaves cached depend on the order of its insertions,
  // not on what it leaked. See zorba_test_qname_pool_flush().
  void flushUnused();

  void insert(
      store::Item_t& res,
      const char* ns,
      const char* pre,
      const char* ln);
  
  void insert(
      store::Item_t& res,
      const zstring& ns,
      const zstring& pre,
      const zstring& ln);
  
  void remove(QNameItem* qn);

protected:
  QNameItem* cacheInsert(QNameItem*& normVictim);

  void undoCacheInsert(QNameItem* qn, QNameItem** normVictim);

  void cachePin(QNameItem* qn);

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
  bool findShared(
      store::Item_t& res,
      const char* ns,
      const char* pre,
      const char* ln,
      csize       nslen,
      csize       prelen,
      csize       lnlen,
      csize       hval);
#endif

  QNHashEntry* hashFind(
      const char* ns,
      const char* pre,
      const char* ln,
      csize       nslen,
      csize       prelen,
      csize       lnlen,
      csize       hval);

  void addInFreeList(QNameItem* qn);

  void removeFromFreeList(QNameItem* qn);

  QNameItem* popFreeList();
};


} // namespace store
} // namespace zorba

#endif
/* vim:set et sw=2 ts=2: */
