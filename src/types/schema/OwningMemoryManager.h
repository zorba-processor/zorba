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
#ifndef ZORBA_OWNINGMEMORYMANAGER_H_
#define ZORBA_OWNINGMEMORYMANAGER_H_

#include "common/common.h"
#ifndef ZORBA_NO_XMLSCHEMA

#include <cstddef>

#include <xercesc/framework/MemoryManager.hpp>

namespace zorba
{

/*******************************************************************************
  A Xerces-C memory manager that frees, when it is destroyed, every block still
  allocated through it. Xerces-C constructors keep what they allocated when an
  allocation fails (for example SchemaGrammar and XMLScanner), so the objects of
  one validation allocate through one of these, and nothing of them outlives it.
  Not thread-safe: one user at a time.
********************************************************************************/
class OwningMemoryManager : public XERCES_CPP_NAMESPACE::MemoryManager
{
public:
  OwningMemoryManager() = default;
  OwningMemoryManager(const OwningMemoryManager&) = delete;
  OwningMemoryManager& operator=(const OwningMemoryManager&) = delete;
  ~OwningMemoryManager() override;

  // Exceptions outlive the objects that throw them.
  XERCES_CPP_NAMESPACE::MemoryManager* getExceptionMemoryManager() override;

  void* allocate(XMLSize_t size) override;

  void deallocate(void* p) override;

private:
  struct alignas(std::max_align_t) Block
  {
    Block* prev;
    Block* next;
  };

  Block* theFirst = nullptr;
};

} // namespace zorba

#endif // ZORBA_NO_XMLSCHEMA
#endif // ZORBA_OWNINGMEMORYMANAGER_H_
/* vim:set et sw=2 ts=2: */
