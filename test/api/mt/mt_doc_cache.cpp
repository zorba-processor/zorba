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

// Concurrent fn:doc of the same and of different URIs. In each round every
// thread loads all the documents, starting with a different one, while none
// of them is in the store yet: the threads parse the same file at the same
// time and race to add it. All threads must get the document that won (the
// same node, compared with "is"), and each document must be complete.
// Between rounds the documents are removed from the store.

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <zorba/document_manager.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xmldatamanager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;
namespace sfs = std::filesystem;

namespace {

const unsigned DOCS = 4;
const unsigned ELEMENTS = 500;
const unsigned ROUNDS = 20;

std::string document(unsigned aDoc)
{
  std::string lDoc = "<doc id='" + std::to_string(aDoc) + "'>";
  for (unsigned e = 0; e < ELEMENTS; ++e)
    lDoc += "<e n='" + std::to_string(e) + "'>text</e>";
  return lDoc + "</doc>";
}

// The document node fn:doc returns, with the number of its elements and its
// id read on the same thread (aContent); or a null item, and the error in
// aContent.
Item load(Zorba* aZorba, StaticContext_t const& aSctx, std::string const& aUri,
          std::string& aContent)
{
  try
  {
    XQuery_t lQuery = aZorba->compileQuery(
      "let $d := fn:doc('" + aUri + "')\n"
      "return ($d, string-join((string(count($d/doc/e)),\n"
      "  string($d/doc/@id)), ' '))", aSctx);
    Iterator_t lIter = lQuery->iterator();
    lIter->open();
    Item lDoc;
    Item lContent;
    lIter->next(lDoc);
    lIter->next(lContent);
    lIter->close();
    aContent = lContent.getStringValue().str();
    return lDoc;
  }
  catch (ZorbaException const& e)
  {
    aContent = e.what();
    return Item();
  }
}

// Runs a query whose external variables $a and $b are bound to the items.
std::string with_items(Zorba* aZorba, std::string const& aQuery,
                       Item const& aA, Item const& aB)
{
  XQuery_t lQuery = aZorba->compileQuery(
    "declare variable $a external; declare variable $b external;\n" + aQuery);
  lQuery->getDynamicContext()->setVariable("a", aA);
  lQuery->getDynamicContext()->setVariable("b", aB);
  return drain(lQuery);
}

} // namespace


int
mt_doc_cache(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  sfs::path const lDir = sfs::temp_directory_path() / "zorba-mt-doc-cache";
  std::vector<std::string> lUris;

  try
  {
    sfs::create_directories(lDir);
    for (unsigned d = 0; d < DOCS; ++d)
    {
      sfs::path const lFile = lDir / ("doc" + std::to_string(d) + ".xml");
      std::ofstream(lFile) << document(d);
      lUris.push_back("file://" + lFile.generic_string());
    }

    // Each thread compiles in its own static context: the store is what the
    // threads share.
    auto lContext = [&]()
    {
      StaticContext_t lSctx = lZorba->createStaticContext();
      lSctx->setSecurityPolicy(
        SecurityPolicy::secure().allow(Capability::FileRead));
      return lSctx;
    };
    XmlDataManager_t lXmlManager = lZorba->getXmlDataManager();
    DocumentManager* lDocs = lXmlManager->getDocumentManager();

    for (unsigned r = 0; r < ROUNDS; ++r)
    {
      std::vector<std::vector<Item>> lSeen(THREADS, std::vector<Item>(DOCS));
      std::vector<std::vector<std::string>> lContents(
        THREADS, std::vector<std::string>(DOCS));

      run_threads([&](unsigned t)
      {
        StaticContext_t lSctx = lContext();
        for (unsigned i = 0; i < DOCS; ++i)
        {
          unsigned const d = (t + i) % DOCS;
          lSeen[t][d] = load(lZorba, lSctx, lUris[d], lContents[t][d]);
        }
      });

      for (unsigned d = 0; d < DOCS; ++d)
      {
        std::string const lDoc = "document " + std::to_string(d);
        for (unsigned t = 0; t < THREADS; ++t)
        {
          if (lSeen[t][d].isNull())
          {
            lChecker.fail("thread " + std::to_string(t) + ": " + lDoc +
                          ": " + lContents[t][d]);
            continue;
          }
          lChecker.expect(lDoc, t, lContents[t][d],
                          std::to_string(ELEMENTS) + " " + std::to_string(d));
          lChecker.expect(lDoc + " is the one thread 0 got", t,
                          with_items(lZorba, "$a is $b", lSeen[t][d],
                                     lSeen[0][d]),
                          "true");
        }
        if (d > 0 && !lSeen[0][d].isNull() && !lSeen[0][0].isNull())
          lChecker.expect(lDoc + " is not document 0", 0,
                          with_items(lZorba, "$a is $b", lSeen[0][d],
                                     lSeen[0][0]),
                          "false");
      }

      for (std::string const& lUri : lUris)
        lDocs->remove(lUri);
    }
  }
  catch (ZorbaException const& e)
  {
    lChecker.fail(std::string("unexpected exception: ") + e.what());
  }
  catch (sfs::filesystem_error const& e)
  {
    lChecker.fail(std::string("unexpected exception: ") + e.what());
  }

  std::error_code lIgnored;
  sfs::remove_all(lDir, lIgnored);

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
