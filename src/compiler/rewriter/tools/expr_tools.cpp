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

#include "compiler/rewriter/tools/expr_tools.h"
#include "compiler/expression/flwor_expr.h"
#include "compiler/expression/expr.h"
#include "compiler/expression/path_expr.h"
#include "compiler/expression/script_exprs.h"
#include "compiler/expression/ft_expr.h"
#include "compiler/expression/ftnode.h"
#include "compiler/expression/function_item_expr.h"
#include "compiler/expression/expr_iter.h"
#include "compiler/api/compilercb.h"
#include "context/static_context.h"

#include "functions/func_errors_and_diagnostics.h"
#include "functions/udf.h"
#include "runtime/hof/function_item.h"
#include "system/globalenv.h"
#include "types/typeimpl.h"

#include "types/typeops.h"

#include <iterator>

namespace zorba
{

namespace expr_tools
{

static void add_wincond_vars(const flwor_wincond*, csize&, VarIdMap&, IdVarMap*);

static void add_var(var_expr*, csize&, VarIdMap&, IdVarMap*);

static void remove_wincond_vars(const flwor_wincond*, const VarIdMap&, DynamicBitset&);

static void set_bit(var_expr*, const VarIdMap&, DynamicBitset&, bool);


/*******************************************************************************

********************************************************************************/
static void normalize_comp(
    CompareConsts::CompareType& comp,
    expr*& arg0,
    expr*& arg1)
{
  switch (comp)
  {
  case CompareConsts::VALUE_GREATER:
  case CompareConsts::GENERAL_GREATER:
  case CompareConsts::VALUE_GREATER_EQUAL:
  case CompareConsts::GENERAL_GREATER_EQUAL:
  {
    comp = static_cast<CompareConsts::CompareType>(static_cast<int>(comp) - 4);
     
    expr* tmp = arg0;
    arg0 = arg1;
    arg1 = tmp;
     
    break;
  }
  default:
    break;
  }
}


/*******************************************************************************

********************************************************************************/
#define MATCH_WINCOND_VAR(qv, vv)                     \
  if (qv != NULL && vv != NULL)                       \
  {                                                   \
    subst[vv] = qv;                                   \
  }                                                   \
  else if (qv != NULL || vv != NULL)                  \
  {                                                   \
    return false;                                     \
  }


/*******************************************************************************

********************************************************************************/
bool match_exact(expr* query, expr* view, expr::substitution_t& subst)
{
  if (query == view)
    return true;

  if (query == NULL || view == NULL)
    return false;

  if (query->get_expr_kind() != view->get_expr_kind())
  {
    if (query->get_expr_kind() == var_expr_kind)
    {
      var_expr* qe = static_cast<var_expr*>(query);

      if (qe->get_kind() != var_expr::let_var)
        return false;

      return match_exact(qe->get_domain_expr(), view, subst);
    }
    else if (view->get_expr_kind() == var_expr_kind)
    {
      var_expr* ve = static_cast<var_expr*>(view);

      switch (ve->get_kind())
      {
      case var_expr::for_var:
      {
        expr::substitution_t::iterator ite = subst.find(view);

        if (ite != subst.end())
        {
          return match_exact(query, ite->second, subst);
        }
        else
        {
          assert(false);
          subst[view] = query;
          return true;
        }
      }
      case var_expr::let_var:
      {
        return match_exact(query, ve->get_domain_expr(), subst);
      }
      case var_expr::pos_var:
      {
        return false;
      }
      case var_expr::win_var:
      case var_expr::wincond_out_var:
      case var_expr::wincond_out_pos_var:
      case var_expr::wincond_in_var:
      case var_expr::wincond_in_pos_var:
      case var_expr::groupby_var:
      case var_expr::non_groupby_var:
      case var_expr::count_var:
      {
        ZORBA_ASSERT(false); // TODO
      }
      case var_expr::score_var:
      case var_expr::prolog_var:
      case var_expr::local_var:
      case var_expr::copy_var:
      case var_expr::catch_var:
      case var_expr::arg_var:
      case var_expr::eval_var:
      {
        ZORBA_ASSERT(false);
      }
      default:
      {
        ZORBA_ASSERT(false);
      }
      }
    }
    else if (view->get_function_kind() == FunctionConsts::OP_UNHOIST_1)
    {
      fo_expr* vfo = static_cast<fo_expr*>(view);
      return match_exact(query, vfo->get_arg(0), subst);
    }
    else if (query->get_function_kind() == FunctionConsts::OP_UNHOIST_1)
    {
      fo_expr* qfo = static_cast<fo_expr*>(query);
      return match_exact(qfo->get_arg(0), view, subst);
    }
    else if (view->get_function_kind() == FunctionConsts::OP_HOIST_1)
    {
      fo_expr* vfo = static_cast<fo_expr*>(view);
      return match_exact(query, vfo->get_arg(0), subst);
    }
    else if (query->get_function_kind() == FunctionConsts::OP_HOIST_1)
    {
      fo_expr* qfo = static_cast<fo_expr*>(query);
      return match_exact(qfo->get_arg(0), view, subst);
    }
    else if (view->get_expr_kind() == wrapper_expr_kind)
    {
      wrapper_expr* vwe = static_cast<wrapper_expr*>(view);
      return match_exact(query, vwe->get_input(), subst);
    }
    else if (query->get_expr_kind() == wrapper_expr_kind)
    {
      wrapper_expr* qwe = static_cast<wrapper_expr*>(query);
      return match_exact(qwe->get_input(), view, subst);
    }
    else
    {
      return false;
    }
  }

  switch (query->get_expr_kind())
  {
  case var_expr_kind:
  {
    var_expr* qe = static_cast<var_expr*>(query);
    var_expr* ve = static_cast<var_expr*>(view);

    var_expr::var_kind qkind = qe->get_kind();
    var_expr::var_kind vkind = ve->get_kind();

    if (qkind != vkind)
    {
      if (qkind == var_expr::let_var)
      {
        return match_exact(qe->get_domain_expr(), view, subst);
      }

      return false;
    }

    switch (qkind)
    {
    case var_expr::for_var:
    case var_expr::pos_var:
    {
      expr::substitution_t::iterator ite = subst.find(view);

      if (ite != subst.end())
      {
        return (qe == ite->second);
      }
      else
      {
        assert(false);
        subst[view] = query;
        return true;
      }
    }
    case var_expr::let_var:
    {
      return match_exact(qe->get_domain_expr(), ve->get_domain_expr(), subst);
    }
    case var_expr::win_var:
    case var_expr::wincond_out_var:
    case var_expr::wincond_out_pos_var:
    case var_expr::wincond_in_var:
    case var_expr::wincond_in_pos_var:
    case var_expr::groupby_var:
    case var_expr::non_groupby_var:
    case var_expr::count_var:
    {
      ZORBA_ASSERT(false); // TODO
    }
    case var_expr::score_var:
    case var_expr::prolog_var:
    case var_expr::local_var:
    case var_expr::copy_var:
    case var_expr::catch_var:
    case var_expr::arg_var:
    case var_expr::eval_var:
    {
      ZORBA_ASSERT(false);
    }
    default:
    {
      ZORBA_ASSERT(false);
    }
    }

    break;
  }

  case fo_expr_kind:
  {
    fo_expr* qe = static_cast<fo_expr*>(query);
    fo_expr* ve = static_cast<fo_expr*>(view);

    if (qe->get_func() != ve->get_func())
    {
      function* vfunc = ve->get_func();
      function* qfunc = qe->get_func();

      if (qfunc->isComparisonFunction() && vfunc->isComparisonFunction())
      {
        CompareConsts::CompareType qcomp = qe->get_func()->comparisonKind();
        CompareConsts::CompareType vcomp = ve->get_func()->comparisonKind();

        expr* qarg0 = qe->get_arg(0);
        expr* qarg1 = qe->get_arg(1);
        expr* varg0 = ve->get_arg(0);
        expr* varg1 = ve->get_arg(1);

        normalize_comp(qcomp, qarg0, qarg1);
        normalize_comp(vcomp, varg0, varg1);

        if (qcomp == vcomp &&
            match_exact(qarg0, varg0, subst) &&
            match_exact(qarg1, varg1, subst))
          return true;
      }
      else if (vfunc->getKind() == FunctionConsts::OP_UNHOIST_1)
      {
        return match_exact(query, ve->get_arg(0), subst);
      }
      else if (qfunc->getKind() == FunctionConsts::OP_UNHOIST_1)
      {
        return match_exact(qe->get_arg(0), view, subst);
      }
      else if (vfunc->getKind() == FunctionConsts::OP_HOIST_1)
      {
        return match_exact(query, ve->get_arg(0), subst);
      }
      else if (qfunc->getKind() == FunctionConsts::OP_HOIST_1)
      {
        return match_exact(qe->get_arg(0), view, subst);
      }

      return false;
    }

    function* func = qe->get_func();

    csize numArgs = qe->num_args();

    if (numArgs != ve->num_args())
      return false;

    csize i = 0;
    for (; i < numArgs; i++)
    {
      if (!match_exact(qe->get_arg(i), ve->get_arg(i), subst))
        break;
    }

    if (i < numArgs)
    {
      if (func->getKind() == FunctionConsts::ZORBA_STORE_STATIC_COLLECTIONS_DML_COLLECTION_1)
      {
        const store::Item* collName1 = ve->get_arg(0)->getQName();
        const store::Item* collName2 = qe->get_arg(0)->getQName();

        if (collName1 != NULL && collName2 != NULL && collName1->equals(collName2))
          return true;
#if 0
        if (collName != NULL && qe->get_arg(0)->get_var() != NULL)
        {
          const var_expr* var = qe->get_arg(0)->get_var();

          if (var->get_kind() == var_expr::prolog_var &&
              var->num_set_exprs() == 1 &&
              var->get_set_expr(0)->get_expr_kind() == var_decl_expr_kind)
          {
            const var_decl_expr* decl = 
            static_cast<const var_decl_expr*>(var->get_set_expr(0));

            const store::Item* collName2 = decl->get_init_expr()->getQName();

            if (collName2 != NULL && collName->equals(collName2))
              return true;
          }
        }
#endif
      }
      else if (func->isComparisonFunction())
      {
        CompareConsts::CompareType compKind = func->comparisonKind();

        if (CompareConsts::VALUE_EQUAL <= compKind && 
            compKind <= CompareConsts::NODE_NOT_EQUAL)
        {
          if (match_exact(qe->get_arg(0), ve->get_arg(1), subst) &&
              match_exact(qe->get_arg(1), ve->get_arg(0), subst))
            return true;
        }
      }

      return false;
    }

    return true;
  }

  case dynamic_function_invocation_expr_kind:
  {
    const dynamic_function_invocation_expr* qe =
    static_cast<const dynamic_function_invocation_expr*>(query);

    const dynamic_function_invocation_expr* ve =
    static_cast<const dynamic_function_invocation_expr*>(view);

    if (!match_exact(qe->get_input(), ve->get_input(), subst))
      return false;

    if (qe->get_args().size() != ve->get_args().size())
      return false;

    std::vector<expr*>::const_iterator qite = qe->get_args().begin();
    std::vector<expr*>::const_iterator qend = qe->get_args().end();
    std::vector<expr*>::const_iterator vite = ve->get_args().begin();

    for (; qite != qend; ++qite, ++vite)
    { 
      if (!match_exact(*qite, *vite, subst))
        return false;
    }

    return true;
  }

  case relpath_expr_kind:
  {
    relpath_expr* qe = static_cast<relpath_expr*>(query);
    relpath_expr* ve = static_cast<relpath_expr*>(view);

    csize vnumSteps = ve->size();
    csize qnumSteps = qe->size();
    csize numSteps = (vnumSteps < qnumSteps ? vnumSteps : qnumSteps);

    for (csize i = numSteps-1; i > 0; --i)
    {
      axis_step_expr* qstep = static_cast<axis_step_expr*>((*qe)[i]);
      axis_step_expr* vstep = static_cast<axis_step_expr*>((*ve)[i]);

      if (qstep->getAxis() != vstep->getAxis())
        return false;

      match_expr* qtest = qstep->getTest();
      match_expr* vtest = vstep->getTest();

      if (!qtest->matches(vtest))
        return false;
    }

    // <vsource>/b/c  vs  <qsource>/a/b/c
    if (vnumSteps < qnumSteps)
    {
      expr* vsource = (*ve)[0];

      if (vsource->get_expr_kind() != var_expr_kind)
        return false;

      relpath_expr* qpath = query->get_ccb()->getExprManager()->
      create_relpath_expr(qe->get_sctx(), qe->get_udf(), qe->get_loc());

      for (csize i = 0; i < qnumSteps - vnumSteps; ++i)
        qpath->add_back((*qe)[i]);

      return match_exact(qpath, vsource, subst);
    }
    // <vsource>/a/b/c  vs  <qsource>/b/c
    else if (qnumSteps < vnumSteps)
    {
      expr* qsource = (*qe)[0];

      if (qsource->get_expr_kind() != var_expr_kind)
        return false;

      relpath_expr* vpath = query->get_ccb()->getExprManager()->
      create_relpath_expr(ve->get_sctx(), ve->get_udf(), ve->get_loc());

      for (csize i = 0; i < vnumSteps - qnumSteps; ++i)
        vpath->add_back((*ve)[i]);

      return match_exact(qsource, vpath, subst);
    }
    else
    {
      return match_exact((*qe)[0], (*ve)[0], subst);
    }
  }

  case flwor_expr_kind:
  {
    flwor_expr* qe = static_cast<flwor_expr*>(query);
    flwor_expr* ve = static_cast<flwor_expr*>(view);

    csize numClauses = qe->num_clauses();

    if (numClauses != ve->num_clauses())
      return false;

    for (csize i = 0; i < numClauses; ++i)
    {
      flwor_clause* qc = qe->get_clause(i);
      flwor_clause* vc = ve->get_clause(i);

      if (qc->get_kind() != vc->get_kind())
        return false;

      switch (qc->get_kind())
      {
      case flwor_clause::for_clause:
      case flwor_clause::let_clause:
      {
        forlet_clause* qflc = static_cast<forlet_clause*>(qc);
        forlet_clause* vflc = static_cast<forlet_clause*>(vc);

        if (!match_exact(qflc->get_expr(), vflc->get_expr(), subst))
          return false;

        subst[vflc->get_var()] = qflc->get_var();

        break;
      }
      case flwor_clause::where_clause:
      {
        where_clause* qwc = static_cast<where_clause*>(qc);
        where_clause* vwc = static_cast<where_clause*>(vc);

        if (!match_exact(qwc->get_expr(), vwc->get_expr(), subst))
          return false;

        break;
      }
      case flwor_clause::orderby_clause:
      {
        orderby_clause* qoc = static_cast<orderby_clause*>(qc);
        orderby_clause* voc = static_cast<orderby_clause*>(vc);

        csize numColumns = qoc->num_columns();

        if (numColumns != voc->num_columns())
          return false;

        for (csize i = 0; i < numColumns; ++i)
        {
          if (match_exact(qoc->get_column_expr(i), voc->get_column_expr(i), subst))
            return false;
        }

        break;
      }
      case flwor_clause::groupby_clause:
      {
        groupby_clause* qgc = static_cast<groupby_clause*>(qc);
        groupby_clause* vgc = static_cast<groupby_clause*>(vc);

        csize numGVars = qgc->numGroupingVars();
        csize numNGVars = qgc->numNonGroupingVars();

        if (numGVars != vgc->numGroupingVars() ||
            numNGVars != vgc->numNonGroupingVars())
          return false;

        var_rebind_list_t::const_iterator qite = qgc->beginGroupVars();
        var_rebind_list_t::const_iterator qend = qgc->endGroupVars();
        var_rebind_list_t::const_iterator vite = vgc->beginGroupVars();

        for (; qite != qend; ++qite, ++vite)
        {
          if (!match_exact((*qite).first, (*vite).first, subst))
            return false;

          subst[(*vite).second] = (*qite).second;
        }
 
        qite = qgc->beginNonGroupVars();
        qend = qgc->endNonGroupVars();
        vite = vgc->beginNonGroupVars();

        for (; qite != qend; ++qite, ++vite)
        {
          if (!match_exact((*qite).first, (*vite).first, subst))
            return false;

          subst[(*vite).second] = (*qite).second;
        }

        break;
      }
      case flwor_clause::window_clause:
      {
        window_clause* qwc = static_cast<window_clause*>(qc);
        window_clause* vwc = static_cast<window_clause*>(vc);

        if (!match_exact(qwc->get_expr(), vwc->get_expr(), subst))
          return false;

        subst[vwc->get_var()] = qwc->get_var();

        flwor_wincond* qcond = qwc->get_win_start();
        flwor_wincond* vcond = vwc->get_win_start();

        if (qcond != NULL && vcond != NULL)
        {
          if (qcond->is_only() != vcond->is_only())
            return false;

          MATCH_WINCOND_VAR(qcond->get_in_vars().posvar, vcond->get_in_vars().posvar);
          MATCH_WINCOND_VAR(qcond->get_in_vars().curr, vcond->get_in_vars().curr);
          MATCH_WINCOND_VAR(qcond->get_in_vars().prev, vcond->get_in_vars().prev);
          MATCH_WINCOND_VAR(qcond->get_in_vars().next, vcond->get_in_vars().next);

          if (!match_exact(qcond->get_expr(), vcond->get_expr(), subst))
            return false;

          MATCH_WINCOND_VAR(qcond->get_out_vars().posvar, vcond->get_out_vars().posvar);
          MATCH_WINCOND_VAR(qcond->get_out_vars().curr, vcond->get_out_vars().curr);
          MATCH_WINCOND_VAR(qcond->get_out_vars().prev, vcond->get_out_vars().prev);
          MATCH_WINCOND_VAR(qcond->get_out_vars().next, vcond->get_out_vars().next);
        }
        else if (qcond != NULL || vcond != NULL)
        {
          return false;
        }

        qcond = qwc->get_win_stop();
        vcond = vwc->get_win_stop();

        if (qcond != NULL && vcond != NULL)
        {
          if (qcond->is_only() != vcond->is_only())
            return false;

          MATCH_WINCOND_VAR(qcond->get_in_vars().posvar, vcond->get_in_vars().posvar);
          MATCH_WINCOND_VAR(qcond->get_in_vars().curr, vcond->get_in_vars().curr);
          MATCH_WINCOND_VAR(qcond->get_in_vars().prev, vcond->get_in_vars().prev);
          MATCH_WINCOND_VAR(qcond->get_in_vars().next, vcond->get_in_vars().next);

          if (!match_exact(qcond->get_expr(), vcond->get_expr(), subst))
            return false;

          MATCH_WINCOND_VAR(qcond->get_out_vars().posvar, vcond->get_out_vars().posvar);
          MATCH_WINCOND_VAR(qcond->get_out_vars().curr, vcond->get_out_vars().curr);
          MATCH_WINCOND_VAR(qcond->get_out_vars().prev, vcond->get_out_vars().prev);
          MATCH_WINCOND_VAR(qcond->get_out_vars().next, vcond->get_out_vars().next);
        }
        else if (qcond != NULL || vcond != NULL)
        {
          return false;
        }
        
        break;
      }
      case flwor_clause::count_clause:
      {
        count_clause* qcc = static_cast<count_clause*>(qc);
        count_clause* vcc = static_cast<count_clause*>(vc);

        subst[vcc->get_var()] = qcc->get_var();

        break;
      }
      default:
        ZORBA_ASSERT(false);
      }
    }

    if (!match_exact(qe->get_return_expr(), ve->get_return_expr(), subst))
      return false;

    return true;
  }

  case cast_expr_kind:
  case castable_expr_kind:
  case instanceof_expr_kind:
  {
    cast_or_castable_base_expr* qe = static_cast<cast_or_castable_base_expr*>(query);
    cast_or_castable_base_expr* ve = static_cast<cast_or_castable_base_expr*>(view);

    TypeManager* tm = qe->get_type_manager();

    if (!TypeOps::is_equal(tm, *qe->get_target_type(), *ve->get_target_type()))
      return false;

    return match_exact(qe->get_input(), ve->get_input(), subst);
  }

  case promote_expr_kind:
  {
    promote_expr* qe = static_cast<promote_expr*>(query);
    promote_expr* ve = static_cast<promote_expr*>(view);

    TypeManager* tm = qe->get_type_manager();

    if (!TypeOps::is_subtype(tm, *qe->get_return_type(), *ve->get_target_type()))
      return false;

    return match_exact(qe->get_input(), ve->get_input(), subst);
  }

  case treat_expr_kind:
  {
    treat_expr* qe = static_cast<treat_expr*>(query);
    treat_expr* ve = static_cast<treat_expr*>(view);

    TypeManager* tm = qe->get_type_manager();

    if (qe->get_check_prime() != ve->get_check_prime())
      return false;

    if (!qe->get_check_prime())
    {
      if (qe->get_target_type()->get_quantifier() !=
          ve->get_target_type()->get_quantifier())
        return false;
    }
    else if (!TypeOps::is_equal(tm, *qe->get_target_type(), *ve->get_target_type()))
    {
      return false;
    }

    return match_exact(qe->get_input(), ve->get_input(), subst);
  }

  case wrapper_expr_kind:
  {
    wrapper_expr* qe = static_cast<wrapper_expr*>(query);
    wrapper_expr* ve = static_cast<wrapper_expr*>(view);

    return match_exact(qe->get_input(), ve->get_input(), subst);
  }

  case const_expr_kind:
  {
    const_expr* qe = static_cast<const_expr*>(query);
    const_expr* ve = static_cast<const_expr*>(view);

    try
    {
      // TODO: collation, timezone ???? Implement the full eq spec ????
      return qe->get_val()->equals(ve->get_val());
    }
    catch (ZorbaException&)
    {
      return false;
    }
    break;
  }

  case delete_expr_kind:
  case insert_expr_kind:
  case rename_expr_kind:
  case replace_expr_kind:
  case transform_expr_kind:
  default:
  {
    ZORBA_ASSERT(false);
  }
  }

  return false;
}


/*******************************************************************************

********************************************************************************/
bool count_var_uses_rec(
    expr* e,
    var_expr* var,
    int limit,
    std::vector<expr**>* path,
    int& count)
{
  if (limit > 0 && count >= limit)
  {
    return false;
  }

  if (e == var)
  {
    ++count;
    return true;
  }

  if (e->get_expr_kind() == if_expr_kind)
  {
    ExprIterator iter(e);
    const if_expr* ifExpr = static_cast<const if_expr*>(e);

    if (path && count == 0)
      path->push_back(*iter);

    bool done = !count_var_uses_rec(ifExpr->get_cond_expr(), var, limit, path, count);
    if (done)
    {
      assert(count > 0);
      return false;
    }

    if (path && count == 0)
      path->pop_back();

    iter.next();

    if (path && count == 0)
      path->push_back(*iter);

    int thenCount = count;
    std::vector<expr**>* thenPath = (count == 0 ? path : NULL);
    
    done = !count_var_uses_rec(ifExpr->get_then_expr(), var, limit, thenPath, thenCount);
    if (done)
    {
      count = thenCount;
      assert(count > 0);
      return false;
    }

    if (path && count == 0)
      path->pop_back();

    iter.next();

    if (path && count == 0)
      path->push_back(*iter);

    int elseCount = count;
    std::vector<expr**>* elsePath = (count == 0 ? path : NULL);

    done = !count_var_uses_rec(ifExpr->get_else_expr(), var, limit, elsePath, elseCount);
    if (done)
    {
      count = elseCount;
      assert(count > 0);
      return false;
    }

    if (path && count == 0)
      path->pop_back();

    count = (thenCount > elseCount ? thenCount : elseCount);
  }
  else
  {
    ExprIterator iter(e);
    while (!iter.done())
    {
      if (path && count == 0)
      {
        path->push_back(*iter);
      }

      bool done = ! count_var_uses_rec(**iter, var, limit, path, count);

      if (done)
      {
        assert(count > 0);
        return false;
      }

      if (path && count == 0)
      {
        path->pop_back();
      }

      iter.next();
    }
  }

  return true;
}


/*******************************************************************************

********************************************************************************/
bool function_runs_opaque_code(const function* f)
{
  switch (f->getKind())
  {
  case FunctionConsts::FN_ZORBA_INVOKE_N:
  case FunctionConsts::FN_ZORBA_INVOKE_N_N:
  case FunctionConsts::FN_ZORBA_INVOKE_U_N:
  case FunctionConsts::FN_ZORBA_INVOKE_S_N:
  case FunctionConsts::FN_ZORBA_EVAL_1:
  case FunctionConsts::FN_ZORBA_EVAL_N_1:
  case FunctionConsts::FN_ZORBA_EVAL_U_1:
  case FunctionConsts::FN_ZORBA_EVAL_S_1:
    return true;
  default:
    break;
  }

  const signature& sig = f->getSignature();
  csize const numParams = sig.isVariadic() ? 1 : sig.paramCount();

  for (csize i = 0; i < numParams; ++i)
  {
    XQType::TypeKind const kind = sig[static_cast<int>(i)]->type_kind();

    if (kind == XQType::FUNCTION_TYPE_KIND || kind == XQType::ANY_FUNCTION_TYPE_KIND)
      return true;
  }

  return false;
}


static bool may_run_side_effects_rec(
    const expr* e,
    std::set<const function*>& visitedUdfs,
    bool withUpdates)
{
  if (withUpdates && e->is_updating())
    return true;

  CompilerCB* ccb = const_cast<expr*>(e)->get_ccb();

  // Nothing with a side effect is reachable (classify_calls): a call has none.
  bool const callsHaveEffects = ccb->theCallEffects == CompilerCB::CALLS_HAVE_EFFECTS;

  switch (e->get_expr_kind())
  {
  case dynamic_function_invocation_expr_kind:
    if (callsHaveEffects)
      return true;

    break;

  case eval_expr_kind:
  case var_set_expr_kind:
  case apply_expr_kind:
  case exit_expr_kind:
  case exit_catcher_expr_kind:
  case flowctl_expr_kind:
  case while_expr_kind:
    return true;

  case fo_expr_kind:
  {
    const function* f = static_cast<const fo_expr*>(e)->get_func();

    if (f->isSequential() || (callsHaveEffects && function_runs_opaque_code(f)))
      return true;

    if (f->isUdf() && visitedUdfs.insert(f).second)
    {
      const expr* body = static_cast<const user_function*>(f)->getBody();

      if (body == NULL || may_run_side_effects_rec(body, visitedUdfs, withUpdates))
        return true;
    }

    break;
  }

  case function_item_expr_kind:
  {
    if (!withUpdates)
      break;

    // A function item can be called where the compiler does not see it.
    const function* f = static_cast<const function_item_expr*>(e)->get_function();

    if (f == NULL || f->isSequential() || f->isUpdating())
      return true;

    break;
  }

  default:
    break;
  }

  ExprConstIterator iter(e);
  while (!iter.done())
  {
    const expr* child = iter.get_expr();

    if (child != NULL && may_run_side_effects_rec(child, visitedUdfs, withUpdates))
      return true;

    iter.next();
  }

  return false;
}


bool may_run_side_effects(const expr* root)
{
  std::set<const function*> visitedUdfs;

  return may_run_side_effects_rec(root, visitedUdfs, false);
}


bool may_run_side_effects_or_updates(const expr* root)
{
  std::set<const function*> visitedUdfs;

  return may_run_side_effects_rec(root, visitedUdfs, true);
}


// Whether what this built-in function returns is atomic or comes from its input, so
// that it returns nothing new. Otherwise it may return a new node or JSON item.
//
// fn:doc and fn:collection return the nodes of the store, the same for the same URI in
// a query; they read a state that a side effect can change, which builtinHasImpurity()
// says separately. A function that calls a function item returns what the calls return:
// a call that is not proven is found by its arguments (ImpurityWalk::isUnprovenCall()),
// and the closed world walks the function of every function item.
static bool builtin_returns_existing_items(const function* f, TypeManager* tm)
{
  switch (f->getKind())
  {
  case FunctionConsts::OP_UNION_2:
  case FunctionConsts::OP_INTERSECT_2:
  case FunctionConsts::OP_EXCEPT_2:
  case FunctionConsts::OP_CHECK_DISTINCT_NODES_1:
  case FunctionConsts::OP_DISTINCT_NODES_1:
  case FunctionConsts::OP_DISTINCT_NODES_OR_ATOMICS_1:
  case FunctionConsts::OP_EITHER_NODES_OR_ATOMICS_1:
  case FunctionConsts::FN_ID_1:
  case FunctionConsts::FN_ID_2:
  case FunctionConsts::FN_IDREF_1:
  case FunctionConsts::FN_IDREF_2:
  case FunctionConsts::FN_ELEMENT_WITH_ID_1:
  case FunctionConsts::FN_ELEMENT_WITH_ID_2:
  case FunctionConsts::FN_DOC_1:
  case FunctionConsts::FN_COLLECTION_0:
  case FunctionConsts::FN_COLLECTION_1:
  case FunctionConsts::OP_ZORBA_SINGLE_OBJECT_LOOKUP_2:
  case FunctionConsts::OP_ZORBA_SINGLE_ARRAY_LOOKUP_2:
  case FunctionConsts::FN_JSONIQ_FLATTEN_1:
  case FunctionConsts::FN_FOR_EACH_2:
  case FunctionConsts::FN_FILTER_2:
  case FunctionConsts::FN_FOLD_LEFT_3:
  case FunctionConsts::FN_FOLD_RIGHT_3:
  case FunctionConsts::FN_FOR_EACH_PAIR_3:
  case FunctionConsts::OP_CONCATENATE_N:
  case FunctionConsts::OP_HOIST_1:
  case FunctionConsts::OP_UNHOIST_1:
  case FunctionConsts::FN_HEAD_1:
  case FunctionConsts::FN_TAIL_1:
  case FunctionConsts::FN_REVERSE_1:
  case FunctionConsts::FN_REMOVE_2:
  case FunctionConsts::FN_INSERT_BEFORE_3:
  case FunctionConsts::FN_SUBSEQUENCE_2:
  case FunctionConsts::FN_SUBSEQUENCE_3:
  case FunctionConsts::OP_ZORBA_SUBSEQUENCE_INT_3:
  case FunctionConsts::OP_ZORBA_SEQUENCE_POINT_ACCESS_2:
  case FunctionConsts::FN_EXACTLY_ONE_1:
  case FunctionConsts::OP_EXACTLY_ONE_NORAISE_1:
  case FunctionConsts::FN_ONE_OR_MORE_1:
  case FunctionConsts::FN_ZERO_OR_ONE_1:
  case FunctionConsts::FN_UNORDERED_1:
  case FunctionConsts::FN_ROOT_1:
  case FunctionConsts::OP_SORT_NODES_ASC_1:
  case FunctionConsts::OP_SORT_NODES_ASC_OR_ATOMICS_1:
  case FunctionConsts::OP_SORT_NODES_DESC_1:
  case FunctionConsts::OP_SORT_NODES_DESC_OR_ATOMICS_1:
  case FunctionConsts::OP_SORT_DISTINCT_NODES_ASC_1:
  case FunctionConsts::OP_SORT_DISTINCT_NODES_ASC_OR_ATOMICS_1:
  case FunctionConsts::OP_SORT_DISTINCT_NODES_DESC_1:
  case FunctionConsts::OP_SORT_DISTINCT_NODES_DESC_OR_ATOMICS_1:
  case FunctionConsts::OP_ZORBA_MULTI_OBJECT_LOOKUP_2:
  case FunctionConsts::OP_ZORBA_MULTI_ARRAY_LOOKUP_2:
  case FunctionConsts::FN_JSONIQ_MEMBERS_1:
    return true;
  default:
    break;
  }

  return TypeOps::is_subtype(tm,
                             *f->getSignature().returnType(),
                             *GENV_TYPESYSTEM.ANY_ATOMIC_TYPE_STAR);
}


/*******************************************************************************
  The walk behind calls_unproven_function() and the like. It looks for a call whose
  target is not proven. In strict mode, used for the body of a target, it looks for
  any impurity as well: a target is proven only if its body has none.

  Any finding makes the whole walk answer true. So a walk that answers false has
  cleared every function it visited, cycles included, and the compiler cb remembers
  them. A rewrite only removes calls from a body or replaces them by what was already
  reachable from it, so a remembered answer stays correct or conservative.
********************************************************************************/
class ImpurityWalk
{
  CompilerCB                    * theCCB;
  TypeManager                   * theTM;
  std::set<const function*>       theVisited[2];

public:
  explicit ImpurityWalk(const expr* e)
    :
    theCCB(const_cast<expr*>(e)->get_ccb()),
    theTM(e->get_type_manager())
  {
  }

