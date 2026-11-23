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

// Lock order of the store lock and the Xerces mutex. A schema import holds the
// Xerces mutex while Xerces resolves an xs:include; here a user URLResolver
// serves the include and runs a query first, as the HTTP stream does to fetch
// a URL. Meanwhile other threads validate (Xerces mutex) inside a query that
// holds the store lock. Compilation must hold the store lock before the Xerces
// mutex, so that the resolver's query takes it recursively; otherwise:
//
//   writer:        the compiling thread holds Xerces and waits for the store
//                  lock; an updating query holds the store lock (WRITE) and
//                  waits for Xerces.
//   reader-writer: a validating reader holds READ and waits for Xerces; a
//                  writer waits for the reader; the resolver's query waits
//                  behind the writer (writer preference).
//
// The resolver waits a moment before its query so that the other threads are
// blocked on their locks by then. A scenario that does not finish within the
// time limit is a deadlock: the test reports it and exits with failure.

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <future>
#include <sstream>
#include <string>

#include <zorba/store_manager.h>
#include <zorba/uri_resolvers.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const auto TIME_LIMIT = std::chrono::seconds(60);

// How long the resolver waits before its query: the other threads have
// reached their locks by then.
const auto SETTLE = std::chrono::milliseconds(500);

const char MAIN_URL[] = "http://zorba-mt.test/lock-order/main.xsd";
const char PART_URL[] = "http://zorba-mt.test/lock-order/part.xsd";

const char MAIN_SRC[] =
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
  "  targetNamespace='urn:zorba:test:mt-lock-order'\n"
  "  xmlns='urn:zorba:test:mt-lock-order' elementFormDefault='qualified'>\n"
  "  <xs:include schemaLocation='part.xsd'/>\n"
  "  <xs:element name='order'>\n"
  "    <xs:complexType>\n"
  "      <xs:sequence>\n"
  "        <xs:element name='code' type='Code'/>\n"
  "      </xs:sequence>\n"
  "    </xs:complexType>\n"
  "  </xs:element>\n"
  "</xs:schema>\n";

const char PART_SRC[] =
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
  "  targetNamespace='urn:zorba:test:mt-lock-order'\n"
  "  xmlns='urn:zorba:test:mt-lock-order' elementFormDefault='qualified'>\n"
  "  <xs:simpleType name='Code'>\n"
  "    <xs:restriction base='xs:string'>\n"
  "      <xs:pattern value='[A-Z]{3}-[0-9]+'/>\n"
  "    </xs:restriction>\n"
  "  </xs:simpleType>\n"
  "</xs:schema>\n";

const char PROLOG[] =
  "import schema namespace s = 'urn:zorba:test:mt-lock-order'\n"
  "  at 'http://zorba-mt.test/lock-order/main.xsd';\n"
  "import module namespace ddl =\n"
  "  'http://zorba.io/modules/store/dynamic/collections/ddl';\n"
  "import module namespace dml =\n"
  "  'http://zorba.io/modules/store/dynamic/collections/dml';\n";

const char VALIDATED[] =
  "validate { <s:order><s:code>ABC-12</s:code></s:order> }";

void releaseStream(std::istream* aStream)
{
  delete aStream;
}

// A one-shot signal from the resolver to the threads it waits for.
class Signal
{
  std::mutex theMutex;
  std::condition_variable theCondition;
  bool theRaised = false;

public:
  void raise()
  {
    std::lock_guard<std::mutex> lGuard(theMutex);
    theRaised = true;
    theCondition.notify_all();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lGuard(theMutex);
    theCondition.wait(lGuard, [this]() { return theRaised; });
  }
};

// Serves the two schema documents. While armed, it raises the signal when
// Xerces asks for the include, waits SETTLE, and runs a query before serving
// it.
class IncludeResolver : public URLResolver
{
  Zorba* theZorba;
  Signal* theSignal = nullptr;
  std::string theQueryResult;

public:
  explicit IncludeResolver(Zorba* aZorba) : theZorba(aZorba) {}

  void arm(Signal* aSignal)
  {
    theSignal = aSignal;
    theQueryResult.clear();
  }

  std::string const& queryResult() const { return theQueryResult; }

