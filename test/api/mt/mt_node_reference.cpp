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

// Concurrent first fn:generate-id and ref:reference of the same nodes. In
// each round a fresh document is parsed, so none of its nodes has an id yet,
// and every thread asks for the ids of all its nodes at the same time (half
// of the threads in document order, half in reverse order). All threads must
// get the same id for a node, and ref:dereference must return the node for
// its reference.

#include <sstream>
#include <string>
#include <vector>

#include <zorba/store_manager.h>
#include <zorba/xmldatamanager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ELEMENTS = 200;
const unsigned ROUNDS = 20;

std::string document(unsigned aRound)
{
  std::string lDoc = "<doc round='" + std::to_string(aRound) + "'>";
  for (unsigned e = 0; e < ELEMENTS; ++e)
    lDoc += "<e n='" + std::to_string(e) + "'>text</e>";
  return lDoc + "</doc>";
}

// For each node of $d: its generate-id, its reference, and whether
// dereferencing the reference gives the node back, one line per node, in
// document order whichever order the ids were asked in.
const char* const QUERY =
  "import module namespace ref = 'http://zorba.io/modules/reference';\n"
  "declare variable $d external;\n"
  "declare variable $reverse external;\n"
  "let $nodes := ($d, $d//node(), $d//@*)\n"
  "let $lines := for $n in (if ($reverse) then reverse($nodes) else $nodes)\n"
  "              let $r := ref:reference($n)\n"
  "              return string-join((generate-id($n), string($r),\n"
  "                string(ref:dereference($r) is $n)), ' ')\n"
  "return if ($reverse) then reverse($lines) else $lines\n";

std::string ids(Zorba* aZorba, Item const& aDoc, bool aReverse)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(QUERY);
    DynamicContext* lDctx = lQuery->getDynamicContext();
    lDctx->setVariable("d", aDoc);
    lDctx->setVariable("reverse",
                       aZorba->getItemFactory()->createBoolean(aReverse));
    return drain(lQuery);
  }
  catch (ZorbaException const& e)
  {
    return std::string("error: ") + e.what();
  }
}

} // namespace


int
mt_node_reference(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    XmlDataManager_t lXmlManager = lZorba->getXmlDataManager();

    for (unsigned r = 0; r < ROUNDS; ++r)
    {
      std::istringstream lInput(document(r));
      Item lDoc = lXmlManager->parseXML(lInput);
      std::vector<std::string> lIds(THREADS);

      run_threads([&](unsigned t)
      {
        lIds[t] = ids(lZorba, lDoc, t % 2 == 1);
      });

      std::string const lWhat = "round " + std::to_string(r);
      // Every node is dereferenced to itself; this also rules out an error.
      if (lIds[0].find(" false") != std::string::npos ||
          lIds[0].find(" true") == std::string::npos)
        lChecker.fail("thread 0: " + lWhat + ": " + lIds[0]);
      for (unsigned t = 1; t < THREADS; ++t)
        lChecker.expect(lWhat + ": the ids thread 0 got", t, lIds[t],
                        lIds[0]);
    }
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
