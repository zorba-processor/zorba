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

// Drives the DBGP debugger server over its socket with a scripted client and
// checks the property_get, property_value and property_set commands.

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <zorba/config.h>
#include <zorba/zorba.h>
#include <zorba/store_manager.h>
#include <zorba/zorba_exception.h>
#include <zorba/util/base64_util.h>

#ifndef WIN32
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
#endif

using namespace zorba;

#if defined(ZORBA_WITH_DEBUGGER) && !defined(WIN32)

namespace {

// the line numbers matter: the breakpoint is set on line 7
char const QUERY[] =
  "declare namespace ann = \"http://zorba.io/annotations\";\n"  // 1
  "declare variable $g as xs:integer := 1;\n"                    // 2
  "declare %ann:nonassignable variable $c := 2;\n"               // 3
  "variable $l := \"local\";\n"                                  // 4
  "variable $s := (1 to 40);\n"                                  // 5
  "for $f in (1, 2)\n"                                           // 6
  "return ($g * 10 + $f, $c, $l, count($s))\n";                  // 7

int theFailures = 0;

void check(bool aCondition, std::string const& aWhat, std::string const& aContext)
{
  if (!aCondition) {
    ++theFailures;
    std::cerr << "FAILED: " << aWhat << "\n  got: " << aContext << std::endl;
  }
}

std::string b64(std::string const& aText)
{
  std::string lResult;
  base64::encode(aText, &lResult);
  return lResult;
}

class Client
{
  int theSocket;
  int theNextId;

  bool readUntilNul(std::string& aResult)
  {
    aResult.clear();
    char c;
    while (true) {
      ssize_t n = ::recv(theSocket, &c, 1, 0);
      if (n <= 0)
        return false;
      if (c == '\0')
        return true;
      aResult += c;
    }
  }

public:
  explicit Client(int aSocket) : theSocket(aSocket), theNextId(0) {}

  ~Client() { ::close(theSocket); }

  // a DBGP engine message is: <length> NUL <xml> NUL
  std::string receive()
  {
    std::string lLength, lXml;
    if (!readUntilNul(lLength) || !readUntilNul(lXml))
      return "";
    return lXml;
  }

  // receive messages until one contains aNeedle (async responses of
  // continuation commands may arrive later than the command was sent)
  std::string receiveUntil(std::string const& aNeedle)
  {
    for (int i = 0; i < 10; ++i) {
      std::string lMsg = receive();
      if (lMsg.empty() || lMsg.find(aNeedle) != std::string::npos)
        return lMsg;
    }
    return "";
  }

  void sendOnly(std::string const& aCommand, std::string const& aArgs = "")
  {
    std::ostringstream lSs;
    lSs << aCommand << " -i " << ++theNextId;
    if (!aArgs.empty())
      lSs << " " << aArgs;
    std::string lMsg = lSs.str();
    ::send(theSocket, lMsg.c_str(), lMsg.size() + 1, 0);  // incl. NUL
  }

