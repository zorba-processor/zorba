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

// The parse-json option zorba-streaming-members: the file stays in the file, and
// what happens when the file changes between the two parses.

#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
# include <sys/resource.h>
# include <unistd.h>
#endif

#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

// AddressSanitizer keeps freed memory resident in its quarantine: there, the
// results are checked, without the bound on the peak.
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

XQuery_t compile(Zorba* z, const std::string& aBody)
{
  StaticContext_t sctx = z->createStaticContext();
  sctx->setSecurityPolicy(SecurityPolicy::unrestricted());
  return z->compileQuery(
    "jsoniq version \"1.0\";\n"
    "import module namespace file = \"http://expath.org/ns/file\";\n" + aBody,
    sctx);
}

std::string text_of(const std::string& aPath, bool aStreaming)
{
  return "parse-json(file:read-text(\"" + aPath + "\")" +
         (aStreaming ? ", { \"zorba-streaming-members\" : true }" : "") + ")[]";
}

std::string run_count(Zorba* z, const std::string& aBody)
{
  try
  {
    XQuery_t q = compile(z, aBody);
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    std::string value;
    if (it->next(item))
      value = item.getStringValue().str();
    it->close();
    return value;
  }
  catch (ZorbaException const& e)
  {
    return std::string("error ") + e.diagnostic().qname().localname();
  }
}

// One array of aCount strings of aSize bytes with their comma, and a last one.
void write_array(const std::string& aPath, int aCount, int aSize)
{
  std::ofstream out(aPath, std::ios::binary);
  std::string const member = "\"" + std::string(aSize - 3, 'x') + "\"";
  out << '[';
  for (int i = 0; i < aCount; ++i)
    out << member << ',';
  out << "\"end\"]";
}

void overwrite(const std::string& aPath, long aOffset, const std::string& aBytes)
{
  std::fstream f(aPath, std::ios::binary | std::ios::in | std::ios::out);
  f.seekp(aOffset);
  f.write(aBytes.data(), static_cast<std::streamsize>(aBytes.size()));
}

// With the option, 320 MiB of members take far less than that, also counting the
// file; without it, the measure sees them. Without the first bound, the test
// would not tell a file read as a stream from one held in memory.
void check_memory(Zorba* z, const std::string& aPath)
{
  int const count = 20000;
  write_array(aPath, count, 16384);

  if (run_count(z, "1 + 1") != "2")
    fail("warm-up query");

  std::string const expected = std::to_string(count + 1);
  long long const base = peak_rss();

  std::string value =
    run_count(z, "count(" + text_of(aPath, true) + ")");
  long long grown = peak_rss() - base;
  std::cout << "with the option: " << value << ", peak RSS grew by "
            << (grown >> 20) << " MiB" << std::endl;
  if (value != expected)
    fail("with the option: expected " + expected + ", got " + value);
#ifndef ZORBA_TEST_SANITIZER
  if (grown >= (64LL << 20))
    fail("with the option: peak RSS grew by " + std::to_string(grown >> 20) + " MiB");
#endif

  value = run_count(z, "count(" + text_of(aPath, false) + ")");
  grown = peak_rss() - base;
  std::cout << "without the option: " << value << ", peak RSS grew by "
            << (grown >> 20) << " MiB" << std::endl;
  if (value != expected)
    fail("without the option: expected " + expected + ", got " + value);
#ifndef ZORBA_TEST_SANITIZER
  if (grown < (256LL << 20))
    fail("without the option: the measure must see the members; it grew by " +
         std::to_string(grown >> 20) + " MiB");
#endif
}

struct Read
{
  int theCount = 0;
  bool theSawNew = false;
  std::string theError;
};

// Reads the first member, changes the file, and reads the rest.
Read read_with_change(Zorba* z, const std::string& aPath, bool aStreaming,
                      long aOffset, const std::string& aBytes)
{
  Read r;
  try
  {
    XQuery_t q = compile(z, text_of(aPath, aStreaming));
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
    {
      if (r.theCount == 0)
        overwrite(aPath, aOffset, aBytes);
      if (item.getStringValue().str()[0] == 'y')
        r.theSawNew = true;
      ++r.theCount;
    }
    it->close();
  }
  catch (ZorbaException const& e)
  {
    r.theError = e.diagnostic().qname().localname();
  }
  return r;
}

// The offset is past any stream buffer: what the second parse reads there is
// what the file holds after the first member.
void check_changed_file(Zorba* z, const std::string& aPath)
{
  int const count = 1000;
  int const size = 1024;
  long const middle = 1 + (count / 2) * size;

  // Malformed content: the second parse raises it after the first member.
  write_array(aPath, count, size);
  Read r = read_with_change(z, aPath, true, middle, "@");
  if (r.theError != "JNDY0021" || r.theCount == 0 || r.theCount >= count)
    fail("with the option, malformed change: got " + std::to_string(r.theCount) +
         " members and error '" + r.theError + "'");

  // Well-formed content of the same size: the new member comes out.
  write_array(aPath, count, size);
  r = read_with_change(z, aPath, true, middle + 1, "y");
  if (!r.theError.empty() || r.theCount != count + 1 || !r.theSawNew)
    fail("with the option, well-formed change: got " + std::to_string(r.theCount) +
         " members, error '" + r.theError + "', new member seen: " +
         (r.theSawNew ? "yes" : "no"));

  // Without the option the array was built before the first member.
  write_array(aPath, count, size);
  r = read_with_change(z, aPath, false, middle, "@");
  if (!r.theError.empty() || r.theCount != count + 1 || r.theSawNew)
    fail("without the option: got " + std::to_string(r.theCount) +
         " members and error '" + r.theError + "'");
}

} // namespace


int
json_parse_members(int, char*[])
{
  void* store = StoreManager::getStore();
  Zorba* z = Zorba::getInstance(store);

  std::string path;
  try
  {
    path = (std::filesystem::temp_directory_path() /
            ("json_parse_members_" +
#if defined(__unix__) || defined(__APPLE__)
             std::to_string(getpid()) +
#endif
             ".json")).string();
    check_changed_file(z, path);
    check_memory(z, path);
  }
  catch (std::exception const& e)
  {
    fail(std::string("exception: ") + e.what());
  }
  std::remove(path.c_str());

  z->shutdown();
  StoreManager::shutdownStore(store);
  return theFailures == 0 ? 0 : 1;
}
