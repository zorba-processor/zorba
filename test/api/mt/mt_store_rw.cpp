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

// Readers, an updating writer and a sequential writer on one collection and an
// automatic index on it. Every item has a value v and a key g; the writers
// insert and delete items in pairs of opposite values, so between two complete
// changes every group of items sums to 0 and has an even count. The updating
// writer changes a pair in one update; the sequential writer changes one item
// per statement, so its pairs are complete only at the end of its query. A
// reader sees a store that no writer is changing for the whole of its query
// (the store is single-writer / many-readers, and a sequential query is a
// writer for its whole run), so the sums and counts it computes, through the
// collection and through the index, always agree.

#include <sstream>
#include <string>

#include <zorba/store_manager.h>
#include <zorba/uri_resolvers.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned WRITER_ROUNDS = 20;
const unsigned READER_ROUNDS = 40;

const char MODULE_URL[] = "http://zorba-mt.test/store-rw/store.xqlib";

const char MODULE_SRC[] =
  "module namespace t = 'urn:zorba:test:mt-store-rw';\n"
  "import module namespace dml =\n"
  "  'http://zorba.io/modules/store/static/collections/dml';\n"
  "declare namespace ann = 'http://zorba.io/annotations';\n"
  "declare collection t:items as node()*;\n"
  "declare %ann:value-equality %ann:automatic index t:by-group\n"
  "  on nodes dml:collection(xs:QName('t:items'))\n"
  "  by xs:integer(@g) as xs:integer;\n";

const char PROLOG[] =
  "import module namespace t = 'urn:zorba:test:mt-store-rw'\n"
  "  at 'http://zorba-mt.test/store-rw/store.xqlib';\n"
  "import module namespace ddl =\n"
  "  'http://zorba.io/modules/store/static/collections/ddl';\n"
  "import module namespace dml =\n"
  "  'http://zorba.io/modules/store/static/collections/dml';\n"
  "import module namespace iddl =\n"
  "  'http://zorba.io/modules/store/static/indexes/ddl';\n"
  "import module namespace idml =\n"
  "  'http://zorba.io/modules/store/static/indexes/dml';\n";

// Per group (1: updating writer, 2: sequential writer) and for the whole
// collection: count mod 2 and sum; then whether the index and the collection
// hold the same number of items.
const char READ_QUERY[] =
  "let $all := dml:collection(xs:QName('t:items'))\n"
  "let $g1 := idml:probe-index-point-value(xs:QName('t:by-group'), 1)\n"
  "let $g2 := idml:probe-index-point-value(xs:QName('t:by-group'), 2)\n"
  "return string-join((\n"
  "  for $g in (1, 2)\n"
  "  let $items := $all[@g = $g]\n"
  "  return (string(count($items) mod 2), string(sum($items/@v))),\n"
  "  string(count($g1) mod 2), string(sum($g1/@v)),\n"
  "  string(count($g2) mod 2), string(sum($g2/@v)),\n"
  "  string(count($g1) + count($g2) eq count($all))), ' ')";

const char READ_EXPECTED[] = "0 0 0 0 0 0 0 0 true";

void releaseStream(std::istream* aStream)
{
  delete aStream;
}

class ModuleResolver : public URLResolver
{
public:
  Resource* resolveURL(String const& aUrl, EntityData const* aEntityData) override
  {
    if (aEntityData->getKind() != EntityData::MODULE || aUrl != MODULE_URL)
      return nullptr;
    return StreamResource::create(new std::istringstream(MODULE_SRC),
                                  &releaseStream);
  }
};

std::string item(unsigned aGroup, unsigned aKey, int aValue)
{
  return "<item g='" + std::to_string(aGroup) + "' k='" +
    std::to_string(aKey) + "' v='" + std::to_string(aValue) + "'/>";
}

std::string pair_items(unsigned aGroup, unsigned aKey)
{
  return "dml:collection(xs:QName('t:items'))[@g = " + std::to_string(aGroup) +
    "][@k = " + std::to_string(aKey) + "]";
}

// One update: inserts the pair of round r and deletes the pair of round r - 1.
std::string updating_round(unsigned r)
{
  int const v = static_cast<int>(r) + 1;
  std::string lQuery =
    "(dml:insert(xs:QName('t:items'), (" + item(1, r, v) + ", " +
    item(1, r, -v) + "))";
  if (r > 0)
    lQuery += ", dml:delete(" + pair_items(1, r - 1) + ")";
  return lQuery + ")";
}

// One statement per item: the pair of round r is inserted, then the pair of
// round r - 1 is deleted, item by item.
std::string sequential_round(unsigned r)
{
  int const v = static_cast<int>(r) + 1;
  std::string lQuery =
    "dml:insert(xs:QName('t:items'), " + item(2, r, v) + ");\n"
    "dml:insert(xs:QName('t:items'), " + item(2, r, -v) + ");\n";
  if (r > 0)
  {
    std::string const lOld = pair_items(2, r - 1);
    lQuery += "dml:delete(" + lOld + "[@v > 0]);\n"
              "dml:delete(" + lOld + "[@v < 0]);\n";
  }
  return lQuery + "'done'";
}

} // namespace


int
mt_store_rw(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    ModuleResolver lResolver;
    // Each thread compiles in its own static context: the store is what the
    // threads share.
    auto lContext = [&]()
    {
      StaticContext_t lSctx = lZorba->createStaticContext();
      lSctx->registerURLResolver(&lResolver);
      Zorba_CompilerHints_t lHints;
      lSctx->loadProlog(PROLOG, lHints);
      return lSctx;
    };
    StaticContext_t lSetup = lContext();

    lZorba->compileQuery("ddl:create(xs:QName('t:items'))", lSetup)->execute();
    lZorba->compileQuery("iddl:create(xs:QName('t:by-group'))", lSetup)
      ->execute();

    run_threads([&](unsigned t)
    {
      try
      {
        StaticContext_t lOwn = lContext();
        if (t == 0)
        {
          for (unsigned r = 0; r < WRITER_ROUNDS; ++r)
            lZorba->compileQuery(updating_round(r), lOwn)->execute();
        }
        else if (t == 1)
        {
          for (unsigned r = 0; r < WRITER_ROUNDS; ++r)
          {
            XQuery_t lQuery = lZorba->compileQuery(sequential_round(r), lOwn);
            if (!lQuery->isSequential())
              lChecker.fail("the sequential writer's query is not sequential");
            lChecker.expect("sequential writer", t, drain(lQuery), "done");
          }
        }
        else
        {
          for (unsigned r = 0; r < READER_ROUNDS; ++r)
            lChecker.expect("reader", t, run(lZorba, lOwn, READ_QUERY),
                            READ_EXPECTED);
        }
      }
      catch (ZorbaException const& e)
      {
        lChecker.fail("thread " + std::to_string(t) + ": " + e.what());
      }
    });

    // Each writer leaves the pair of its last round.
    lChecker.expect("final state", 0,
                    run(lZorba, lSetup,
                        "let $all := dml:collection(xs:QName('t:items'))\n"
                        "return string-join((string(count($all)),\n"
                        "  string(sum($all/@v))), ' ')"),
                    "4 0");
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
