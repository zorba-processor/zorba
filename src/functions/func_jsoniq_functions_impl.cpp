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

#include <algorithm>
#include <optional>
#include <set>
#include <string_view>

#include "functions/func_jsoniq_functions.h"
#include "functions/func_jsoniq_functions_impl.h"

#include "runtime/json/json_constructors.h"
#include "runtime/json/jsoniq_functions.h"
#include "runtime/json/json_loader.h"
#include "runtime/core/item_iterator.h"
#include "runtime/errors_and_diagnostics/other_diagnostics.h"

#include "compiler/parser/query_loc.h"
#include "compiler/expression/expr_consts.h"
#include "compiler/api/compilercb.h"
#include "compiler/expression/expr.h"
#include "compiler/expression/expr_iter.h"
#include "compiler/expression/flwor_expr.h"
#include "compiler/expression/fo_expr.h"
#include "compiler/expression/function_item_expr.h"
#include "compiler/expression/json_exprs.h"
#include "compiler/expression/var_expr.h"

#include "types/typeops.h"

namespace zorba
{

/*******************************************************************************

********************************************************************************/
PlanIter_t fn_jsoniq_parse_json::codegen(
    CompilerCB*,
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& argv,
    expr& ann) const
{
  SingletonIterator* lArg = dynamic_cast<SingletonIterator*>(argv[0].getp());
  if (lArg && lArg->getValue()->isAtomic())
  {
    const QueryLoc& lArgLoc = lArg->getLocation();
    return new JSONParseIterator(sctx, loc, argv, lArgLoc);
  }
  else
  {
    QueryLoc lArgLoc = QueryLoc::null;
    return new JSONParseIterator(sctx, loc, argv, lArgLoc);
  }
}


/*******************************************************************************
  Parse-time projection: a parse whose values the query reads only through
  lookups of literal keys, or only counts, builds only what is read (see
  json::projection).
  At -O0 every value is built, so that the optimized levels have a reference.
********************************************************************************/
static bool is_parse_json(const expr* e)
{
  // An inlined function body is traced for the call stack of its errors.
  while (e->get_expr_kind() == function_trace_expr_kind ||
         e->get_expr_kind() == wrapper_expr_kind)
  {
    e = e->get_expr_kind() == function_trace_expr_kind ?
      static_cast<const function_trace_expr*>(e)->get_input() :
      static_cast<const wrapper_expr*>(e)->get_input();
  }
  if (e->get_expr_kind() != fo_expr_kind)
    return false;
  FunctionConsts::FunctionKind const kind =
    static_cast<const fo_expr*>(e)->get_func()->getKind();
  return kind == FunctionConsts::FN_JSONIQ_PARSE_JSON_1 ||
         kind == FunctionConsts::FN_JSONIQ_PARSE_JSON_2;
}


/**
 * Gets the parse that iter is, also as an inlined function body.
 */
static JSONParseIterator* parse_iterator(PlanIterator* iter)
{
  while (FunctionTraceIterator* trace = dynamic_cast<FunctionTraceIterator*>(iter))
    iter = trace->getChildren()[0].getp();
  return dynamic_cast<JSONParseIterator*>(iter);
}


/**
 * Gets the expressions from root down to target.
 *
 * @return false if target is not in root.
 */
static bool path_to(expr* root, const expr* target, std::vector<expr*>* path)
{
  path->push_back(root);
  if (root == target)
    return true;
  for (ExprIterator i(root); !i.done(); i.next())
  {
    if (path_to(**i, target, path))
      return true;
  }
  path->pop_back();
  return false;
}


/**
 * Whether the values of a child of e can reach a use of e that reads only
 * keys of them: a lookup, a count, a FLWOR over them, a choice between them.
 */
static bool passes_values(const expr* e)
{
  switch (e->get_expr_kind())
  {
  case flwor_expr_kind:
  case if_expr_kind:
  case fo_expr_kind:
  case dynamic_function_invocation_expr_kind:
  case function_trace_expr_kind:
    return true;
  default:
    return false;
  }
}


/**
 * Whether casting s to xs:integer succeeds: such a key in a dynamic call
 * selects a member of an array, and no member is built where keys are read.
 */
static bool is_integer_text(const zstring& s)
{
  auto const space = [](char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
  };
  std::string_view t(s.data(), s.size());
  while (!t.empty() && space(t.front()))
    t.remove_prefix(1);
  while (!t.empty() && space(t.back()))
    t.remove_suffix(1);
  if (!t.empty() && (t.front() == '+' || t.front() == '-'))
    t.remove_prefix(1);
  return !t.empty() &&
    std::all_of(t.begin(), t.end(), [](char c) { return c >= '0' && c <= '9'; });
}


/**
 * Gets the key of a lookup of a literal key: $x.k, or $x("k") with a key that
 * does not select a member of an array.
 *
 * @param e The expression.
 * @param key Set to the key.
 * @return Returns the expression the key is looked up in, or null.
 */
static expr* literal_key_lookup(expr* e, zstring* key)
{
  expr* input;
  const expr* name;
  if (e->get_expr_kind() == fo_expr_kind)
  {
    const fo_expr* lookup = static_cast<const fo_expr*>(e);
    FunctionConsts::FunctionKind const kind = lookup->get_func()->getKind();
    if (kind != FunctionConsts::OP_ZORBA_SINGLE_OBJECT_LOOKUP_2 &&
        kind != FunctionConsts::OP_ZORBA_MULTI_OBJECT_LOOKUP_2)
      return nullptr;
    input = lookup->get_arg(0);
    name = lookup->get_arg(1);
  }
  else if (e->get_expr_kind() == dynamic_function_invocation_expr_kind)
  {
    const dynamic_function_invocation_expr* call =
      static_cast<const dynamic_function_invocation_expr*>(e);
    if (call->get_args().size() != 1)
      return nullptr;
    input = call->get_input();
    name = call->get_args()[0];
  }
  else
    return nullptr;

  if (name->get_expr_kind() != const_expr_kind)
    return nullptr;
  const store::Item* item = static_cast<const const_expr*>(name)->get_val();
  if (!item->isAtomic() || item->getTypeCode() != store::XS_STRING)
    return nullptr;
  *key = item->getStringValue();
  if (e->get_expr_kind() == dynamic_function_invocation_expr_kind &&
      is_integer_text(*key))
    return nullptr;
  return input;
}


namespace {

/**
 * Adds to vars the variables of a window clause bound to the items of its
 * domain: the window and the items of its conditions.
 */
void add_window_vars(const window_clause* wc, std::vector<const var_expr*>* vars)
{
  vars->push_back(wc->get_var());
  for (const flwor_wincond* cond : { wc->get_win_start(), wc->get_win_stop() })
  {
    if (!cond)
      continue;
    for (const flwor_wincond_vars* condVars : { &cond->get_in_vars(), &cond->get_out_vars() })
    {
      for (const var_expr* var : { condVars->curr, condVars->prev, condVars->next })
      {
        if (var)
          vars->push_back(var);
      }
    }
  }
}


/**
 * What an expression reads of the values of the variables of a for or window
 * clause over a parse.  The values are also bound to the variables of a group
 * by that collect them, and to those of for and window clauses over them, as
 * in the translation of a predicate: the uses of all of these are checked.
 * The FLWOR of the clause may also give the values to an enclosing use that
 * reads only keys of them, which is checked as well.
 */
class read_analysis
{
public:
  read_analysis(std::vector<const var_expr*> vars, json::projection* read) :
    theVars(std::move(vars)), theRead(read) { }

