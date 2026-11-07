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
#ifndef ZORBA_SIMPLE_STORE_CONTENT_READER
#define ZORBA_SIMPLE_STORE_CONTENT_READER

#include <cstdint>

namespace zorba
{
namespace simplestore
{

/*******************************************************************************
  An open iteration over the contents of a store item: the children or the
  attributes of a node, the members of an array, the keys of an object.

  An iteration returns the contents the item had when the iteration started
  (init() of a node iterator, open() or reset() of a JSON iterator). A
  change to the item while the iteration is open, such as an update applied
  by a sequential expression or a collection edit, does not reach it, and
  the iteration never reads storage that the change freed or moved. This is
  the value the expression would have if it were evaluated in full when the
  iteration started, which is what a non-lazy evaluation returns.

  The iteration reads the item's own storage while it does not change. The
  code that changes the contents calls contentWillChange() first, with the
  same key that the iteration passed to startReading() (the address of the
  storage). Every iteration reading that key then calls copyContent(): it
  copies what it may still return, holding a reference to each node or item,
  and goes on with the copy. Nothing is copied while nothing changes.

  The open iterations are kept in lists hashed by key, so that a change pays
  only a table lookup when nothing reads that storage. There is one table
  for the process: the engine is used from one thread at a time
  (doc/zorba/threading.dox).

  copyContent() must leave the iteration unchanged if it throws, and must
  not read the key's storage after it returns. A subclass must call
  stopReading() in its destructor, before its members are destroyed.
********************************************************************************/
class ContentReader
{
public:
  static void contentWillChange(const void* key)
  {
    if (theReaders[bucket(key)] != nullptr)
      copyContents(key);
  }

protected:
  ContentReader() = default;

  ContentReader(const ContentReader&) = delete;
  ContentReader& operator=(const ContentReader&) = delete;

  virtual ~ContentReader() { stopReading(); }

  bool isReading() const { return theKey != nullptr; }

  void startReading(const void* key);

  void stopReading()
  {
    if (theKey != nullptr)
      unlink();
  }

  virtual void copyContent() = 0;

private:
  static constexpr unsigned BUCKETS = 256;

  static ContentReader* theReaders[BUCKETS];

  static unsigned bucket(const void* key)
  {
    uintptr_t const k = reinterpret_cast<uintptr_t>(key);
    return static_cast<unsigned>((k >> 4) ^ (k >> 12)) & (BUCKETS - 1);
  }

  static void copyContents(const void* key);

  void unlink();

  const void*     theKey = nullptr;
  ContentReader*  thePrev = nullptr;
  ContentReader*  theNext = nullptr;
};


} // namespace simplestore
} // namespace zorba

#endif

/* vim:set et sw=2 ts=2: */
