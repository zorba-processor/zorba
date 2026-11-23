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

// Threads that start on a cold engine and first use, together, the state the
// engine initializes lazily: the error message dictionary, the full-text stop
// word tables, the JSound builtin types and constraint compilation, and the
// XQueryX converter. Only the first round is cold; later rounds check that
// the shared state stays correct.

#include <string>

#include <zorba/config.h>
#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 5;

#ifndef ZORBA_NO_FULL_TEXT
// Two stop words of each language with a default stop word list.
struct StopWords
{
  char const* lang;
  char const* first;
  char const* second;
};

const StopWords STOP_WORDS[THREADS] = {
  { "en", "a",    "about" },
  { "de", "aber", "alle"  },
  { "fr", "ai",   "aie"   },
  { "es", "a",    "al"    },
  { "it", "a",    "ad"    },
  { "nl", "aan",  "al"    },
  { "sv", "alla", "allt"  },
  { "da", "ad",   "af"    },
};
#endif

} // namespace


int
mt_globals(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  run_threads([&](unsigned t)
  {
    std::string const n = std::to_string(t);

    // The message of a dynamic error, and of a static one, from the
    // dictionary.
    std::string const lCast =
      "declare namespace err = 'http://www.w3.org/2005/xqt-errors';"
      " try { xs:integer('x" + n + "') } catch err:FORG0001 {"
      " contains($err:description, 'x" + n + "') }";
    std::string const lSyntax = "1 + ";

#ifndef ZORBA_NO_FULL_TEXT
    // Matches only when the two different stop words are both ignored.
    StopWords const& w = STOP_WORDS[t];
    std::string const lStopWords =
      "'" + std::string(w.first) + " zorba" + n + "' contains text '" +
      w.second + " zorba" + n + "' using stop words default using language '" +
      w.lang + "'";
#endif

    // A builtin atomic base type and a constraint, compiled per validation.
    std::string const lJSound =
      "import module namespace jsd = 'http://jsound.io/modules/jsound';"
      " let $s := { '$namespace' : 'urn:mt', '$types' : [ {"
      " '$kind' : 'atomic', '$name' : 't', '$baseType' : 'integer',"
      " '$constraints' : [ '$$ mod 8 eq " + n + "' ] } ] }"
      " return (jsd:validate($s, 't', " + std::to_string(8 + t) + "),"
      " jsd:validate($s, 't', " + std::to_string(9 + t) + "))";

#ifdef ZORBA_XQUERYX
    std::string const lXQueryX =
      "<xqx:module xmlns:xqx='http://www.w3.org/2005/XQueryX'><xqx:mainModule>"
      "<xqx:queryBody><xqx:integerConstantExpr><xqx:value>" + n +
      "</xqx:value></xqx:integerConstantExpr></xqx:queryBody>"
      "</xqx:mainModule></xqx:module>";
#endif

    for (unsigned r = 0; r < ROUNDS; ++r)
    {
      lChecker.expect(lCast, t, run(lZorba, lZorba->createStaticContext(), lCast),
                      "true");
      std::string const lSyntaxResult =
        run(lZorba, lZorba->createStaticContext(), lSyntax);
      if (lSyntaxResult.find("syntax error") == std::string::npos)
        lChecker.expect(lSyntax, t, lSyntaxResult, "error: ... syntax error ...");
#ifndef ZORBA_NO_FULL_TEXT
      lChecker.expect(lStopWords, t,
                      run(lZorba, lZorba->createStaticContext(), lStopWords),
                      "true");
#endif
      lChecker.expect(lJSound, t,
                      run(lZorba, lZorba->createStaticContext(), lJSound),
                      "true false");
#ifdef ZORBA_XQUERYX
      lChecker.expect(lXQueryX, t,
                      run(lZorba, lZorba->createStaticContext(), lXQueryX), n);
#endif
    }
  });

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);
  return lChecker.result();
}
/* vim:set et sw=2 ts=2: */