  /**
   * Adds what e reads of the values.
   *
   * @return false if e uses them other than through lookups of literal keys,
   * counting them, or predicates over them that do so.
   */
  bool reads(expr* e);

private:
  std::vector<const var_expr*> theVars; // bound to values
  std::vector<const expr*> theBindings; // what binds values to theVars
  json::projection* theRead;

  bool is_values(const expr* e) const;
  bool only_values(expr* e);
  bool counted(expr* e);
  bool lookup_path(expr* e, std::vector<zstring>* path);
  void bind(const flwor_expr* flwor);
  bool reads_except(expr* e, const expr* skipped);
};


/**
 * Whether e is a reference to a variable bound to values.
 */
bool read_analysis::is_values(const expr* e) const
{
  return e->get_expr_kind() == wrapper_expr_kind &&
    std::find(theVars.begin(), theVars.end(),
              static_cast<const wrapper_expr*>(e)->get_input()) != theVars.end();
}


/**
 * Whether e gives only values, unchanged, as a predicate over them does.
 * Adds what the rest of e reads of them.
 */
bool read_analysis::only_values(expr* e)
{
  if (is_values(e))
    return true;

  switch (e->get_expr_kind())
  {
  case fo_expr_kind:
  {
    fo_expr* fo = static_cast<fo_expr*>(e);
    switch (fo->get_func()->getKind())
    {
    case FunctionConsts::OP_CONCATENATE_N:
      return fo->num_args() == 0;
    case FunctionConsts::OP_ZORBA_SEQUENCE_POINT_ACCESS_2:
      return only_values(fo->get_arg(0)) && reads(fo->get_arg(1));
    default:
      return false;
    }
  }
  case if_expr_kind:
  {
    const if_expr* ifExpr = static_cast<const if_expr*>(e);
    return reads(ifExpr->get_cond_expr()) &&
      only_values(ifExpr->get_then_expr()) &&
      only_values(ifExpr->get_else_expr());
  }
  case flwor_expr_kind:
  {
    flwor_expr* flwor = static_cast<flwor_expr*>(e);
    bind(flwor);
    return only_values(flwor->get_return_expr()) &&
      reads_except(flwor, flwor->get_return_expr());
  }
  default:
    return false;
  }
}


/**
 * Whether the number of items of e does not depend on what is built of the
 * values.  Adds what e reads of them.
 */
bool read_analysis::counted(expr* e)
{
  if (is_values(e))
    return true;

  if (e->get_expr_kind() == if_expr_kind)
  {
    const if_expr* ifExpr = static_cast<const if_expr*>(e);
    return reads(ifExpr->get_cond_expr()) &&
      counted(ifExpr->get_then_expr()) &&
      counted(ifExpr->get_else_expr());
  }

  if (e->get_expr_kind() == flwor_expr_kind)
  {
    flwor_expr* flwor = static_cast<flwor_expr*>(e);
    bind(flwor);
    return counted(flwor->get_return_expr()) &&
      reads_except(flwor, flwor->get_return_expr());
  }

  return reads(e);
}


/**
 * Gets the keys of e if it is V.k1.k2... with literal keys, V giving only
 * values.
 */
bool read_analysis::lookup_path(expr* e, std::vector<zstring>* path)
{
  zstring key;
  expr* input = literal_key_lookup(e, &key);
  if (!input)
    return false;
  if (!only_values(input) && !lookup_path(input, path))
    return false;
  path->push_back(key);
  return true;
}


/**
 * Adds to theVars the variables of flwor bound to values: those of a for or
 * window clause over values and those of a group by that collect values.
 */
void read_analysis::bind(const flwor_expr* flwor)
{
  for (csize i = 0; i < flwor->num_clauses(); ++i)
  {
    const flwor_clause* clause = flwor->get_clause(i);
    if (clause->get_kind() == flwor_clause::for_clause)
    {
      const for_clause* fc = static_cast<const for_clause*>(clause);
      if (only_values(fc->get_expr()))
      {
        theVars.push_back(fc->get_var());
        theBindings.push_back(fc->get_expr());
      }
    }
    else if (clause->get_kind() == flwor_clause::window_clause)
    {
      const window_clause* wc = static_cast<const window_clause*>(clause);
      if (only_values(wc->get_expr()))
      {
        add_window_vars(wc, &theVars);
        theBindings.push_back(wc->get_expr());
      }
    }
    else if (clause->get_kind() == flwor_clause::groupby_clause)
    {
      const groupby_clause* gc = static_cast<const groupby_clause*>(clause);
      for (const auto& [input, var] : gc->get_nongrouping_vars())
      {
        if (is_values(input))
        {
          theVars.push_back(var);
          theBindings.push_back(input);
        }
      }
    }
  }
}


bool read_analysis::reads_except(expr* e, const expr* skipped)
{
  for (ExprIterator i(e); !i.done(); i.next())
  {
    if (**i != skipped && !reads(**i))
      return false;
  }
  return true;
}


bool read_analysis::reads(expr* e)
{
  if (std::find(theBindings.begin(), theBindings.end(), e) != theBindings.end())
    return true;

  if (is_values(e) ||
      std::find(theVars.begin(), theVars.end(), e) != theVars.end())
    return false;

  std::vector<zstring> path;
  if (lookup_path(e, &path))
  {
    theRead->add(path);
    return true;
  }

  switch (e->get_function_kind())
  {
  case FunctionConsts::FN_COUNT_1:
  case FunctionConsts::FN_EMPTY_1:
  case FunctionConsts::FN_EXISTS_1:
    return counted(static_cast<fo_expr*>(e)->get_arg(0));
  default:
    break;
  }

  if (e->get_expr_kind() == flwor_expr_kind)
    bind(static_cast<const flwor_expr*>(e));

  return reads_except(e, nullptr);
}

} // namespace


void project_parse_json(
    CompilerCB* ccb,
    expr* root,
    const forletwin_clause* clause,
    const PlanIter_t& domainIter)
{
  if (ccb->theConfig.opt_level == CompilerCB::config::O0 ||
      !is_parse_json(clause->get_expr()))
    return;

  JSONParseIterator* parse = parse_iterator(domainIter.getp());
  if (!parse)
    return;

  std::vector<const var_expr*> vars;
  if (clause->get_kind() == flwor_clause::for_clause)
    vars.push_back(clause->get_var());
  else if (clause->get_kind() == flwor_clause::window_clause)
    add_window_vars(static_cast<const window_clause*>(clause), &vars);
  else
    return;

  // The FLWOR, or the nearest enclosing expression that reads only keys of
  // the values the FLWOR gives; a lookup in that one may read fewer of them.
  flwor_expr* flwor = clause->get_flwor_expr();
  std::vector<expr*> path;
  if (!root || !path_to(root, flwor, &path))
    path.assign(1, flwor);
  std::optional<json::projection> found;
  for (auto e = path.rbegin(); e != path.rend(); ++e)
  {
    json::projection read;
    if (read_analysis(vars, &read).reads(*e))
      found = std::move(read);
    auto const parent = std::next(e);
    zstring key;
    if (parent == path.rend() ||
        (found ? literal_key_lookup(*parent, &key) != *e : !passes_values(*parent)))
      break;
  }
  if (found && !found->all)
    parse->setProjection(found->str());
}


void project_counted_parse_json(
    CompilerCB* ccb,
    const expr* arg,
    const PlanIter_t& argIter)
{
  if (ccb->theConfig.opt_level == CompilerCB::config::O0 || !is_parse_json(arg))
    return;

  if (JSONParseIterator* parse = parse_iterator(argIter.getp()))
    parse->setProjection(json::projection().str());
}


/**
 * Makes a parse whose values lookup only looks up a literal key in build only
 * that key's value.  Not at -O0.
 */
static void project_looked_up_parse_json(
    CompilerCB* ccb,
    expr* lookup,
    const PlanIter_t& inputIter)
{
  zstring key;
  const expr* input = literal_key_lookup(lookup, &key);
  if (ccb->theConfig.opt_level == CompilerCB::config::O0 || !input ||
      !is_parse_json(input))
    return;

  if (JSONParseIterator* parse = parse_iterator(inputIter.getp()))
  {
    json::projection read;
    read.add({ key });
    parse->setProjection(read.str());
  }
}


/*******************************************************************************

********************************************************************************/
PlanIter_t fn_jsoniq_keys::codegen(
    CompilerCB*,
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& argv,
    expr& arg) const
{
  if (arg.get_return_type()->max_card() <= 1)
    return new SingleObjectKeysIterator(sctx, loc, argv[0]);

  return new MultiObjectKeysIterator(sctx, loc, argv[0]);
}


/*******************************************************************************

********************************************************************************/
xqtref_t op_zorba_multi_object_lookup::getReturnType(const fo_expr* caller) const
{
  if (caller->get_arg(0)->get_return_type()->max_card() == 0)
    return GENV_TYPESYSTEM.EMPTY_TYPE;

  return theSignature.returnType();
}


PlanIter_t op_zorba_multi_object_lookup::codegen(
  CompilerCB* ccb,
  static_context* sctx,
  const QueryLoc& loc,
  std::vector<PlanIter_t>& argv,
  expr& arg) const
{
  project_looked_up_parse_json(ccb, &arg, argv[0]);

  if (arg.get_return_type()->max_card() <= 1)
    return new SingleArrayLookupIterator(sctx, loc, argv[0], argv[1]);

  return new MultiObjectLookupIterator(sctx, loc, argv[0], argv[1]);
}


/*******************************************************************************

********************************************************************************/
xqtref_t op_zorba_single_object_lookup::getReturnType(const fo_expr* caller) const
{
  if (caller->get_arg(0)->get_return_type()->max_card() == 0)
    return GENV_TYPESYSTEM.EMPTY_TYPE;

  return theSignature.returnType();
}


/*******************************************************************************

********************************************************************************/
xqtref_t fn_jsoniq_project::getReturnType(const fo_expr* caller) const
{
  return caller->get_arg(0)->get_return_type();
}


/*******************************************************************************

********************************************************************************/
xqtref_t fn_jsoniq_trim::getReturnType(const fo_expr* caller) const
{
  return caller->get_arg(0)->get_return_type();
}


/*******************************************************************************

********************************************************************************/
static const expr* skip_type_wrappers(const expr* e)
{
  while (true)
  {
    if (e->get_expr_kind() == promote_expr_kind ||
        e->get_expr_kind() == treat_expr_kind)
      e = static_cast<const cast_base_expr*>(e)->get_input();
    else if (e->get_expr_kind() == function_trace_expr_kind)
      e = static_cast<const function_trace_expr*>(e)->get_input();
    else
      return e;
  }
}


static bool is_read_text_utf8(const expr* e)
{
  e = skip_type_wrappers(e);

  if (e->get_expr_kind() != fo_expr_kind)
    return false;

  const fo_expr* call = static_cast<const fo_expr*>(e);
  const store::Item* name = call->get_func()->getName();

  if (name->getNamespace() != "http://expath.org/ns/file" ||
      name->getLocalName() != "read-text")
    return false;

  if (call->num_args() == 1)
    return true;

  const expr* encoding = call->get_arg(1);
  return encoding->get_expr_kind() == const_expr_kind &&
         static_cast<const const_expr*>(encoding)->get_val()->getStringValue() ==
           "UTF-8";
}


// What the translator wraps around a literal in an object constructor, until the
// optimizer removes it. Nothing here can raise on a literal; a cast or a treat
// that a query writes around a value can, so it does not count as a literal.
static const expr* skip_object_value_wrappers(const expr* e)
{
  while (e->get_expr_kind() == fo_expr_kind &&
         static_cast<const fo_expr*>(e)->get_func()->getKind() ==
           FunctionConsts::OP_ZORBA_JSON_BOX_1)
    e = static_cast<const fo_expr*>(e)->get_arg(0);
  return e;
}


static const expr* skip_object_name_wrappers(const expr* e)
{
  if (e->get_expr_kind() == cast_expr_kind)
    e = static_cast<const cast_base_expr*>(e)->get_input();

  if (e->get_expr_kind() == fo_expr_kind &&
      static_cast<const fo_expr*>(e)->get_func()->getKind() ==
        FunctionConsts::FN_DATA_1)
    e = static_cast<const fo_expr*>(e)->get_arg(0);

  return e;
}


// The optimizer moves an expr that does not depend on a FLWOR loop out of it:
// $$temp := op:hoist(e) before the loop, and op:unhoist($$temp) in its place.
// The options object written in the call is then the hoisted expr.
static const expr* skip_hoisting(const expr* e)
{
  if (e->get_expr_kind() != fo_expr_kind ||
      static_cast<const fo_expr*>(e)->get_func()->getKind() !=
        FunctionConsts::OP_UNHOIST_1)
    return e;

  const expr* arg = static_cast<const fo_expr*>(e)->get_arg(0);

  if (arg->get_expr_kind() == wrapper_expr_kind)
    arg = static_cast<const wrapper_expr*>(arg)->get_input();

  if (arg->get_expr_kind() != var_expr_kind)
    return e;

  const var_expr* var = static_cast<const var_expr*>(arg);

  if (var->get_kind() != var_expr::let_var && var->get_kind() != var_expr::for_var)
    return e;

  const expr* domain = var->get_domain_expr();

  if (domain == NULL ||
      domain->get_expr_kind() != fo_expr_kind ||
      static_cast<const fo_expr*>(domain)->get_func()->getKind() !=
        FunctionConsts::OP_HOIST_1)
    return e;

  return static_cast<const fo_expr*>(domain)->get_arg(0);
}


namespace {

/**
 * The parse-json options of an object written as a literal.
 */
class LiteralParseOptions
{
  std::set<zstring> theSeen;
  bool theStreaming = false;
  bool theStrip = false;
  bool theAllowMultiple = true;

