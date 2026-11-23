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

// An upgrade of the store's Lock (READ to WRITE) that has to wait for another
// reader. Thread A and thread B hold READ; A asks for WRITE and waits; B
// releases and A gets WRITE. After A releases, the lock must be free with no
// upgrade request left: a second waited upgrade must be granted (not refused
// with ZXQP0030 as a deadlock), and a recursive READ inside the upgraded
// WRITE must work.

#include <atomic>
#include <string>
#include <thread>

#include <zorba/zorba_exception.h>

#include "zorbautils/lock.h"

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

class ProbedLock : public Lock
{
public:
  ulong waiters()
  {
    AutoMutex lGuard(&theMutex);
    return theNumWaiters;
  }

  // The lock's state when no thread holds it, as text; "free" when clean.
  std::string idleState()
  {
    AutoMutex lGuard(&theMutex);
    std::string lState;
    if (theMode != NOLOCK)
      lState += " mode";
    if (!theHolders.empty())
      lState += " holders";
    if (theHaveUpgradeReq)
      lState += " upgrade-request";
    if (theNumWaiters != 0)
      lState += " waiters";
    return lState.empty() ? "free" : "not free:" + lState;
  }
};

// One waited upgrade; the result is "granted" or the error it raised.
std::string waitedUpgrade(ProbedLock& aLock)
{
  std::atomic<bool> lReaderHolds{false};
  std::atomic<bool> lReaderMayRelease{false};

  std::thread lReader([&]()
  {
    aLock.rlock();
    lReaderHolds = true;
    while (!lReaderMayRelease)
      std::this_thread::yield();
    aLock.unlock();
  });

  std::string lResult;
  aLock.rlock();
  while (!lReaderHolds)
    std::this_thread::yield();

  std::thread lReleaser([&]()
  {
    // The upgrade below is waiting once it is counted as a waiter.
    while (aLock.waiters() == 0)
      std::this_thread::yield();
    lReaderMayRelease = true;
  });

  try
  {
    aLock.wlock();
    aLock.rlock();
    aLock.unlock();
    aLock.unlock();
    lResult = "granted";
  }
  catch (ZorbaException const& e)
  {
    lResult = std::string("error: ") + e.what();
    lReaderMayRelease = true;
  }
  aLock.unlock();

  lReleaser.join();
  lReader.join();
  return lResult;
}

} // namespace


int
mt_lock_upgrade(int, char*[])
{
  Checker lChecker;
  ProbedLock lLock;

  for (unsigned i = 0; i < 2; ++i)
  {
    std::string const lWhat = "waited upgrade " + std::to_string(i + 1);
    lChecker.expect(lWhat, 0, waitedUpgrade(lLock), "granted");
    lChecker.expect(lWhat + ", lock after release", 0, lLock.idleState(),
                    "free");
  }

  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