  std::string command(std::string const& aCommand, std::string const& aArgs = "")
  {
    sendOnly(aCommand, aArgs);
    return receive();
  }
};

bool contains(std::string const& aHaystack, std::string const& aNeedle)
{
  return aHaystack.find(aNeedle) != std::string::npos;
}

// the text content of the (only) property element, base64 decoded
std::string propertyValue(std::string const& aResponse)
{
  std::string::size_type lStart = aResponse.find("<property ");
  if (lStart == std::string::npos)
    return "<no property>";
  lStart = aResponse.find('>', lStart) + 1;
  std::string::size_type lEnd = aResponse.find("</property>", lStart);
  std::string lDecoded;
  try {
    base64::decode(aResponse.substr(lStart, lEnd - lStart), &lDecoded);
  } catch (base64::exception const&) {
    return "<invalid base64>";
  }
  return lDecoded;
}

// the text content of a property_value response, base64 decoded
std::string responseValue(std::string const& aResponse)
{
  std::string::size_type lStart = aResponse.find('>') + 1;
  std::string::size_type lEnd = aResponse.find("</response>", lStart);
  if (lEnd == std::string::npos)
    return "<no value>";
  std::string lDecoded;
  try {
    base64::decode(aResponse.substr(lStart, lEnd - lStart), &lDecoded);
  } catch (base64::exception const&) {
    return "<invalid base64>";
  }
  return lDecoded;
}

std::string allNumbers(int aFrom, int aTo)
{
  std::ostringstream lSs;
  for (int i = aFrom; i <= aTo; ++i)
    lSs << (i > aFrom ? " " : "") << i;
  return lSs.str();
}

void runClient(Client& aClient, std::string const& aQueryPath)
{
  std::string r = aClient.receive();
  check(contains(r, "<init "), "init message", r);

  r = aClient.command("breakpoint_set", "-f \"" + aQueryPath + "\" -n 7");
  check(contains(r, "id=\"") && !contains(r, "<error"), "breakpoint_set", r);
  std::string lBkpId;
  std::string::size_type lPos = r.find("id=\"");
  if (lPos != std::string::npos)
    lBkpId = r.substr(lPos + 4, r.find('"', lPos + 4) - lPos - 4);

  aClient.sendOnly("run");
  r = aClient.receiveUntil("status=\"");
  check(contains(r, "status=\"break\""), "run stops at the breakpoint", r);
  if (!contains(r, "status=\"break\"")) {
    // nothing else can be checked
    aClient.sendOnly("stop");
    aClient.receiveUntil("command=\"stop\"");
    return;
  }

  // property_get
  r = aClient.command("property_get", "-n g");
  check(propertyValue(r) == "1", "property_get g", r);

  // property_value: the full value, -m max data, -p page
  r = aClient.command("property_value", "-n s");
  std::string const lAll = allNumbers(1, 40);
  check(responseValue(r) == lAll, "property_value s", r);
  std::ostringstream lSize;
  lSize << "size=\"" << lAll.size() << "\"";
  check(contains(r, lSize.str()), "property_value s size", r);

  r = aClient.command("property_value", "-n s -m 5");
  check(responseValue(r) == "1 2 3", "property_value s -m 5", r);
  check(contains(r, lSize.str()), "property_value s -m 5 reports the full size", r);

  r = aClient.command("property_value", "-n s -p 1");
  check(responseValue(r) == allNumbers(33, 40), "property_value s -p 1", r);

  r = aClient.command("property_value", "-n l");
  check(responseValue(r) == "local", "property_value l", r);

  r = aClient.command("property_value", "-n nosuch");
  check(contains(r, "<error code=\"300\""), "property_value of an unknown variable", r);

  r = aClient.command("property_value", "-n g -d 1");
  check(contains(r, "<error code=\"301\""), "property_value with an invalid depth", r);

  // property_set: prolog variable, block-local variables
  r = aClient.command("property_set", "-n g -- " + b64("5"));
  check(contains(r, "success=\"1\""), "property_set g", r);
  r = aClient.command("property_get", "-n g");
  check(propertyValue(r) == "5", "property_get g after property_set", r);

  r = aClient.command("property_set", "-n l -- " + b64("\"changed\""));
  check(contains(r, "success=\"1\""), "property_set l", r);

  r = aClient.command("property_set", "-n s -- " + b64("(1, 2, 3)"));
  check(contains(r, "success=\"1\""), "property_set s", r);
  r = aClient.command("property_value", "-n s");
  check(responseValue(r) == "1 2 3", "property_value s after property_set", r);

  // property_set errors
  r = aClient.command("property_set", "-n c -- " + b64("3"));
  check(contains(r, "<error code=\"300\""), "property_set of a non-assignable variable", r);

  r = aClient.command("property_set", "-n f -- " + b64("3"));
  check(contains(r, "<error code=\"300\""), "property_set of a FLWOR variable", r);

  r = aClient.command("property_set", "-n nosuch -- " + b64("3"));
  check(contains(r, "<error code=\"300\""), "property_set of an unknown variable", r);

  r = aClient.command("property_set", "-n g -- " + b64("\"text\""));
  check(contains(r, "<error code=\"206\""), "property_set with a value of the wrong type", r);

  r = aClient.command("property_set", "-n g -- " + b64("1 +"));
  check(contains(r, "<error code=\"206\""), "property_set with an invalid expression", r);

  r = aClient.command("property_set", "-n g");
  check(contains(r, "<error code=\"3\""), "property_set without data", r);

  // the failed attempts did not change anything
  r = aClient.command("property_get", "-n g");
  check(propertyValue(r) == "5", "property_get g after the failed property_set", r);
  r = aClient.command("property_get", "-n c");
  check(propertyValue(r) == "2", "property_get c after the failed property_set", r);

  // finish the query; the result must reflect the assignments
  r = aClient.command("breakpoint_remove", "-d " + lBkpId);
  check(!contains(r, "<error"), "breakpoint_remove", r);

  aClient.sendOnly("run");
  r = aClient.receiveUntil("status=\"stopping\"");
  check(contains(r, "status=\"stopping\""), "run to the end of the query", r);

  r = aClient.command("property_set", "-n g -- " + b64("1"));
  check(contains(r, "<error code=\"5\""), "property_set when not suspended", r);

  aClient.sendOnly("stop");
  aClient.receiveUntil("command=\"stop\"");
}

} // namespace

