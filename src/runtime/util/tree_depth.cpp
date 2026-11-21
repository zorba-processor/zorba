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
#include "runtime/util/tree_depth.h"

#include "diagnostics/xquery_diagnostics.h"
#include "store/api/item.h"
#include "zorbatypes/zstring.h"

namespace zorba {
namespace tree_depth {

void raise_stack(const store::Item* aNode, const char* aVerb,
                 const QueryLoc* aLoc)
{
  // Walking up takes no stack.
  uint64_t lDepth = 1;
  const store::Item* lRoot = aNode;
  while (const store::Item* lParent = lRoot->getParent())
  {
    lRoot = lParent;
    ++lDepth;
  }

  zstring lUri;
  if (lRoot->getNodeKind() == store::StoreConsts::documentNode)
    lRoot->getDocumentURI(lUri);
  // The loaders name a document read without a URI this way.
  if (lUri.compare(0, 28, "zorba://internalDocumentURI-") == 0)
    lUri.clear();

  XQueryException e(
    lUri.empty() ?
    XQUERY_EXCEPTION(zerr::ZXQP0072_XML_NESTING_EXCEEDED,
                     ERROR_PARAMS(aVerb, lDepth)) :
    XQUERY_EXCEPTION(zerr::ZXQP0072_XML_NESTING_EXCEEDED,
                     ERROR_PARAMS(aVerb, lDepth, lUri)));
  if (aLoc)
    set_source(e, *aLoc);
  throw e;
}

void locate(ZorbaException& e, const QueryLoc& aLoc)
{
  if (e.diagnostic() == zerr::ZXQP0072_XML_NESTING_EXCEEDED)
    set_source(e, aLoc, false);
}

} // namespace tree_depth
} // namespace zorba
/* vim:set et sw=2 ts=2: */
