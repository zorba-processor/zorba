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
#include <zorba/config.h>

#include <cassert>
#include <new>

#ifdef ZORBA_HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include "zorbatypes/m_apm.h"

// MAPM has no global lock: its internal state is per-thread (see M_APM_TLS
// and MAPM::threadInit() in m_apm.h).

namespace {

#ifdef ZORBA_HAVE_PTHREAD_H
// The key whose destructor frees the MAPM state of a thread at its exit; a
// thread sets it once its state is complete (m_apm_free_all_mem() cannot free
// what a failed initialization leaves). Making the key allocates nothing; a
// thread sets its value with no allocation either while the process has fewer
// than PTHREAD_KEY_2NDLEVEL_SIZE (32) keys, and gets ENOMEM otherwise, which
// initThread() reports.
class ThreadStateKey
{
public:
  ThreadStateKey()
  {
    if (pthread_key_create(&theKey, &atThreadExit) != 0)
      throw std::bad_alloc();
  }

  // At process exit or library unload. Key destructors do not run for the
  // thread that exits the process, so this frees its state, as the
  // destructor of a thread_local would.
  ~ThreadStateKey()
  {
    if (pthread_getspecific(theKey))
      m_apm_free_all_mem();
    // Fails only for an invalid key, which theKey never is once constructed.
    [[maybe_unused]] int const deleted = pthread_key_delete(theKey);
    assert(deleted == 0);
  }

  ThreadStateKey(ThreadStateKey const&) = delete;
  ThreadStateKey& operator=(ThreadStateKey const&) = delete;

  pthread_key_t get() const { return theKey; }

private:
  static void atThreadExit(void*) { m_apm_free_all_mem(); }

  pthread_key_t theKey;
};
#else
struct ThreadState
{
  ~ThreadState() { m_apm_free_all_mem(); }
};
#endif

} // namespace

void MAPM::initThread()
{
#ifdef ZORBA_HAVE_PTHREAD_H
  static ThreadStateKey const theThreadStateKey;
  m_apm_free(m_apm_init());
  if (pthread_setspecific(theThreadStateKey.get(), &theThreadReady) != 0)
  {
    m_apm_free_all_mem();
    throw std::bad_alloc();
  }
#else
  m_apm_free(m_apm_init());
  thread_local ThreadState theThreadState;
  (void)theThreadState;
#endif
  theThreadReady = true;
}

// MAPM is out of memory: see M_apm_log_error_msg.
extern "C" void M_apm_out_of_memory() {
  throw std::bad_alloc();
}


#if 0
#include "zorbatypes/numconversions.h"
#include "zorbautils/hashfun.h"


uint32_t MAPM::hash() const
{
  if (is_integer())
  {
    if (sign() < 0)
    {
      if (*this >= MAPM::getMinInt64())
      {
        char buffer[exponent() + 3];
        toIntegerString(buffer);
        std::stringstream strstream(buffer);
        int64_t longv;
        strstream >> longv;
        assert(strstream.eof());
        return static_cast<uint32_t>(longv & 0xffffffff);
      }
    }
    else if (*this <= MAPM::getMaxUInt64())
    {
      char buffer[exponent() + 3];
      toIntegerString(buffer);
      std::stringstream strstream(buffer);
      uint64_t longv;
      strstream >> longv;
      assert(strstream.eof());
      return static_cast<uint32_t>(longv & 0xffffffff);
    }
  }

  // In all other cases, hash it as double
  char buffer[exponent() + 3];
  toIntegerString(buffer);
  std::stringstream strstream(buffer);
  double doublev;
  strstream >> doublev;
  assert(strstream.eof());
  return static_cast<uint32_t>(doublev);
}
#endif

/* vim:set et sw=2 ts=2: */
