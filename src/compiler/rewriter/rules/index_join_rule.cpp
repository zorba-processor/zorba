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

#include "types/node_test.h"
#include "types/typeops.h"

#include <zorba/properties.h>

#include "diagnostics/assert.h"

#include <optional>


namespace zorba
{

struct PredicateInfo
{
  flwor_expr  * theFlworExpr;
  csize         theWherePos;
  expr        * thePredicate;

  expr        * theOuterOp;
  var_expr    * theOuterVar;
  csize         theOuterVarId;

  expr        * theInnerOp;
  var_expr    * theInnerVar;
  csize         theInnerVarId;

  bool          theIsGeneral;

  // True if the operands of a value comparison do not have static types that
  // allow indexing them directly (e.g., both are xs:anyAtomicType?, as is the
  // case for JSONiq navigation and for untyped XML). The index is then built
  // on a normalized xs:string key (see createNormalizedKey()) that is equal
  // for every pair of items for which "eq" may be true, and the predicate is
  // kept in the WHERE clause to filter out the extra candidates.
  bool          theNormalizeKeys;

  csize         theStackPos;
  csize         theChildPos;
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

  theIdVarMap->reserve(32);
  theIdVarMap->push_back(0); // because varids are starting from 1

  csize numVars = 0;
  expr_tools::index_flwor_vars(theRootExpr, numVars, *theVarIdMap, theIdVarMap);

  DynamicBitset freeset(numVars+1);
  expr_tools::build_expr_to_vars_map(theRootExpr,
                                     *theVarIdMap,
                                     freeset,
                                     *theExprVarsMap);
}


/*******************************************************************************

********************************************************************************/
IndexJoinRule::~IndexJoinRule()
{
  assert(theVarDefExprs.empty());
  assert(theChildPositions.empty());

  delete theVarIdMap;
  delete theIdVarMap;
  delete theExprVarsMap;
}


/*******************************************************************************
  Pushes a var-defining expr on theVarDefExprs and theChildPositions, and pops
  both when the scope ends, also when an exception leaves it.
********************************************************************************/
class IndexJoinRule::VarDefScope
{
  IndexJoinRule& theRule;

public:
  VarDefScope(IndexJoinRule& rule, expr* varDefExpr) : theRule(rule)
  {
    theRule.theVarDefExprs.push_back(varDefExpr);
    try
    {
      theRule.theChildPositions.push_back(0);
    }
    catch (...)
    {
      theRule.theVarDefExprs.pop_back();
      throw;
    }
  }

  VarDefScope(const VarDefScope&) = delete;
  VarDefScope& operator=(const VarDefScope&) = delete;

