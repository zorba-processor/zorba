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

#include "zorbautils/fatal.h"
#include "zorbautils/mutex.h"
#include "zorbautils/condition.h"
#include <cassert>
#include <cerrno>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string>

namespace zorba { 

#ifdef ZORBA_HAVE_PTHREAD_H


Condition::Condition(Mutex& m) : theMutex(m) 
{
  int ret = pthread_cond_init(&theCondition, NULL); 

  ZORBA_FATAL(!ret, "Failed to create condition variable. Error code = " << ret);
}


Condition::~Condition()
{
  //int ret = pthread_cond_destroy(&theCondition); 

  //ZORBA_FATAL(!ret, "Failed to destroy condition variable. Error code = " << ret);
}


void Condition::wait() 
{
  //theMutex.lock();
  int ret = pthread_cond_wait(&theCondition, theMutex.getMutex());

  if (ret)
    std::cerr << "Condition wait error: " << strerror(ret) << std::endl;

  ZORBA_FATAL(!ret, "Failed to wait on condition variable. Error code = " << ret);
}

void Condition::timedWait(unsigned long aTimeInsMs)
{
  struct timespec lTimespec;
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  memset(&lTimespec, 0, sizeof(lTimespec));
  lTimespec.tv_sec = tv.tv_sec + aTimeInsMs/1000;
  int ret = pthread_cond_timedwait(&theCondition, theMutex.getMutex(), &lTimespec);
  ZORBA_FATAL(!ret || ret == ETIMEDOUT,
              "Failed to wait on condition variable. Error code = " << ret);
}


void Condition::signal() 
{
  int ret = pthread_cond_signal(&theCondition);

  ZORBA_FATAL(!ret, "Failed to signal condition variable. Error code = " << ret);
}


void Condition::broadcast() 
{
  int ret = pthread_cond_broadcast(&theCondition);

  ZORBA_FATAL(!ret, "Failed to brodcast condition variable. Error code = " << ret);
}

#else

Condition::Condition(Mutex& m) : theMutex(m) {}
Condition::~Condition() {}

#endif // ZORBA_HAVE_PTHREAD_H

}
/* vim:set et sw=2 ts=2: */
