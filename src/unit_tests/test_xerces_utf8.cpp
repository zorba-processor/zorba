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
// Conversion of Xerces-C strings (XMLCh, UTF-16) to and from UTF-8 must not
// depend on the process locale, and XMLChArray / StrX must not leak or write
// through a null buffer.

#include "stdafx.h"

#ifndef ZORBA_NO_XMLSCHEMA

#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include "types/schema/StrX.h"
#include "zorbatypes/xerces_xmlcharray.h"
#include "zorbatypes/zstring.h"

namespace zorba {
namespace UnitTests {

namespace {

int theFailures = 0;

void check(bool aCond, const char* aWhat)
{
  if (!aCond)
  {
    std::cerr << "FAILED: " << aWhat << std::endl;
    ++theFailures;
  }
}

// "Größe" and U+1F600 (a surrogate pair in UTF-16)
const char GROESSE[] = "Gr\xC3\xB6\xC3\x9F" "e";
const char SMILEY[] = "\xF0\x9F\x98\x80";

void test_to_xmlch()
{
  XMLChArray lArr(zstring(GROESSE));
  const XMLCh* p = lArr.get();
  check(p && p[0] == 'G' && p[1] == 'r' && p[2] == 0xF6 && p[3] == 0xDF &&
        p[4] == 'e' && p[5] == 0, "UTF-8 -> XMLCh keeps non-ASCII characters");

  XMLChArray lSmiley(zstring(SMILEY));
  p = lSmiley.get();
  check(p && p[0] == 0xD83D && p[1] == 0xDE00 && p[2] == 0,
        "UTF-8 -> XMLCh makes a surrogate pair");

  XMLChArray lBad(zstring("a\xFF" "z"));
  p = lBad.get();
  check(p && p[0] == 'a' && p[1] == 0xFFFD && p[2] == 'z' && p[3] == 0,
        "invalid UTF-8 becomes U+FFFD");
}

void test_ostream()
{
  // operator<< used to leak the buffer returned by XMLString::transcode
  XMLChArray lArr(zstring(GROESSE));
  std::ostringstream lOut;
  lOut << lArr;
  check(lOut.str() == GROESSE, "XMLChArray streams as UTF-8");
}

void test_strx()
{
  XMLChArray lArr(zstring(GROESSE));
  StrX lAll(lArr.get());
  check(lAll.localForm() && std::string(lAll.localForm()) == GROESSE,
        "StrX(str) is UTF-8");

  // the length-limited constructor used to write into a null buffer
  StrX lPrefix(lArr.get(), 3);
  check(lPrefix.localForm() && std::string(lPrefix.localForm()) == "Gr\xC3\xB6",
        "StrX(str, 3) is the UTF-8 form of the first 3 characters");

  XMLChArray lSmiley(zstring(SMILEY));
  StrX lS(lSmiley.get());
  check(lS.localForm() && std::string(lS.localForm()) == SMILEY,
        "surrogate pair round-trips to UTF-8");

  StrX lNull(static_cast<const XMLCh*>(0));
  check(lNull.localForm() == 0 &&
        std::string(lNull.localFormOrDefault("dflt")) == "dflt",
        "StrX(null) has no local form");
}

} // namespace

int test_xerces_utf8( int, char*[] )
{
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
  test_to_xmlch();
  test_ostream();
  test_strx();
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();

  std::cout << theFailures << " test(s) failed\n";
  return theFailures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba

#endif /* ZORBA_NO_XMLSCHEMA */

/* vim:set et sw=2 ts=2: */
