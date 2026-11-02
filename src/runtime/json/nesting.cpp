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

#include "diagnostics/xquery_diagnostics.h"

#include "nesting.h"

namespace zorba {
namespace json_nesting {

void raise_limit() {
  throw XQUERY_EXCEPTION(
    zerr::ZXQP0071_JSON_NESTING_EXCEEDED,
    ERROR_PARAMS( ZED( ZXQP0071_Limit_2 ), MAX_DEPTH )
  );
}

void raise_stack() {
  throw XQUERY_EXCEPTION(
    zerr::ZXQP0071_JSON_NESTING_EXCEEDED,
    ERROR_PARAMS( ZED( ZXQP0071_Stack ) )
  );
}

} // namespace json_nesting
} // namespace zorba
/* vim:set et sw=2 ts=2: */
