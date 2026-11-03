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

// Memory and cost of window clauses. The items are strings of 16 KiB, so that a
// clause that retains its input stands out: 20000 of them are 320 MiB, and the
// queries with few open windows must stay far below that. The last query does
// need its input, and shows that the measure sees it.

#include <chrono>
#include <exception>
#include <iostream>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
# include <sys/resource.h>
#endif

#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

// AddressSanitizer keeps freed memory resident in its quarantine: there, the
// queries run and their results are checked, without the bound on the peak.
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
# define ZORBA_TEST_SANITIZER 1
#elif defined(__has_feature)
# if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#  define ZORBA_TEST_SANITIZER 1
# endif
#endif

namespace {

int theFailures = 0;

void fail(const std::string& aWhat)
{
  std::cerr << "FAILED: " << aWhat << std::endl;
  ++theFailures;
}

long long peak_rss()
{
#if defined(__unix__) || defined(__APPLE__)
  struct rusage u;
  getrusage(RUSAGE_SELF, &u);
# ifdef __APPLE__
  return static_cast<long long>(u.ru_maxrss);
# else
  return static_cast<long long>(u.ru_maxrss) * 1024;
# endif
#else
  return 0;
#endif
}

std::string run(Zorba* z, const SecurityPolicy& p, const std::string& aQuery)
{
  try
  {
    StaticContext_t sctx = z->createStaticContext();
    sctx->setSecurityPolicy(p);
    XQuery_t q = z->compileQuery(aQuery, sctx);
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    std::string value;
    while (it->next(item))
    {
      if (!value.empty())
        value += ' ';
      value += item.getStringValue().str();
    }
    it->close();
    return value;
  }
  catch (ZorbaException const& e)
  {
    return std::string("error ") + e.diagnostic().qname().localname() + ": " + e.what();
  }
}

// %D% is the domain: 20000 distinct strings of 16 KiB, made as they are read.
const char PROLOG[] =
  "declare variable $pad := string-join(for $i in 1 to 16384 return 'x', '');\n"
  "declare function local:d() { for $i in 1 to 20000 return concat($pad, $i) };\n";

struct Case
{
  const char* theName;
  const char* theBody;
  const char* theResult;
};

// Few windows are open at a time, or none needs the items.
const Case BOUNDED[] =
{
  { "sliding over 10 items",
    "sum(for sliding window $w in local:d()\n"
    "      start at $s when true\n"
    "      only end at $e when $e - $s eq 9\n"
    "    return count($w))",
    "199910" },
  { "sliding over 10 items, end on the current item",
    "sum(for sliding window $w in local:d()\n"
    "      start at $s when $s mod 10 eq 1\n"
    "      only end at $e when $e mod 10 eq 0\n"
    "    return count($w))",
    "20000" },
  { "sliding with all the neighbors",
    "sum(for sliding window $w in local:d()\n"
    "      start $s at $p previous $sp next $sn when true\n"
    "      only end $e at $q previous $ep next $en when $q - $p eq 2\n"
    "    return count($w) + count($sp) + count($sn) + count($ep) + count($en))",
    "139984" },
  { "sliding, read after the input was read twice",
    "sum(for $k in 1 to 2\n"
    "    for sliding window $w in local:d()\n"
    "      start at $s when true\n"
    "      only end at $e when $e - $s eq 1\n"
    "    return count($w))",
    "79996" },
  { "sliding, no item starts a window",
    "count(for sliding window $w in local:d()\n"
    "        start $x at $p when $p lt 0 and string-length($x) gt 0\n"
    "        only end when true\n"
    "      return count($w))",
    "0" },
  { "tumbling, items before the only window",
    "sum(for tumbling window $w in local:d()\n"
    "      start at $p when $p eq 20000\n"
    "      end when true\n"
    "    return count($w))",
    "1" },
  { "tumbling without end, no item starts a window",
    "count(for tumbling window $w in local:d()\n"
    "        start $x at $p when $p lt 0 and string-length($x) gt 0\n"
    "      return count($w))",
    "0" },
  { "sliding that never closes, nothing reads the items",
    "count(for sliding window $w in local:d()\n"
    "        start when true\n"
    "        only end $e when string-length($e) lt 0\n"
    "      return 1)",
    "0" },
};

const char RETAINED_NAME[] = "sliding that never closes, the window variable is read";
const char RETAINED[] =
  "count(for sliding window $w in local:d()\n"
  "        start when true\n"
  "        only end $e when string-length($e) lt 0\n"
  "      return count($w))";

void check_memory(Zorba* z)
{
  SecurityPolicy const p =
    SecurityPolicy::unrestricted().setTimeout(std::chrono::seconds(240));
  std::string const prolog(PROLOG);

  // Warm up the process: the first query loads what every query needs.
  if (run(z, p, "count(for sliding window $w in 1 to 3 start when true only end when true return 1)") != "3")
    fail("warm-up query");

  long long const base = peak_rss();
  for (const Case& c : BOUNDED)
  {
    std::string const value = run(z, p, prolog + c.theBody);
    long long const grown = peak_rss() - base;
    std::cout << c.theName << ": " << value << ", peak RSS grew by "
              << (grown >> 20) << " MiB" << std::endl;
    if (value != c.theResult)
      fail(std::string(c.theName) + ": expected " + c.theResult + ", got " + value);
#ifndef ZORBA_TEST_SANITIZER
    if (grown >= (64LL << 20))
      fail(std::string(c.theName) + ": peak RSS grew by " + std::to_string(grown >> 20) + " MiB");
#endif
  }

  // Every window stays open and may be read: the input must stay in memory.
  std::string const value = run(z, p, prolog + RETAINED);
  long long const grown = peak_rss() - base;
  std::cout << RETAINED_NAME << ": " << value << ", peak RSS grew by "
            << (grown >> 20) << " MiB" << std::endl;
  if (value != "0")
    fail(std::string(RETAINED_NAME) + ": got " + value);
#if !defined(ZORBA_TEST_SANITIZER) && (defined(__unix__) || defined(__APPLE__))
  if (grown < (256LL << 20))
    fail(std::string(RETAINED_NAME) + ": the measure does not see the input, grew by "
         + std::to_string(grown >> 20) + " MiB");
#endif
}

// 100000 records and every window open: 5 * 10^9 evaluations of the end condition
// if each window evaluates it. The limit is hundreds of times what one evaluation
// per record takes, so that it does not depend on the speed of the machine.
void check_cost(Zorba* z)
{
  SecurityPolicy const p =
    SecurityPolicy::unrestricted().setTimeout(std::chrono::seconds(240));

  const Case cases[] =
  {
    { "never closes, only end",
      "count(for sliding window $w in 1 to 100000\n"
      "        start when true\n"
      "        only end $e when $e lt 0\n"
      "      return 1)",
      "0" },
    { "never closes, returned at the end",
      "count(for sliding window $w in 1 to 100000\n"
      "        start when true\n"
      "        end $e when $e lt 0\n"
      "      return 1)",
      "100000" },
  };

  for (const Case& c : cases)
  {
    std::string const value = run(z, p, c.theBody);
    std::cout << c.theName << ": " << value << std::endl;
    if (value != c.theResult)
      fail(std::string(c.theName) + ": expected " + c.theResult + ", got " + value);
  }
}

} // anonymous namespace


int
window_memory(int argc, char* argv[])
{
  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);

  try
  {
    check_cost(lZorba);
    check_memory(lZorba);
  }
  catch (std::exception const& e)
  {
    fail(std::string("unexpected exception: ") + e.what());
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
