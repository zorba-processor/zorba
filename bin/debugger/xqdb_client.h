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
#pragma once
#ifndef ZORBA_DEBUGGER_XQDB_CLIENT_H
#define ZORBA_DEBUGGER_XQDB_CLIENT_H

#include <zorba/internal/unique_ptr.h>

#include "command_prompt.h"
#include "command_line_handler.h"


namespace zorba { namespace debugger {

class XqdbClient {

  public:

    XqdbClient(unsigned int aPort);

    ~XqdbClient();

    void
    start();

  private:

    // Members are destroyed in reverse order of declaration: the command line
    // handler goes first, the queues last.
    std::unique_ptr<LockFreeQueue<std::size_t>> theIdQueue;
    std::unique_ptr<LockFreeQueue<bool>>        theQuitQueue;

    std::unique_ptr<EventHandler>       theEventHandler;
    std::unique_ptr<CommandPrompt>      theCommandPrompt;
    std::unique_ptr<CommandLineHandler> theCommandLineHandler;
};

  
} // namespace zorba
} // namespace debugger

#endif // ZORBA_DEBUGGER_XQDB_CLIENT_H
