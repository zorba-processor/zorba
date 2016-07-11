/*
 * Copyright 2006-2013 The FLWOR Foundation.
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
#ifndef HELPERS_H
#define HELPERS_H

#include <zorba/zorba.h>
#include <node.h>
#include <v8.h>

#define LOG(STRING) std::cout << STRING << std::endl;

zorba::Zorba* getZorbaInstance();

// FUNCTION DEFINITIONS
v8::Handle<v8::Value> Execute(const v8::Arguments& args);



#endif // N_DIAGNOSTIC_HANDLER_H