  enum Value { NOT_BOOLEAN, IS_FALSE, IS_TRUE };

  // false if the object's value is not known here, or evaluating it raises.
  bool add(const zstring& name, Value value)
  {
    if (!theSeen.insert(name).second)
      return false;

    bool* option = nullptr;
    if (name == "zorba-streaming-members")
      option = &theStreaming;
    else if (name == "jsoniq-strip-top-level-array")
      option = &theStrip;
    else if (name == "jsoniq-multiple-top-level-items")
      option = &theAllowMultiple;
    else
      return true;

    if (value == NOT_BOOLEAN)
      return false;

    *option = (value == IS_TRUE);
    return true;
  }

  static Value value_of(const store::Item* item)
  {
    if (!item->isAtomic() || item->getTypeCode() != store::XS_BOOLEAN)
      return NOT_BOOLEAN;
    return item->getBooleanValue() ? IS_TRUE : IS_FALSE;
  }

public:
  bool read(const expr* options)
  {
    options = skip_type_wrappers(skip_hoisting(skip_type_wrappers(options)));

    if (options->get_expr_kind() == const_expr_kind)
    {
      store::Item_t object = static_cast<const const_expr*>(options)->get_val();
      if (!object->isObject())
        return false;

      store::Item_t key;
      store::Iterator_t keys = object->getObjectKeys();
      keys->open();
      while (keys->next(key))
      {
        store::Item_t value = object->getObjectValue(key);
        if (!add(key->getStringValue(), value_of(value.getp())))
        {
          keys->close();
          return false;
        }
      }
      keys->close();
      return true;
    }

    if (options->get_expr_kind() != json_direct_object_expr_kind)
      return false;

    const json_direct_object_expr* object =
      static_cast<const json_direct_object_expr*>(options);

    for (csize i = 0; i < object->num_pairs(); ++i)
    {
      const expr* name = skip_object_name_wrappers(object->get_name_expr(i));
      const expr* value = skip_object_value_wrappers(object->get_value_expr(i));

      if (name->get_expr_kind() != const_expr_kind)
        return false;

      Value v;
      if (value->get_expr_kind() == const_expr_kind)
      {
        v = value_of(static_cast<const const_expr*>(value)->get_val());
      }
      else if (value->get_expr_kind() == fo_expr_kind &&
               static_cast<const fo_expr*>(value)->get_func()->getKind() ==
                 FunctionConsts::FN_TRUE_0)
      {
        v = IS_TRUE;
      }
      else if (value->get_expr_kind() == fo_expr_kind &&
               static_cast<const fo_expr*>(value)->get_func()->getKind() ==
                 FunctionConsts::FN_FALSE_0)
      {
        v = IS_FALSE;
      }
      else
      {
        return false;
      }

      if (!add(static_cast<const const_expr*>(name)->get_val()->getStringValue(), v))
        return false;
    }
    return true;
  }