  ~VarDefScope()
  {
    theRule.theVarDefExprs.pop_back();
    theRule.theChildPositions.pop_back();
  }
};


/*******************************************************************************
  Whether an expr is or contains a point probe of an index: see rewriteJoin(),
  whose shape later rewrites may simplify.
********************************************************************************/
static bool containsIndexProbe(expr* e)
{
  if (e->get_function_kind() == FunctionConsts::FN_ZORBA_XQDDF_PROBE_INDEX_POINT_VALUE_N ||
      e->get_function_kind() == FunctionConsts::FN_ZORBA_XQDDF_PROBE_INDEX_POINT_GENERAL_N)
    return true;

  ExprIterator iter(e);
  while (!iter.done())
  {
    if (containsIndexProbe(**iter))
      return true;
    iter.next();
  }
  return false;
}


static bool isStringOrUntyped(const TypeManager* tm, const XQType& type)
{
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  return (TypeOps::is_subtype(tm, type, *rtm.STRING_TYPE_ONE) ||
          TypeOps::is_subtype(tm, type, *rtm.UNTYPED_ATOMIC_TYPE_ONE));
}


static bool isAtomicAndNeverUntyped(const TypeManager* tm, const XQType& type)
{
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  return (TypeOps::is_subtype(tm, type, *rtm.ANY_ATOMIC_TYPE_ONE) &&
          !TypeOps::is_subtype(tm, *rtm.UNTYPED_ATOMIC_TYPE_ONE, type));
}


/*******************************************************************************
  Whether "=" between an item of prime type "type1" and one of prime type
  "type2" cannot raise an error. A general index skips the pairs for which the
  comparison raises one (an untyped value whose cast to the type of the other
  item fails, or items of types that cannot be compared), so it selects what
  the comparison selects only when there are no such pairs.
********************************************************************************/
static bool generalEqualityRaisesNoError(
    const TypeManager* tm,
    const XQType& type1,
    const XQType& type2)
{
  // Compared as strings.
  if (isStringOrUntyped(tm, type1) && isStringOrUntyped(tm, type2))
    return true;

  if (TypeOps::is_numeric(tm, type1) && TypeOps::is_numeric(tm, type2))
    return true;

  // Compared as values of one type, with no untyped value to cast.
  return (isAtomicAndNeverUntyped(tm, type1) &&
          isAtomicAndNeverUntyped(tm, type2) &&
          (TypeOps::is_subtype(tm, type1, type2) ||
           TypeOps::is_subtype(tm, type2, type1)));
}


/*******************************************************************************
  Whether "eq" between an item of prime type "type1" and one of prime type
  "type2" cannot raise an error. "eq" compares an untyped value, an xs:string
  and an xs:anyURI as strings, and raises XPTY0004 for an untyped value and an
  item of another type; the pairs that "=" compares without an error compare
  so with "eq" too. A value index never matches the pairs for which "eq"
  raises an error, so it selects what "eq" selects only when there are none.
********************************************************************************/
static bool valueEqualityRaisesNoError(
    const TypeManager* tm,
    const XQType& type1,
    const XQType& type2)
{
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  if ((isStringOrUntyped(tm, type1) ||
       TypeOps::is_subtype(tm, type1, *rtm.ANY_URI_TYPE_ONE)) &&
      (isStringOrUntyped(tm, type2) ||
       TypeOps::is_subtype(tm, type2, *rtm.ANY_URI_TYPE_ONE)))
    return true;

  return generalEqualityRaisesNoError(tm, type1, type2);
}


/*******************************************************************************
  This rule analyzes the where clauses of flwor exprs to deterimne whether any
  predicate in a clause is a join predicate and whether the associated join
  can be converted into a hashjoin using an index that is built on-the-fly.
********************************************************************************/
expr* IndexJoinRule::apply(RewriterContext& rCtx, expr* node, bool& modified)
{
  assert(node != theRootExpr || theVarDefExprs.empty());
  assert(node != theRootExpr || theChildPositions.empty());

  flwor_expr* flworExpr = NULL;

  expr_kind_t nodeKind = node->get_expr_kind();

  std::optional<VarDefScope> flworScope;

  if (nodeKind == flwor_expr_kind)
  {
    flworExpr = static_cast<flwor_expr *>(node);

    flworScope.emplace(*this, flworExpr);

    csize numClauses = flworExpr->num_clauses();
    for (csize i = 0; i < numClauses; ++i)
    {
      flwor_clause* c = flworExpr->get_clause(i);

      if (c->get_kind() != flwor_clause::where_clause)
        continue;

      const where_clause* wc = static_cast<const where_clause *>(c);
      expr* whereExpr = wc->get_expr();

      // Only the first conjunct: the earlier ones decide for which tuples a later one is evaluated.
      PredicateInfo predInfo;
      predInfo.theFlworExpr = flworExpr;
      predInfo.theWherePos = i;
      predInfo.thePredicate =
        (whereExpr->get_function_kind() == FunctionConsts::OP_AND_N ?
         static_cast<fo_expr*>(whereExpr)->get_arg(0) :
         whereExpr);

      if (isIndexJoinPredicate(predInfo))
      {
        rewriteJoin(predInfo);
        modified = true;

        ZORBA_ASSERT(theVarDefExprs.back() == node);
        return node;
      }
    }
  }

  // No index-join rewrite done, so drill down.

  if (nodeKind == flwor_expr_kind)
  {
    csize numClauses = flworExpr->num_clauses();
    csize clausePos = 0;
    csize numClauseChildren;

    ExprIterator iter(node);
    while (!iter.done() && !modified)
    {
      theChildPositions.back() = clausePos;

      if (clausePos == numClauses)
      {
        numClauseChildren = 1;
      }
      else
      {
        flwor_clause* c = flworExpr->get_clause(clausePos);

        switch (c->get_kind())
        {
        case flwor_clause::for_clause:
        case flwor_clause::let_clause:
        case flwor_clause::where_clause:
        {
          numClauseChildren = 1;
          break;
        }
        case flwor_clause::window_clause:
        {
          window_clause* wc = static_cast<window_clause *>(c);
          numClauseChildren = 1;
          if (wc->get_win_start()) ++numClauseChildren;
          if (wc->get_win_stop()) ++numClauseChildren;
          break;
        }
        case flwor_clause::groupby_clause:
        {
          groupby_clause* gc = static_cast<groupby_clause *>(c);
          numClauseChildren = gc->numGroupingVars() + gc->numNonGroupingVars();
          break;
        }
        case flwor_clause::orderby_clause:
        {
          orderby_clause* oc = static_cast<orderby_clause *>(c);
          numClauseChildren = oc->num_columns();
          break;
        }
        case flwor_clause::count_clause:
        case flwor_clause::materialize_clause:
        {
          numClauseChildren = 0;
          break;
        }
        default:
          ZORBA_ASSERT(false);
        }
      }

      for (csize j = 0; j < numClauseChildren; ++j)
      {
        expr* currChild = **iter;
        expr* newChild = apply(rCtx, currChild, modified);
        ZORBA_ASSERT(currChild == newChild);

        if (modified)
          break;

        iter.next();
      }

      ++clausePos;
    }

    ZORBA_ASSERT(theVarDefExprs.back() == node);
    return node;
  }
  else if (nodeKind == trycatch_expr_kind)
  {
    VarDefScope trycatchScope(*this, node);

    ExprIterator iter(node);
    while (!iter.done())
    {
      expr* currChild = **iter;
      expr* newChild = apply(rCtx, currChild, modified);
      ZORBA_ASSERT(currChild == newChild);

      if (modified)
        break;

      iter.next();
    }

    return node;
  }
  else if (nodeKind == block_expr_kind)
  {
    VarDefScope blockScope(*this, node);

    block_expr* block = static_cast<block_expr*>(node);
    csize numArgs = block->size();
    for (csize i = 0; i < numArgs; ++i)
    {
      theChildPositions.back() = i;

      expr* currChild = (*block)[i];
      expr* newChild = apply(rCtx, currChild, modified);
      ZORBA_ASSERT(currChild == newChild);

      if (modified)
        break;
    }

    ZORBA_ASSERT(theVarDefExprs.back() == node);
    return node;
  }
  else
  {
    ExprIterator iter(node);
    while (!iter.done())
    {
      expr* currChild = **iter;
      expr* newChild = apply(rCtx, currChild, modified);
      ZORBA_ASSERT(currChild == newChild);
      
      if (modified)
        break;

      iter.next();
    }

    return node;
  }
}


/*******************************************************************************
  Check whether the given predicate is a join predicate that can be converted
  to a hashjoin.
********************************************************************************/
bool IndexJoinRule::isIndexJoinPredicate(PredicateInfo& predInfo)
{
  // The join would evaluate the domain of the inner var and the predicate once,
  // ahead of the tuples, and a call that can have a side effect, in this body or
  // in the code that consumes its result lazily, can change what they read
  // (CompilerCB::theNoHoisting).
  if (theRCtx->getCompilerCB()->theNoHoisting)
    return false;

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
  predInfo.theNormalizeKeys = false;

  expr* op1 = foExpr->get_arg(0);
  expr* op2 = foExpr->get_arg(1);

  // Analyze each operand of the eq to see if it depends on a single LOOP
  // variable. If that is not true, we reject this predicate.
  csize var1id;
  var_expr* var1 = findLoopVar(op1, var1id);
  if (var1 == NULL)
    return false;

  csize var2id;
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
    predInfo.theInnerVarId = var2id;
  }
  else
  {
    predInfo.theOuterOp = op2;
    predInfo.theOuterVar = var2;
    predInfo.theOuterVarId = var2id;
    predInfo.theInnerOp = op1;
    predInfo.theInnerVar = var1;
    predInfo.theInnerVarId = var1id;
  }

