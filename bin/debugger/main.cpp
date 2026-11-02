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

#include <unistd.h>

#include <vector>

#include <zorba/config.h>
#include <zorba/internal/unique_ptr.h>

#include "xqdb_client.h"
#include "process_listener.h"


using namespace zorba;
using namespace zorba::debugger;


std::unique_ptr<XqdbClient> theClient;

// this will make sure the xqdb process will not quit when Ctrl-C is pressed
void
ctrlC_Handler(int lParam)
{
  // an empty sugnal handler on Linux should do the job
}


// this handler function is passed the the zorba process listener and will
// the client if the zorba process terminates
void
onExitProcess(ExitCode aExitCode) {
  std::cout << std::endl << "Terminating debugger client." << std::endl;

  theClient.reset();

  exit(aExitCode);
}


int
startZorba(std::string& aExec, std::vector<std::string>& aArgs, std::unique_ptr<ProcessListener>& aProcessListener) 
{
  // ************************
  // start a process on Linux

  pid_t pID = fork();
  if (pID == 0) {
    // Code only executed by child process
    std::stringstream lCommand;
    lCommand << aExec;
    for (std::vector<std::string>::size_type j = 0; j < aArgs.size(); j++) {
      lCommand << " " << aArgs.at(j);
    }

    int lRes = system(lCommand.str().c_str());
    exit(lRes);
  }
  else {
    // Code only executed by parent process
    if (pID < 0) {
      std::cerr << "Failed to fork Zorba" << std::endl;
      return pID;
    }
    
    // Watch the process
    aProcessListener = std::make_unique<ProcessListener>(pID, &onExitProcess);

    return 0;
  }
}

void printUsage(std::string& aProgram)
{
  std::cerr << "Usage:" << std::endl
    << "    " << aProgram << " <zorba_arguments>" << std::endl
    << "        this will start a debugger command line and a zorba process with the given arguments" << std::endl;
}

bool
processArguments(
  int argc,
  char* argv[],
  std::string& aProgram,
  bool& aStandalone,
  std::string& aZorba,
  unsigned int& aPort,
  std::vector<std::string>& aZorbaArgs)
{
  aPort = 28028;

  // find the path to Zorba and this executable name
  aProgram = argv[0];

  char lSep = '/';
  std::string::size_type lPos = aProgram.find_last_of(lSep);

  std::stringstream lZs;

  if (lPos == aProgram.npos) {
    lZs << "." << lSep;
  } else {
    lZs << aProgram.substr(0, lPos + 1);
    aProgram = aProgram.substr(lPos + 1);
  }
  lZs << "zorba";
  aZorba = lZs.str();


  bool lHasFileArg = false;
  bool lHasQueryArg = false;
  bool lHasQueryVal = false;

  // find if the user asked for help or specified a specific port
  for (int i = 1; i < argc; i++) {
    std::string lArg = argv[i];
    if (lArg == "-h" || lArg == "--help") {
      return false;
    }
    else if (lArg == "-p" || lArg == "--debug-port") {
      // if there is one more argument
      if (i < argc - 1) {
        // get the port value
        int lPort;
        std::stringstream lStream(argv[i + 1]);
        lStream >> lPort;
        if (!lStream.fail()) {
          aPort = lPort;
        }
      }
    }
    else if (lArg == "-f") {
      lHasFileArg = true;
    }
    else if (lArg == "-q") {
      lHasQueryArg = true;
      if (++i < argc) {
        lHasQueryVal = true;
      }
    }
  }

  if (!lHasFileArg || !lHasQueryArg || !lHasQueryVal) {
    std::cout << "Not enough arguments to start Zorba." << std::endl;
    std::cout << "Running the standalone XQuery debugger client on port: " << aPort << std::endl;
    return true;
  }

  // zorba will need the -d flag
  aZorbaArgs.push_back("-d");

  // gather all arguments (excepting the program name)
  for (int i = 1; i < argc; i++) {
    aZorbaArgs.push_back(argv[i]);
  }

  aStandalone = false;
  return true;
}

int
main(int argc, char* argv[])
{
  if (signal(SIGINT, ctrlC_Handler) == SIG_ERR) {
    std::cerr << "Failed to install the Ctrl-C handler" << std::endl;
    return 1;
  }

  // **************************************************************************
  // processing arguments

  std::string lProgram, lZorbaExec;
  unsigned int lPort = 28028;
  std::vector<std::string> lZorbaArgs;

  bool lStandalone = true;
  if (!processArguments(argc, argv, lProgram, lStandalone, lZorbaExec, lPort, lZorbaArgs)) {
    printUsage(lProgram);
    return 1;
  }

#ifndef NDEBUG
  // **************************************************************************
  // debug reporting

  if (!lStandalone) {
    std::cout << "Communication port: " << lPort << std::endl;
    std::cout << "Zorba executable:   " << lZorbaExec << std::endl;
    std::cout << "Zorba arguments:    ";
    for (std::vector<std::string>::size_type j = 0; j < lZorbaArgs.size(); j++) {
      std::cout << lZorbaArgs.at(j) << " ";
    }
    std::cout << std::endl;
  }
#endif

  try {
    // **************************************************************************
    // start a zorba

    // This is a process listener used to watch the Zorba process termination.
    std::unique_ptr<ProcessListener> lProcessListener;

    if (!lStandalone) {
      int lResult = startZorba(lZorbaExec, lZorbaArgs, lProcessListener);
      if (lResult) {
        return lResult;
      }
    } else {
      std::cout << "Listening for an incomming Zorba connection on port " << lPort << "..." << std::endl;
    }

    // **************************************************************************
    // start the debugger command line

    theClient = std::make_unique<XqdbClient>(lPort);
    theClient->start();

  } catch (...) {
    return -1;
  }

  theClient.reset();

  return 0;
}
