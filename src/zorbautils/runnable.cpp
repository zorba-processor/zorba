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

#include "runnable.h"

#include <cassert>
#include <errno.h>

#include "diagnostics/assert.h"
#include "zorbautils/fatal.h"

#ifdef ZORBA_HAVE_PTHREAD_H
#include <unistd.h>
#include <pthread.h>
#define ZORBA_THREAD_RETURN void *
#endif

#define THREAD_ID ""


/*******************************************************************************

********************************************************************************/
zorba::Runnable::Runnable()
  : 
  theStatus(IDLE),
  theFinishCalled(false),
  theMutex(),
  theFinishMutex(),
  theFinishCondition(theFinishMutex)
#ifdef ZORBA_HAVE_PTHREAD_H
  ,
  theCondition(theMutex),
  theCalledTerminate(false)
#endif
{
}


/*******************************************************************************

********************************************************************************/
zorba::Runnable::~Runnable()
{
}


/*******************************************************************************
  This method is invoked by the parent thread. It creates and starts a child 
  thread. The child thread will use the static method startImpl as its "main"
  function.
********************************************************************************/
void
zorba::Runnable::start()
{
  theStatus = RUNNING;
  if (pthread_create(&theThread, NULL, &startImpl, (void *) this))
    assert(false);
}


/*******************************************************************************
  This method is invoked by the parent thread
********************************************************************************/
void
zorba::Runnable::terminate()
{
retry:
  theMutex.lock();

#ifdef ZORBA_HAVE_PTHREAD_H
  theCalledTerminate = true;
#endif

  if (theStatus == SUSPENDED) {
    int ret = pthread_cancel(theThread);
    ZORBA_FATAL(!ret, "Failed to cancel thread. Error code = " << ret);
    theCondition.signal();
  } else if (theStatus == TERMINATED) {
    theMutex.unlock();
    return;
  }
  else if (theStatus == RUNNING) {
    // busy wait until the child thread terminates or suspends itself.
    theMutex.unlock();
    usleep(1000);
    goto retry;
  } else {
    assert(theStatus == IDLE);
  }

  theMutex.unlock();

  finishImpl();
}


/*******************************************************************************
  This method is invoked by the parent thread
********************************************************************************/
void
zorba::Runnable::join()
{
  theMutex.lock();

  if (theStatus == TERMINATED || theStatus == IDLE) {
    theMutex.unlock();
    return;
  }

  // the finish condition will wait on this mutex until finishImpl
  // will signal the finish condition.
  theFinishMutex.lock();
  theMutex.unlock();

  // wait for the Runnable to finish executing the user code
  // only finishImpl will signal this as the last thing to to
  theFinishCondition.wait();
  theFinishMutex.unlock();

  theMutex.lock();
  // wait for the real thread to terminate
  if (!theFinishCalled) {
    int status = pthread_join(theThread, 0);
    if (status != EINVAL && status != 0) {
       assert(false);
    }
  }
  theMutex.unlock();
}


/*******************************************************************************
  This method is called by the child thread to suspend itself for a given
  amount of time.
********************************************************************************/
void
zorba::Runnable::suspend(unsigned long aTimeInMs /*= 0*/)
{

  theMutex.lock();

  int ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  ZORBA_FATAL(!ret, "Failed to enable thread cancellation. Error code = " << ret);
  ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  ZORBA_FATAL(!ret, "Failed to set thread cancellation type. Error code = " << ret);
  pthread_cleanup_push(mutexCleanupHandler, &theCondition);

  theStatus = SUSPENDED;

  if (aTimeInMs != 0) 
  {
    theCondition.timedWait(aTimeInMs);
  }
  else 
  {
    theCondition.wait();
  }

  theStatus = RUNNING;

  pthread_testcancel();
  ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  ZORBA_FATAL(!ret, "Failed to disable thread cancellation. Error code = " << ret);
  pthread_cleanup_pop(1);
  theMutex.unlock();
}


#ifdef ZORBA_HAVE_PTHREAD_H
/*******************************************************************************
  Static private method
********************************************************************************/
void
zorba::Runnable::mutexCleanupHandler(void* param)
{
  //Mutex* lMutex = static_cast<Mutex*>(param);
  //lMutex->unlock();
  Condition *lCondition = static_cast<Condition*>(param);
  lCondition->signal();
}
#endif


/*******************************************************************************

********************************************************************************/
void
zorba::Runnable::resume()
{
  theMutex.lock();


  assert (theStatus == SUSPENDED);

  theCondition.signal();

  theStatus = RUNNING;

  theMutex.unlock();
}


/*******************************************************************************
  This is a private static method that serves as the "main" method of the child 
  thread. It is called by the start() method, and is given the Runnable obj as
  input.
********************************************************************************/
ZORBA_THREAD_RETURN
zorba::Runnable::startImpl( void* params )
{
  Runnable* r = static_cast<Runnable*>(params);

  // run the users function in the thread
  r->run();

  // finish if the thread reaches the end of the control flow
  r->finishImpl();

  return NULL;
}


/*******************************************************************************
  Called if the child thread reaches the end of the control flow or terminate
  is requested
********************************************************************************/
void
zorba::Runnable::finishImpl()
{
  theMutex.lock();

#ifndef NDEBUG
  assert (!theFinishCalled);
#endif

  // call the user defined finish
  finish();

  theFinishCalled = true;
  theStatus = TERMINATED;

  if (!theCalledTerminate) {
    int ret = pthread_detach(theThread);
    ZORBA_FATAL(!ret, "Failed to detach thread. Error code = " << ret);
  }

  // the last thing to do is to signal a possible join waiting
  // for this Runnable to terminate
  theFinishCondition.signal();

  theMutex.unlock();
}


void
zorba::Runnable::sleep_(unsigned long useconds) {
  usleep(useconds);
}

/*******************************************************************************

********************************************************************************/
void
zorba::Runnable::reset()
{
  theFinishCalled = false;
  theStatus = IDLE;
}
/* vim:set et sw=2 ts=2: */
