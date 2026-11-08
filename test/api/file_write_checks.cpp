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

// file:write-text, file:write-text-lines, file:write-binary and the append
// variants copy a streamable item into the file. A read error of the item's
// stream or a write error on the file raises FOFL9999; an empty stream writes
// an empty file.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>

#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;
namespace sfs = std::filesystem;

namespace {

int theFailures = 0;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      std::cerr << __FILE__ << ":" << __LINE__ << ": check failed: "    \
                << #cond << std::endl;                                  \
      ++theFailures;                                                    \
    }                                                                   \
  } while (0)

// Delivers its text, then fails to read.
class FailingBuf : public std::streambuf
{
public:
  explicit FailingBuf(std::string const& aText) : theText(aText)
  {
    setg(&theText[0], &theText[0], &theText[0] + theText.size());
  }

protected:
  int_type underflow() override
  {
    throw std::runtime_error("read error");
  }

private:
  std::string theText;
};

class FailingStream : public std::istream
{
public:
  explicit FailingStream(std::string const& aText)
    : std::istream(nullptr), theBuf(aText)
  {
    rdbuf(&theBuf);
  }

private:
  FailingBuf theBuf;
};

void release(std::istream* aStream)
{
  delete aStream;
}

std::istream* good_stream(std::string const& aText)
{
  return new std::istringstream(aText);
}

enum Kind { TEXT, TEXT_LINES, BINARY };

struct Outcome
{
  bool ok = false;
  std::string code;
  std::string message;
};

// Runs the file function on a streamable item.
Outcome write(Zorba* z, Kind aKind, bool aAppend, std::string const& aPath,
              std::istream* aStream)
{
  char const* lFunction = "";
  switch (aKind)
  {
    case TEXT:       lFunction = aAppend ? "append-text" : "write-text"; break;
    case TEXT_LINES: lFunction = aAppend ? "append-text-lines"
                                         : "write-text-lines"; break;
    case BINARY:     lFunction = aAppend ? "append-binary" : "write-binary";
                     break;
  }

  Item lItem = aKind == BINARY
    ? z->getItemFactory()->createStreamableBase64Binary(*aStream, release)
    : z->getItemFactory()->createStreamableString(*aStream, release);

  Outcome o;
  try
  {
    XQuery_t q = z->compileQuery(
      std::string("import module namespace file = "
                  "\"http://expath.org/ns/file\";\n"
                  "declare variable $path external;\n"
                  "declare variable $content external;\n"
                  "file:") + lFunction + "($path, $content)");
    q->getDynamicContext()->setVariable(
      "path", z->getItemFactory()->createString(aPath));
    q->getDynamicContext()->setVariable("content", lItem);
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while (it->next(item))
      ;
    it->close();
    o.ok = true;
  }
  catch (ZorbaException const& e)
  {
    o.code = e.diagnostic().qname().localname();
    o.message = e.what();
  }
  return o;
}

std::string read_all(std::string const& aPath)
{
  std::ifstream in(aPath, std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(in),
                     std::istreambuf_iterator<char>());
}

void report(char const* aWhat, Outcome const& o)
{
  std::cerr << aWhat << ": "
            << (o.ok ? std::string("returned success")
                     : o.code + ": " + o.message)
            << std::endl;
}

Kind const kinds[] = { TEXT, TEXT_LINES, BINARY };
char const* const kindNames[] = { "text", "text-lines", "binary" };

// Valid input is copied as it was.
void test_valid(Zorba* z, std::string const& aDir)
{
  std::string const lPath = aDir + "/valid";
  for (int k = 0; k < 3; ++k)
  {
    std::string const lContent(100000, 'x');
    Outcome o = write(z, kinds[k], false, lPath, good_stream(lContent));
    CHECK(o.ok);
    std::string const lExpected = kinds[k] == TEXT_LINES ? lContent + "\n"
                                                         : lContent;
    CHECK(read_all(lPath) == lExpected);
    o = write(z, kinds[k], true, lPath, good_stream("ab"));
    CHECK(o.ok);
    CHECK(read_all(lPath) == lExpected + (kinds[k] == TEXT_LINES ? "ab\n"
                                                                 : "ab"));
  }
}

// An empty stream is not an error: the file is written, empty.
void test_empty(Zorba* z, std::string const& aDir)
{
  std::string const lPath = aDir + "/empty";
  for (int k = 0; k < 3; ++k)
  {
    { std::ofstream(lPath) << "old content"; }
    Outcome o = write(z, kinds[k], false, lPath, good_stream(""));
    if (!o.ok)
      report(kindNames[k], o);
    CHECK(o.ok);
    CHECK(sfs::exists(lPath));
    std::string const lExpected = kinds[k] == TEXT_LINES ? "\n" : "";
    CHECK(read_all(lPath) == lExpected);
  }
}

// A read error of the item's stream is reported.
void test_read_error(Zorba* z, std::string const& aDir)
{
  std::string const lPath = aDir + "/read-error";
  for (int k = 0; k < 3; ++k)
  {
    Outcome o = write(z, kinds[k], false, lPath,
                      new FailingStream("some text"));
    if (o.ok)
      report(kindNames[k], o);
    CHECK(!o.ok);
    CHECK(o.code == "FOFL9999");
    CHECK(o.message.find(lPath) != std::string::npos);
  }
}

// A write error on the file is reported. A file of /proc whose write is
// refused by the kernel is the portable way to make one; elsewhere the check
// is skipped.
void test_write_error(Zorba* z)
{
  std::string const lPath = "/proc/self/oom_score_adj";
  std::string const lOriginal = read_all(lPath);
  bool lUsable = false;
  {
    std::ofstream out(lPath);
    if (out)
    {
      out << "not a number";
      out.flush();
      lUsable = !out;                 // the kernel refuses the value
    }
  }
  if (!lUsable)
  {
    std::cerr << "no refusing file in /proc: write error check skipped"
              << std::endl;
    return;
  }

  for (int k = 0; k < 3; ++k)
  {
    Outcome o = write(z, kinds[k], false, lPath, good_stream("not a number"));
    if (o.ok)
      report(kindNames[k], o);
    CHECK(!o.ok);
    CHECK(o.code == "FOFL9999");
    CHECK(o.message.find(lPath) != std::string::npos);
    CHECK(o.message.find("can not open") == std::string::npos);
  }
  // The value of the setting was never changed.
  CHECK(read_all(lPath) == lOriginal);
}

} // anonymous namespace


int
file_write_checks(int argc, char* argv[])
{
  sfs::path lDir = sfs::temp_directory_path() /
    ("zorba-file-write-" + std::to_string(std::rand()) + "-" +
     std::to_string(std::chrono::steady_clock::now()
                      .time_since_epoch().count()));
  sfs::create_directories(lDir);

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  lZorba->setSecurityPolicy(SecurityPolicy::unrestricted());

  try
  {
    test_valid(lZorba, lDir.generic_string());
    test_empty(lZorba, lDir.generic_string());
    test_read_error(lZorba, lDir.generic_string());
    test_write_error(lZorba);
  }
  catch (ZorbaException const& e)
  {
    std::cerr << "unexpected exception: " << e << std::endl;
    ++theFailures;
  }
  catch (std::exception const& e)
  {
    std::cerr << "unexpected exception: " << e.what() << std::endl;
    ++theFailures;
  }

  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  std::error_code ec;
  sfs::remove_all(lDir, ec);

  if (theFailures)
    std::cerr << theFailures << " check(s) failed" << std::endl;
  return theFailures == 0 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