  static_context* sctx = predExpr->get_sctx();
  TypeManager* tm = sctx->get_typemanager();
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  // Make sure that the inner loop var has a domain expression
  if (predInfo.theInnerVar->get_kind() != var_expr::for_var)
    return false;

  // The inner operand becomes the index key expr, where only the inner var is
  // replaced by the index domain var. So, the inner operand must reference the
  // inner var directly: findLoopVar() may have reached it through a LET var or
  // a FOR var over a single item (e.g. after FLWOR unnesting), and such a var
  // is not in scope where the index is built.
  {
    std::vector<csize> innerOpVars;
    (*theExprVarsMap)[predInfo.theInnerOp].getSet(innerOpVars);

    if (innerOpVars.size() != 1 || innerOpVars[0] != predInfo.theInnerVarId)
      return false;
  }

  if (predInfo.theInnerVar->get_flwor_clause() ==
      predInfo.theOuterVar->get_flwor_clause())
    return false;

  forlet_clause* innerVarClause = predInfo.theInnerVar->get_forlet_clause();

  if (innerVarClause->get_pos_var() != NULL)
    return false;

  // The inner var is already bound to an index probe (the predicate is kept in
  // the WHERE clause after a rewrite).
  if (containsIndexProbe(innerVarClause->get_expr()))
    return false;

  // The inner var must not be an outer FOR var
  if (innerVarClause->is_allowing_empty())
    return false;

  // The predicate must be in the same flwor that defines the inner var (this
  // way we can be sure that the pred acts as a filter over the inner var).
  if (!predInfo.theFlworExpr->defines_var(predInfo.theInnerVar))
    return false;

  // A clause between the inner var clause and the WHERE clause would see only
  // the tuples of the probe, not every inner tuple. Only a LET of a hoisted
  // expr may be there: op:hoist keeps an error until the value is used.
  for (csize i = predInfo.theWherePos; ; --i)
  {
    if (i == 0)
      return false;

    flwor_clause* c = predInfo.theFlworExpr->get_clause(i-1);

    if (c == innerVarClause)
      break;

    if (c->get_kind() != flwor_clause::let_clause ||
        static_cast<forlet_clause*>(c)->get_expr()->get_function_kind() !=
        FunctionConsts::OP_HOIST_1)
      return false;
  }

  if (innerVarClause->get_expr()->is_sequential())
    return false;

  // Let OV be the outer LOOP var and BV be the inner-most non-LET variable that
  // the index expression depends on. The index must be created somewhere between
  // BV anmd OV. The checkVarDeps method finds BV and checks that BV is defined
  // before OV.
  expr* innerDomainExpr = innerVarClause->get_expr();

  // The join evaluates the domain of the inner var once, not once per outer tuple, and
  // each operand of the predicate once per item, not once per pair: a call that can
  // give a different result or new nodes per evaluation must not be in them.
  if (expr_tools::call_results_can_differ(innerDomainExpr) ||
      expr_tools::call_results_can_differ(predInfo.thePredicate))
    return false;

  csize boundaryVarId = 0;
  if (checkVarDeps(innerDomainExpr, predInfo.theOuterVarId, boundaryVarId))
    return false;

  // Check whether it is possible to place the index creation somewhere between
  // BV and OV, and if so, determine the exact location.
  if (!findIndexPos(predInfo, boundaryVarId))
    return false;

  // Type checks
  xqtref_t outerType = predInfo.theOuterOp->get_return_type();
  xqtref_t innerType = predInfo.theInnerOp->get_return_type();
  xqtref_t primeOuterType = TypeOps::prime_type(tm, *outerType);
  xqtref_t primeInnerType = TypeOps::prime_type(tm, *innerType);
  SequenceType::Quantifier outerQuant = outerType->get_quantifier();
  SequenceType::Quantifier innerQuant = innerType->get_quantifier();
  const QueryLoc& innerLoc = predInfo.theInnerOp->get_loc();
  const QueryLoc& outerLoc = predInfo.theOuterOp->get_loc();

