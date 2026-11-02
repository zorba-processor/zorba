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

#include "api/zorbaimpl.h"

#include "zorbautils/fatal.h"
#include "zorbautils/latch.h"
#include <cassert>
#ifdef ZORBA_HAVE_PTHREAD_H
#  include <pthread.h>
#endif


namespace zorba { 


#ifndef ZORBA_FOR_ONE_THREAD_ONLY


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  pthread Latch                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#ifdef ZORBA_HAVE_PTHREAD_H


Latch::Latch()
{
  pthread_rwlockattr_t attr;
  int ret = pthread_rwlockattr_init(&attr);
  ZORBA_FATAL(!ret, "Failed to initialize latch attributes. Error code = " << ret);

#ifndef NDEBUG
  int pshared;
  ret = pthread_rwlockattr_getpshared(&attr, &pshared);
  ZORBA_FATAL(!ret, "Failed to get latch pshared attribute. Error code = " << ret);
  ZORBA_FATAL(pshared == PTHREAD_PROCESS_PRIVATE,
              "rwlock pshared = " << pshared);
#endif

  ret = pthread_rwlock_init(&theLatch, &attr);

  // Cannot fail on an initialized attributes object (POSIX, glibc).
  (void)pthread_rwlockattr_destroy(&attr);

  ZORBA_FATAL(!ret, "Failed to initialize latch. Error code = " << ret);
}



Latch::~Latch()
{
  int ret = pthread_rwlock_destroy(&theLatch);
  ZORBA_FATAL(!ret, "Failed to destroy latch. Error code = " << ret);
}


void Latch::rlock()
{ 
  int ret= pthread_rwlock_rdlock(&theLatch); 
  ZORBA_FATAL(!ret, "Failed to acquire latch. Error code = " << ret);
}


void Latch::wlock()
{ 
  int ret = pthread_rwlock_wrlock(&theLatch); 
  ZORBA_FATAL(!ret, "Failed to acquire latch. Error code = " << ret);
}


void Latch::unlock()
{ 
  int ret= pthread_rwlock_unlock(&theLatch);
  ZORBA_FATAL(!ret, "Failed to release latch. Error code = " << ret);
}


#endif


#endif

} // namespace zorba
/* vim:set et sw=2 ts=2: */
