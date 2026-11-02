/*
 * Copyright 2026 Zorba contributors
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

// sizeof() of the in-memory store classes, for the store memory benchmark
// (bench/store/run.sh). Compiled to assembly only, with the flags the build
// uses for src/store/naive/node_items.cpp; run.sh reads the constants out of
// the assembly, so nothing is linked against the store's internals.

#include "store/naive/atomic_items.h"
#include "store/naive/json_items.h"
#include "store/naive/node_items.h"
#include "store/naive/ordpath.h"
#include "store/naive/text_node_content.h"
#include "zorbatypes/zstring.h"

using namespace zorba;
using namespace zorba::simplestore;
using namespace zorba::simplestore::json;

// Non-const objects with C linkage: each one is emitted under its plain name
// as "zsize_NAME: .quad VALUE".
#define ZSIZE( name, ... ) \
  extern "C" { \
    __attribute__(( used )) unsigned long zsize_##name = sizeof( __VA_ARGS__ ); \
  }

ZSIZE( store_Item, store::Item )
ZSIZE( XmlTree, XmlTree )
ZSIZE( XmlNode, XmlNode )
ZSIZE( OrdPathNode, OrdPathNode )
ZSIZE( InternalNode, InternalNode )
ZSIZE( DocumentNode, DocumentNode )
ZSIZE( ElementNode, ElementNode )
ZSIZE( AttributeNode, AttributeNode )
ZSIZE( TextNode, TextNode )
ZSIZE( CommentNode, CommentNode )
ZSIZE( PiNode, PiNode )
ZSIZE( OrdPath, OrdPath )
ZSIZE( TextNodeContent, TextNodeContent )
ZSIZE( SimpleJSONObject, SimpleJSONObject )
ZSIZE( SimpleJSONArray, SimpleJSONArray )
ZSIZE( JSONObject_pair, std::pair<store::Item*, store::Item*> )
ZSIZE( JSONObject_keys_map, std::unordered_map<zstring, std::size_t> )
ZSIZE( AtomicItem, AtomicItem )
ZSIZE( StringItem, StringItem )
ZSIZE( UntypedAtomicItem, UntypedAtomicItem )
ZSIZE( QNameItem, QNameItem )
ZSIZE( IntegerItem, IntegerItem )
ZSIZE( LongItem, LongItem )
ZSIZE( DecimalItem, DecimalItem )
ZSIZE( DoubleItem, DoubleItem )
ZSIZE( BooleanItem, BooleanItem )
ZSIZE( JSONNull, JSONNull )
ZSIZE( zstring, zstring )
ZSIZE( zstring_rep, zstring::rep_type )
ZSIZE( xs_integer, xs_integer )
ZSIZE( xs_decimal, xs_decimal )