  if (!predInfo.theIsGeneral)
  {
    // Normally, other rewrite rules should have added the necessary casting
    // to the eq operands so that their static types have quantifiers ONE
    // or QUESTION. But just in case those rules have been disabled, we
    // check again here and reject the hashjoin rewrite if these conditions
    // are violated.

    if (innerQuant != SequenceType::QUANT_ONE &&
        innerQuant != SequenceType::QUANT_QUESTION)
      return false;

    if (outerQuant != SequenceType::QUANT_ONE &&
        outerQuant != SequenceType::QUANT_QUESTION)
      return false;

    if (!valueEqualityRaisesNoError(tm, *primeInnerType, *primeOuterType))
      return false;

    // The index key type is the type of the inner operand, so it must not be
    // xs:anyAtomic, and the prime type of the outer operand must be a subtype
    // of it. Otherwise, if both operands are atomic, index normalized keys.
    if (TypeOps::is_equal(tm, *primeInnerType, *rtm.ANY_ATOMIC_TYPE_ONE, innerLoc) ||
        !TypeOps::is_subtype(tm, *primeOuterType, *primeInnerType, outerLoc))
    {
      if (!TypeOps::is_subtype(tm, *primeInnerType, *rtm.ANY_ATOMIC_TYPE_ONE, innerLoc) ||
          !TypeOps::is_subtype(tm, *primeOuterType, *rtm.ANY_ATOMIC_TYPE_ONE, outerLoc))
        return false;

      predInfo.theNormalizeKeys = true;
    }
  }
  else
  {
    // A general index holds numbers as xs:double, but an xs:integer or xs:decimal
    // compared with an xs:float is cast to xs:float (XPath 3.0 B.1), which an
    // index lookup can not express.
    bool const innerFloat = TypeOps::is_subtype(tm, *primeInnerType, *rtm.FLOAT_TYPE_ONE, innerLoc);
    bool const outerFloat = TypeOps::is_subtype(tm, *primeOuterType, *rtm.FLOAT_TYPE_ONE, outerLoc);
    if (innerFloat != outerFloat)
      return false;

    if (!generalEqualityRaisesNoError(tm, *primeInnerType, *primeOuterType))
      return false;

    // TODO: allow domain exprs that return atomic items?
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
  Check if "curExpr" references a single var and that var is a LOOP var, i.e.,
  a GROUPBY var or a FOR var whose domain expr has a max cardinality > 1. If so,
  return that LOOP var and its prefix id; otherwise return NULL. If "curExpr"
  references a single LET var, the check is applied recurdively to the domain
  expr of the LET var.

  TODO: it is possible to relax the conditions to allow curExpr to reference
  a single LOOP var plus local/global vars. But then we must make sure we
  don't cross these additional var when we look for the place where to put the
  index creation.  
********************************************************************************/
var_expr* IndexJoinRule::findLoopVar(expr* curExpr, csize& varid)
{
  var_expr* var = NULL;

  while (true)
  {
    std::vector<csize> varidSet;

    const DynamicBitset& bitset = (*theExprVarsMap)[curExpr];

    bitset.getSet(varidSet);

    if (varidSet.size() != 1)
      return NULL;

    varid = varidSet[0];
    var = (*theIdVarMap)[varid];

    switch (var->get_kind())
    {
    case var_expr::for_var:
    case var_expr::pos_var:
    {
      for_clause* fc = var->get_forlet_clause();
      curExpr = fc->get_expr();

      xqtref_t domainType = curExpr->get_return_type();
      SequenceType::Quantifier quant = domainType->get_quantifier();

      if (quant == SequenceType::QUANT_STAR || quant == SequenceType::QUANT_PLUS)
      {
        return var;
      }
      else if (quant == SequenceType::QUANT_ONE)
      {
        // this FOR var is equivalent to a LET var, so we drill into its
        // domain expr
        continue;
      }
      else
      {
        return NULL;
      }
    }
    case var_expr::groupby_var:
    case var_expr::wincond_out_var:
    case var_expr::wincond_out_pos_var:
    case var_expr::wincond_in_var:
    case var_expr::wincond_in_pos_var:
    {
      return var;
    }
    case var_expr::let_var:
    {
      curExpr = var->get_domain_expr();
      break;
    }
    default:
    {
      return NULL;
    }
    }
  }

  return var;
}


/*******************************************************************************
  Return true iff "idxExpr" references the outer loop var OV or any other var
  that is defined after OV. Also find and return the id of the "boundary" var
  BV that is the inner-most variable that the indExp depends on.

  NOTE: for the purposes of this method, LET vars are "transparent", i.e., the
  method is applied recursively on the domain expr of each LET var that is
  referenced by idxExpr. 
********************************************************************************/
bool IndexJoinRule::checkVarDeps(
    expr* idxExpr,
    csize outerVarId,
    csize& boundVarId)
{
  const DynamicBitset& bitset = (*theExprVarsMap)[idxExpr];

  csize numVars = bitset.size();
  for (csize varid = 0; varid < numVars; ++varid)
  {
    if (!bitset.get(varid))
      continue;

    if (varid >= outerVarId)
    {
      if (varid == outerVarId)
        return true;

      const var_expr* var = (*theIdVarMap)[varid];

      if (var->get_kind() == var_expr::let_var)
      {
        expr* domExpr = var->get_forlet_clause()->get_expr();
        if (checkVarDeps(domExpr, outerVarId, boundVarId))
          return true;
      }
      else
      {
        return true;
      }
    }
    else if (varid > boundVarId)
    {
      boundVarId = varid;
    }
  }

  return false;
}


/*******************************************************************************

********************************************************************************/
bool IndexJoinRule::findIndexPos(PredicateInfo& predInfo, csize boundVarId)
{
  csize numStackedExprs = theVarDefExprs.size();

  //
  // Find the position of the outer loop var
  //
  var_expr* outerVar = predInfo.theOuterVar;
  flwor_expr* outerFlwor = outerVar->get_flwor_clause()->get_flwor_expr();
  csize outerStackPos = numStackedExprs;
  csize outerVarPos; 

  bool found = outerFlwor->get_var_pos(outerVar, outerVarPos);
  ZORBA_ASSERT(found);

  for (csize i = 0; i < numStackedExprs; ++i)
  {
    if (theVarDefExprs[i] == outerFlwor)
    {
      outerStackPos = i;
      break;
    }
  }

  assert(outerStackPos < numStackedExprs);

  //
  // Find the position of the inner loop var
  //
  var_expr* innerVar = predInfo.theInnerVar;
  flwor_expr* innerFlwor = innerVar->get_flwor_clause()->get_flwor_expr();
  csize innerStackPos = numStackedExprs - 1;
  csize innerVarPos; 

  found = innerFlwor->get_var_pos(innerVar, innerVarPos);
  ZORBA_ASSERT(found);

  //
  // Make sure that there are no sequential clauses/exprs and no trycatch exprs
  // in the path between the inner and the outer loop vars.
  //
  theChildPositions[numStackedExprs-1] = innerVarPos;

  for (csize i = outerStackPos; i <= innerStackPos; ++i)
  {
    expr* currExpr = theVarDefExprs[i];
    expr_kind_t currKind = currExpr->get_expr_kind();

    if (currKind == flwor_expr_kind)
    {
      flwor_expr* flwor = static_cast<flwor_expr*>(currExpr);
      csize numClauses = flwor->num_clauses();
      csize startClause;
      csize stopClause;
      bool checkReturn = false;

      if (outerStackPos == innerStackPos)
      {
        startClause = outerVarPos;
        stopClause = innerVarPos;
      }
      else if (i == outerStackPos)
      {
        startClause = outerVarPos;
        stopClause = numClauses - 1;
        checkReturn = true;
      }
      else
      {
        startClause = 0;
        stopClause = theChildPositions[i];
        if (stopClause == numClauses)
        {
          --stopClause;
          checkReturn = true;
        }
      }

      for (csize j = startClause; j <= stopClause; ++j)
      {
        flwor_clause* c = flwor->get_clause(j);

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
        default:
          break;
        }
      }

      if (checkReturn && flwor->get_return_expr()->is_sequential())
        return false;
    }
    else if (currKind == trycatch_expr_kind)
    {
      return false;
    }
    else if (currKind == block_expr_kind)
    {
      block_expr* block = static_cast<block_expr*>(currExpr);

      for (csize j = 0; j <= theChildPositions[i]; ++j)
      {
        if ((*block)[j]->is_sequential())
          return false;
      } 
    }
  }

  //
  // Find the position of the boundary var
  //
  var_expr* boundVar = NULL; 
  expr* boundExpr = NULL;
  csize boundStackPos = numStackedExprs; // not found; checked by the assert below
  csize boundVarPos; 

  if (boundVarId == 0)
  {
    boundStackPos = 0;
    boundVarPos = 0;
  }
  else
  {
    boundVar = (*theIdVarMap)[boundVarId];
    ZORBA_ASSERT(boundVar);

    if (boundVar->get_kind() == var_expr::catch_var)
    {
      // Not worth bothering about index exprs accessing catch vars
      return false; 
    }
    else if (boundVar->get_kind() == var_expr::prolog_var ||
             boundVar->get_kind() == var_expr::local_var)
    {
      block_expr* blockExpr = boundVar->get_block_expr();
      ZORBA_ASSERT(blockExpr);
      boundExpr = blockExpr;

      found = blockExpr->get_var_pos(boundVar, boundVarPos);
      ZORBA_ASSERT(found);
    }
    else
    {
      flwor_clause* boundClause = boundVar->get_flwor_clause();
      ZORBA_ASSERT(boundClause);
      flwor_expr* boundFlwor = boundClause->get_flwor_expr();
      boundExpr = boundFlwor;
      
      found = boundFlwor->get_var_pos(boundVar, boundVarPos);
      ZORBA_ASSERT(found);
    }
    
    for (csize i = 0; i < numStackedExprs; ++i)
    {
      if (theVarDefExprs[i] == boundExpr)
      {
        boundStackPos = i;
        break;
      }
    }
  }

  ZORBA_ASSERT(boundStackPos < numStackedExprs);

  //
  // Try to place the index creation as early as possible, but without crossing
  // any sequential expr or trycatch expr or the definition of the boundary var.
  //
  theChildPositions[outerStackPos] = outerVarPos;

  for (csize i = outerStackPos; i >= boundStackPos; --i)
  {
    expr* currExpr = theVarDefExprs[i];
    expr_kind_t currKind = currExpr->get_expr_kind();

    if (currKind == flwor_expr_kind)
    {
      flwor_expr* flwor = static_cast<flwor_expr*>(currExpr);
      csize numClauses = flwor->num_clauses();
      csize stopClause = 0;
      csize startClause = theChildPositions[i];
      bool checkReturn = false;

      if (startClause == numClauses)
      {
        checkReturn = true;
        --startClause;
      }

      if (checkReturn && flwor->get_return_expr()->is_sequential())
      {
        predInfo.theStackPos = i+1;
        predInfo.theChildPos = 0;
        return true;
      }

      for (csize j = startClause+1; j > stopClause; --j)
      {
        if (i == boundStackPos && j-1 == boundVarPos)
        {
          predInfo.theStackPos = i;
          predInfo.theChildPos = (boundVarId == 0 ? j-1 : j);
          return true;
        }

        flwor_clause* c = flwor->get_clause(j-1);

        switch (c->get_kind())
        {
        case flwor_clause::for_clause:
        case flwor_clause::let_clause:
        case flwor_clause::window_clause:
        {
          forletwin_clause* flwc = static_cast<forletwin_clause*>(c);

          if (flwc->get_expr()->is_sequential())
          {
            predInfo.theStackPos = i;
            predInfo.theChildPos = j;
            return true;
          }
          break;
        }
        default:
          break;
        }
      }
    }
    else if (currKind == trycatch_expr_kind)
    {
      predInfo.theStackPos = i+1;
      predInfo.theChildPos = 0;
      return true;
    }
    else if (currKind == block_expr_kind)
    {
      block_expr* block = static_cast<block_expr*>(currExpr);
      csize stopArg = 0;
      csize startArg = theChildPositions[i];

      for (csize j = startArg+1; j > stopArg; --j)
      {
        csize arg = j-1;

        if ((*block)[arg]->is_sequential() ||
            (i == boundStackPos && arg == boundVarPos))
        {
          predInfo.theStackPos = i;
          predInfo.theChildPos = j;
          return true;
        }
      }
    }

    if (i == 0)
      break;
  }

  predInfo.theStackPos = 0;
  predInfo.theChildPos = 0;
  return true;
}


/*******************************************************************************
  Wrap "keyExpr" (of static type xs:anyAtomicType?) into an expr that computes
  a normalized xs:string key for it:

    let $k := keyExpr
    return
      if (empty($k)) then ()
      else if ($k instance of xs:float) then "f"
      else if ($k instance of xs:decimal or $k instance of xs:double)
      then concat("n", string(abs(xs:double($k))))
      else if ($k instance of xs:string or $k instance of xs:untypedAtomic or
               $k instance of xs:anyURI)
      then concat("s", string($k))
      else "o"

  If "$a eq $b" can be true and neither is an xs:float, then the keys of $a and
  $b are equal: other numbers are compared via promotion to xs:double, so equal
  numbers have equal xs:double values; xs:untypedAtomic and xs:anyURI are
  compared as xs:string (the index uses the default collation, like "eq"). Items
  of other types share the key "o". The converse does not hold, so the join
  predicate must still be applied to the probe results. Pairs for which "eq"
  would raise a type error get different keys and are never compared, so the
  rule uses these keys only for operand types for which "eq" raises no error
  (valueEqualityRaisesNoError).

  An xs:integer or xs:decimal compared with an xs:float is cast to xs:float
  (XPath 3.0 B.1): 16777217 eq xs:float(16777216). One key can not cover both a
  number's xs:double and its xs:float rounding, and keying all numbers on their
  xs:float rounding would put up to 2^(e-23) integers around 2^e on one key. So
  inner xs:float items share the key "f", and createNormalizedProbe() scans the
  domain when an xs:float takes part.
********************************************************************************/
expr* IndexJoinRule::createNormalizedKey(
    expr* keyExpr,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc)
{
  ExprManager* em = theRootExpr->get_ccb()->getExprManager();
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  var_expr* keyVar = theRCtx->createTempVar(sctx, loc, var_expr::let_var);
  keyVar->set_type(keyExpr->get_return_type());

  expr* isFloat = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.FLOAT_TYPE_ONE);

