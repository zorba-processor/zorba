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
#ifndef ZORBA_REWRITER_EXPR_TOOLS_H
#define ZORBA_REWRITER_EXPR_TOOLS_H

#include "common/shared_types.h"

#include "compiler/expression/expr_base.h"
#include "compiler/expression/fo_expr.h"
#include "compiler/rewriter/framework/rewriter_context.h"

#include "functions/function.h"

#include "store/api/item_factory.h"
#include "system/globalenv.h"

#include <set>
#include <algorithm>

#define LOOKUP_FN( pfx, local, arity ) \
(GENV.getRootStaticContext().lookup_fn (pfx, local, arity))

#define ITEM_FACTORY (GENV.getStore().getItemFactory())

#define LOC(expr) (expr)->get_loc ()



namespace zorba
{

namespace expr_tools
{

bool match_exact(expr* query, expr* ast, expr::substitution_t& subst);

int count_variable_uses(expr* root, var_expr* v, int limit, std::vector<expr**>* path);

int estimate_expression_size(expr* root, int limit);

// Whether a call to this function can run code it receives as an argument or
// finds by name: a function type among its parameters, or fn:invoke and the like.
bool function_runs_opaque_code(const function* f);

// Whether a side effect can run during the evaluation of "root": a sequential
// expr or function, or a call whose target is not visible to the compiler. The
// scripting kind of an expr does not say this: a dynamic function call has the kind
// of a simple expr whatever it calls. User functions are followed into their bodies.
bool may_run_side_effects(const expr* root);

// Whether "root" can call a function that the compiler cannot prove to be
// deterministic and free of side effects. Only a function item whose origin is
// visible (a function item expression, reached through let and for variables and
// prolog variables that cannot be assigned) can be proven. A call that cannot, and
// anything that contains one or calls a user function containing one, is not moved,
// merged, duplicated or eliminated, and nothing moves across it: the function can
// change the state that other expressions read. The scripting kind of the expr
// does not say this, because a dynamic function call is a simple expr whatever it
// calls, and it must remain one for simple functions to be able to make such calls.
bool calls_unproven_function(const expr* root);

// The same for the call itself, without looking at its arguments.
bool is_unproven_call(const expr* call);

// Whether "root" can read or change a state that a function can change, or return
// new nodes: its value can depend on when it is evaluated and how many times.
bool depends_on_evaluation_order(const expr* root);

// Whether a clause of this flwor, or its return clause, makes an unproven call that
// another clause could observe: the call and everything else can run in clause order
// or interleaved, and only the first is what a sequential function gets.
bool evaluates_in_clause_order(const flwor_expr* flwor);


/*******************************************************************************

********************************************************************************/
expr* fix_annotations(expr* new_expr, const expr* old_expr = NULL);


/*******************************************************************************
  Each entry of VarIdMap maps a var_expr to its unique "prefix" id. The "prefix"
  id has the following property: for 2 vars v1 and v2, v1 is defined before v2
  if and only if prefix-id(v1) < prefix-id(v2). See the index_flwor_vars()
  function in expr_tools.cpp for more details.
********************************************************************************/
typedef std::map<var_expr *, ulong> VarIdMap;


/*******************************************************************************
  This is the reverse mapping of VarIdMap.
********************************************************************************/
typedef std::vector<var_expr*> IdVarMap;


/*******************************************************************************
  An entry into this map maps an expr to the variables that are referenced by
  that expr and/or its sub-exprs. (Note: in this context, the domain expr of a
  var $x is not considered a sub-expr of $x, and as a result, if $x is 
  referenced by an expr E and the domain expr of $x references another var $y,
  $y is NOT considered to be referenced by E). Only variables that have been
  assigned a prefix id (i.e., vars that appear in an associated VarIdMap) are
  considered. The set of vars referenced by an expr is implemented by a bitset
  that is indexed by prefix var id and whose size (in number of bits) is equal
  to the size of the associated VarIdMap.
********************************************************************************/
typedef std::map<const expr *, DynamicBitset> ExprVarsMap;


/*******************************************************************************
  Util functions used by rules: HoistExprsOutOfLoops and IndexJoin.
********************************************************************************/

void index_flwor_vars(const expr*, csize&, VarIdMap&, IdVarMap*);

void build_expr_to_vars_map(expr*, const VarIdMap&, DynamicBitset&, ExprVarsMap&);

void replace_var(expr*, const var_expr* oldVar, var_expr* newVar);


/*******************************************************************************

********************************************************************************/
inline bool is_data(expr* e)
{
  return (e->get_expr_kind() == fo_expr_kind &&
          static_cast<fo_expr *>(e)->get_func()->getKind() == FunctionConsts::FN_DATA_1);
}


}
}

#endif /* ZORBA_EXPR_TOOLS_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
