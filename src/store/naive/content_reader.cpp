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

#include "content_reader.h"

namespace zorba
{
namespace simplestore
{

ContentReader* ContentReader::theReaders[ContentReader::BUCKETS] = {};


void ContentReader::startReading(const void* key)
{
  assert(key != nullptr);

  if (theKey == key)
    return;

  stopReading();

  ContentReader*& head = theReaders[bucket(key)];
  theKey = key;
  thePrev = nullptr;
  theNext = head;
  if (head != nullptr)
    head->thePrev = this;
  head = this;
}


void ContentReader::unlink()
{
  if (thePrev != nullptr)
    thePrev->theNext = theNext;
  else
    theReaders[bucket(theKey)] = theNext;

  if (theNext != nullptr)
    theNext->thePrev = thePrev;

  theKey = nullptr;
  thePrev = nullptr;
  theNext = nullptr;
}


// Each reader of the key leaves the list before it copies, so a reader is
// copied at most once per change; copyContent() only takes references, which
// changes no list.
void ContentReader::copyContents(const void* key)
{
  ContentReader* reader = theReaders[bucket(key)];

  while (reader != nullptr)
  {
    ContentReader* const next = reader->theNext;

    if (reader->theKey == key)
    {
      reader->unlink();
      reader->copyContent();
    }

    reader = next;
  }
}


} // namespace simplestore
} // namespace zorba

/* vim:set et sw=2 ts=2: */
