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
#include <iostream>
#include <memory>

#include "zorbamisc/ns_consts.h"

#include "functions/library.h"
#include "functions/function.h"
#include "functions/signature.h"

#include "functions/Accessors.h"
#include "functions/Collections.h"
#include "functions/Numerics.h"
#include "functions/QNames.h"
#include "functions/Sequences.h"
#include "functions/nodeid_internal.h"
#include "functions/Strings.h"
#include "functions/Boolean.h"
#include "functions/EnclosedExpr.h"
#include "functions/ContextFunctions.h"
#include "functions/DateTime.h"
#include "functions/VarDecl.h"
#include "functions/Misc.h"
#include "functions/arithmetic.h"
#include "functions/Nodes.h"
#include "functions/tuple_functions.h"
#include "functions/InternalOperators.h"
#include "functions/Index.h"
#include "functions/Alexis.h"
#include "functions/Convertors.h"
#include "functions/XQDocFunctions.h"
#include "functions/Rest.h"
#include "functions/Email.h"
#include "functions/Fop.h"


namespace zorba 
{

// clear static initializer state

// dummy function to tell the windows linker to keep the library.obj
// even though it contains no public objects or functions
// this is called at initializeZorba
void library_init()
{
}

void BuiltinFunctionLibrary::populateContext(static_context* sctx)
{
  zorba::serialization::Archiver &ar = *::zorba::serialization::ClassSerializer::getInstance()->getArchiverForHardcodedObjects();
  ar.set_loading_hardcoded_objects(true);

  populateContext_Accesors(sctx);
  populateContext_Nodes(sctx);
  populateContext_AnyURI(sctx);
  populateContext_Arithmetics(sctx);
  populateContext_Numerics(sctx);
  populateContext_Context(sctx);
  populateContext_QNames(sctx);
  populateContext_Sequences(sctx);
  populateContext_DocOrder(sctx);
  populateContext_Comparison(sctx);
  populateContext_Strings(sctx);
  populateContext_Boolean(sctx);
  populateContext_Constructors(sctx);
  populateContext_Datetime(sctx);
  populateContext_Collections(sctx);
  populateContext_VarDecl(sctx);
  populateContext_Error(sctx);
  populateContext_Debug(sctx);
  populateContext_Math(sctx);
  populateContext_Index(sctx);
  populateContext_Hoisting(sctx);
  populateContext_Alexis(sctx);
  populateContext_Rest(sctx);
  populateContext_Email(sctx);
  populateContext_JSON(sctx);
  populateContext_FOP(sctx);
  populateContext_XQDOC(sctx);

  ar.set_loading_hardcoded_objects(false);
}


} /* namespace zorba */
