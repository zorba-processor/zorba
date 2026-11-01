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
#ifndef ZORBA_RUNTIME_LOAD_DOCUMENT_H
#define ZORBA_RUNTIME_LOAD_DOCUMENT_H

#include "common/shared_types.h"
#include "store/api/item.h"
#include "zorbatypes/zstring.h"

namespace zorba
{

/**
 * Loads the document at a URI as fn:doc does: the URI is resolved against
 * the static base URI and through the URI resolvers of the static context,
 * and access is checked against its security policy (err:FODC0002 if
 * denied, not found or not parseable).
 *
 * @param aHtml If true, the resource is parsed as HTML and the document is
 * neither looked up in nor added to the store's documents (which hold what
 * fn:doc returns for the URI).
 */
void load_document_from_uri(
    zstring const& aUri,
    static_context* aSctx,
    PlanState& aPlanState,
    QueryLoc const& loc,
    store::Item_t& oResult,
    bool aHtml = false);

} // namespace zorba

#endif
/* vim:set et sw=2 ts=2: */
