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

#include "runtime/function_item/function_item.h"
#include "runtime/core/fncall_iterator.h"
#include "runtime/base/plan_iterator.h"

#include "compiler/api/compilercb.h"
#include "compiler/expression/var_expr.h"
#include "compiler/expression/function_item_expr.h"
#include "compiler/expression/expr_manager.h"

#include "functions/signature.h"
#include "functions/udf.h"

#include "zorbaserialization/serialize_template_types.h"
#include "zorbaserialization/serialize_zorba_types.h"

// TODO: delete, debugging purposes
#include "runtime/visitors/printer_visitor_api.h"
#include "runtime/visitors/iterprinter.h"


namespace zorba
{

SERIALIZABLE_CLASS_VERSIONS(DynamicFunctionInfo)

SERIALIZABLE_CLASS_VERSIONS(FunctionItem)


/*******************************************************************************

********************************************************************************/
DynamicFunctionInfo::DynamicFunctionInfo(const QueryLoc& loc,
                                         function* func,
                                         store::Item_t qname,
                                         uint32_t arity)
  :
  theCCB(NULL),
  theSctx(NULL),
  theLoc(loc),
  theFunction(func),
  theQName(qname),
  theArity(arity)
{
}


DynamicFunctionInfo::~DynamicFunctionInfo()
{
  /*
  std::cerr << "--> ~DynamicFunctionInfo(): static_context: " << theScopedSctx.getp() << " ("
      << (theScopedSctx.getp()?theScopedSctx->getRefCount() : -1) << ")" << std::endl;
  */
}


DynamicFunctionInfo::DynamicFunctionInfo(::zorba::serialization::Archiver& ar)
{
}


void DynamicFunctionInfo::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theCCB;
  ar & theSctx;
  // ar & theLoc; TODO
  ar & theQName;
  ar & theArity;
  ar & theFunction;
  ar & theScopedVarsNames;
  ar & theScopedVarsIterators;

  if (ar.is_serializing_out())
  {
    uint32_t planStateSize;
    (void)static_cast<user_function*>(theFunction.getp())->getPlan(planStateSize);
  }
}


void DynamicFunctionInfo::add_variable(expr* var, var_expr* substVar, const store::Item_t& name, int isGlobal)
{
  theScopedVarsValues.push_back(var);
  theSubstVarsValues.push_back(substVar);
  theScopedVarsNames.push_back(name);
  theIsGlobalVar.push_back(isGlobal);
}

/*******************************************************************************

********************************************************************************/
FunctionItem::FunctionItem(::zorba::serialization::Archiver& ar)
  :
  store::Item(store::Item::FUNCTION)
{
}


FunctionItem::FunctionItem(const DynamicFunctionInfo_t& dynamicFunctionInfo,
                           const std::vector<store::Iterator_t>& varsValues, // TODO: remove this param
                           CompilerCB* ccb,
                           dynamic_context* dctx)
  :
  store::Item(store::Item::FUNCTION),
  theCCB(ccb),
  theDynamicFunctionInfo(dynamicFunctionInfo),
  theArity(dynamicFunctionInfo->theArity),
  // theArgumentsValues(varsValues), // TODO: remove this initalization, varsValues is always empty
  theDctx(dctx)
{
  assert(theDynamicFunctionInfo->theFunction->isUdf());
  theArgumentsValues.resize(theDynamicFunctionInfo->theArity);
}


FunctionItem::FunctionItem(const DynamicFunctionInfo_t& dynamicFunctionInfo)
  :
  store::Item(store::Item::FUNCTION),
  theDynamicFunctionInfo(dynamicFunctionInfo),
  theArity(dynamicFunctionInfo->theArity)
{
  assert(theDynamicFunctionInfo->theFunction->isUdf());
  theArgumentsValues.resize(theDynamicFunctionInfo->theArity);
}