  std::vector<expr*> numArgs(2);
  numArgs[0] = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.DECIMAL_TYPE_ONE);
  numArgs[1] = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.DOUBLE_TYPE_ONE);
  expr* isNumeric = em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(OP_OR_N), numArgs);

  std::vector<expr*> strArgs(3);
  strArgs[0] = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.STRING_TYPE_ONE);
  strArgs[1] = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.UNTYPED_ATOMIC_TYPE_ONE);
  strArgs[2] = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.ANY_URI_TYPE_ONE);
  expr* isString = em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(OP_OR_N), strArgs);

  expr* numValue =
  em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_STRING_1),
    em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_ABS_1),
      em->create_cast_expr(sctx, udf, loc, keyVar, rtm.DOUBLE_TYPE_ONE, false)));

  std::vector<expr*> numKeyArgs(2);
  numKeyArgs[0] = em->create_const_expr(sctx, udf, loc, "n");
  numKeyArgs[1] = numValue;
  expr* numKey = em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_CONCAT_N), numKeyArgs);

  std::vector<expr*> strKeyArgs(2);
  strKeyArgs[0] = em->create_const_expr(sctx, udf, loc, "s");
  strKeyArgs[1] = em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_STRING_1), keyVar);
  expr* strKey = em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_CONCAT_N), strKeyArgs);

  expr* body =
  em->create_if_expr(sctx, udf, loc,
    em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_EMPTY_1), keyVar),
    em->create_seq(sctx, udf, loc),
    em->create_if_expr(sctx, udf, loc,
      isFloat,
      em->create_const_expr(sctx, udf, loc, "f"),
      em->create_if_expr(sctx, udf, loc,
        isNumeric,
        numKey,
        em->create_if_expr(sctx, udf, loc,
          isString,
          strKey,
          em->create_const_expr(sctx, udf, loc, "o")))));

  flwor_expr* flwor = em->create_flwor_expr(sctx, udf, loc);
  flwor->add_clause(em->create_let_clause(sctx, loc, keyVar, keyExpr));
  flwor->set_return_expr(body);

  return flwor;
}


