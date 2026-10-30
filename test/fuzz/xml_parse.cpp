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

// Fuzz target: parse arbitrary bytes as an XML document with fn:parse-xml.
// The default load properties neither load external DTDs nor substitute
// external entities, but libxml2 still tries to open some external entities,
// so the harness installs an entity loader that refuses everything: the
// fuzzer must stay off the network and the filesystem.
//
// XmlDataManager is not used because it needs XML Schema support (it imports
// the parse options schema), which the core fuzz build leaves out.

#include <libxml/parser.h>

#include "fuzz_common.h"

namespace {
zorba_fuzz::ParseQuery* theQuery;

xmlParserInputPtr denyEntityLoader(const char*, const char*, xmlParserCtxtPtr)
{
  return nullptr;
}
}

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
  zorba_fuzz::engine().init();
  xmlSetExternalEntityLoader(&denyEntityLoader);
  theQuery = new zorba_fuzz::ParseQuery();
  theQuery->init(
      "declare variable $input as xs:string external;\n"
      "fn:parse-xml($input)");
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  theQuery->run(data, size);
  return 0;
}