  bool walk(const expr* e, bool strict);

  bool isUnprovenCall(const expr* call);

  bool functionHasImpurity(const function* f, bool strict);

  // The answer for "f" alone, remembered whatever it is.
  bool functionAnswer(const function* f, bool strict);

  void remember()
  {
    for (int i = 0; i < 2; ++i)
    {
      std::set<const function*>::const_iterator ite = theVisited[i].begin();
      for (; ite != theVisited[i].end(); ++ite)
        theCCB->theFunctionPurity[i][*ite] = false;
    }
  }

private:
  bool varCanChange(const var_expr* var) const;

  const expr* boundExpr(const var_expr* var) const;

  bool isNeverFunctionItem(const expr* e) const;

  bool targetIsProven(const expr* target);

  bool builtinHasImpurity(const function* f) const;
};


static bool takes_function_item(const function* f, csize i)
{
  const signature& sig = f->getSignature();

  if (i >= sig.paramCount())
    return false;

  XQType::TypeKind const kind = sig[static_cast<int>(i)]->type_kind();

  return kind == XQType::FUNCTION_TYPE_KIND || kind == XQType::ANY_FUNCTION_TYPE_KIND;
}


// A prolog variable is assignable by default. It can only change if there is an
// assignment to it, or if a query that eval compiles at run time can add one.
bool ImpurityWalk::varCanChange(const var_expr* var) const
{
  if ((var->get_kind() != var_expr::prolog_var &&
       var->get_kind() != var_expr::local_var) ||
      !var->is_mutable())
    return false;

  return theCCB->theHasEval || var->num_set_exprs() != 1;
}


// What the value of the variable comes from, if a query cannot change it.
const expr* ImpurityWalk::boundExpr(const var_expr* var) const
{
  if (varCanChange(var))
    return NULL;

  switch (var->get_kind())
  {
  case var_expr::for_var:
  case var_expr::let_var:
    return var->get_domain_expr();

  case var_expr::prolog_var:
  case var_expr::local_var:
  {
    if (var->is_external() || var->num_set_exprs() == 0)
      return NULL;

    const var_set_expr* decl = var->get_set_expr(0);

    return decl->get_expr_kind() == var_decl_expr_kind ? decl->get_expr() : NULL;
  }
  default:
    return NULL;
  }
}


// $x(1) with an $x that is not a function item is a lookup in a JSON item, which
// has no effect. Collections hold nodes and JSON items only, and a member of a
// JSON item is never a function item.
bool ImpurityWalk::isNeverFunctionItem(const expr* e) const
{
  while (e != NULL)
  {
    xqtref_t const type = const_cast<expr*>(e)->get_return_type();

    if (TypeOps::is_subtype(theTM, *type, *GENV_TYPESYSTEM.STRUCTURED_ITEM_TYPE_STAR) ||
        TypeOps::is_subtype(theTM, *type, *GENV_TYPESYSTEM.ANY_ATOMIC_TYPE_STAR))
      return true;

    switch (e->get_expr_kind())
    {
    case wrapper_expr_kind:
      e = static_cast<const wrapper_expr*>(e)->get_input();
      break;

    case function_trace_expr_kind:
      e = static_cast<const function_trace_expr*>(e)->get_input();
      break;

    case treat_expr_kind:
    case promote_expr_kind:
      e = static_cast<const cast_base_expr*>(e)->get_input();
      break;

    case flwor_expr_kind:
      e = static_cast<const flwor_expr*>(e)->get_return_expr();
      break;

    case var_expr_kind:
      e = boundExpr(static_cast<const var_expr*>(e));
      break;

    case dynamic_function_invocation_expr_kind:
      e = static_cast<const dynamic_function_invocation_expr*>(e)->get_input();
      break;

    case fo_expr_kind:
      switch (static_cast<const fo_expr*>(e)->get_func()->getKind())
      {
      case FunctionConsts::ZORBA_STORE_STATIC_COLLECTIONS_DML_COLLECTION_1:
      case FunctionConsts::ZORBA_STORE_STATIC_COLLECTIONS_DML_COLLECTION_2:
      case FunctionConsts::ZORBA_STORE_STATIC_COLLECTIONS_DML_COLLECTION_3:
      case FunctionConsts::ZORBA_STORE_DYNAMIC_COLLECTIONS_DML_COLLECTION_1:
      case FunctionConsts::ZORBA_STORE_DYNAMIC_COLLECTIONS_DML_COLLECTION_2:
      case FunctionConsts::ZORBA_STORE_DYNAMIC_COLLECTIONS_DML_COLLECTION_3:
        return true;
      default:
        return false;
      }

    default:
      return false;
    }
  }

  return false;
}


bool ImpurityWalk::targetIsProven(const expr* target)
{
  if (isNeverFunctionItem(target))
    return true;

  while (target != NULL)
  {
    switch (target->get_expr_kind())
    {
    case wrapper_expr_kind:
      target = static_cast<const wrapper_expr*>(target)->get_input();
      break;

    case function_trace_expr_kind:
      target = static_cast<const function_trace_expr*>(target)->get_input();
      break;

    case treat_expr_kind:
    case promote_expr_kind:
      target = static_cast<const cast_base_expr*>(target)->get_input();
      break;

    case flwor_expr_kind:
      // The function item is what the return clause gives. The clauses are looked
      // at as children of the call.
      target = static_cast<const flwor_expr*>(target)->get_return_expr();
      break;

    case var_expr_kind:
      target = boundExpr(static_cast<const var_expr*>(target));
      break;

    case function_item_expr_kind:
    {
      const function_item_expr* fi = static_cast<const function_item_expr*>(target);

      // The body of a coercion calls the function item it wraps, which is the
      // value of its only closure variable.
      if (fi->is_coercion())
      {
        const std::vector<expr*>& values =
        const_cast<function_item_expr*>(fi)->get_fi_info()->theInScopeVarValues;

        if (values.size() != 1)
          return false;

        target = values[0];
        break;
      }

      const function* f = fi->get_function();

      return f != NULL && !functionHasImpurity(f, true);
    }

    default:
      return false;
    }
  }

  return false;
}


bool ImpurityWalk::isUnprovenCall(const expr* call)
{
  if (call->get_expr_kind() == dynamic_function_invocation_expr_kind)
  {
    return !targetIsProven(
        static_cast<const dynamic_function_invocation_expr*>(call)->get_input());
  }

  if (call->get_expr_kind() != fo_expr_kind)
    return false;

  const fo_expr* fo = static_cast<const fo_expr*>(call);
  const function* f = fo->get_func();

  if (f->isUdf())
    return functionHasImpurity(f, false);

  switch (f->getKind())
  {
  case FunctionConsts::FN_FUNCTION_NAME_1:
  case FunctionConsts::FN_FUNCTION_ARITY_1:
    return false;
  default:
    break;
  }

  if (!function_runs_opaque_code(f))
    return false;

  bool checked = false;

  for (csize i = 0; i < fo->num_args(); ++i)
  {
    if (takes_function_item(f, i))
    {
      checked = true;

      if (!targetIsProven(fo->get_arg(i)))
        return true;
    }
  }

  // fn:invoke and the like find the function by name.
  return !checked;
}


bool ImpurityWalk::builtinHasImpurity(const function* f) const
{
  if (!f->isDeterministic() || f->isSequential() || f->isUpdating() ||
      f->accessesDynCtx())
    return true;

  switch (f->getKind())
  {
  case FunctionConsts::FN_TRACE_2:
  case FunctionConsts::FN_ERROR_0:
  case FunctionConsts::FN_ERROR_1:
  case FunctionConsts::FN_ERROR_2:
  case FunctionConsts::FN_ERROR_3:
  case FunctionConsts::FN_COLLECTION_0:
  case FunctionConsts::FN_COLLECTION_1:
  case FunctionConsts::FN_URI_COLLECTION_0:
  case FunctionConsts::FN_URI_COLLECTION_1:
  case FunctionConsts::FN_DOC_1:
  case FunctionConsts::FN_DOC_AVAILABLE_1:
    return true;
  default:
    break;
  }

  // The stores of collections, documents, indexes and maps change while a query runs,
  // and so do the references of nodes.
  const zstring& ns = f->getName()->getNamespace();

  if (ns.find("/modules/store/") != zstring::npos ||
      ns == "http://zorba.io/modules/reference")
    return true;

  return !builtin_returns_existing_items(f, theTM);
}


bool ImpurityWalk::functionHasImpurity(const function* f, bool strict)
{
  if (strict &&
      (!f->isDeterministic() || f->isSequential() || f->isUpdating()))
    return true;

  if (!f->isUdf())
    return strict && builtinHasImpurity(f);

  const CompilerCB::FunctionPurityMap& cache = theCCB->theFunctionPurity[strict ? 1 : 0];
  CompilerCB::FunctionPurityMap::const_iterator cached = cache.find(f);

  if (cached != cache.end())
    return cached->second;

  if (!theVisited[strict ? 1 : 0].insert(f).second)
    return false;

  const expr* body = static_cast<const user_function*>(f)->getBody();

  return body == NULL || walk(body, strict);
}


bool ImpurityWalk::functionAnswer(const function* f, bool strict)
{
  bool const answer = functionHasImpurity(f, strict);

  if (!answer)
    remember();

  theCCB->theFunctionPurity[strict ? 1 : 0][f] = answer;

  return answer;
}


bool ImpurityWalk::walk(const expr* e, bool strict)
{
  switch (e->get_expr_kind())
  {
  case dynamic_function_invocation_expr_kind:
  {
    if (isUnprovenCall(e))
      return true;

    break;
  }
  case fo_expr_kind:
  {
    if (isUnprovenCall(e))
      return true;

    if (strict &&
        functionHasImpurity(static_cast<const fo_expr*>(e)->get_func(), true))
      return true;

    break;
  }
  case var_expr_kind:
  {
    if (strict && varCanChange(static_cast<const var_expr*>(e)))
      return true;

    break;
  }
  case doc_expr_kind:
  case elem_expr_kind:
  case attr_expr_kind:
  case namespace_expr_kind:
  case text_expr_kind:
  case pi_expr_kind:
  case json_direct_object_expr_kind:
  case json_object_expr_kind:
  case json_array_expr_kind:
  case validate_expr_kind:
  case transform_expr_kind:
  case delete_expr_kind:
  case insert_expr_kind:
  case rename_expr_kind:
  case replace_expr_kind:
  case var_set_expr_kind:
  case apply_expr_kind:
  case exit_expr_kind:
  case exit_catcher_expr_kind:
  case flowctl_expr_kind:
  case while_expr_kind:
  case eval_expr_kind:
  {
    if (strict)
      return true;

    break;
  }
  default:
    break;
  }

  ExprConstIterator iter(e);
  while (!iter.done())
  {
    const expr* child = iter.get_expr();

    if (child != NULL && walk(child, strict))
      return true;

    iter.next();
  }

  return false;
}


static CompilerCB::CallEffects call_effects(const expr* e)
{
  return const_cast<expr*>(e)->get_ccb()->theCallEffects;
}


static bool has_unproven_call(const expr* root)
{
  ImpurityWalk w(root);

  bool const answer = w.walk(root, false);

  if (!answer)
    w.remember();

  return answer;
}


static bool is_unproven_call_itself(const expr* call)
{
  ImpurityWalk w(call);

  if (call->get_expr_kind() == fo_expr_kind)
  {
    const function* f = static_cast<const fo_expr*>(call)->get_func();

    if (f->isUdf())
      return w.functionAnswer(f, false);
  }

  bool const answer = w.isUnprovenCall(call);

  if (!answer)
    w.remember();

  return answer;
}


bool calls_unproven_function(const expr* root)
{
  return call_effects(root) == CompilerCB::CALLS_HAVE_EFFECTS && has_unproven_call(root);
}


bool is_unproven_call(const expr* call)
{
  return call_effects(call) == CompilerCB::CALLS_HAVE_EFFECTS &&
         is_unproven_call_itself(call);
}


bool call_results_can_differ(const expr* root)
{
  return call_effects(root) != CompilerCB::CALLS_ARE_PURE && has_unproven_call(root);
}


bool is_call_with_varying_result(const expr* call)
{
  return call_effects(call) != CompilerCB::CALLS_ARE_PURE &&
         is_unproven_call_itself(call);
}


bool depends_on_call_effects(const expr* root)
{
  if (call_effects(root) != CompilerCB::CALLS_HAVE_EFFECTS)
    return false;

  ImpurityWalk w(root);

  bool const answer = w.walk(root, true);

  if (!answer)
    w.remember();

  return answer;
}


/*******************************************************************************
  The closed world of a query: the walk covers the query body, what its prolog
  variables are initialized with, and every function that it names in a call or a
  function item. A dynamic function call can only reach what this walk reaches,
  unless something here brings in code that it cannot see. Anything that can do so,
  or that can have a side effect itself, means CALLS_HAVE_EFFECTS. A query with no
  dynamic call has no call to classify: its direct sequential calls have their own
  rules, and CALLS_ARE_PURE leaves them alone at no cost.
********************************************************************************/
class ClosedWorldWalk
{
  TypeManager                   * theTM;
  std::set<const function*>       theFunctions;
  std::set<const var_expr*>       theVars;

public:
  bool theHasCalls;
  bool theResultsCanDiffer;
  bool theHasEffects;

