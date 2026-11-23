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

// Lock order of the engine's users mutex and the store lock. The first
// Zorba::getInstance initializes the store, taking the store lock while it
// holds the users mutex. External functions get the engine with
// Zorba::getInstance(0) while their query holds the store lock (the modules
// shipped with Zorba do so for their item factory); that call must not take
// the users mutex. ThreadSanitizer reports the inversion even when the
// threads do not deadlock.

#include <string>

#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_factory.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char* const MODULE_URI = "urn:mt-module-instance";

// Returns its argument, made with the item factory of Zorba::getInstance(0).
class EchoFunction : public NonContextualExternalFunction
{
public:
  String getURI() const { return MODULE_URI; }

  String getLocalName() const { return "echo"; }

  ItemSequence_t evaluate(const ExternalFunction::Arguments_t& aArgs) const
  {
    Iterator_t lArg = aArgs[0]->getIterator();
    lArg->open();
    Item lItem;
    lArg->next(lItem);
    lArg->close();
    ItemFactory* lFactory = Zorba::getInstance(0)->getItemFactory();
    return new SingletonItemSequence(
      lFactory->createString(lItem.getStringValue()));
  }
};

class EchoModule : public ExternalModule
{
public:
  EchoFunction theFunction;

  String getURI() const { return MODULE_URI; }

  ExternalFunction* getExternalFunction(const String& aLocalName)
  {
    return aLocalName == "echo" ? &theFunction : nullptr;
  }
};

} // namespace


int
mt_module_instance(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  EchoModule lModule;
  {
    StaticContext_t lSctx = lZorba->createStaticContext();
    lSctx->registerModule(&lModule);

    auto lQuery = [](std::string const& aValue)
    {
      return std::string("declare namespace m = '") + MODULE_URI + "';\n"
        "declare namespace an = 'http://zorba.io/annotations';\n"
        "declare %an:nondeterministic function m:echo($s as xs:string)"
        " as xs:string external;\n"
        "m:echo('" + aValue + "')";
    };

    // The first compilation against the context completes its setup (it gives
    // the context its type manager), so it runs before the threads start.
    lChecker.expect(lQuery("main"), 0, run(lZorba, lSctx, lQuery("main")),
                    "main");

    run_threads([&](unsigned t)
    {
      std::string const n = std::to_string(t);
      for (unsigned r = 0; r < ROUNDS; ++r)
        lChecker.expect(lQuery(n), t, run(lZorba, lSctx, lQuery(n)), n);
    });
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