  Resource* resolveURL(String const& aUrl, EntityData const* aEntityData) override
  {
    if (aEntityData->getKind() != EntityData::SCHEMA)
      return nullptr;

    if (aUrl == MAIN_URL)
      return StreamResource::create(new std::istringstream(MAIN_SRC),
                                    &releaseStream);
    if (aUrl != PART_URL)
      return nullptr;

    if (theSignal)
    {
      Signal* lSignal = theSignal;
      theSignal = nullptr;
      lSignal->raise();
      std::this_thread::sleep_for(SETTLE);
      theQueryResult = run(theZorba, theZorba->createStaticContext(),
                           "string-join(('inner', 'query'), ' ')");
    }
    return StreamResource::create(new std::istringstream(PART_SRC),
                                  &releaseStream);
  }
};

// Runs the bodies on their own threads; exits the process if they do not all
// finish within TIME_LIMIT (deadlocked threads cannot be joined).
template <class... Bodies>
void run_scenario(std::string const& aName, Bodies... aBodies)
{
  std::vector<std::future<void>> lDone;
  std::vector<std::thread> lThreads;
  auto lStart = [&](auto aBody)
  {
    std::packaged_task<void()> lTask(aBody);
    lDone.push_back(lTask.get_future());
    lThreads.emplace_back(std::move(lTask));
  };
  (lStart(aBodies), ...);

  auto const lDeadline = std::chrono::steady_clock::now() + TIME_LIMIT;
  for (std::future<void>& lFuture : lDone)
  {
    if (lFuture.wait_until(lDeadline) != std::future_status::ready)
    {
      std::cerr << "scenario " << aName << ": deadlock, the threads did not "
                << "finish within " << TIME_LIMIT.count() << " s" << std::endl;
      std::_Exit(1);
    }
  }
  for (std::thread& lThread : lThreads)
    lThread.join();
  for (std::future<void>& lFuture : lDone)
    lFuture.get();
}

} // namespace


int
mt_lock_order(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    IncludeResolver lResolver(lZorba);

    // Compiled before the scenarios, with the resolver not armed.
    StaticContext_t lShared = lZorba->createStaticContext();
    lShared->registerURLResolver(&lResolver);
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(PROLOG, lHints);
    lZorba->compileQuery("ddl:create(xs:QName('local:orders'))", lShared)
      ->execute();

    std::string const lImport =
      std::string("import schema namespace s = 'urn:zorba:test:mt-lock-order'"
                  " at '") + MAIN_URL + "';\n";

    // The thread that imports the schema and so runs the resolver's query. The
    // signal is raised again in case the resolver did not raise it.
    auto lCompile = [&](Signal& aSignal)
    {
      StaticContext_t lSctx = lZorba->createStaticContext();
      lSctx->registerURLResolver(&lResolver);
      lChecker.expect("compile with import", 0,
                      run(lZorba, lSctx, lImport + "string(s:Code('XYZ-7'))"),
                      "XYZ-7");
      aSignal.raise();
    };

    // The other threads' queries are compiled before the scenarios: their
    // compilation would wait for the Xerces mutex.
    {
      Signal lSignal;
      lResolver.arm(&lSignal);
      XQuery_t lInsertValidated = lZorba->compileQuery(
        std::string("dml:insert-last(xs:QName('local:orders'), ") +
        VALIDATED + ")", lShared);
      run_scenario("writer",
                   [&]() { lCompile(lSignal); },
                   [&]()
                   {
                     lSignal.wait();
                     lInsertValidated->execute();
                   });
      lChecker.expect("writer: resolver's query", 0, lResolver.queryResult(),
                      "inner query");
    }

    {
      Signal lSignal;
      lResolver.arm(&lSignal);
      XQuery_t lValidatingRead = lZorba->compileQuery(
        std::string("string(data((") + VALIDATED + ")/s:code))", lShared);
      XQuery_t lInsert = lZorba->compileQuery(
        "dml:insert-last(xs:QName('local:orders'), <plain/>)", lShared);
      run_scenario("reader-writer",
                   [&]() { lCompile(lSignal); },
                   [&]()
                   {
                     lSignal.wait();
                     lChecker.expect("reader-writer: validating reader", 1,
                                     drain(lValidatingRead), "ABC-12");
                   },
                   [&]()
                   {
                     lSignal.wait();
                     // The reader takes READ first.
                     std::this_thread::sleep_for(SETTLE / 5);
                     lInsert->execute();
                   });
      lChecker.expect("reader-writer: resolver's query", 0,
                      lResolver.queryResult(), "inner query");
    }

    lChecker.expect("collection", 0,
                    run(lZorba, lShared,
                        "string(count(dml:collection(xs:QName('local:orders'))))"),
                    "2");
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
