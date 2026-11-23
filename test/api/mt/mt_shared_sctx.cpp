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

// A static context with a prolog loaded by loadProlog, shared by threads that
// compile and run queries against it and against child contexts of it. The
// functions and the variable of the prolog belong to the compilation of the
// prolog; the queries only call the functions (statically, through function
// items, and recursively), read the variable, and navigate the nodes they
// construct.

#include <string>

#include <zorba/store_manager.h>

#include "mt_test.h"

using namespace zorba;
using namespace zorba::mt_test;

namespace {

const unsigned ROUNDS = 20;

const char PROLOG[] =
  "declare namespace p = 'urn:p';\n"
  "declare function p:sq($x) { $x * $x };\n"
  "declare function p:tree($n) {\n"
  "  <t>{for $i in 1 to $n return <c v='{p:sq($i)}'/>}</t> };\n"
  "declare function p:fib($k) {\n"
  "  if ($k lt 2) then $k else p:fib($k - 1) + p:fib($k - 2) };\n"
  "declare function p:apply($f, $x) { $f($x) };\n"
  "declare function p:twice($f, $x) { $f($f($x)) };\n"
  "declare function p:wrap($n) { <w>{p:tree($n)/c}</w> };\n"
  "declare variable $p:doc := <d><e/></d>;\n"
  "declare function p:copy() { <w>{$p:doc/e}</w> };\n";

unsigned fib(unsigned k)
{
  return k < 2 ? k : fib(k - 1) + fib(k - 2);
}

unsigned sumOfSquares(unsigned n)
{
  return n * (n + 1) * (2 * n + 1) / 6;
}

} // namespace


int
mt_shared_sctx(int, char*[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  Checker lChecker;

  try
  {
    StaticContext_t lShared = lZorba->createStaticContext();
    Zorba_CompilerHints_t lHints;
    lShared->loadProlog(PROLOG, lHints);

    run_threads([&](unsigned t)
    {
      unsigned const n = 3 + t;
      std::string const lN = std::to_string(n);

      for (unsigned r = 0; r < ROUNDS; ++r)
      {
        lChecker.expect("tree", t,
            run(lZorba, lShared, "sum(p:tree(" + lN + ")/c/@v)"),
            std::to_string(sumOfSquares(n)));

        lChecker.expect("parent", t,
            run(lZorba, lShared, "string-join(p:wrap(" + lN + ")/c/../name(), ' ')"),
            "w");

        lChecker.expect("prolog variable", t,
            run(lZorba, lShared,
                "string-join(($p:doc/e/../name(), p:copy()/e/../name(), "
                "string(" + lN + ")), ' ')"),
            "d w " + lN);

        lChecker.expect("recursive", t,
            run(lZorba, lShared, "p:fib(" + lN + ")"),
            std::to_string(fib(n)));

        lChecker.expect("function item", t,
            run(lZorba, lShared, "p:apply(p:sq#1, " + lN + ")"),
            std::to_string(n * n));

        lChecker.expect("inline function", t,
            run(lZorba, lShared, "p:twice(function($x) { $x + 1 }, " + lN + ")"),
            std::to_string(n + 2));

        lChecker.expect("child context", t,
            run(lZorba, lShared->createChildContext(), "p:sq(" + lN + ") + 1"),
            std::to_string(n * n + 1));
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
