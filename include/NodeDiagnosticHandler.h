/*
 * Copyright 2006-2013 2006-2016 zorba.io.
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
#ifndef NODE_DIAGNOSTIC_HANDLER_H
#define NODE_DIAGNOSTIC_HANDLER_H

#include <zorba/zorba.h>
#include <node.h>
#include <v8.h>

using namespace v8;

class NodeDiagnosticHandler : public zorba::DiagnosticHandler {
  Local<Function> callback;
public:
  NodeDiagnosticHandler(Local<Function> &lCallback): callback(lCallback) {};
  void error( zorba::ZorbaException const &exception );
  void warning( zorba::XQueryWarning const &warning );

};

#endif // NODE_DIAGNOSTIC_HANDLER_H