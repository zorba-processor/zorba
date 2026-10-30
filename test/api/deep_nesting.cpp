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

// Deeply nested input must end with an error or a result, never with a
// crash from exhausting the machine stack.

#include <iostream>
#include <sstream>
#include <string>

#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

std::string repeat(const std::string& s, size_t n)
{
  std::string r;
  r.reserve(s.size() * n);
  for (size_t i = 0; i < n; ++i)
    r += s;
  return r;
}

void probe(Zorba* z, const char* aName, const std::string& aQuery)
{
  std::cout << "deep nesting: " << aName << " ... " << std::flush;
  try
  {
    XQuery_t q = z->compileQuery(aQuery);
    std::ostringstream lOut;
    q->execute(lOut);
    std::cout << "ok (" << lOut.str().size() << " bytes)" << std::endl;
  }
  catch (ZorbaException const& e)
  {
    std::cout << e.diagnostic().qname().localname() << std::endl;
  }
}

} // anonymous namespace


int
deep_nesting(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  lZorba->setSecurityPolicy(SecurityPolicy::unrestricted());

  size_t const N = 100000;
  probe(lZorba, "parse-xml elements",
        "parse-xml(\"" + repeat("<a>", N) + repeat("</a>", N) + "\")");
  probe(lZorba, "parse-json arrays (count)",
        "count(parse-json(\"" + repeat("[", N) + repeat("]", N) + "\"))");
  probe(lZorba, "parse-json arrays (serialized)",
        "parse-json(\"" + repeat("[", N) + repeat("]", N) + "\")");
  probe(lZorba, "parentheses",
        repeat("(", N) + "1" + repeat(")", N));
  probe(lZorba, "element constructors",
        repeat("<a>", N) + repeat("</a>", N));
  probe(lZorba, "additions", "1" + repeat(" + 1", N));

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return 0;
}
/* vim:set et sw=2 ts=2: */
