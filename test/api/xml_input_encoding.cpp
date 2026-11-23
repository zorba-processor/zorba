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

// XmlDataManager::parseXML() on raw byte streams in encodings other than
// UTF-8, as a document and as an external parsed entity (fragment): UTF-16
// and UTF-32 input parses, errors are located in characters of the input, and
// bytes that are not valid in the encoding the input is read in are reported
// as such.

#include <iostream>
#include <sstream>
#include <string>

#include <zorba/item_sequence.h>
#include <zorba/serializer.h>
#include <zorba/store_manager.h>
#include <zorba/xmldatamanager.h>
#include <zorba/xquery_exception.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;
Zorba* theZorba = nullptr;

#define CHECK_EQ(actual, expected)                                       \
  do {                                                                   \
    std::string const lActual = (actual);                                \
    std::string const lExpected = (expected);                            \
    if (lActual != lExpected) {                                          \
      std::cerr << __FILE__ << ":" << __LINE__ << ": expected \""        \
                << lExpected << "\", got \"" << lActual << "\""          \
                << std::endl;                                            \
      ++theFailures;                                                     \
    }                                                                    \
  } while (0)

enum Route { DOCUMENT, FRAGMENT };

// The serialized result, or "code line:column message" of the error, with
// the message reduced to what follows "loader parsing error: ".
std::string parse(std::string const& aBytes, Route aRoute)
{
  std::istringstream lIn(aBytes);
  try
  {
    XmlDataManager::ParseOptions lOptions;
    lOptions.setExternalEntityProcessing(aRoute == FRAGMENT);
    ItemSequence_t lSeq = theZorba->getXmlDataManager()->parseXML(lIn, lOptions);
    Zorba_SerializerOptions_t lSerOptions;
    lSerOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
    std::ostringstream lOut;
    Serializer::createSerializer(lSerOptions)->serialize(lSeq->getIterator(), lOut);
    return lOut.str();
  }
  catch (XQueryException const& e)
  {
    std::string lMessage(e.what());
    std::string const lPrefix("loader parsing error: ");
    std::string::size_type const lPos = lMessage.find(lPrefix);
    if (lPos != std::string::npos)
      lMessage.erase(0, lPos + lPrefix.size());
    std::ostringstream lOut;
    lOut << e.diagnostic().qname().localname() << ' ' << e.data_line() << ':'
         << e.data_column() << ' ' << lMessage;
    return lOut.str();
  }
}

// aText in UTF-16 or UTF-32, with a byte order mark if aBom.
std::string encode(std::u32string const& aText, unsigned aUnit, bool aBigEndian,
                   bool aBom)
{
  std::string lBytes;
  auto lPut = [&](char32_t aCodeUnit) {
    for (unsigned i = 0; i < aUnit; ++i)
    {
      unsigned const lShift = 8 * (aBigEndian ? aUnit - 1 - i : i);
      lBytes += static_cast<char>((aCodeUnit >> lShift) & 0xFF);
    }
  };
  if (aBom)
    lPut(0xFEFF);
  for (char32_t c : aText)
  {
    if (aUnit == 2 && c > 0xFFFF)
    {
      lPut(0xD800 + ((c - 0x10000) >> 10));
      lPut(0xDC00 + ((c - 0x10000) & 0x3FF));
    }
    else
    {
      lPut(c);
    }
  }
  return lBytes;
}

std::u32string const DECLARED_UTF16 =
  U"<?xml version=\"1.0\" encoding=\"UTF-16\"?>"
  U"<!DOCTYPE r [<!ENTITY e \"©±é\">]>";

