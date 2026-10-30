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

// Fuzz target: parse arbitrary bytes as an XML document with
// XmlDataManager::parseXML (fn:parse-xml). External entities and DTDs are
// not loaded with the default load properties (no XML_PARSE_DTDLOAD /
// XML_PARSE_NOENT), so this does not touch the network or the filesystem.

#include <sstream>

#include <zorba/item.h>
#include <zorba/xmldatamanager.h>

#include "fuzz_common.h"

namespace {
zorba::XmlDataManager_t theXmlMgr;
}

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  zorba_fuzz::Engine& eng = zorba_fuzz::engine();
  eng.init();
  theXmlMgr = eng.zorba->getXmlDataManager();
  theXmlMgr->registerDiagnosticHandler(&eng.handler);
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::istringstream in(std::string(reinterpret_cast<const char*>(data), size));
  try
  {
    zorba::Item doc = theXmlMgr->parseXML(in);
  }
  catch (zorba::ZorbaException const& e)
  {
    zorba_fuzz::check_internal(e);
  }
  return 0;
}