/*******************************************************************************
  The expr that replaces the domain of the inner var, for normalized keys:

    let $k := outerKeyExpr
    return
      if ($k instance of xs:float or exists(probe-index-point-value(index, "f")))
      then scanExpr
      else probe-index-point-value(index, normalized key of $k)

  Without any xs:float, this costs a second probe for each outer tuple. With one,
  the join runs as a nested loop, which the predicate kept in the WHERE clause
  filters, in the order of the domain.
********************************************************************************/
expr* IndexJoinRule::createNormalizedProbe(
    expr* qnameExpr,
    expr* outerKeyExpr,
    expr* scanExpr,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc)
{
  ExprManager* em = theRootExpr->get_ccb()->getExprManager();
  RootTypeManager& rtm = GENV_TYPESYSTEM;

  var_expr* keyVar = theRCtx->createTempVar(sctx, loc, var_expr::let_var);
  keyVar->set_type(outerKeyExpr->get_return_type());

  // First: the type of keyVar, asked for below, comes from its clause.
  flwor_expr* flwor = em->create_flwor_expr(sctx, udf, loc);
  flwor->add_clause(em->create_let_clause(sctx, loc, keyVar, outerKeyExpr));

  expr* floatProbe =
  em->create_fo_expr(sctx, udf, loc,
                     BUILTIN_FUNC(FN_ZORBA_XQDDF_PROBE_INDEX_POINT_VALUE_N),
                     qnameExpr,
                     em->create_const_expr(sctx, udf, loc, "f"));

  std::vector<expr*> args(2);
  args[0] = em->create_instanceof_expr(sctx, udf, loc, keyVar, rtm.FLOAT_TYPE_ONE);
  args[1] = em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(FN_EXISTS_1), floatProbe);

  expr* keyProbe =
  em->create_fo_expr(sctx, udf, loc,
                     BUILTIN_FUNC(FN_ZORBA_XQDDF_PROBE_INDEX_POINT_VALUE_N),
                     qnameExpr,
                     createNormalizedKey(keyVar, sctx, udf, loc));

  flwor->set_return_expr(
    em->create_if_expr(sctx, udf, loc,
                       em->create_fo_expr(sctx, udf, loc, BUILTIN_FUNC(OP_OR_N), args),
                       scanExpr,
                       keyProbe));
  return flwor;
}