  explicit ClosedWorldWalk(TypeManager* tm)
    :
    theTM(tm),
    theHasCalls(false),
    theResultsCanDiffer(false),
    theHasEffects(false)
  {
  }

  void opaque()
  {
    theHasEffects = true;
  }

  bool done() const
  {
    return theHasEffects && theHasCalls;
  }

  void walk(const expr* e);

private:
  void walkFunction(const function* f);

  void walkBuiltin(const fo_expr* fo);

  void walkVar(const var_expr* var);
};


void ClosedWorldWalk::walkFunction(const function* f)
{
  if (f == NULL)
  {
    opaque();
    return;
  }

  if (f->isSequential() || f->isUpdating())
    theHasEffects = true;

  if (!f->isDeterministic())
    theResultsCanDiffer = true;

  if (!f->isUdf())
    return;

  if (!theFunctions.insert(f).second)
    return;

  const expr* body = static_cast<const user_function*>(f)->getBody();

  // An external function, or one loaded from a plan: nothing says what it runs.
  if (body == NULL)
    opaque();
  else
    walk(body);
}


void ClosedWorldWalk::walkBuiltin(const fo_expr* fo)
{
  const function* f = fo->get_func();

  if (f->isExternal())
    opaque();

  switch (f->getKind())
  {
  case FunctionConsts::FN_FUNCTION_LOOKUP_2:
  case FunctionConsts::OP_ZORBA_FUNCTION_LOOKUP_5:
  case FunctionConsts::FN_ZORBA_INVOKE_N:
  case FunctionConsts::FN_ZORBA_INVOKE_N_N:
  case FunctionConsts::FN_ZORBA_INVOKE_U_N:
  case FunctionConsts::FN_ZORBA_INVOKE_S_N:
  case FunctionConsts::FN_ZORBA_EVAL_1:
  case FunctionConsts::FN_ZORBA_EVAL_N_1:
  case FunctionConsts::FN_ZORBA_EVAL_U_1:
  case FunctionConsts::FN_ZORBA_EVAL_S_1:
  // It gives a node a reference, which ref:has-reference reads, but it is declared
  // neither sequential nor nondeterministic.
  case FunctionConsts::FN_REFERENCE_REFERENCE_1:
    opaque();
    break;
  case FunctionConsts::FN_TRACE_2:
    theResultsCanDiffer = true;
    break;
  default:
    break;
  }

  walkFunction(f);

  if (function_runs_opaque_code(f))
    theHasCalls = true;

  if (!builtin_returns_existing_items(f, theTM))
    theResultsCanDiffer = true;
}


void ClosedWorldWalk::walkVar(const var_expr* var)
{
  if (var->get_kind() != var_expr::prolog_var &&
      var->get_kind() != var_expr::local_var)
    return;

  if (!theVars.insert(var).second)
    return;

  // The value comes from outside of the query. Only a type can say that it is not a
  // function item that reaches code the walk does not see.
  if (var->is_external())
  {
    xqtref_t type = const_cast<var_expr*>(var)->get_return_type();

    if (!TypeOps::is_subtype(theTM, *type, *GENV_TYPESYSTEM.STRUCTURED_ITEM_TYPE_STAR) &&
        !TypeOps::is_subtype(theTM, *type, *GENV_TYPESYSTEM.ANY_ATOMIC_TYPE_STAR))
      opaque();
  }

  for (csize i = 0; i < var->num_set_exprs(); ++i)
  {
    const var_set_expr* setExpr = var->get_set_expr(i);

    if (setExpr->get_expr_kind() != var_decl_expr_kind)
      theHasEffects = true;
    else if (setExpr->get_expr() != NULL)
      walk(setExpr->get_expr());
  }
}


void ClosedWorldWalk::walk(const expr* e)
{
  // The kinds of expr and the functions below are all that gives an expr a sequential
  // or updating scripting kind. The kind of the expr itself cannot be used: a block
  // with the declaration of a prolog variable has one.
  if (done())
    return;

  switch (e->get_expr_kind())
  {
  case dynamic_function_invocation_expr_kind:
    theHasCalls = true;
    break;

  case fo_expr_kind:
  {
    const fo_expr* fo = static_cast<const fo_expr*>(e);

    if (fo->get_func()->isUdf())
      walkFunction(fo->get_func());
    else
      walkBuiltin(fo);

    break;
  }
  case function_item_expr_kind:
    walkFunction(static_cast<const function_item_expr*>(e)->get_function());
    break;

  case var_expr_kind:
    walkVar(static_cast<const var_expr*>(e));
    break;

  case var_set_expr_kind:
  case exit_expr_kind:
  case exit_catcher_expr_kind:
  case flowctl_expr_kind:
  case while_expr_kind:
    theHasEffects = true;
    break;

  case apply_expr_kind:
  case eval_expr_kind:
  case debugger_expr_kind:
  case delete_expr_kind:
  case insert_expr_kind:
  case rename_expr_kind:
  case replace_expr_kind:
    opaque();
    break;

  case doc_expr_kind:
  case elem_expr_kind:
  case attr_expr_kind:
  case namespace_expr_kind:
  case text_expr_kind:
  case pi_expr_kind:
  case json_direct_object_expr_kind:
  case json_object_expr_kind:
  case json_array_expr_kind:
  case validate_expr_kind:
  case transform_expr_kind:
    theResultsCanDiffer = true;
    break;

  default:
    break;
  }

  ExprConstIterator iter(e);
  while (!iter.done() && !done())
  {
    const expr* child = iter.get_expr();

    if (child != NULL)
      walk(child);

    iter.next();
  }
}


/*******************************************************************************
  Marks, for every for clause reachable from the query, whether its domain has an
  unproven call, on the tree before the optimizer runs. One walk, bottom up: the
  answer for an expr is whether it or one of its children is an unproven call, and
  the answer for a call of a user function covers its body (is_unproven_call()). The
  bodies of the functions that the query names are marked as well, once.
********************************************************************************/
class BindingSequenceWalk
{
  CompilerCB                    * theCCB;
  std::set<const function*>       theFunctions;
  std::set<const var_expr*>       theVars;

public:
  // Whether the walk found an unproven call.
  bool                            theFoundCall;