  bool streaming() const { return theStreaming && !theStrip; }

  bool allowMultiple() const { return theAllowMultiple; }
};

} // namespace


PlanIter_t fn_jsoniq_members::codegen(
  CompilerCB* ccb,
  static_context* sctx,
  const QueryLoc& loc,
  std::vector<PlanIter_t>& argv,
  expr& arg) const
{
  // Unboxing a parse directly, with the option zorba-streaming-members written
  // as a literal: no one else can see the top-level arrays, so they need not be
  // built. See JSONParseMembersIterator.
  const expr* input = static_cast<const fo_expr&>(arg).get_arg(0);

  if (input->get_expr_kind() == fo_expr_kind &&
      static_cast<const fo_expr*>(input)->get_func()->getKind() ==
        FunctionConsts::FN_JSONIQ_PARSE_JSON_2)
  {
    const fo_expr* parseCall = static_cast<const fo_expr*>(input);
    JSONParseIterator* parse = dynamic_cast<JSONParseIterator*>(argv[0].getp());
    LiteralParseOptions options;

    if (parse && options.read(parseCall->get_arg(1)) && options.streaming())
    {
      // The options stay a child, not read: when the optimizer has hoisted
      // them, their iterator is a reference to a FLWOR variable, which must
      // stay in the plan as long as the variable does.
      std::vector<PlanIter_t> source(parse->getChildren());

      QueryLoc relativeLoc = QueryLoc::null;
      if (SingletonIterator* literal =
            dynamic_cast<SingletonIterator*>(source[0].getp()))
      {
        if (literal->getValue()->isAtomic())
          relativeLoc = literal->getLocation();
      }

      // Every call of file:read-text makes its own stream. Anything that can
      // write during the evaluation could change the file between the two
      // reads of it: a sequential expression, an update, a function item of a
      // sequential or updating function, or an eval. A dynamic call, an invoke
      // and a call of a function that takes a function as argument count when
      // the query can have calls with effects (CALLS_HAVE_EFFECTS); a query
      // that reads a file always can, since file:read-text is an external
      // function, so here they always count, even for a function item that
      // cannot write. The query and the functions it calls are checked.
      bool const streamIsPrivate =
        !ccb->isSequential() &&
        !ccb->theMayRunSideEffectsOrUpdates &&
        is_read_text_utf8(parseCall->get_arg(0));

      return new JSONParseMembersIterator(
        sctx, parse->getLocation(), source, relativeLoc, streamIsPrivate,
        options.allowMultiple());
    }
  }

  if (arg.get_return_type()->max_card() <= 1)
    return new SingleArrayMembersIterator(sctx, loc, argv[0]);

  return new MultiArrayMembersIterator(sctx, loc, argv[0]);
}


/*******************************************************************************

********************************************************************************/
xqtref_t op_zorba_multi_array_lookup::getReturnType(const fo_expr* caller) const
{
  if (caller->get_arg(0)->get_return_type()->max_card() == 0)
    return GENV_TYPESYSTEM.EMPTY_TYPE;

  return theSignature.returnType();
}


PlanIter_t op_zorba_multi_array_lookup::codegen(
  CompilerCB*,
  static_context* sctx,
  const QueryLoc& loc,
  std::vector<PlanIter_t>& argv,
  expr& arg) const
{
  if (arg.get_return_type()->max_card() <= 1)
    return new SingleArrayLookupIterator(sctx, loc, argv[0], argv[1]);

  return new MultiArrayLookupIterator(sctx, loc, argv[0], argv[1]);
}

/*******************************************************************************

********************************************************************************/
xqtref_t op_zorba_single_array_lookup::getReturnType(const fo_expr* caller) const
{
  if (caller->get_arg(0)->get_return_type()->max_card() == 0)
    return GENV_TYPESYSTEM.EMPTY_TYPE;

  return theSignature.returnType();
}


/*******************************************************************************

********************************************************************************/
xqtref_t fn_jsoniq_size::getReturnType(const fo_expr* caller) const
{
  if (caller->get_arg(0)->get_return_type()->get_quantifier() == SequenceType::QUANT_ONE)
    return GENV_TYPESYSTEM.INTEGER_TYPE_ONE;

  return theSignature.returnType();
}


/*******************************************************************************

********************************************************************************/
bool op_zorba_json_array_insert::mustCopyInputNodes(expr* fo, csize producer) const
{
  static_context* sctx = fo->get_sctx();

  if (producer == 2 && sctx->preserve_ns() && sctx->inherit_ns())
  {
    return true;
  }

  return false;
}


/*******************************************************************************

********************************************************************************/
bool op_zorba_json_object_insert::mustCopyInputNodes(expr* fo, csize producer) const
{
  if (producer == 2 && fo->get_sctx()->preserve_ns() && fo->get_sctx()->inherit_ns())
  {
    return true;
  }

  return false;
}


/*******************************************************************************

********************************************************************************/
bool op_zorba_json_replace_value::mustCopyInputNodes(expr* fo, csize producer) const
{
  static_context* sctx = fo->get_sctx();

  if (producer == 2 && sctx->preserve_ns() && sctx->inherit_ns())
  {
    return true;
  }

  return false;
}


/*******************************************************************************

********************************************************************************/
PlanIter_t op_zorba_json_replace_value::codegen(
  CompilerCB*,
  static_context* sctx,
  const QueryLoc& loc,
  std::vector<PlanIter_t>& argv,
  expr& ann) const
{
  return new JSONReplaceValueIterator(sctx, loc, argv, true);
}


/*******************************************************************************

********************************************************************************/
bool op_zorba_json_array_append::mustCopyInputNodes(expr* fo, csize producer) const
{
  static_context* sctx = fo->get_sctx();

  if (producer == 2 && sctx->preserve_ns() && sctx->inherit_ns())
  {
    return true;
  }

  return false;
}


/*******************************************************************************

********************************************************************************/
xqtref_t op_zorba_json_box::getReturnType(const fo_expr* fo) const
{
  xqtref_t type = fo->get_arg(0)->get_return_type();

  if (type->get_quantifier() == SequenceType::QUANT_ONE)
    return type;

  if (type->max_card() == 0)
    return GENV_TYPESYSTEM.JS_NULL_TYPE_ONE;

  return theSignature.returnType();
}


/*******************************************************************************

********************************************************************************/
PlanIter_t jn_object::codegen(
  CompilerCB*,
  static_context* sctx,
  const QueryLoc& loc,
  std::vector<PlanIter_t>& argv,
  expr& ann) const
{
  return new JSONObjectIterator(sctx, loc, argv, true, false);
}


void populate_context_jsoniq_functions_impl(static_context* sctx)
{
  DECL(sctx, jn_object,
       (createQName("http://jsoniq.org/functions", "", "object"),
        GENV_TYPESYSTEM.JSON_OBJECT_TYPE_STAR,
        GENV_TYPESYSTEM.JSON_OBJECT_TYPE_ONE));
}


} /* namespace zorba */

/* vim:set et sw=2 ts=2: */
