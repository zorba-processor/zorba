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
#ifndef ZORBA_ROOT_STATIC_CONTEXT_H
#define ZORBA_ROOT_STATIC_CONTEXT_H

#include "context/static_context.h"

namespace zorba {

class root_static_context : public static_context 
{
  friend class GlobalEnvironment;
  friend class static_context;

private:
  static const char* theBuiltinURIPath[];
  static const char* theBuiltinLibPath[];

protected:
  zstring theImplementationBaseUri;

private:
  root_static_context();

  virtual ~root_static_context();

  void init();

  virtual void free();

  const char** get_builtin_uri_path() const { return theBuiltinURIPath; }
  const char** get_builtin_lib_path() const { return theBuiltinLibPath; }
};

}

#endif /* ZORBA_ROOT_STATIC_CONTEXT_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
