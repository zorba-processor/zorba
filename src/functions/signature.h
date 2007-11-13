/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: signature.h,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Authors: John Cowan,Paul Pedersen
 *
 */
 
#ifndef XQP_SIGNATURE_H
#define XQP_SIGNATURE_H

#include <string>
#include <vector>

#include "context/common.h"
#include "types/typesystem.h"
#include "util/fx/fxhashmap.h"
#include "store/api/item.h"

namespace xqp {

class context;
class Item;


/*******************************************************************************
  
  By convention, argv[0]    = return type
	               argv[1]    = first input argument type
                 argv[2]    = second input argument type
	                ...       =  ...  

********************************************************************************/
class signature : public rcobject
{
public:
	QNameItem_t qname_p;
	std::vector<TypeSystem::xqtref_t> argv;
	static fxhashmap<signature const*> sigmap;	// map: fname -> signture

public:
	signature() {}
	signature(QNameItem_t name,
						TypeSystem::xqtref_t return_type);
	signature(QNameItem_t name,
						TypeSystem::xqtref_t arg1,
						TypeSystem::xqtref_t return_type);
	signature(QNameItem_t name,
						TypeSystem::xqtref_t arg1,
						TypeSystem::xqtref_t arg2,
						TypeSystem::xqtref_t return_type);
	signature(QNameItem_t name,
						TypeSystem::xqtref_t arg1,
						TypeSystem::xqtref_t arg2,
						TypeSystem::xqtref_t arg3,
						TypeSystem::xqtref_t return_type);
	signature(QNameItem_t name,
						TypeSystem::xqtref_t arg1,
						TypeSystem::xqtref_t arg2,
						TypeSystem::xqtref_t arg3,
						TypeSystem::xqtref_t arg4,
						TypeSystem::xqtref_t return_type);
	signature(QNameItem_t name,
						const std::vector<TypeSystem::xqtref_t>& argv);
	~signature();

public:
	const QNameItem_t& get_name() const            { return qname_p; }
	void set_name(const QNameItem_t& name)         { qname_p = name; }
	void add_arg(TypeSystem::xqtref_t t)                { argv.push_back(t); }
	uint32_t arg_count() const                     { return argv.size() - 1; }

    TypeSystem::xqtref_t const& operator[](int i) const { return argv[i]; }
    TypeSystem::xqtref_t & operator[](int i)            { return argv[i]; }
    TypeSystem::xqtref_t const& return_type() const     { return argv[0]; }
    TypeSystem::xqtref_t & return_type()                { return argv[0]; }

	void set_return_type(TypeSystem::xqtref_t t);
};

} /* namespace xqp */
#endif  /* XQP_SIGNATURE_H */