void test_utf16_parses()
{
  std::u32string const lText = DECLARED_UTF16 + U"<a>&e;</a>";
  std::string const lWant = "<a>\xC2\xA9\xC2\xB1\xC3\xA9</a>";
  for (Route lRoute : { DOCUMENT, FRAGMENT })
  {
    CHECK_EQ(parse(encode(lText, 2, false, true), lRoute), lWant);
    CHECK_EQ(parse(encode(lText, 2, true, true), lRoute), lWant);
    // Without a byte order mark, "<?" tells UTF-16 (XML 1.0 F.1).
    CHECK_EQ(parse(encode(lText, 2, false, false), lRoute), lWant);
  }
}

void test_utf32_parses()
{
  std::u32string const lText = U"<a>é\U0001F600</a>";
  std::string const lWant = "<a>\xC3\xA9&#128512;</a>";
  CHECK_EQ(parse(encode(lText, 4, false, true), FRAGMENT), lWant);
  CHECK_EQ(parse(encode(lText, 4, true, false), FRAGMENT), lWant);
}

void test_error_location()
{
  // A fragment error is where a document puts it.
  std::u32string const lOneLine = DECLARED_UTF16 + U"<a></b>";
  std::string const lAtEnd =
    "FODC0006 1:81 \"</b>\": closing tag mismatch for \"<a>\"";
  for (Route lRoute : { DOCUMENT, FRAGMENT })
  {
    CHECK_EQ(parse(encode(lOneLine, 2, false, true), lRoute), lAtEnd);
    CHECK_EQ(parse(encode(lOneLine, 2, true, true), lRoute), lAtEnd);
  }

  // Columns count characters: a surrogate pair is one.
  std::u32string const lTwoLines =
    DECLARED_UTF16 + U"\n<x>\U0001F600é</x><a></b>";
  std::string const lWant =
    "FODC0006 2:17 \"</b>\": closing tag mismatch for \"<a>\"";
  CHECK_EQ(parse(encode(lTwoLines, 2, false, true), FRAGMENT), lWant);
  CHECK_EQ(parse(encode(lTwoLines, 2, true, true), FRAGMENT), lWant);
  CHECK_EQ(parse(encode(U"<a>é</b>", 4, true, true), FRAGMENT),
           "FODC0006 1:9 \"</b>\": closing tag mismatch for \"<a>\"");
}

void test_invalid_bytes()
{
  // An unpaired UTF-16 surrogate.
  std::string const lSurrogate = encode(U"<a>x", 2, false, true) +
    std::string("\x00\xD8", 2) + encode(U"y</a>", 2, false, false);
  CHECK_EQ(parse(lSurrogate, FRAGMENT),
           "FODC0006 1:5 input is not valid UTF-16LE");

  // A broken UTF-8 sequence.
  std::string const lUtf8 = "<a>\n<b>\xC3\xA9\xC3\x28</b></a>";
  CHECK_EQ(parse(lUtf8, FRAGMENT), "FODC0006 2:5 input is not valid UTF-8");
  CHECK_EQ(parse(lUtf8, DOCUMENT),
           "FODC0006 2:5 input is not valid in its character encoding");

  // The stream is taken as a string, so its declaration does not apply:
  // ISO-8859-1 bytes are read as UTF-8.
  std::string const lLatin1 =
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<a>x\xE9</a>";
  CHECK_EQ(parse(lLatin1, FRAGMENT), "FODC0006 2:5 input is not valid UTF-8");
  CHECK_EQ(parse(lLatin1, DOCUMENT),
           "FODC0006 2:5 input is not valid in its character encoding");
}

} // namespace


int
xml_input_encoding(int, char*[])
{
  void* lStore = StoreManager::getStore();
  theZorba = Zorba::getInstance(lStore);

  try
  {
    std::cout << "test_utf16_parses" << std::endl;
    test_utf16_parses();
    std::cout << "test_utf32_parses" << std::endl;
    test_utf32_parses();
    std::cout << "test_error_location" << std::endl;
    test_error_location();
    std::cout << "test_invalid_bytes" << std::endl;
    test_invalid_bytes();
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "unexpected exception: " << e << std::endl;
    ++theFailures;
  }

  theZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
