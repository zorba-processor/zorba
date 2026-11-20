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

#ifndef ZORBA_RUNTIME_PARSING_AND_SERIALIZING_INVALID_CONTENT_H
#define ZORBA_RUNTIME_PARSING_AND_SERIALIZING_INVALID_CONTENT_H

#include <memory>
#include <sstream>

#include "compiler/parser/query_loc.h"
#include "diagnostics/xquery_diagnostics.h"

namespace zorba
{

/**
 * Throws err:FODC0006 for the error that parsing the argument of \a aFunction
 * raised: the message names that error's code and quotes its message, and the
 * data location (where in the parsed string) is that error's. An exceeded
 * limit of the parser (err:XPDY0130) is not invalid content: it is thrown
 * as it is, at \a aLoc.
 */
[[noreturn]] inline void throw_invalid_content(
    char const* aFunction,
    ZorbaException const& aCause,
    QueryLoc const& aLoc)
{
  if (aCause.diagnostic() == err::XPDY0130)
  {
    std::unique_ptr<ZorbaException> const lLimit(clone(aCause));
    set_source(*lLimit, aLoc);
    lLimit->polymorphic_throw();
  }

  std::ostringstream lCause;
  lCause << '[' << aCause.diagnostic().qname() << "]: " << aCause.what();

  XQueryException xe(XQUERY_EXCEPTION(err::FODC0006,
    ERROR_PARAMS(aFunction, ERROR_TEXT( lCause.str() )), ERROR_LOC(aLoc)));
  set_data(xe, aCause);
  throw xe;
}

} // namespace zorba

#endif /* ZORBA_RUNTIME_PARSING_AND_SERIALIZING_INVALID_CONTENT_H */
/* vim:set et sw=2 ts=2: */
