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

// The result iterator of a query that was closed raises ZAPI0042, also when
// another query reads it through an external function.

#include <exception>
#include <functional>
#include <iostream>
#include <string>

#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_sequence.h>
#include <zorba/iterator.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;

void fail(const std::string& aWhat)
{
  std::cerr << "FAILED: " << aWhat << std::endl;
  ++theFailures;
}

const char* const theModuleUri = "urn:iterator-after-close";

class IteratorSequence : public ItemSequence
{
  Iterator_t theIterator;

public:
  explicit IteratorSequence(const Iterator_t& aIterator) : theIterator(aIterator) {}

  Iterator_t getIterator() { return theIterator; }
};

// Returns the result iterator of another query, which it does not open.
class ItemsFunction : public NonContextualExternalFunction
{
public:
  Iterator_t theIterator;

  String getURI() const { return theModuleUri; }

  String getLocalName() const { return "items"; }

  ItemSequence_t evaluate(const ExternalFunction::Arguments_t&) const
  {
    return new IteratorSequence(theIterator);
  }
};

class ItemsModule : public ExternalModule
{
public:
  ItemsFunction theFunction;

  String getURI() const { return theModuleUri; }

  ExternalFunction* getExternalFunction(const String& aLocalName)
  {
    return aLocalName == "items" ? &theFunction : nullptr;
  }
};

std::string error_of(const std::function<void()>& aCall)
{
  try
  {
    aCall();
  }
  catch (ZorbaException const& e)
  {
    return e.diagnostic().qname().localname();
  }
  return "";
}

void check_direct(Zorba* z)
{
  XQuery_t q = z->compileQuery("1 to 3");
  Iterator_t it = q->iterator();
  it->open();
  Item item;
  it->next(item);
  q->close();

  std::string error = error_of([&]() { it->next(item); });
  if (error != "ZAPI0042")
    fail("next after close: error '" + error + "'");

  error = error_of([&]() { it->close(); });
  if (error != "ZAPI0042")
    fail("close after close: error '" + error + "'");
}

void check_through_external_function(Zorba* z)
{
  ItemsModule module;
  StaticContext_t sctx = z->createStaticContext();
  sctx->registerModule(&module);

  XQuery_t a = z->compileQuery("1 to 3");
  module.theFunction.theIterator = a->iterator();

  XQuery_t b = z->compileQuery(
    std::string("declare namespace i = \"") + theModuleUri + "\";\n"
    "declare namespace an = \"http://zorba.io/annotations\";\n"
    "declare %an:nondeterministic function i:items() external;\n"
    "i:items()", sctx);
  Iterator_t it = b->iterator();
  it->open();
  Item item;
  if (!it->next(item) || item.getStringValue() != "1")
    fail("through an external function: first item");
  a->close();

  std::string const error = error_of([&]() { while (it->next(item)) {} });
  if (error != "ZAPI0042")
    fail("through an external function, after close: error '" + error + "'");

  it->close();
  b->close();
  module.theFunction.theIterator = nullptr;
}

} // namespace


int
iterator_after_close(int, char*[])
{
  void* store = StoreManager::getStore();
  Zorba* z = Zorba::getInstance(store);

  try
  {
    check_direct(z);
    check_through_external_function(z);
  }
  catch (std::exception const& e)
  {
    fail(std::string("exception: ") + e.what());
  }

  z->shutdown();
  StoreManager::shutdownStore(store);
  return theFailures == 0 ? 0 : 1;
}
