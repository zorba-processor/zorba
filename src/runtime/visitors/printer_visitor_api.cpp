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

#include "runtime/visitors/printer_visitor_api.h"
#include "runtime/visitors/printer_visitor.h"
#include "store/api/item.h"

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

void print_iter_plan( IterPrinter &p, PlanIterator *i, PlanState *s ) {
  PrinterVisitor v( p, i, s );
  v.print();
}

///////////////////////////////////////////////////////////////////////////////

std::string plan_function_name( store::Item const *name ) {
  static std::string const lInlinePrefix( "inline-function(" );
  std::string lName( name->getStringValue().str() );

  if ( name->getNamespace().empty() &&
       name->getLocalName().str().compare( 0, lInlinePrefix.size(), lInlinePrefix ) == 0 ) {
    std::string::size_type const lSeparator = lName.find_last_of( "/\\" );
    if ( lSeparator != std::string::npos )
      lName = lInlinePrefix + lName.substr( lSeparator + 1 );
  }
  return lName;
}


} // namespace zorba
/* vim:set et sw=2 ts=2: */
