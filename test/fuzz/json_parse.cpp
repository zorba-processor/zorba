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

// Fuzz target: parse arbitrary bytes as JSON with JsonDataManager::parseJSON
// (jn:parse-json), allowing multiple top-level items, and drain the result
// so every item is actually parsed.

#include <sstream>

#include <zorba/item.h>
#include <zorba/item_sequence.h>
#include <zorba/iterator.h>
#include <zorba/jsondatamanager.h>

#include "fuzz_common.h"

namespace {
zorba::JsonDataManager_t theJsonMgr;
}

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  zorba_fuzz::Engine& eng = zorba_fuzz::engine();
  eng.init();
  theJsonMgr = eng.zorba->getJsonDataManager();
  theJsonMgr->registerDiagnosticHandler(&eng.handler);
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream in(std::string(reinterpret_cast<const char*>(data), size));
  try
  {
    zorba::ItemSequence_t seq = theJsonMgr->parseJSON(in);
    if (!seq.isNull())
    {
      zorba::Iterator_t it = seq->getIterator();
      it->open();
      zorba::Item item;
      while (it->next(item))
      {
      }
      it->close();
    }
  }
  catch (zorba::ZorbaException const& e)
  {
    zorba_fuzz::check_internal(e);
  }
  return 0;
}
