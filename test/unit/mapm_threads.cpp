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

// Concurrent xs:decimal (and, in ZORBA_WITH_BIG_INTEGER builds, xs:integer)
// arithmetic from several threads, each with its own XQuery objects. Both
// types are backed by MAPM; every thread must get exactly the results that a
// single thread gets. Meant to be run under ThreadSanitizer as well.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <zorba/config.h>
#include <zorba/item_factory.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

namespace {

char const *const theQueries[] = {
  // xs:decimal: arithmetic, division, rounding, casts from and to strings,
  // comparisons, aggregates.
  "declare variable $d as xs:decimal external;\n"
  "let $s := sum(for $i in 1 to 2000\n"
  "              return (xs:decimal($i) * 1.0825 + $d) div 7)\n"
  "return (round-half-to-even($s, 12),\n"
  "        string(xs:decimal('12345.678901') * $d),\n"
  "        (for $i in 1 to 300\n"
  "         return xs:decimal(concat($i, '.', $i)) mod 3.7)[last()],\n"
  "        avg(for $i in 1 to 500 return $i div 3),\n"
  "        max(for $i in 1 to 500 return $d * $i - 0.5 * $i),\n"
  "        floor($d * 1000.5), ceiling(-$d * 3.25), abs(-$d) lt 1.0,\n"
  "        round(xs:decimal('2.5') * $d * 1000))",

  // xs:integer (machine integers by default, MAPM with big integers).
  "declare variable $d as xs:decimal external;\n"
  "(sum(for $i in 1 to 2000 return $i * $i - 7 idiv 2),\n"
  " (for $i in 1 to 2000 return $i * 31 mod 1000003)[last()],\n"
  " xs:integer($d * 1000000) * 3 idiv 7)",

#ifdef ZORBA_WITH_BIG_INTEGER
  // Arbitrary-precision xs:integer.
  "declare function local:fact($n as xs:integer) as xs:integer\n"
  "{ if ($n le 1) then 1 else $n * local:fact($n - 1) };\n"
  "declare variable $d as xs:decimal external;\n"
  "(local:fact(80),\n"
  " local:fact(60) idiv local:fact(57),\n"
  " xs:integer('123456789012345678901234567890') * 3 + 1,\n"
  " (local:fact(40) - 1) mod 1000000007,\n"
  " sum(for $i in 1 to 300 return $i * 99999999999999999999))",
#endif
};

size_t const theNumQueries = sizeof(theQueries) / sizeof(theQueries[0]);

std::string run(zorba::Zorba *z, char const *query, zorba::Item const &d)
{
  zorba::XQuery_t q = z->compileQuery(query);
  q->getDynamicContext()->setVariable("d", d);

  Zorba_SerializerOptions opts;
  opts.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;

  std::ostringstream os;
  q->execute(os, &opts);
  return os.str();
}

double seconds_since(std::chrono::steady_clock::time_point start)
{
  return std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
      .count();
}

} // namespace

int mapm_threads(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  unsigned const hw = std::thread::hardware_concurrency();
  unsigned const num_threads = std::clamp(hw, 4u, 8u);
  unsigned const iterations = 10;

  void *store = zorba::StoreManager::getStore();
  zorba::Zorba *z = zorba::Zorba::getInstance(store);

  // One decimal item shared by all threads: its MAPM value is copied
  // (reference-counted) concurrently.
  zorba::Item const d = z->getItemFactory()->createDecimal("0.123456789");

  std::vector<std::string> expected(theNumQueries);
  auto start = std::chrono::steady_clock::now();
  try
  {
    for (unsigned i = 0; i < iterations; ++i)
      for (size_t k = 0; k < theNumQueries; ++k)
        expected[k] = run(z, theQueries[k], d);
  }
  catch (zorba::ZorbaException const &e)
  {
    std::cerr << "single-thread reference failed: " << e << std::endl;
    return 1;
  }
  double const single = seconds_since(start);

  for (size_t k = 0; k < theNumQueries; ++k)
    std::cout << "query " << k << ": " << expected[k] << std::endl;

  std::atomic<unsigned> mismatches(0);
  std::atomic<unsigned> errors(0);

  start = std::chrono::steady_clock::now();
  std::vector<std::thread> threads;
  for (unsigned t = 0; t < num_threads; ++t)
  {
    threads.emplace_back([&, t]() {
      for (unsigned i = 0; i < iterations; ++i)
      {
        // Different threads run the queries in different orders.
        for (size_t j = 0; j < theNumQueries; ++j)
        {
          size_t const k = (j + t) % theNumQueries;
          try
          {
            std::string const r = run(z, theQueries[k], d);
            if (r != expected[k])
            {
              if (mismatches++ < 5)
                std::cerr << "thread " << t << " query " << k << ": got\n  "
                          << r << "\nexpected\n  " << expected[k] << std::endl;
            }
          }
          catch (zorba::ZorbaException const &e)
          {
            if (errors++ < 5)
              std::cerr << "thread " << t << " query " << k << ": " << e
                        << std::endl;
          }
        }
      }
    });
  }
  for (std::thread &th : threads)
    th.join();
  double const multi = seconds_since(start);

  std::cout << num_threads << " threads x " << iterations << " iterations x "
            << theNumQueries << " queries: " << mismatches << " mismatches, "
            << errors << " errors" << std::endl;
  std::cout << "time: 1 thread " << single << " s, " << num_threads
            << " threads " << multi << " s (" << num_threads << "x the work)"
            << std::endl;

  return (mismatches == 0 && errors == 0) ? 0 : 1;
}
