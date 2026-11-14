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
#include <cstdlib>
#include <iostream>
#include <limits>
#include <streambuf>
#include <string>
#include <string.h>

#include <zorba/zorba.h>
#include <zorba/store_manager.h>
#include <zorba/serializer.h>
#include <zorba/zorba_exception.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/diagnostic_list.h>


using namespace zorba;


// test for bug #3085093 "ZorbaError not caught in SerializerImpl::serialize"
bool
test_serialization_error(Zorba* aZorba)
{
  XQuery_t lQuery = aZorba->compileQuery("<a a='1'/>/@a"); 

  Zorba_SerializerOptions_t lOptions;
  Serializer_t lSerializer = Serializer::createSerializer(lOptions);

  Iterator_t lIterator = lQuery->iterator();

  try {
    lIterator->open();

    Item lItem;
    while ( lIterator->next(lItem) ) {
      // we have to wrap the item in a Serializable object
      SingletonItemSequence lSequence(lItem);
      lSerializer->serialize(&lSequence, std::cout);
    }

    lIterator->close();

  } catch (ZorbaException& e) {
    std::cerr << e << std::endl;
    lIterator->close();
    return true;
  }
	return false;
}


// Takes the first theCapacity characters, then fails every write, as a full
// disk does; the stream it backs does not throw.
class limited_streambuf : public std::streambuf
{
public:
  explicit limited_streambuf(std::string::size_type aCapacity)
    : theCapacity(aCapacity) {}

  std::string const& text() const { return theText; }

protected:
  int_type overflow(int_type c) override
  {
    if (traits_type::eq_int_type(c, traits_type::eof()))
      return traits_type::not_eof(c);
    if (theText.size() == theCapacity)
      return traits_type::eof();
    theText += traits_type::to_char_type(c);
    return c;
  }

private:
  std::string theText;
  std::string::size_type const theCapacity;
};


char const* const theLongQuery =
  "<r>{ for $i in 1 to 100 return <e>{ $i }</e> }</r>";


// Expects aSerialize to raise ZOSE0004 for a stream that took only part of
// the text.
template<typename Serialize>
bool expect_write_failure(char const* aCase, Serialize aSerialize)
{
  limited_streambuf lBuf(16);
  std::ostream lOut(&lBuf);
  try {
    aSerialize(lOut);
  } catch (ZorbaException const& e) {
    if (e.diagnostic() == zerr::ZOSE0004_IO_ERROR)
      return true;
    std::cerr << aCase << ": wrong error: " << e << std::endl;
    return false;
  }
  std::cerr << aCase << ": returned normally with \"" << lBuf.text()
            << "\"" << std::endl;
  return false;
}


bool
test_failed_stream(Zorba* aZorba)
{
  bool lOk = true;

  lOk &= expect_write_failure("XQuery::execute", [&](std::ostream& os) {
    XQuery_t lQuery = aZorba->compileQuery(theLongQuery);
    lQuery->execute(os);
  });

  lOk &= expect_write_failure("Serializer::serialize", [&](std::ostream& os) {
    XQuery_t lQuery = aZorba->compileQuery(theLongQuery);
    Zorba_SerializerOptions_t lOptions;
    Serializer::createSerializer(lOptions)->serialize(lQuery->iterator(), os);
  });

  // The transcoder resets the stream's state when it is detached.
  lOk &= expect_write_failure("ISO-8859-1", [&](std::ostream& os) {
    XQuery_t lQuery = aZorba->compileQuery(theLongQuery);
    Zorba_SerializerOptions_t lOptions;
    lOptions.set("encoding", "ISO-8859-1");
    lQuery->execute(os, &lOptions);
  });

  // A stream that takes the whole text gets it, and no error.
  limited_streambuf lBuf(std::numeric_limits<std::string::size_type>::max());
  std::ostream lOut(&lBuf);
  Zorba_SerializerOptions_t lOptions;
  lOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
  aZorba->compileQuery("<a>{ 1 to 3 }</a>")->execute(lOut, &lOptions);
  if (lBuf.text() != "<a>1 2 3</a>") {
    std::cerr << "whole text: got \"" << lBuf.text() << "\"" << std::endl;
    lOk = false;
  }

  return lOk;
}


int
serializer(int argc, char* argv[]) 
{
  Zorba* lZorba = Zorba::getInstance(zorba::StoreManager::getStore());

  if (!test_serialization_error(lZorba)) {
    return 1;
  }

  if (!test_failed_stream(lZorba)) {
    return 1;
  }

  return 0;
}

