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
#ifndef ZORBA_NO_XMLSCHEMA

#include "types/schema/OwningMemoryManager.h"

#include <limits>
#include <new>

#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_USE;

namespace zorba
{

OwningMemoryManager::~OwningMemoryManager()
{
  while (theFirst)
  {
    Block* const next = theFirst->next;
    ::operator delete(theFirst);
    theFirst = next;
  }
}


XERCES_CPP_NAMESPACE::MemoryManager*
OwningMemoryManager::getExceptionMemoryManager()
{
  return XMLPlatformUtils::fgMemoryManager->getExceptionMemoryManager();
}


void* OwningMemoryManager::allocate(XMLSize_t size)
{
  if (size > std::numeric_limits<std::size_t>::max() - sizeof(Block))
    throw OutOfMemoryException();

  Block* block;
  try
  {
    block = static_cast<Block*>(::operator new(sizeof(Block) + size));
  }
  catch (const std::bad_alloc&)
  {
    // What Xerces-C expects of a memory manager.
    throw OutOfMemoryException();
  }

  std::lock_guard<std::mutex> lock(theMutex);
  block->prev = nullptr;
  block->next = theFirst;
  if (theFirst)
    theFirst->prev = block;
  theFirst = block;
  return block + 1;
}


void OwningMemoryManager::deallocate(void* p)
{
  if (!p)
    return;

  Block* const block = static_cast<Block*>(p) - 1;
  {
    std::lock_guard<std::mutex> lock(theMutex);
    if (block->prev)
      block->prev->next = block->next;
    else
      theFirst = block->next;
    if (block->next)
      block->next->prev = block->prev;
  }
  ::operator delete(block);
}

} // namespace zorba

#endif // ZORBA_NO_XMLSCHEMA
/* vim:set et sw=2 ts=2: */
