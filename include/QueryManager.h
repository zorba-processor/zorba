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
#ifndef QUERY_MANAGER_H
#define QUERY_MANAGER_H

#include <zorba/zorba.h>
#include <thread>
#include <mutex>
#include <string>
#include "NodeDiagnosticHandler.h"
#include "helpers.h"
#include <queue>

class NodeQuery {
public:
  std::string query;
  std::shared_ptr<Local<Function>> resultCallback;
  std::shared_ptr<Local<Function>> errorCallback;
  NodeQuery(std::string lQuery, std::shared_ptr<Local<Function>> lResultCallback, std::shared_ptr<Local<Function>> lErrorCallback): 
    query(lQuery), resultCallback(lResultCallback), errorCallback(lErrorCallback) {};
};

class QueryManager {
private:

  static std::queue<std::shared_ptr<NodeQuery>> querysToExecute;
  static void executor();
  static std::mutex mutex;
  static std::thread QueryManager::thread;

public:
  static void pushQuery(std::string lQuery, std::shared_ptr<Local<Function>> lResultCallback, std::shared_ptr<Local<Function>> lErrorCallback);

};

#endif // QUERY_MANAGER_H