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
#ifndef ZORBA_RUNTIME_UTIL_TREE_DEPTH_H
#define ZORBA_RUNTIME_UTIL_TREE_DEPTH_H

#include <zorba/zorba_exception.h>

#include "compiler/parser/query_loc.h"
#include "runtime/util/stack_limit.h"

namespace zorba {

namespace store {
class Item;
}

namespace tree_depth {

/**
 * Raises zerr:ZXQP0072: there is not enough stack left to aVerb (serialize,
 * validate, copy, ...) aNode's children. The message gives aNode's depth and
 * the URI of its document, if it has one.
 */
[[noreturn]] void raise_stack(const store::Item* aNode, const char* aVerb,
                              const QueryLoc* aLoc = nullptr);

/**
 * To be called once per level by what walks over an XML tree recursively,
 * before it goes down into aNode's children.
 */
inline void check_stack(const store::Item* aNode, const char* aVerb)
{
  if (stack_limit::exhausted())
    raise_stack(aNode, aVerb);
}

/** The same, for a walk that knows the location of its expression. */
inline void check_stack(const store::Item* aNode, const char* aVerb,
                        const QueryLoc& aLoc)
{
  if (stack_limit::exhausted())
    raise_stack(aNode, aVerb, &aLoc);
}

/**
 * Gives a zerr:ZXQP0072 raised below an expression that expression's
 * location; other errors are left as they are.
 */
void locate(ZorbaException& e, const QueryLoc& aLoc);

/** Runs aWalk, giving a zerr:ZXQP0072 it raises aLoc. */
template <class Walk>
auto located(const QueryLoc& aLoc, Walk aWalk) -> decltype(aWalk())
{
  try
  {
    return aWalk();
  }
  catch (ZorbaException& e)
  {
    locate(e, aLoc);
    throw;
  }
}

} // namespace tree_depth
} // namespace zorba

#endif /* ZORBA_RUNTIME_UTIL_TREE_DEPTH_H */
/* vim:set et sw=2 ts=2: */
