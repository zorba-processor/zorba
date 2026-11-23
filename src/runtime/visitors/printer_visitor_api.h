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
#ifndef ZORBA_RUNTIME_PRINTER_VISITOR_API_H
#define ZORBA_RUNTIME_PRINTER_VISITOR_API_H

#include <string>

namespace zorba {

namespace store {
class Item;
}

///////////////////////////////////////////////////////////////////////////////

class PlanIterator;
class PlanState;
class IterPrinter;

void print_iter_plan( IterPrinter&, PlanIterator*, PlanState* = 0 );

/**
 * The name of a function as a plan prints it: an inline function, named after
 * its location, keeps only the file name of its query, so that the plan does
 * not depend on the directory of the query.
 */
std::string plan_function_name( store::Item const *name );

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba
#endif /* ZORBA_RUNTIME_PRINTER_VISITOR_API_H */
/* vim:set et sw=2 ts=2: */