/*******************************************************************************
  "try { tryExpr } catch * { catchExpr }"
********************************************************************************/
static expr* createCatchAll(
    ExprManager* em,
    expr* tryExpr,
    expr* catchExpr,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc)
{
  trycatch_expr* trycatch = em->create_trycatch_expr(sctx, udf, loc, tryExpr);

  catch_clause* clause = em->create_catch_clause();
  clause->add_nametest_h(new NodeNameTest(zstring(), zstring()));

  trycatch->add_clause(clause);
  trycatch->add_catch_expr(catchExpr);

  return trycatch;
}


/*******************************************************************************
  The index evaluates the inner domain and the inner operand of the predicate
  before the outer tuples, and the outer operand once per outer tuple, also
  where the query without the index evaluates neither: for an empty domain,
  under an empty outer domain, after fn:exists() has its first item. An error
  they raise there is not a result, so:

    let $dummy := create-index(...)
    ...
    for $innerVar in try { probe(index, outerOp) } catch * { domain }
    where predicate

  The predicate stays in the WHERE clause. A build that raises leaves the index
  unbound (CreateInternalIndexIterator), so every probe raises; an outer operand
  that raises fails one probe. A failed probe gives the whole domain, and the
  predicate evaluates what the query without the index evaluates, errors
  included. The domain and the operands are deterministic (see
  isIndexJoinPredicate()), so evaluating them again changes nothing else.
********************************************************************************/
void IndexJoinRule::rewriteJoin(PredicateInfo& predInfo)
{
  CompilerCB* ccb = theRootExpr->get_ccb();
  ExprManager* em = ccb->getExprManager();
  static_context* sctx = predInfo.thePredicate->get_sctx();
  user_function* udf = predInfo.thePredicate->get_udf();
  const QueryLoc& loc = predInfo.theInnerVar->get_loc();

  for_clause* innerClause = predInfo.theInnerVar->get_forlet_clause();

  //
  // The index domain expr is the expr that defines the inner var, expanded so
  // that it does not reference any LET variables defined after the outer var
  // 
  expr* indexDomainExpr = innerClause->get_expr();
  expandVars(indexDomainExpr, predInfo.theOuterVarId, predInfo.theInnerVarId);

  expr* scanExpr = indexDomainExpr->clone(udf);

  //
  // Create the index declaration
  //
  std::ostringstream os;
  os << "tempIndex" << GENV_ROOT_STATIC_CONTEXT.create_temporary_index_id();

  store::Item_t qname;
  GENV_ITEMFACTORY->createQName(qname, "", "", os.str().c_str());

  IndexDecl_t idx = new IndexDecl(sctx, ccb, loc, qname);

  if (predInfo.theIsGeneral)
    idx->setGeneral(true);

  idx->setTemp(true);

  idx->setDomainExpr(indexDomainExpr);

  idx->setDomainVariable(theRCtx->createTempVar(sctx, loc, var_expr::for_var));

  idx->setDomainPositionVariable(theRCtx->createTempVar(sctx, loc, var_expr::pos_var));

  std::vector<expr*> columnExprs(1);
  std::vector<xqtref_t> columnTypes(1);
  std::vector<OrderModifier> modifiers(1);

  // The predicate stays in the WHERE clause, so its operands are cloned.
  expr::substitution_t subst;
  subst[predInfo.theInnerVar] = idx->getDomainVariable();
  expr* indexKeyExpr = predInfo.theInnerOp->clone(udf, subst);

  expr* probeKeyExpr = predInfo.theOuterOp->clone(udf);

  if (predInfo.theNormalizeKeys)
  {
    columnExprs[0] = createNormalizedKey(indexKeyExpr, sctx, udf, loc);
    columnTypes[0] = GENV_TYPESYSTEM.STRING_TYPE_QUESTION;
  }
  else
  {
    columnExprs[0] = indexKeyExpr;

    columnTypes[0] = (predInfo.theIsGeneral ?
                      NULL :
                      predInfo.theInnerOp->get_return_type());
  }

  modifiers[0].theAscending = true;
  modifiers[0].theEmptyLeast = true;
  modifiers[0].theCollation = sctx->get_default_collation(QueryLoc::null);

  idx->setKeyExpressions(columnExprs);

  idx->setKeyTypes(columnTypes);

  idx->setOrderModifiers(modifiers);

  //
  // Create the "create-index()" expr
  //
  expr* qnameExpr = em->create_const_expr(sctx, udf, loc, qname);

  expr* buildExpr = idx->getBuildExpr(loc);

  function* f = BUILTIN_FUNC(OP_CREATE_INTERNAL_INDEX_2);
  fo_expr* createExpr = em->create_fo_expr(sctx, udf, loc, f, qnameExpr, buildExpr);

  DynamicBitset indexVars;
  expr_tools::build_expr_to_vars_map(createExpr,
                                     *theVarIdMap,
                                     indexVars, 
                                     *theExprVarsMap);

  //
  // Place the create-index expr to its target position
  //
  expr* targetExpr = theVarDefExprs[predInfo.theStackPos];
  csize targetPos = predInfo.theChildPos;
  expr_kind_t targetKind = targetExpr->get_expr_kind();

  if (targetKind == block_expr_kind)
  {
    block_expr* block = static_cast<block_expr*>(targetExpr);

    block->add(targetPos, createExpr);
  }
  else if (targetKind == flwor_expr_kind)
  {
    flwor_expr* flwor = static_cast<flwor_expr*>(targetExpr);
    csize numClauses = flwor->num_clauses();

    if (targetPos == numClauses)
    {
      std::vector<expr*> args(2);
      args[0] = createExpr;
      args[1] = flwor->get_return_expr();

      block_expr* block = em->create_block_expr(sctx, udf, loc, false, args, NULL);

      DynamicBitset blockVars = indexVars;
      blockVars.set_union((*theExprVarsMap)[flwor->get_return_expr()]);
      (*theExprVarsMap)[block] = blockVars;

      flwor->set_return_expr(block);
    }
    else
    {
      var_expr* dummyVar = theRCtx->createTempVar(sctx, loc, var_expr::let_var);

      let_clause* createClause = 
      em->create_let_clause(sctx, loc, dummyVar, createExpr, false);

      flwor->add_clause(targetPos, createClause);
    }
  }
  else
  {
    ZORBA_ASSERT(false);
  }

  //
  // Replace the expr defining the inner var with an index probe.
  //
  expr* probeExpr = NULL;

  if (predInfo.theIsGeneral)
  {
    probeExpr = em->
    create_fo_expr(sctx,
                   udf,
                   loc,
                   BUILTIN_FUNC(FN_ZORBA_XQDDF_PROBE_INDEX_POINT_GENERAL_N),
                   qnameExpr,
                   probeKeyExpr);
    
    probeExpr = em->
    create_fo_expr(sctx,
                   udf,
                   loc,
                   BUILTIN_FUNC(OP_SORT_DISTINCT_NODES_ASC_1),
                   probeExpr);
  }
  else if (predInfo.theNormalizeKeys)
  {
    probeExpr = createNormalizedProbe(qnameExpr, probeKeyExpr, scanExpr->clone(udf),
                                      sctx, udf, loc);
  }
  else
  {
    probeExpr = em->
    create_fo_expr(sctx,
                   udf,
                   loc,
                   BUILTIN_FUNC(FN_ZORBA_XQDDF_PROBE_INDEX_POINT_VALUE_N),
                   qnameExpr,
                   probeKeyExpr);
  }

  probeExpr = createCatchAll(em, probeExpr, scanExpr, sctx, udf, loc);

  DynamicBitset probeVars;
  expr_tools::build_expr_to_vars_map(probeExpr,
                                     *theVarIdMap,
                                     probeVars,
                                     *theExprVarsMap);

  innerClause->set_expr(probeExpr);

  sctx->bind_index(idx, loc);

  if (Properties::instance().getPrintIntermediateOpt())
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

********************************************************************************/
void IndexJoinRule::expandVars(expr* e, csize outerVarId, csize innerVarId)
{
  if (e->get_expr_kind() == wrapper_expr_kind)
  {
    wrapper_expr* wrapper = static_cast<wrapper_expr*>(e);

    if (wrapper->get_input()->get_expr_kind() == var_expr_kind)
    {
      var_expr* var = static_cast<var_expr*>(wrapper->get_input());
      csize varid = 0;

      ZORBA_ASSERT(theVarIdMap->find(var) != theVarIdMap->end());
      
      varid = (*theVarIdMap)[var];

      // If it is a variable that is defined after the outer var
      if (outerVarId <= varid && varid < innerVarId)
      {
        if (var->get_kind() == var_expr::let_var)
        {
          wrapper->set_expr(var->get_forlet_clause()->get_expr());

          return expandVars(wrapper, outerVarId, innerVarId);
        }
        else if (var->get_kind() == var_expr::for_var)
        {
#if 1
          ZORBA_ASSERT(false);
#else
          // TODO: to expand a FOR var, we must make sure that the expr is a
          // map w.r.t. that var.
          wrapper->set_expr(var->get_forlet_clause()->get_expr());
          return expandVars(wrapper, outerVarId, maxVarId);
#endif
        }
        else
        {
          ZORBA_ASSERT(false);
        }
      }
    }
  }
  else if (e->get_expr_kind() == var_expr_kind)
  {
    ZORBA_ASSERT(false);
  }
  else
  {
    ExprIterator iter(e);
    while (!iter.done())
    {
      expandVars(**iter, outerVarId, innerVarId);
      iter.next();
    }
  }
}


}
/* vim:set et sw=2 ts=2: */
