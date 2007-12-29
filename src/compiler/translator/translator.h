/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: translator.h,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *
 *  Author: John Cowan, Paul Pedersen
 *
 */

#ifndef XQP_NORMALIZE_VISITOR_H
#define XQP_NORMALIZE_VISITOR_H

#include "compiler/parsetree/parsenode_visitor.h"
#include "compiler/expression/expr.h"

/*______________________________________________________________________
|  
|	 Design note: Visitor pattern.  See, for example:
|	 "Modern C++ Design" by Andrei Alexandrescu, Addison Wesley (2001),
|	 Chapter 10.
|_______________________________________________________________________*/

namespace xqp {

  class Translator : public parsenode_visitor {
  public:
    virtual rchandle<expr> result () = 0;
  };

  Translator *make_translator (static_context *);
  rchandle<expr> translate (static_context *, const parsenode &);
  
} /* namespace xqp */
#endif /* XQP_NORMALIZE_VISITOR_H */
