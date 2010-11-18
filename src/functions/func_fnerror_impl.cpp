/*
 * Copyright 2006-2008 The FLWOR Foundation.
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

#include "functions/func_fnerror.h"

#include "context/static_context.h"

#include "compiler/api/compilercb.h"

#include "runtime/fnerror/fnerror.h"

namespace zorba 
{

fn_error::fn_error(const signature& sig) 
  :
  function(sig, FunctionConsts::FN_UNKNOWN)
{
  switch (sig.paramCount())
  {
  case 0:
    theKind = FunctionConsts::FN_ERROR_0;
    break;

  case 1:
    theKind = FunctionConsts::FN_ERROR_1;
    break;

  case 2:
    theKind = FunctionConsts::FN_ERROR_2;
    break;

  case 3:
    theKind = FunctionConsts::FN_ERROR_3;
    break;

  default:
    ZORBA_ASSERT(false);
  }
}

} // namespace zorba