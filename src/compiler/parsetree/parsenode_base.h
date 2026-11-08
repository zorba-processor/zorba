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
#ifndef ZORBA_PARSENODE_BASE_H
#define ZORBA_PARSENODE_BASE_H

#include <zorba/config.h>
#include <zorba/diagnostic_list.h>
#include <zorba/error.h>

#include "compiler/parser/parse_constants.h"
#include "compiler/parser/query_loc.h"
#include "compiler/parser/xqdoc_comment.h"
#include "zorbatypes/rchandle.h"

#include "store/api/update_consts.h"
#include "zorbatypes/zstring.h"

namespace zorba {

class parsenode_visitor;


/*
**  base class: syntax-only nodes
*/
// exported for unit testing only
class ZORBA_DLL_PUBLIC parsenode : public SimpleRCObject
{
protected:
  QueryLoc loc;

  friend class ParseNodePrintXMLVisitor;

public:
  class DeferFree;

private:
  // The next node waiting to be destroyed by free(): the queue allocates nothing,
  // since free() runs in destructors.
  parsenode* theNextPending = nullptr;

  // The DeferFree keeping this node, and its neighbours in that DeferFree's list.
  DeferFree* theDeferredBy = nullptr;
  parsenode* thePrevDeferred = nullptr;
  parsenode* theNextDeferred = nullptr;

public:
  parsenode(const QueryLoc& loc_) : loc(loc_) { }
  virtual ~parsenode();

  // Destroys the node without recursing into its subtree (see parsenodes.cpp).
  void free() override;

public:
  const QueryLoc& get_location() const { return loc; }

  virtual void accept(parsenode_visitor&) const = 0;

};


/**
 * While a DeferFree is alive, a parse node of its thread whose last reference
 * goes away is kept instead of freed; when the DeferFree ends, it frees each
 * kept node that still has no reference. The parser holds one: its semantic
 * values are nodes nobody holds a reference to, and when an exception leaves a
 * rule's action, a handle the action had taken on such a node may already
 * have freed it before the parser releases the node again.
 */
class ZORBA_DLL_PUBLIC parsenode::DeferFree
{
  friend class parsenode;

  DeferFree* thePrevious;
  parsenode* theFirst = nullptr;
  parsenode* theLast = nullptr;

  void keep(parsenode* node);
  void forget(parsenode* node);

public:
  DeferFree();
  ~DeferFree();

  DeferFree(const DeferFree&) = delete;
  DeferFree& operator=(const DeferFree&) = delete;
};

/**
 * XQDocumentable: nodes with XQDoc comments attached
 */
class XQDocumentable: public parsenode
{
  private:
    zstring theStringComment;
    mutable const XQDocComment* theComment;

  public:
    XQDocumentable(const QueryLoc& loc): parsenode(loc), theComment(0){}
    ~XQDocumentable(){ delete theComment; }

    XQDocumentable* setComment(const zstring& aComment)
    {
      theStringComment = aComment;
      return this;
    }

    const XQDocComment* getComment() const
    {
      if(theComment == 0)
      {
        theComment = new XQDocComment(theStringComment);
      }
      return theComment;
    }

    zstring getCommentString() const { return theStringComment; }
};


/**
 * exprnode:  nodes with values.
 */
class exprnode : public parsenode
{
public:
  exprnode(const QueryLoc& loc) : parsenode(loc) { }

public:
  virtual void accept(parsenode_visitor&) const = 0;

};


class ZORBA_DLL_PUBLIC ParseErrorNode : public parsenode 
{
public:
  Error const &err;
  zstring const msg;
  bool const useParam;   // if true, msg is a parameter for the ZORBA_ERROR_LOC_PARAM() macro

  ParseErrorNode(const QueryLoc& loc_, Error const &err_ = err::XPST0003, zstring msg_ = "", bool useParam_ = false)
    :
    parsenode (loc_), err(err_), msg(msg_), useParam(useParam_)
  {}

  void accept(parsenode_visitor&) const {}
};


} // namespace zorba

#endif /* ZORBA_PARSENODE_BASE_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