int
debugger_property(int argc, char* argv[])
{
  // the query must be in a file: breakpoints refer to file and line
  char lCwd[4096];
  if (!getcwd(lCwd, sizeof lCwd)) {
    std::cerr << "getcwd failed" << std::endl;
    return 1;
  }
  std::string const lQueryPath = std::string(lCwd) + "/debugger_property.xq";
  {
    std::ofstream lFile(lQueryPath.c_str());
    lFile << QUERY;
  }

  // the server connects to the client: listen on an ephemeral port
  int lListen = ::socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in lAddr;
  std::memset(&lAddr, 0, sizeof lAddr);
  lAddr.sin_family = AF_INET;
  lAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  lAddr.sin_port = 0;
  socklen_t lAddrLen = sizeof lAddr;
  if (lListen < 0 ||
      ::bind(lListen, reinterpret_cast<sockaddr*>(&lAddr), sizeof lAddr) != 0 ||
      ::listen(lListen, 1) != 0 ||
      ::getsockname(lListen, reinterpret_cast<sockaddr*>(&lAddr), &lAddrLen) != 0) {
    std::cerr << "cannot listen on a local port" << std::endl;
    return 1;
  }
  unsigned short const lPort = ntohs(lAddr.sin_port);

  // never hang the test if the server does not connect or stops answering
  timeval lTimeout;
  lTimeout.tv_sec = 30;
  lTimeout.tv_usec = 0;
  ::setsockopt(lListen, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof lTimeout);

  void* lStore = StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  std::ostringstream lOutput;
  int lResult = 0;

  try {
    XQuery_t lQuery = lZorba->createQuery();
    lQuery->setFileName(lQueryPath);
    lQuery->setDebugMode(true);
    Zorba_CompilerHints_t lHints;
    lHints.opt_level = ZORBA_OPT_LEVEL_O0;
    std::ifstream lIn(lQueryPath.c_str());
    lQuery->compile(lIn, lHints);

    Zorba_SerializerOptions_t lSerOptions;
    lSerOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;

    std::string lServerError;
    std::thread lServer([&]() {
      try {
        lQuery->debug(lOutput, lSerOptions, "127.0.0.1", lPort);
      } catch (ZorbaException const& e) {
        lServerError = e.what();
      }
    });

    int lSocket = ::accept(lListen, NULL, NULL);
    if (lSocket < 0) {
      std::cerr << "accept failed" << std::endl;
      lResult = 1;
    } else {
      ::setsockopt(lSocket, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof lTimeout);

      Client lClient(lSocket);
      runClient(lClient, lQueryPath);
    }

    lServer.join();
    lQuery->close();

    check(lServerError.empty(), "debugger server error", lServerError);
    check(lOutput.str() == "51 2 changed 3 52 2 changed 3",
          "query result reflects property_set", lOutput.str());
  } catch (ZorbaException const& e) {
    std::cerr << e << std::endl;
    lResult = 1;
  }

  ::close(lListen);
  lZorba->shutdown();
  StoreManager::shutdownStore(lStore);

  if (theFailures > 0) {
    std::cerr << theFailures << " check(s) failed" << std::endl;
    return 2;
  }
  return lResult;
}

#else

int
debugger_property(int argc, char* argv[])
{
  // the debugger is not built, or this platform has no POSIX sockets
  return 0;
}

#endif
/* vim:set et sw=2 ts=2: */
