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

#include <cassert>

#include "diagnostics/xquery_diagnostics.h"
#include "zorbautils/fatal.h"
#include "zorbautils/lock.h"
#include "api/zorbaimpl.h"

#if defined(__SANITIZE_THREAD__)
# define ZORBA_LOCK_TSAN 1
#elif defined(__has_feature)
# if __has_feature(thread_sanitizer)
#  define ZORBA_LOCK_TSAN 1
# endif
#endif

#ifdef ZORBA_LOCK_TSAN
# include <sanitizer/tsan_interface.h>
#endif


namespace zorba {

namespace {

/*******************************************************************************
  ThreadSanitizer sees a Lock through its mutex annotations as a reader-writer
  mutex, so that its deadlock detector sees the lock order through it.

  Only a thread's first acquisition and last release are shown: a recursive
  acquisition never waits, and shown it would add false lock-order edges. An
  acquisition is shown once granted, after the internal mutex is released, so
  that no lock-order edge leads from the internal mutex to the lock (an
  inversion is then reported when it is seen, not before a thread blocks on
  it). A release is shown before the lock's state changes, so that another
  thread's acquisition is never shown before it.
********************************************************************************/
enum class Acquisition { FIRST, RECURSIVE, UPGRADE };

#ifdef ZORBA_LOCK_TSAN

unsigned tsanFlags(Lock::Mode aMode)
{
  return aMode == Lock::READ ? __tsan_mutex_read_lock : 0;
}

void tsanUnlock(void* aId, Lock::Mode aMode)
{
  __tsan_mutex_pre_unlock(aId, tsanFlags(aMode));
  __tsan_mutex_post_unlock(aId, tsanFlags(aMode));
}

void tsanLocked(void* aId, Lock::Mode aMode, Acquisition aAcquisition)
{
  switch (aAcquisition)
  {
  case Acquisition::FIRST:
    __tsan_mutex_pre_lock(aId, tsanFlags(aMode));
    __tsan_mutex_post_lock(aId, tsanFlags(aMode), 0);
    break;
  case Acquisition::RECURSIVE:
    break;
  case Acquisition::UPGRADE:
    tsanUnlock(aId, Lock::READ);
    tsanLocked(aId, Lock::WRITE, Acquisition::FIRST);
    break;
  }
}

#else

void tsanLocked(void*, Lock::Mode, Acquisition) {}
void tsanUnlock(void*, Lock::Mode) {}

#endif

}


/*******************************************************************************

********************************************************************************/
Lock::Lock()
  :
  theCondition(theMutex),
  theMode(NOLOCK),
  theHaveUpgradeReq(false),
  theNumWaiters(0),
  theTsanId(0)
{
#ifdef ZORBA_LOCK_TSAN
  __tsan_mutex_create(&theTsanId, 0);
#endif
}

Lock::~Lock()
{
#ifdef ZORBA_LOCK_TSAN
  __tsan_mutex_destroy(&theTsanId, 0);
#endif
  assert(theMode == NOLOCK);
  assert(theNumWaiters == 0);
  assert(theHolders.empty());
  assert(!theHaveUpgradeReq);
}


/*******************************************************************************

********************************************************************************/
void Lock::lock(Mode aMode)
{
  if (aMode == READ)
    rlock();
  else
    wlock();
}


/*******************************************************************************

********************************************************************************/
void Lock::rlock()
{
  theMutex.lock();

  ThreadId thisThread = Runnable::self();
  Acquisition lAcquisition = Acquisition::FIRST;

retry:

  switch (theMode)
  {
  case NOLOCK:
  {
    assert(theHolders.size() == 0);

    LockRequest req(READ, thisThread);
    theHolders.push_back(req);
    theMode = READ;
    break;
  }
  case READ:
  {
    std::vector<LockRequest>::iterator iter = theHolders.begin();
    std::vector<LockRequest>::iterator end = theHolders.end();
    for (; iter != end; iter++)
    {
      if (iter->theThread == thisThread)
        break;
    }

    if (iter != end)
    {
      iter->theCount++;
      lAcquisition = Acquisition::RECURSIVE;
    }
    else if (theNumWaiters > 0)
    {
      theNumWaiters++;
      theCondition.wait();
      theNumWaiters--;
      goto retry;
    }
    else
    {
      LockRequest req(READ, thisThread);
      theHolders.push_back(req);
    }

    break;
  }
  case WRITE:
  {
    assert(theHolders.size() == 1);
    assert(!theHaveUpgradeReq);

    if (theHolders[0].theThread == thisThread)
    {
      assert(theHolders[0].theMode == WRITE);
      theHolders[0].theCount++;
      lAcquisition = Acquisition::RECURSIVE;
    }
    else
    {
      theNumWaiters++;
      theCondition.wait();
      theNumWaiters--;
      goto retry;
    }

    break;
  }
  default:
  {
    ZORBA_FATAL(false, "Corrupted lock structure");
  }
  }

  theMutex.unlock();

  tsanLocked(&theTsanId, READ, lAcquisition);
}


/*******************************************************************************

********************************************************************************/
void Lock::wlock()
{
  theMutex.lock();

  ThreadId thisThread = Runnable::self();
  Acquisition lAcquisition = Acquisition::FIRST;

retry:

  switch (theMode)
  {
  case NOLOCK:
  {
    assert(theHolders.size() == 0);

    LockRequest req(WRITE, thisThread);
    theHolders.push_back(req);
    theMode = WRITE;
    break;
  }
  case READ:
  {
    std::vector<LockRequest>::iterator iter = theHolders.begin();
    std::vector<LockRequest>::iterator end = theHolders.end();
    for (; iter != end; iter++)
    {
      if (iter->theThread == thisThread)
        break;
    }

    if (iter != end)
    {
      // This is an upgrade request
      lAcquisition = Acquisition::UPGRADE;

      // If the current thread is the only holder of the lock, the upgrade
      // request is granted, immediately or after the wait below.
      if (theHolders.size() == 1)
      {
        iter->theMode = WRITE;
        iter->theCount++;
        iter->theUpgrade = false;
        theHaveUpgradeReq = false;
        theMode = WRITE;
        break;
      }

      if (!iter->theUpgrade)
      {
        // If another thread is waiting for an upgrade, allowing the current
        // to wait as well will result in a deadlock. Instead, we throw an
        // exception.
        if (theHaveUpgradeReq)
        {
          theMutex.unlock();
          throw ZORBA_EXCEPTION( zerr::ZXQP0030_DEADLOCK );
        }

        // Else, the thread is made to wait
        iter->theUpgrade = true;
        theHaveUpgradeReq = true;
      }
    }

    theNumWaiters++;
    theCondition.wait();
    theNumWaiters--;
    goto retry;
  }
  case WRITE:
  {
    assert(theHolders.size() == 1);

    if (theHolders[0].theThread == thisThread)
    {
      assert(theHolders[0].theMode == WRITE);
      theHolders[0].theCount++;
      lAcquisition = Acquisition::RECURSIVE;
      break;
    }
    else
    {
      theNumWaiters++;
      theCondition.wait();
      theNumWaiters--;
      goto retry;
    }
  }
  default:
  {
    ZORBA_FATAL(false, "Corrupted lock structure");
  }
  }

  theMutex.unlock();

  tsanLocked(&theTsanId, WRITE, lAcquisition);
}


/*******************************************************************************

********************************************************************************/
void Lock::unlock()
{
  theMutex.lock();

  ThreadId thisThread = Runnable::self();

  if (theMode == WRITE)
  {
    assert(theHolders.size() == 1);
    assert(theHolders[0].theThread == thisThread);
    assert(!theHaveUpgradeReq);

    LockRequest& req = theHolders[0];

    req.theCount--;

    if (req.theCount == 0)
    {
      tsanUnlock(&theTsanId, WRITE);

      theMode = NOLOCK;
      theHolders.clear();

      if (theNumWaiters > 0)
        theCondition.broadcast();
    }
  }
  else
  {
    assert(theMode == READ);

    std::vector<LockRequest>::iterator iter = theHolders.begin();
    std::vector<LockRequest>::iterator end = theHolders.end();
    for (; iter != end; iter++)
    {
      if (iter->theThread == thisThread)
        break;
    }

    assert(iter != end);

    LockRequest& req = *iter;

    assert(req.theMode == READ);
    assert(!req.theUpgrade);

    req.theCount--;

    if (req.theCount == 0)
    {
      tsanUnlock(&theTsanId, READ);

      theHolders.erase(iter);

      if (theHolders.empty())
      {
        theMode = NOLOCK;

        if (theNumWaiters > 0)
          theCondition.broadcast();
      }
      else if (theHaveUpgradeReq && theHolders.size() == 1)
      {
        assert(theNumWaiters > 0);
        assert(theHolders[0].theUpgrade);

        theCondition.broadcast();
      }
    }
  }

  theMutex.unlock();
}


/*******************************************************************************

********************************************************************************/
bool Lock::heldForWrite()
{
  AutoMutex lGuard(&theMutex);
  return theMode == WRITE && theHolders[0].theThread == Runnable::self();
}

}
/* vim:set et sw=2 ts=2: */
