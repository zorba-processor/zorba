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
#include "stdafx.h"

#include "compiler/api/compilercb.h"
#include "compiler/rewriter/rules/index_join_rule.h"
#include "compiler/expression/flwor_expr.h"
#include "compiler/expression/script_exprs.h"
#include "compiler/expression/expr.h"
#include "compiler/rewriter/tools/expr_tools.h"
#include "compiler/xqddf/value_index.h"
#include "compiler/expression/expr_iter.h"

#include "functions/function.h"
#include "functions/library.h"

#include "types/typeops.h"

#include "system/properties.h"

#include "diagnostics/assert.h"


namespace zorba
{

struct PredicateInfo
{
  flwor_expr  * theFlworExpr;
  csize         theWherePos;
  expr        * thePredicate;
  expr        * theOuterOp;
  var_expr    * theOuterVar;
  ulong         theOuterVarId;
  expr        * theInnerOp;
  var_expr    * theInnerVar;
  bool          theIsGeneral;
};


/*******************************************************************************

********************************************************************************/
IndexJoinRule::IndexJoinRule(RewriterContext* rctx)
  :
  RewriteRule(RewriteRule::IndexJoin, "IndexJoin"),
  theRCtx(rctx),
  theRootExpr(rctx->getRoot())
{
  theVarIdMap = new expr_tools::VarIdMap;
  theIdVarMap = new expr_tools::IdVarMap;
  theExprVarsMap = new expr_tools::ExprVarsMap;

  csize numVars = 0;
  expr_tools::index_flwor_vars(theRootExpr, numVars, *theVarIdMap, theIdVarMap);
}


/*******************************************************************************

********************************************************************************/
IndexJoinRule::~IndexJoinRule()
{
  delete theVarIdMap;
  delete theIdVarMap;
  delete theExprVarsMap;
}


/*******************************************************************************

********************************************************************************/
void IndexJoinRule::reset()
{
  assert(theFlworStack.empty());

  theExprVarsMap->clear();

  csize numVars = theIdVarMap->size();
  DynamicBitset freeset(numVars);
  expr_tools::build_expr_to_vars_map(theRootExpr,
                                     *theVarIdMap,
                                     freeset,
                                     *theExprVarsMap);
}


/*******************************************************************************
  This rule analyzes the where clauses of flwor exprs to deterimne whether any
  predicate in a clause is a join predicate and whether the associated join
  can be converted into a hashjoin using an index that is built on-the-fly.
********************************************************************************/
expr* IndexJoinRule::apply(RewriterContext& rCtx, expr* node, bool& modified)
{
  flwor_expr* flworExpr = NULL;

  expr_kind_t nodeKind = node->get_expr_kind();

  if (nodeKind == trycatch_expr_kind)
  {
    theFlworStack.push_back(node);
  }
  else if (nodeKind == flwor_expr_kind || nodeKind == gflwor_expr_kind)
  {
    flworExpr = static_cast<flwor_expr *>(node);

    // Push the flwor expr into the stack
    theFlworStack.push_back(flworExpr);
    theInReturnClause.push_back(false);

    csize numClauses = flworExpr->num_clauses();
    for (csize i = 0; i < numClauses; ++i)
    {
      flwor_clause* c = flworExpr->get_clause(i);

      if (c->get_kind() != flwor_clause::where_clause)
        continue;

      const where_clause* wc = static_cast<const where_clause *>(c);
      expr* whereExpr = wc->get_expr();

      PredicateInfo predInfo;
      predInfo.theFlworExpr = flworExpr;
      predInfo.theWherePos = i;
      predInfo.thePredicate = whereExpr;

      if (isIndexJoinPredicate(predInfo))
      {
        rewriteJoin(predInfo, modified);

        if (modified)
        {
          predInfo.theFlworExpr->remove_clause(c, i);

          expr* e = theFlworStack.back();

          ZORBA_ASSERT(e == node || e->get_expr_kind() == block_expr_kind);
          
          theFlworStack.pop_back();
          theInReturnClause.pop_back();
          return e;
        }
      }
      else if (whereExpr->get_function_kind() == FunctionConsts::OP_AND_N)
      {
        // TODO: consider multi-key indices
        ExprIterator iter(whereExpr);
        while (!iter.done())
        {
          PredicateInfo predInfo;
          predInfo.theFlworExpr = flworExpr;
          predInfo.theWherePos = i;
          predInfo.thePredicate = (**iter);
          
          if (isIndexJoinPredicate(predInfo))
          {
            rewriteJoin(predInfo, modified);
            
            if (modified)
            {
              // TODO: just remove the pred instead of replacing it with true.
              expr* trueExpr = rCtx.theEM->
              create_const_expr(flworExpr->get_sctx(),
                                flworExpr->get_udf(),
                                flworExpr->get_loc(),
                                true);
              (**iter) = trueExpr;
              
              expr* e = theFlworStack.back();
              
              ZORBA_ASSERT(e == node || e->get_expr_kind() == block_expr_kind);
              
              theFlworStack.pop_back();
              theInReturnClause.pop_back();
              return e;
            }
          }

          iter.next();
        }
      }
    }
  }

  ExprIterator iter(node);
  while (!iter.done())
  {
    expr* currChild = **iter;

    if (flworExpr != NULL && currChild == flworExpr->get_return_expr())
      theInReturnClause.back() = true;

    expr* newChild = apply(rCtx, currChild, modified);

    if (currChild != newChild)
    {
      assert(modified);
      **iter = newChild;
    }

    if (modified)
      break;

    iter.next();
  }

  if (nodeKind == trycatch_expr_kind)
  {
    ZORBA_ASSERT(theFlworStack.back() == node);

    theFlworStack.pop_back();
    return node;
  }
  else if (nodeKind == flwor_expr_kind || nodeKind == gflwor_expr_kind)
  {
    expr* e = theFlworStack.back();

    ZORBA_ASSERT(e == node || e->get_expr_kind() == block_expr_kind);

    theFlworStack.pop_back();
    theInReturnClause.pop_back();
    return e;
  }
  else
  {
    return node;
  }
}


/*******************************************************************************
  Check whether the given predicate is a join predicate that can be converted
  to a hashjoin.
********************************************************************************/
bool IndexJoinRule::isIndexJoinPredicate(PredicateInfo& predInfo)
{
  const fo_expr* foExpr = NULL;
  const function* fn;
  const expr* predExpr = predInfo.thePredicate;

  // skip fn:boolean() wrapper
  while (true)
  {
    if (predExpr->get_expr_kind() != fo_expr_kind)
      return false;

    foExpr = static_cast<const fo_expr*>(predExpr);
    fn = foExpr->get_func();

    if (fn->getKind() == FunctionConsts::FN_BOOLEAN_1)
    {
      predExpr = foExpr->get_arg(0);
      continue;
    }

    break;
  }

  CompareConsts::CompareType opKind = fn->comparisonKind();

  if (opKind != CompareConsts::VALUE_EQUAL && opKind != CompareConsts::GENERAL_EQUAL)
    return false;

  predInfo.theIsGeneral = (opKind == CompareConsts::GENERAL_EQUAL);

  expr* op1 = foExpr->get_arg(0);
  expr* op2 = foExpr->get_arg(1);

  // Analyze each operand of the eq to see if it depends on a single LOOP
  // variable. If that is not true, we reject this predicate.
  ulong var1id;
  var_expr* var1 = findLoopVar(op1, var1id);
  if (var1 == NULL)
    return false;

  ulong var2id;
  var_expr* var2 = findLoopVar(op2, var2id);
  if (var2 == NULL)
    return false;

  if (var1 == var2)
    return false;

  // Determine the outer and inner side of the join
  if (var1id < var2id)
  {
    predInfo.theOuterOp = op1;
    predInfo.theOuterVar = var1;
    predInfo.theOuterVarId = var1id;
    predInfo.theInnerOp = op2;
    predInfo.theInnerVar = var2;
  }
  else
  {
    predInfo.theOuterOp = op2;
    predInfo.theOuterVar = var2;
    predInfo.theOuterVarId = var2id;
    predInfo.theInnerOp = op1;
    predInfo.theInnerVar = var1;
  }

  static_context* sctx = predExpr->get_sctx();
  TypeManager* tm = sctx->get_typemanager();
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  // Make sure we don't have a pred between two groupby vars 
  if (predInfo.theInnerVar->get_kind() != var_expr::for_var)
    return false;

  forlet_clause* innerVarClause = predInfo.theInnerVar->get_forlet_clause();

  // The inner var must not be an outer FOR var
  if (innerVarClause->is_allowing_empty())
    return false;

  // The predicate must be in the same flwor that defines the inner var (this
  // way we can be sure that the pred acts as a filter over the inner var).
  if (innerVarClause->get_flwor_expr() != predInfo.theFlworExpr)
    return false;

  // There should be no COUNT clause and no sequential clauses between the
  // WHERE clause and the inner var clause.
  for (csize i = predInfo.theWherePos; i > 0; --i)
  {
    flwor_clause* c = predInfo.theFlworExpr->get_clause(i-1);

    switch (c->get_kind())
    {
    case flwor_clause::for_clause:
    case flwor_clause::let_clause:
    case flwor_clause::window_clause:
    {
      forletwin_clause* flwc = static_cast<forletwin_clause*>(c);
      if (flwc->get_expr()->is_sequential())
        return false;

      break;
    }
    case flwor_clause::count_clause:
    {
      return false;
    }
    case flwor_clause::order_clause:
    case flwor_clause::where_clause:
    {
      break;
    }
    case flwor_clause::groupby_clause:
    default:
    {
      ZORBA_ASSERT(false);
    }
    }
    
    if (c == innerVarClause)
      break;
  }

  // Type checks
  xqtref_t outerType = predInfo.theOuterOp->get_return_type();
  xqtref_t innerType = predInfo.theInnerOp->get_return_type();
  xqtref_t primeOuterType = TypeOps::prime_type(tm, *outerType);
  xqtref_t primeInnerType = TypeOps::prime_type(tm, *innerType);
  TypeConstants::quantifier_t outerQuant = outerType->get_quantifier();
  TypeConstants::quantifier_t innerQuant = innerType->get_quantifier();
  const QueryLoc& innerLoc = predInfo.theInnerOp->get_loc();
  const QueryLoc& outerLoc = predInfo.theOuterOp->get_loc();

  if (!predInfo.theIsGeneral)
  {
    // Normally, other rewrite rules should have added the necessary casting
    // to the eq operands so that their static types have quantifiers ONE
    // or QUESTION. But just in case those rules have been disabled, we
    // check again here and reject the hashjoin rewrite if these conditions
    // are violated.

    if (innerQuant != TypeConstants::QUANT_ONE &&
        innerQuant != TypeConstants::QUANT_QUESTION)
      return false;

    if (outerQuant != TypeConstants::QUANT_ONE &&
        outerQuant != TypeConstants::QUANT_QUESTION)
      return false;

    // The type of the inner operand in the join predicate must not be
    // xs:anyAtomic.
    if (TypeOps::is_equal(tm, *primeInnerType, *rtm.ANY_ATOMIC_TYPE_ONE, innerLoc))
      return false;

    // The prime type of the outer operand in the join predicate must be a
    // subtype of the prime type of the inner operand.
    if (!TypeOps::is_subtype(tm, *primeOuterType, *primeInnerType, outerLoc))
      return false;
  }
  else
  {
    // TODO: allow domain exprs that return atomic items?
    expr* innerDomainExpr = predInfo.theInnerVar->get_forlet_clause()->get_expr();

    if (! TypeOps::is_subtype(tm,
                              *innerDomainExpr->get_return_type(),
                              *rtm.ANY_NODE_TYPE_STAR,
                              innerDomainExpr->get_loc()))
      return false;

    if (innerDomainExpr->getProducesDistinctNodes() != ANNOTATION_TRUE &&
        innerDomainExpr->getProducesDistinctNodes() != ANNOTATION_TRUE_FIXED)
      return false;
  }

  return true;
}


/*******************************************************************************
  Check if "curExpr" references a single flwor or catch var and that var is a
  LOOP var, i.e., a GROUPBY var or a FOR var whose domain expr has a max
  cardinality > 1. If so, return that LOOP var and its prefix id; otherwise
  return NULL. If "curExpr" references a single flwor or catch var and that
  var is LET var, the check is applied recurdively to the domain expr of the
  LET var.
********************************************************************************/
var_expr* IndexJoinRule::findLoopVar(expr* curExpr, ulong& varid)
{
  var_expr* var = NULL;

  while (true)
  {
    std::vector<ulong> varidSet;

    const DynamicBitset& bitset = (*theExprVarsMap)[curExpr];

    bitset.getSet(varidSet);

    if (varidSet.size() != 1)
      return NULL;

    varid = varidSet[0];
    var = (*theIdVarMap)[varid];
    var_expr::var_kind vkind = var->get_kind();

    if (vkind == var_expr::for_var)
    {
      curExpr = var->get_domain_expr();

      xqtref_t domainType = curExpr->get_return_type();
      TypeConstants::quantifier_t quant = domainType->get_quantifier();

      if (quant == TypeConstants::QUANT_STAR || quant == TypeConstants::QUANT_PLUS)
      {
        return var;
      }
      else if (quant == TypeConstants::QUANT_ONE)
      {
        // this FOR var is equivalent to a LET var, so we drill into its
        // domain expr
        continue;
      }
      else
      {
        return false;
      }
    }
    else if (vkind == var_expr::groupby_var)
    {
      return var;
    }
    else if (vkind == var_expr::let_var)
    {
      curExpr = var->get_domain_expr();
    }
    else
    {
      return NULL;
    }
  }

  return var;
}


/*******************************************************************************

********************************************************************************/
void IndexJoinRule::rewriteJoin(PredicateInfo& predInfo, bool& modified)
{
  // std::cout << "!!!!! Found Join Index Predicate !!!!!" << std::endl << std::endl;

  CompilerCB* ccb = theRootExpr->get_ccb();
  ExprManager* em = ccb->getExprManager();
  const QueryLoc& loc = predInfo.thePredicate->get_loc();
  static_context* sctx = predInfo.thePredicate->get_sctx();
  user_function* udf = predInfo.thePredicate->get_udf();
  for_clause* fc = predInfo.theInnerVar->get_forlet_clause();

  long maxInnerVarId = -1;

  // The index domain expr is the expr that defines the inner var, expanded, if
  // possible, so that it does not reference any variables defined after the outer
  // var (because the index must be built ouside the loop of the outer FOR var).
  // Note: must clone fc->get_expr() because expandVars modifies its input, but
  // fc->get_expr should not be modified, because we may discover later that the
  // rewrite is not possible after all,
  expr::substitution_t subst;
  expr* domainExpr = fc->get_expr()->clone(udf, subst);

  expr::subst_iter_t ite = subst.begin();
  expr::subst_iter_t end = subst.end();
  for (; ite != end; ++ite)
  {
    if (ite->first->get_expr_kind() == var_expr_kind)
    {
      assert(ite->second->get_expr_kind() == var_expr_kind);

      const var_expr* v = static_cast<const var_expr*>(ite->first);
      var_expr* var = const_cast<var_expr*>(v);
      var_expr* cloneVar = static_cast<var_expr*>(ite->second);

      assert(theVarIdMap->find(var) != theVarIdMap->end());

      (*theVarIdMap)[cloneVar] = (*theVarIdMap)[var];
    }
  }

  if (!expandVars(domainExpr, predInfo.theOuterVarId, maxInnerVarId))
    return;

  //
  // Create the index name and the "create-index()" expr
  //
  std::ostringstream os;
  os << "tempIndex" << ccb->theTempIndexCounter++;

  store::Item_t qname;
  GENV_ITEMFACTORY->createQName(qname, "", "", os.str().c_str());

  expr* qnameExpr(em->create_const_expr(sctx, udf, loc, qname));
  expr* buildExpr = NULL;

  function* f = BUILTIN_FUNC(OP_CREATE_INTERNAL_INDEX_2);
  fo_expr* createExpr = em->create_fo_expr(sctx, udf, loc, f, qnameExpr, buildExpr);

  //
  // Find where to place the create-index expr
  //
  if (maxInnerVarId >= 0)
  {
    // The domain expr depends on some flwor var that is defined before the outer
    // var. In this case, we find the flwor expr defining the inner-most var V
    // referenced by the domain expr of the index. Let F be this flwor expr. If
    // F does not define the outer var as well, then we create the index in the
    // return expr of F. Otherwise, we first break up F by creating a sub-flwor
    // expr (subF) and moving all clauses of F that appear after V's defining
    // clause into subF, making the return expr of F be the return expr of subF,
    // and setting subF as the return expr of F. Then, we create the index in
    // the return expr of F.

    const var_expr* mostInnerVar = (*theIdVarMap)[maxInnerVarId];
    flwor_clause* mostInnerVarClause = mostInnerVar->get_flwor_clause();

    flwor_expr* innerFlwor = NULL;
    ulong innerPosInStack = 0;
    ulong mostInnerVarPos = 0;

    if (!findFlworForVar(mostInnerVar,
                         innerFlwor,
                         mostInnerVarPos,
                         innerPosInStack))
      return;

    csize numClauses = innerFlwor->num_clauses();

    if (innerFlwor->defines_variable(predInfo.theOuterVar) >= 0 ||
        mostInnerVarPos < numClauses-1)
    {
      ZORBA_ASSERT(mostInnerVarPos < numClauses-1);

      for (csize i = mostInnerVarPos+1; i < numClauses; ++i)
      {
        if (innerFlwor->get_clause(i)->get_kind() == flwor_clause::count_clause)
          return;
      }

      const QueryLoc& nestedLoc = mostInnerVarClause->get_loc();

      flwor_expr* nestedFlwor = em->
      create_flwor_expr(sctx, udf, nestedLoc, false);

      for (csize i = mostInnerVarPos+1; i < numClauses; ++i)
      {
        nestedFlwor->add_clause(innerFlwor->get_clause(i));
      }

      for (csize i = numClauses - 1; i > mostInnerVarPos; --i)
      {
        innerFlwor->remove_clause(i);
      }

      nestedFlwor->set_return_expr(innerFlwor->get_return_expr());

      std::vector<expr*> args(2);
      args[0] = createExpr;
      args[1] = nestedFlwor;

      block_expr* seqExpr = em->
      create_block_expr(sctx, udf, loc, false, args, NULL);

      innerFlwor->set_return_expr(seqExpr);

      if (predInfo.theFlworExpr == innerFlwor)
      {
        // The where clause has moved to the nested flwor.
        predInfo.theFlworExpr = nestedFlwor;
      }
    }
    else
    {
      expr* returnExpr = innerFlwor->get_return_expr();

      if (returnExpr->get_expr_kind() == block_expr_kind)
      {
        block_expr* seqExpr = static_cast<block_expr*>(returnExpr);

        csize numArgs = seqExpr->size();
        csize arg;
        for (arg = 0; arg < numArgs; ++arg)
        {
          if ((*seqExpr)[arg]->get_function_kind() !=
              FunctionConsts::OP_CREATE_INTERNAL_INDEX_2)
          {
            break;
          }
        }

        seqExpr->add_at(arg, createExpr);
      }
      else
      {
        std::vector<expr*> args(2);
        args[0] = createExpr;
        args[1] = returnExpr;

        block_expr* seqExpr = em->
        create_block_expr(sctx, udf, loc, false, args, NULL);

        innerFlwor->set_return_expr(seqExpr);
      }
    }
  }
  else
  {
    // The inner domain expr does not reference any flwor vars. In this case,
    // find the flwor expr defining the outer var and create the index just
    // before this flwor.
    flwor_expr* outerFlworExpr = NULL;
    ulong outerPosInStack = 0;
    ulong dummy = 0;

    if (!findFlworForVar(predInfo.theOuterVar,
                         outerFlworExpr,
                         dummy,
                         outerPosInStack))
      return;

    // Build outer sequential expr
    std::vector<expr*> args(2);
    args[0] = createExpr;
    args[1] = outerFlworExpr;

    block_expr* seqExpr = em->create_block_expr(sctx, udf, loc, false, args, NULL);

    theFlworStack[outerPosInStack] = seqExpr;
  }

  modified = true;

  //
  // Replace the expr defining the inner var with an index probe.
  //
  fo_expr* probeExpr = NULL;

  if (predInfo.theIsGeneral)
  {
    probeExpr = em->
    create_fo_expr(sctx,
                   udf,
                   loc,
                   BUILTIN_FUNC(FN_ZORBA_XQDDF_PROBE_INDEX_POINT_GENERAL_N),
                   qnameExpr,
                   const_cast<expr*>(predInfo.theOuterOp));
    
    probeExpr = em->
    create_fo_expr(sctx,
                   udf,
                   loc,
                   BUILTIN_FUNC(OP_SORT_DISTINCT_NODES_ASC_1),
                   probeExpr);
  }
  else
  {
    probeExpr = em->
    create_fo_expr(sctx,
                   udf,
                   loc,
                   BUILTIN_FUNC(FN_ZORBA_XQDDF_PROBE_INDEX_POINT_VALUE_N),
                   qnameExpr,
                   const_cast<expr*>(predInfo.theOuterOp));
  }

  fc->set_expr(probeExpr);

  //
  // Create the IndexDecl obj
  //
  IndexDecl_t idx = new IndexDecl(sctx, ccb, loc, qname);

  if (predInfo.theIsGeneral)
    idx->setGeneral(true);

  idx->setTemp(true);

  idx->setDomainExpr(domainExpr);

  idx->setDomainVariable(theRCtx->createTempVar(sctx, loc, var_expr::for_var));

  idx->setDomainPositionVariable(theRCtx->createTempVar(sctx, loc, var_expr::pos_var));

  std::vector<expr*> columnExprs(1);
  std::vector<xqtref_t> columnTypes(1);
  std::vector<OrderModifier> modifiers(1);

  columnExprs[0] = predInfo.theInnerOp;

  columnTypes[0] = (predInfo.theIsGeneral ?
                    NULL :
                    predInfo.theInnerOp->get_return_type());

  modifiers[0].theAscending = true;
  modifiers[0].theEmptyLeast = true;
  modifiers[0].theCollation = sctx->get_default_collation(QueryLoc::null);

  expr_tools::replace_var(columnExprs[0], predInfo.theInnerVar, idx->getDomainVariable());

  idx->setKeyExpressions(columnExprs);

  idx->setKeyTypes(columnTypes);

  idx->setOrderModifiers(modifiers);

  sctx->bind_index(idx, loc);

  buildExpr = idx->getBuildExpr(loc);

  createExpr->set_arg(1, buildExpr);

  if (Properties::instance()->printIntermediateOpt())
  {
    std::cout << std::endl << idx->toString() << std::endl;
  }

#if 0
  if (predInfo.theIsGeneral)
    std::cout << "!!!!! Applied General Hash Join !!!!!" << std::endl << std::endl;
  else
    std::cout << "!!!!! Applied Value Hash Join !!!!!" << std::endl << std::endl;
#endif
}



/*******************************************************************************
  Return true if the given expr (a) does not reference any local vars and (b)
  does not reference any var V that is defined after the outer loop var, unless
  V is a LET var whose donmain expr recursively satisfies (a) and (b).
********************************************************************************/
bool IndexJoinRule::expandVars(expr* e, ulong outerVarId, long& maxVarId)
{
  if (e->get_expr_kind() == wrapper_expr_kind)
  {
    wrapper_expr* wrapper = static_cast<wrapper_expr*>(e);

    if (wrapper->get_input()->get_expr_kind() == var_expr_kind)
    {
      var_expr* var = static_cast<var_expr*>(wrapper->get_input());
      long varid = -1;

      if (theVarIdMap->find(var) != theVarIdMap->end())
      {
        varid = (*theVarIdMap)[var];
      }
      else if (var->get_kind() == var_expr::local_var)
      {
        // TODO: allow index domain exprs to reference local vars
        return false;
      }
      else
      {
        ZORBA_ASSERT(var->get_kind() == var_expr::prolog_var ||
                     var->get_kind() == var_expr::arg_var);
      }

      // If it is a variable that is defined after the outer var
      if (varid > (long)outerVarId)
      {
        if (var->get_kind() == var_expr::let_var)
        {
          wrapper->set_expr(var->get_forlet_clause()->get_expr());

          return expandVars(wrapper, outerVarId, maxVarId);
        }
        else if (var->get_kind() == var_expr::for_var)
        {
#if 1
          return false;
#else
          // TODO: to expand a FOR var, we must make sure that the expr is a
          // map w.r.t. that var.
          wrapper->set_expr(var->get_forlet_clause()->get_expr());
          return expandVars(wrapper, outerVarId, maxVarId);
#endif
        }
        else
        {
          return false;
        }
      }
      // Else, if it is a variable that is defined before the outer var
      else
      {
        if (varid > maxVarId)
          maxVarId = varid;

        return true;
      }
    }
  }
  else if (e->get_expr_kind() == var_expr_kind)
  {
    ZORBA_ASSERT(false);
  }

  ExprIterator iter(e);
  while (!iter.done())
  {
    if (!expandVars(**iter, outerVarId, maxVarId))
      return false;

    iter.next();
  }

  return true;
}


/*******************************************************************************
  Find the flwor expr defining the given var.
********************************************************************************/
bool IndexJoinRule::findFlworForVar(
    const var_expr* var,
    flwor_expr*& flworExpr,
    ulong& varPos,
    ulong& posInStack)
{
  flworExpr = NULL;

  csize numFlwors = theFlworStack.size();

  for (csize i = numFlwors; i > 0; --i)
  {
    if (theFlworStack[i-1]->get_expr_kind() == trycatch_expr_kind)
      return false;

    assert(theFlworStack[i-1]->get_expr_kind() == flwor_expr_kind ||
           theFlworStack[i-1]->get_expr_kind() == gflwor_expr_kind);

    flworExpr = static_cast<flwor_expr*>(theFlworStack[i-1]);

    if (i < numFlwors &&
        theInReturnClause[i-1] == true &&
        flworExpr->is_sequential())
      return false;

    // This condition is rather conservative and can be relaxed. TODO
    if (flworExpr->has_sequential_clauses())
      return false;

    long pos;
    if ((pos = flworExpr->defines_variable(var)) >= 0)
    {
      varPos = pos;
      posInStack = i - 1;
      break;
    }

    flworExpr = NULL;
  }

  assert(flworExpr != NULL);
  return true;
}


}
/* vim:set et sw=2 ts=2: */