  explicit BindingSequenceWalk(CompilerCB* ccb) : theCCB(ccb), theFoundCall(false) {}

  bool walk(expr* e);

  void walkFunction(const function* f);

private:

  void walkVar(const var_expr* var);
};


void BindingSequenceWalk::walkFunction(const function* f)
{
  if (f == NULL || !f->isUdf() || !theFunctions.insert(f).second)
    return;

  const user_function* udf = static_cast<const user_function*>(f);

  // A function of another compilation (the query that runs an eval, or a static
  // context shared by several queries) was decided by it, and may be optimized or
  // running: it is left as it is.
  if (udf->getCCB() != theCCB || udf->isOptimized() || udf->getPlan() != NULL)
    return;

  expr* body = udf->getBody();

  if (body != NULL)
    walk(body);
}


void BindingSequenceWalk::walkVar(const var_expr* var)
{
  if ((var->get_kind() != var_expr::prolog_var &&
       var->get_kind() != var_expr::local_var) ||
      const_cast<var_expr*>(var)->get_ccb() != theCCB ||
      !theVars.insert(var).second)
    return;

  for (csize i = 0; i < var->num_set_exprs(); ++i)
  {
    var_set_expr* setExpr = var->get_set_expr(i);

    if (setExpr->get_expr() != NULL)
      walk(setExpr->get_expr());
  }
}


bool BindingSequenceWalk::walk(expr* e)
{
  bool found = false;

  switch (e->get_expr_kind())
  {
  case dynamic_function_invocation_expr_kind:
    found = is_unproven_call(e);
    theFoundCall = theFoundCall || found;
    break;

  case fo_expr_kind:
  {
    const function* f = static_cast<const fo_expr*>(e)->get_func();
    found = is_unproven_call(e);
    theFoundCall = theFoundCall || found;
    walkFunction(f);
    break;
  }
  case function_item_expr_kind:
    walkFunction(static_cast<const function_item_expr*>(e)->get_function());
    break;

  case var_expr_kind:
    walkVar(static_cast<const var_expr*>(e));
    break;

  default:
    break;
  }

  if (e->get_expr_kind() == flwor_expr_kind)
  {
    flwor_expr* flwor = static_cast<flwor_expr*>(e);

    // The domain of each for clause is walked on its own, for its answer. The rest
    // of the flwor is walked below, with the domains of the for clauses skipped.
    std::set<const expr*> domains;

    for (csize i = 0; i < flwor->num_clauses(); ++i)
    {
      flwor_clause* c = flwor->get_clause(i);

      if (c->get_kind() != flwor_clause::for_clause)
        continue;

      for_clause* fc = static_cast<for_clause*>(c);
      bool const domainFound = walk(fc->get_expr());

      // Decided once: a clause already marked keeps its mark.
      if (fc->get_binding_sequence() == for_clause::BINDING_SEQUENCE_UNKNOWN)
        fc->set_binding_sequence(domainFound ?
                                 for_clause::BINDING_SEQUENCE_YES :
                                 for_clause::BINDING_SEQUENCE_NO);

      domains.insert(fc->get_expr());
      found = found || domainFound;
    }

    ExprIterator iter(e);
    while (!iter.done())
    {
      expr* child = **iter;

      if (child != NULL && domains.find(child) == domains.end() && walk(child))
        found = true;

      iter.next();
    }

    return found;
  }

  ExprIterator iter(e);
  while (!iter.done())
  {
    expr* child = **iter;

    if (child != NULL && walk(child))
      found = true;

    iter.next();
  }

  return found;
}


bool is_binding_sequence(const for_clause* fc)
{
  switch (fc->get_binding_sequence())
  {
  case for_clause::BINDING_SEQUENCE_YES:
    return true;
  case for_clause::BINDING_SEQUENCE_NO:
    return false;
  default:
    // A clause that a rewrite created, or one in code that the walk of
    // classify_calls() did not reach.
    return calls_unproven_function(fc->get_expr());
  }
}


void classify_calls(expr* root)
{
  CompilerCB* ccb = root->get_ccb();

  // What an eval query can see comes from the query that runs it.
  if (ccb->theIsEval)
  {
    ccb->theCallEffects = CompilerCB::CALLS_HAVE_EFFECTS;
  }
  else
  {
    ClosedWorldWalk w(root->get_type_manager());

    w.walk(root);

    if (!w.theHasCalls)
      ccb->theCallEffects = CompilerCB::CALLS_ARE_PURE;
    else if (w.theHasEffects)
      ccb->theCallEffects = CompilerCB::CALLS_HAVE_EFFECTS;
    else if (w.theResultsCanDiffer)
      ccb->theCallEffects = CompilerCB::CALLS_RESULTS_CAN_DIFFER;
    else
      ccb->theCallEffects = CompilerCB::CALLS_ARE_PURE;
  }

  if (ccb->theCallEffects == CompilerCB::CALLS_HAVE_EFFECTS)
  {
    BindingSequenceWalk w(ccb);
    w.walk(root);

    // A function that the query does not name can still run: fn:function-lookup,
    // an invoke function or eval can find it by name, and it is then optimized
    // when it is first called. Its for clauses are decided here too, so that the
    // decision does not depend on the level.
    std::vector<static_context*> sctxs;
    sctxs.push_back(ccb->theRootSctx);

    CompilerCB::SctxMap::const_iterator ite = ccb->theSctxMap.begin();
    for (; ite != ccb->theSctxMap.end(); ++ite)
      sctxs.push_back(ite->second.getp());

    std::set<const static_context*> visitedSctxs;
    std::vector<function*> functions;

    for (csize i = 0; i < sctxs.size(); ++i)
    {
      if (sctxs[i] != NULL)
        sctxs[i]->get_udfs(functions, visitedSctxs);
    }

    for (csize j = 0; j < functions.size(); ++j)
      w.walkFunction(functions[j]);

    ccb->theNoHoisting = w.theFoundCall;
  }
  else
  {
    ccb->theNoHoisting = false;
  }
}


/*******************************************************************************

********************************************************************************/
int count_variable_uses(
    expr* root,
    var_expr* var,
    int limit,
    std::vector<expr**>* path)
{
  int count = 0;

  count_var_uses_rec(root, var, limit, path, count);

  return count;
}

/*******************************************************************************

********************************************************************************/
bool estimate_expression_size_rec(
    expr* e,
    int limit,
    int& count)
{
  ++count;

  if (limit > 0 && count >= limit)
    return false;

  ExprIterator iter(e);
  while (!iter.done())
  {
    bool done = !estimate_expression_size_rec(**iter, limit, count);

    if (done)
      return false;

    iter.next();
  }
  return true;
}


/*******************************************************************************

********************************************************************************/
int estimate_expression_size(expr* root, int limit)
{
  int count = 0;

  estimate_expression_size_rec(root, limit, count);

  return count;
}


/*******************************************************************************
  copy annotations when wrapping an expression in a new one
********************************************************************************/
expr* fix_annotations(expr* new_expr, const expr* old_expr)
{
  if (old_expr == NULL)
  {
    switch (new_expr->get_expr_kind())
    {
    case fo_expr_kind:
      old_expr = static_cast<const fo_expr*>(new_expr)->get_arg(0);
      break;
    default:
      assert(false);
      return NULL;
    }
  }

  const expr::FreeVars& old_set = old_expr->getFreeVars();
  const expr::FreeVars& new_set = new_expr->getFreeVars();

  expr::FreeVars s;
  std::set_union(old_set.begin(),
                 old_set.end(),
                 new_set.begin(),
                 new_set.end(),
                 inserter(s, s.begin()));

  new_expr->setFreeVars(s);

  return new_expr;
}



/*******************************************************************************
  Replace all references to "oldVar" inside "e" with references to "newVar".
********************************************************************************/
void replace_var(expr* e, const var_expr* oldVar, var_expr* newVar)
{
  if (e->get_expr_kind() == wrapper_expr_kind)
  {
    wrapper_expr* wrapper = reinterpret_cast<wrapper_expr*>(e);

    if (wrapper->get_input() == oldVar)
    {
      wrapper->set_expr(newVar);
      return;
    }
  }

  ExprIterator iter(e);
  while (!iter.done())
  {
    replace_var((**iter), oldVar, newVar);
    iter.next();
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
  Let VDE(e) be the set of variable-defining exprs (i.e., flwor, trycatch, or 
  vardecl exprs) within the expr tree rooted at expr e. Let V(e) be the set of
  variables defined in any of the exprs in VDE(e). This method assigns a prefix
  id to each variable in V(e) and stores the mapping between var_expr and prefix
  id in "varidmap" and the reverse mapping in "idvapmap". It also returns the
  number of vars in V(e).

  Given 2 vars v1 and v2 in V(e), their prefix ids allows to check if v1 is
  defined before v2: v1 is defined before v2 iff id(v1) < id(v2).
********************************************************************************/
void index_flwor_vars(
    const expr* e,
    csize& numVars,
    VarIdMap& varidmap,
    IdVarMap* idvarmap)
{
  if (e->get_expr_kind() == flwor_expr_kind)
  {
    const flwor_expr* flwor = static_cast<const flwor_expr *>(e);

    flwor_expr::clause_list_t::const_iterator i = flwor->clause_begin();
    flwor_expr::clause_list_t::const_iterator end = flwor->clause_end();
    for (; i != end; ++i)
    {
      const flwor_clause* c = *i;

      switch (c->get_kind())
      {
      case flwor_clause::for_clause:
      case flwor_clause::let_clause:
      {
        const forlet_clause* flc = static_cast<const forlet_clause *>(c);

        add_var(flc->get_var(), numVars, varidmap, idvarmap);
        add_var(flc->get_pos_var(), numVars, varidmap, idvarmap);
        add_var(flc->get_score_var(), numVars, varidmap, idvarmap);

        index_flwor_vars(flc->get_expr(), numVars, varidmap, idvarmap);

        break;
      }
      case flwor_clause::window_clause:
      {
        const window_clause* wc = static_cast<const window_clause *>(c);

        add_var(wc->get_var(), numVars, varidmap, idvarmap);

        index_flwor_vars(wc->get_expr(), numVars, varidmap, idvarmap);

        flwor_wincond* startCond = wc->get_win_start();
        flwor_wincond* stopCond = wc->get_win_stop();

        if (startCond != NULL)
          add_wincond_vars(startCond, numVars, varidmap, idvarmap);

        if (stopCond != NULL)
          add_wincond_vars(stopCond, numVars, varidmap, idvarmap);

        break;
      }
      case flwor_clause::groupby_clause:
      {
        const groupby_clause* gc = static_cast<const groupby_clause *>(c);

        const flwor_clause::rebind_list_t& gvars = gc->get_grouping_vars();
        const flwor_clause::rebind_list_t& ngvars = gc->get_nongrouping_vars();
        csize numGroupVars = gvars.size();
        csize numNonGroupVars = ngvars.size();

        for (csize i = 0; i < numGroupVars; ++i)
        {
          index_flwor_vars(gvars[i].first, numVars, varidmap, idvarmap);
        }

        for (csize i = 0; i < numNonGroupVars; ++i)
        {
          index_flwor_vars(ngvars[i].first, numVars, varidmap, idvarmap);
        }

        for (csize i = 0; i < numGroupVars; ++i)
        {
          add_var(gvars[i].second, numVars, varidmap, idvarmap);
        }
        
        for (csize i = 0; i < numNonGroupVars; ++i)
        {
          add_var(ngvars[i].second, numVars, varidmap, idvarmap);
        }

        break;
      }
      case flwor_clause::count_clause:
      {
        const count_clause* cc = static_cast<const count_clause *>(c);

        add_var(cc->get_var(), numVars, varidmap, idvarmap);

        break;
      }
      case flwor_clause::where_clause:
      {
        const where_clause* wc = static_cast<const where_clause *>(c);

        index_flwor_vars(wc->get_expr(), numVars, varidmap, idvarmap);

        break;
      }
      case flwor_clause::orderby_clause:
      {
        const orderby_clause* obc = static_cast<const orderby_clause *>(c);
        csize numExprs = obc->num_columns();
        for (csize i = 0; i < numExprs; ++i)
        {
          index_flwor_vars(obc->get_column_expr(i), numVars, varidmap, idvarmap);
        }

        break;
      }
      case flwor_clause::materialize_clause:
      {
        break;
      }
      default:
      {
        ZORBA_ASSERT(false);
      }
      }
    }

    index_flwor_vars(flwor->get_return_expr(), numVars, varidmap, idvarmap);
  }
  else if (e->get_expr_kind() == trycatch_expr_kind)
  {
    const trycatch_expr* trycatch = static_cast<const trycatch_expr*>(e);

    index_flwor_vars(trycatch->get_try_expr(), numVars, varidmap, idvarmap);

    csize numClauses = trycatch->clause_count();

    for (csize i = 0; i < numClauses; ++i)
    {
      const catch_clause* clause = (*trycatch)[i];

      catch_clause::var_map_t& trycatchVars =
      const_cast<catch_clause*>(clause)->get_vars();

      catch_clause::var_map_t::const_iterator ite = trycatchVars.begin();
      catch_clause::var_map_t::const_iterator end = trycatchVars.end();
      for (; ite != end; ++ite)
      {
        var_expr* trycatchVar = (*ite).second;
        add_var(trycatchVar, numVars, varidmap, idvarmap);
      }

      index_flwor_vars(trycatch->get_catch_expr(i), numVars, varidmap, idvarmap);
    }
  }
  else if (e->get_expr_kind() == var_decl_expr_kind)
  {
    const var_decl_expr* vdecl = static_cast<const var_decl_expr*>(e);
    add_var(vdecl->get_var_expr(), numVars, varidmap, idvarmap);

    if (vdecl->get_init_expr())
      index_flwor_vars(vdecl->get_init_expr(), numVars, varidmap, idvarmap);
  }
  else
  {
    ExprConstIterator iter(e);
    while(!iter.done())
    {
      const expr* ce = iter.get_expr();
      if (ce)
      {
        index_flwor_vars(ce, numVars, varidmap, idvarmap);
      }
      iter.next();
    }
  }
}


/*******************************************************************************

********************************************************************************/
static void add_wincond_vars(
    const flwor_wincond* cond,
    csize& numVars,
    VarIdMap& varidmap,
    IdVarMap* idvarmap)
{
  const flwor_wincond::vars& inVars = cond->get_in_vars();
  const flwor_wincond::vars& outVars = cond->get_out_vars();

  add_var(inVars.posvar, numVars, varidmap, idvarmap);
  add_var(inVars.curr, numVars, varidmap, idvarmap);
  add_var(inVars.prev, numVars, varidmap, idvarmap);
  add_var(inVars.next, numVars, varidmap, idvarmap);

  index_flwor_vars(cond->get_expr(), numVars, varidmap, idvarmap);

  add_var(outVars.posvar, numVars, varidmap, idvarmap);
  add_var(outVars.curr, numVars, varidmap, idvarmap);
  add_var(outVars.prev, numVars, varidmap, idvarmap);
  add_var(outVars.next, numVars, varidmap, idvarmap);
}


/*******************************************************************************

********************************************************************************/
static void add_var(
    var_expr* v,
    csize& numVars,
    VarIdMap& varidmap,
    IdVarMap* idvarmap)
{
  if (v != NULL)
  {
    varidmap[v] = ++numVars;
    if (idvarmap)
      idvarmap->push_back(v);
  }
}


/*******************************************************************************
  For each expr E in the expr tree rooted at "e", this method computes the set
  of variables that belong to V(e) and are referenced by E. Let V(E) be this
  set. V(E) is implemented as a bitset ("freeset") whose size is equal to the
  size of V(e) and whose i-th bit is on iff the var with prefix id i belongs
  to V(E). The mapping between E and V(E) is stored in "freevarMap".
********************************************************************************/
void build_expr_to_vars_map(
    expr* e,
    const VarIdMap& varmap,
    DynamicBitset& freeset,
    ExprVarsMap& freevarMap)
{
  if (e->get_expr_kind() == var_expr_kind)
  {
    set_bit(static_cast<var_expr*>(e), varmap, freeset, true);
    freevarMap[e] = freeset;
    return;
  }

  csize numVars = freeset.size();

  DynamicBitset eFreeset(numVars);
  ExprIterator iter(e);
  while(!iter.done())
  {
    expr* ce = **iter;
    if (ce)
    {
      eFreeset.reset();
      build_expr_to_vars_map(ce, varmap, eFreeset, freevarMap);
      freeset.set_union(eFreeset);
    }
    iter.next();
  }

  // A flwor does not depend on the vars that are defined inside the flwor itself,
  // so remove these vars from the freeset of the flwor, if they have been added
  // there.
  if (e->get_expr_kind() == flwor_expr_kind)
  {
    flwor_expr* flwor = static_cast<flwor_expr *>(e);

    for (flwor_expr::clause_list_t::const_iterator i = flwor->clause_begin();
         i != flwor->clause_end();
         ++i)
    {
      const flwor_clause* c = *i;

      switch (c->get_kind())
      {
      case flwor_clause::for_clause:
      case flwor_clause::let_clause:
      {
        const forlet_clause* flc = static_cast<const forlet_clause *>(c);

        set_bit(flc->get_var(), varmap, freeset, false);
        set_bit(flc->get_pos_var(), varmap, freeset, false);
        set_bit(flc->get_score_var(), varmap, freeset, false);

        break;
      }
      case flwor_clause::window_clause:
      {
        const window_clause* wc = static_cast<const window_clause *>(c);

        set_bit(wc->get_var(), varmap, freeset, false);

        flwor_wincond* startCond = wc->get_win_start();
        flwor_wincond* stopCond = wc->get_win_stop();

        if (startCond != NULL)
          remove_wincond_vars(startCond, varmap, freeset);

        if (stopCond != NULL)
          remove_wincond_vars(stopCond, varmap, freeset);

        break;
      }
      case flwor_clause::groupby_clause:
      {
        const groupby_clause* gc = static_cast<const groupby_clause *>(c);

        const flwor_clause::rebind_list_t& gvars = gc->get_grouping_vars();
        csize numGroupVars = gvars.size();

        for (csize i = 0; i < numGroupVars; ++i)
        {
          set_bit(gvars[i].second, varmap, freeset, false);
        }

        const flwor_clause::rebind_list_t& ngvars = gc->get_nongrouping_vars();
        csize numNonGroupVars = ngvars.size();

        for (csize i = 0; i < numNonGroupVars; ++i)
        {
          set_bit(ngvars[i].second, varmap, freeset, false);
        }

        break;
      }
      case flwor_clause::count_clause:
      {
        const count_clause* cc = static_cast<const count_clause *>(c);

        set_bit(cc->get_var(), varmap, freeset, false);
        
        break;
      }
      case flwor_clause::where_clause:
      case flwor_clause::orderby_clause:
      case flwor_clause::materialize_clause:
      {
        break;
      }
      default:
      {
        ZORBA_ASSERT(false);
      }
      }
    }
  }
  else if (e->get_expr_kind() == trycatch_expr_kind)
  {
    trycatch_expr* trycatch = static_cast<trycatch_expr*>(e);

    csize numClauses = trycatch->clause_count();

    for (csize i = 0; i < numClauses; ++i)
    {
      const catch_clause* clause = (*trycatch)[i];

      catch_clause::var_map_t& trycatchVars =
      const_cast<catch_clause*>(clause)->get_vars();

      catch_clause::var_map_t::const_iterator ite = trycatchVars.begin();
      catch_clause::var_map_t::const_iterator end = trycatchVars.end();
      for (; ite != end; ++ite)
      {
        var_expr* v = (*ite).second;
        set_bit(v, varmap, freeset, false);
      }
    }
  }
  else if (e->get_expr_kind() == block_expr_kind)
  {
    block_expr* block = static_cast<block_expr*>(e);

    csize numArgs = block->size();

    for (csize i = 0; i < numArgs; ++i)
    {
      if ((*block)[i]->get_expr_kind() != var_decl_expr_kind)
        continue;

      var_expr* v = static_cast<var_decl_expr*>((*block)[i])->get_var_expr();
      set_bit(v, varmap, freeset, false);
    }
  }

  freevarMap[e] = freeset;
}


/*******************************************************************************

********************************************************************************/
static void remove_wincond_vars(
    const flwor_wincond* cond,
    const VarIdMap& varmap,
    DynamicBitset& freeset)
{
  const flwor_wincond::vars& inVars = cond->get_in_vars();
  const flwor_wincond::vars& outVars = cond->get_out_vars();

  set_bit(inVars.posvar, varmap, freeset, false);
  set_bit(inVars.curr, varmap, freeset, false);
  set_bit(inVars.prev, varmap, freeset, false);
  set_bit(inVars.next, varmap, freeset, false);

  set_bit(outVars.posvar, varmap, freeset, false);
  set_bit(outVars.curr, varmap, freeset, false);
  set_bit(outVars.prev, varmap, freeset, false);
  set_bit(outVars.next, varmap, freeset, false);
}


/*******************************************************************************

********************************************************************************/
static void set_bit(
    var_expr* v,
    const VarIdMap& varmap,
    DynamicBitset& freeset,
    bool value)
{
  if (v == NULL)
    return;

  VarIdMap::const_iterator i = varmap.find(v);
  if (i != varmap.end())
    freeset.set(i->second, value);
}


}
}

/* vim:set et sw=2 ts=2: */
