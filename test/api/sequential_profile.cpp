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

// A query that calls sequential functions gives the same result with and
// without profiling.

#include <iostream>
#include <string>

#include <zorba/properties.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

int theFailures = 0;

const char* const theProlog =
  "import module namespace ddl =\n"
  "  \"http://zorba.io/modules/store/dynamic/collections/ddl\";\n"
  "import module namespace dml =\n"
  "  \"http://zorba.io/modules/store/dynamic/collections/dml\";\n"
  "declare namespace an = \"http://zorba.io/annotations\";\n"
  "declare namespace err = \"http://www.w3.org/2005/xqt-errors\";\n";

// ddl:delete after a sequential function bound a node of the collection to a
// local variable and returned.
const char* const theDeleteQuery =
  "declare %an:sequential function local:bind() as xs:string\n"
  "{\n"
  "  dml:insert-last(xs:QName(\"local:c\"), <e v=\"1\"/>);\n"
  "  variable $nodes := dml:collection(xs:QName(\"local:c\"));\n"
  "  fn:string($nodes[1]/@v)\n"
  "};\n"
  "ddl:create(xs:QName(\"local:c\"));\n"
  "variable $bound := local:bind();\n"
  "variable $outcome :=\n"
  "  try {{ ddl:delete(xs:QName(\"local:c\")); \"deleted\" }}\n"
  "  catch * {{\n"
  "    dml:delete(dml:collection(xs:QName(\"local:c\")));\n"
  "    ddl:delete(xs:QName(\"local:c\"));\n"
  "    fn:local-name-from-QName($err:code)\n"
  "  }};\n"
  "fn:concat($bound, \" \", $outcome)";

// A sequential function that returns at most one item runs to its end when
// the caller needs only its first item.
const char* const theReadAheadQuery =
  "declare %an:sequential function local:item-then-insert() as xs:integer?\n"
  "{\n"
  "  for $i in (1, 2)\n"
  "  return\n"
  "    if ($i eq 1)\n"
  "    then $i\n"
  "    else { dml:insert-last(xs:QName(\"local:c\"), <e/>); () }\n"
  "};\n"
  "ddl:create(xs:QName(\"local:c\"));\n"
  "variable $exists := fn:exists(local:item-then-insert());\n"
  "variable $count := fn:count(dml:collection(xs:QName(\"local:c\")));\n"
  "dml:delete(dml:collection(xs:QName(\"local:c\")));\n"
  "ddl:delete(xs:QName(\"local:c\"));\n"
  "fn:concat($exists, \" \", $count)";

std::string run(Zorba* aZorba, const char* aQuery)
{
  XQuery_t lQuery = aZorba->compileQuery(std::string(theProlog) + aQuery);
  std::string lResult;
  Iterator_t lIter = lQuery->iterator();
  lIter->open();
  Item lItem;
  while (lIter->next(lItem))
    lResult += lItem.getStringValue().str();
  lIter->close();
  return lResult;
}

void check(Zorba* aZorba,
           const char* aName,
           const char* aQuery,
           const std::string& aExpected)
{
  for (bool lProfile : {false, true})
  {
    Properties::instance().setCollectProfile(lProfile);
    std::string lResult = run(aZorba, aQuery);
    if (lResult != aExpected)
    {
      std::cerr << "FAILED: " << aName
                << (lProfile ? " with" : " without") << " profiling: got \""
                << lResult << "\", expected \"" << aExpected << "\""
                << std::endl;
      ++theFailures;
    }
  }
  Properties::instance().setCollectProfile(false);
}

} // namespace

int sequential_profile(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    check(lZorba, "delete after a local reference", theDeleteQuery,
          "1 deleted");
    check(lZorba, "read-ahead", theReadAheadQuery, "true 1");
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "FAILED: unexpected exception: " << e << std::endl;
    ++theFailures;
  }

  Properties::instance().setCollectProfile(false);
  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return theFailures == 0 ? 0 : 1;
}
