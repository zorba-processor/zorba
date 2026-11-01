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
#ifndef ZORBA_STORE_LATCH_H
#define ZORBA_STORE_LATCH_H

#include <zorba/config.h>
#include "zorbamisc/config/platform.h"


namespace zorba { 

#ifndef ZORBA_FOR_ONE_THREAD_ONLY

/*******************************************************************************

********************************************************************************/

#ifdef ZORBA_HAVE_PTHREAD_H

class ZORBA_DLL_PUBLIC Latch
{
public:
  enum Mode { READ, WRITE };

protected:
  pthread_rwlock_t  theLatch;

public:
  Latch();

  ~Latch();
    
  void rlock();
  void wlock();

  void unlock();

 private:
  Latch(const Latch &);
  void operator=(const Latch &);
};


/*******************************************************************************

********************************************************************************/

#else
  #error Unsupported thread system
#endif


/*******************************************************************************

********************************************************************************/
class ZORBA_DLL_PUBLIC AutoLatch
{
private:
  Latch & theLatch;

public:
  AutoLatch(Latch& l, Latch::Mode m) : theLatch(l)
  {
    if (m == Latch::READ)
      theLatch.rlock();
    else
      theLatch.wlock();
  }
    
  ~AutoLatch()
  {
    theLatch.unlock();
  }
};


#else // ZORBA_FOR_ONE_THREAD_ONLY

class ZORBA_DLL_PUBLIC Latch
{
public:
  void rlock() {}
  void wlock() {}

  void unlock() {}
};


class  AutoLatch
{
};
#endif // ZORBA_FOR_ONE_THREAD_ONLY


} // namespace zorba


#endif
/* vim:set et sw=2 ts=2: */
