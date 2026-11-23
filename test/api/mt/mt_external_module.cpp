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

// External functions in a shared static context, used by several threads: a
// module the application registers (its functions are resolved by every
// compilation that declares them, and called concurrently), and a module
// shipped with Zorba (the EXPath file module) imported by a prolog loaded with
// loadProlog, whose functions every query calls, and imported by the queries
// themselves.

#include <map>
#include <memory>
#include <string>

#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_factory.h>
#include <zorba/security.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char NS[] = "urn:mt-external";

class TwiceFunction : public NonContextualExternalFunction
{
public:
  String getURI() const { return NS; }

  String getLocalName() const { return "twice"; }

  ItemSequence_t evaluate(const ExternalFunction::Arguments_t& aArgs) const
  {
    Iterator_t lIter = aArgs[0]->getIterator();
    Item lItem;
    lIter->open();
    lIter->next(lItem);
    lIter->close();
    return ItemSequence_t(new SingletonItemSequence(
        Zorba::getInstance(0)->getItemFactory()->createInteger(
            2 * lItem.getLongValue())));
  }
};

// Its functions are created up front, so that compilations on several
// threads can look them up (the contract for a module registered in a shared
// static context).
class TwiceModule : public ExternalModule
{
  std::map<std::string, std::unique_ptr<ExternalFunction>> theFunctions;

public:
  TwiceModule() { theFunctions["twice"] = std::make_unique<TwiceFunction>(); }

  String getURI() const { return NS; }

  ExternalFunction* getExternalFunction(const String& aLocalName)
  {
    auto const lIter = theFunctions.find(aLocalName.str());
    return lIter == theFunctions.end() ? nullptr : lIter->second.get();
  }
};

const char PROLOG[] =
  "import module namespace file = 'http://expath.org/ns/file';\n"
  "declare namespace p = 'urn:p';\n"
  "declare namespace e = 'urn:mt-external';\n"
  "declare function e:twice($x as xs:integer) as xs:integer external;\n"
  "declare function p:sep() { file:directory-separator() };\n"
  "declare function p:quad($x) { e:twice(e:twice($x)) };\n";

} // namespace


int
mt_external_module(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;
  TwiceModule lModule;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    lShared->registerModule(&lModule);
    lShared->setSecurityPolicy(
        SecurityPolicy::unrestricted().allow(Capability::FileRead));
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(PROLOG, lHints);

    std::string const lSeparator = run(lZorba, lShared, "p:sep()");

    run_threads([&](unsigned t)
    {
      unsigned const n = 3 + t;
      std::string const lN = std::to_string(n);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("registered module, through the prolog", t,
            run(lZorba, lShared, "p:quad(" + lN + ")"),
            std::to_string(4 * n));

        lChecker.expect("registered module, declared by the query", t,
            run(lZorba, lShared->createChildContext(),
                "declare namespace m = 'urn:mt-external';\n"
                "declare function m:twice($x as xs:integer) as xs:integer"
                " external;\n"
                "m:twice(" + lN + ")"),
            std::to_string(2 * n));

        lChecker.expect("shipped module, through the prolog", t,
            run(lZorba, lShared, "p:sep()"),
            lSeparator);

        lChecker.expect("shipped module, imported by the query", t,
            run(lZorba, lShared,
                "import module namespace f = 'http://expath.org/ns/file';\n"
                "f:directory-separator() eq p:sep()"),
            "true");
      }
    });
  }
  catch (ZorbaException const& e)
  {
    lChecker.fail(std::string("unexpected exception: ") + e.what());
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