FunctionItem::~FunctionItem()
{
  /*
  std::cerr << "--> ~FunctionItem(): (static_context) theDynamicFunctionInfo: " << theDynamicFunctionInfo.getp()
      << " (" << theDynamicFunctionInfo->getRefCount() << ")" << std::endl;
  */
}


void FunctionItem::serialize(::zorba::serialization::Archiver& ar)
{
  // ar & theDynamicFunctionInfo;
  ar & theArgumentsValues;
}


const store::Item_t FunctionItem::getFunctionName() const
{
  return theDynamicFunctionInfo->theQName;
}


uint32_t FunctionItem::getArity() const
{
  return theArity;
}


const signature& FunctionItem::getSignature() const
{
  return theDynamicFunctionInfo->theFunction->getSignature();
}


const std::vector<PlanIter_t>& FunctionItem::getVariablesIterators() const
{
  return theDynamicFunctionInfo->theScopedVarsIterators;
}


const std::vector<PlanIter_t>& FunctionItem::getArgumentsValues() const
{
  return theArgumentsValues;
}

/*
store::Iterator_t FunctionItem::getVariableValue(unsigned int i) const
{
  if (i < theVariablesValues.size())
    return theVariablesValues[i];
  else
    return NULL;
}
*/

void FunctionItem::setArgumentValue(unsigned int pos, const PlanIter_t& value)
{
  theArity--;

  // find the pos-th NULL value and fill it
  for (unsigned int i=0; i<theArgumentsValues.size(); i++)
    if (theArgumentsValues[i] == NULL)
    {
      if (pos == 0)
      {
        theArgumentsValues[i] = value;
        return;
      }
      else
        pos--;
    }

  assert(false);
}

PlanIter_t FunctionItem::getImplementation(const std::vector<PlanIter_t>& dynChildren)
{
  std::vector<PlanIter_t> args;
  args.resize(theArgumentsValues.size());

  std::vector<PlanIter_t>::iterator argsIte = args.begin();
  std::vector<PlanIter_t>::iterator ite = theArgumentsValues.begin();
  std::vector<PlanIter_t>::const_iterator ite2 = dynChildren.begin();
  ++ite2; // skip the first child because it's the function item

  for( ; argsIte != args.end(); ++argsIte, ++ite)
  {
    if (*ite != NULL)
    {
      *argsIte = *ite;
    }
    else
    {
      *argsIte = *ite2;
      ++ite2;
    }
  }

  expr* dummy = theCCB->theEM->create_function_item_expr(NULL, NULL, theDynamicFunctionInfo->theLoc);


  PlanIter_t udfCallIterator =
      theDynamicFunctionInfo->theFunction->codegen(theDynamicFunctionInfo->theCCB,
                                                   theDynamicFunctionInfo->theSctx,
                                                   theDynamicFunctionInfo->theLoc,
                                                   args,
                                                   *dummy);
  /* TODO: remove this code and also clean-up the code in item_iterator.cpp
  PlanIter_t udfCallIterator =
      theDynamicFunctionInfo->theFunction->codegen(theCCB.get(),
                                                   theCCB->theRootSctx,
                                                   theDynamicFunctionInfo->theLoc,
                                                   args,
                                                   *dummy);
  */

  /*
  std::cerr << "Iterator tree for dynamic function call:\n";
  XMLIterPrinter vp(std::cerr);
  print_iter_plan(vp, udfCallIterator);
  std::cerr << std::endl;
  */

  UDFunctionCallIterator* udfIter = static_cast<UDFunctionCallIterator*>(udfCallIterator.getp());
  udfIter->setDynamic();
  udfIter->setFunctionItem(this);
  return udfCallIterator;
}


zstring FunctionItem::show() const
{
  std::ostringstream lRes;
  if (getFunctionName() != NULL)
  {
    lRes << getFunctionName()->getStringValue() << "#" << getArity();
  }
  else
  {
    lRes << "inline function";
  }
  lRes << " (" << theDynamicFunctionInfo->theLoc << ")";
  return lRes.str();

  return "";
}


} //namespace zorba
/* vim:set et sw=2 ts=2: */
