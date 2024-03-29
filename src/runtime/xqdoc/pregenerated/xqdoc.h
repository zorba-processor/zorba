/*
 * Copyright 2006-2012 2006-2016 zorba.io.
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
 
// ******************************************
// *                                        *
// * THIS IS A GENERATED FILE. DO NOT EDIT! *
// * SEE .xml FILE WITH SAME NAME           *
// *                                        *
// ******************************************

#ifndef ZORBA_RUNTIME_XQDOC_XQDOC_H
#define ZORBA_RUNTIME_XQDOC_XQDOC_H


#include "common/shared_types.h"



#include "runtime/base/narybase.h"


namespace zorba {

/**
 * zorba:XQDoc
 * Author: Zorba Team
 */
class XQDocContentIterator : public NaryBaseIterator<XQDocContentIterator, PlanIteratorState>
{ 
public:
  SERIALIZABLE_CLASS(XQDocContentIterator);

  SERIALIZABLE_CLASS_CONSTRUCTOR2T(XQDocContentIterator,
    NaryBaseIterator<XQDocContentIterator, PlanIteratorState>);

  void serialize( ::zorba::serialization::Archiver& ar);

  XQDocContentIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& children)
    : 
    NaryBaseIterator<XQDocContentIterator, PlanIteratorState>(sctx, loc, children)
  {}

  virtual ~XQDocContentIterator();

  zstring getNameAsString() const;

  void accept(PlanIterVisitor& v) const;

  bool nextImpl(store::Item_t& result, PlanState& aPlanState) const;
};


}
#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */ 
