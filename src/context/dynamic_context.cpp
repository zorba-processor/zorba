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
#include "common/common.h"
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#ifdef UNIX
#include <sys/time.h>
#endif
#include "store/api/iterator.h"
#include "store/api/temp_seq.h"
#include "store/api/item_factory.h"
#include "store/api/store.h"

#include "system/globalenv.h"

#include "context/dynamic_context.h"
#include "context/static_context.h"
#include "context/namespace_context.h"

#include "compiler/expression/var_expr.h"

#include "types/root_typemanager.h"

#include "runtime/api/plan_wrapper.h"

#include "zorbautils/strutil.h"

#include "indexing/value_index.h"

using namespace std;

namespace zorba 
{

bool dynamic_context::static_init = false;


std::string dynamic_context::var_key(const void* var) 
{
  if (var == NULL)
    return "";

  const var_expr *ve = static_cast<const var_expr *> (var);

  return xqpString::concat(to_string(ve->get_unique_id()), ":", ve->get_varname()->getStringValue());
}


xqp_string dynamic_context::expand_varname(
    static_context* sctx,
    xqpString& qname)
{
  if(!sctx) 
  {
    ///actually the whole static context is missing
    ZORBA_ERROR_PARAM( XPST0001, "entire static context", "");
    return (const char*)NULL;
  }

  void* var = static_cast<void *> (sctx->lookup_var(qname));
  return var_key(var);
}


void dynamic_context::init()
{
	if (!dynamic_context::static_init) {
		dynamic_context::static_init = true;
	}
}


dynamic_context::dynamic_context(dynamic_context* parent)
  :
  theAvailableIndices(NULL)
{
  this->parent = parent;

  if(!parent)
  {
#if defined (WIN32)
    struct _timeb timebuffer;
    _ftime_s( &timebuffer );
    struct ::tm gmtm;
    localtime_s(&gmtm, &timebuffer.time); //thread safe localtime on Windows
#else
    struct timeb timebuffer;
    ftime( &timebuffer );
    struct ::tm gmtm;
    localtime_r(&timebuffer.time, &gmtm); //thread safe localtime on Linux
#endif

    {
      int lSummerTimeShift = 0;
      if (gmtm.tm_isdst != 0) {
        lSummerTimeShift = 3600;
      }
      set_implicit_timezone( -timebuffer.timezone * 60 + lSummerTimeShift );//in seconds
    }

    GENV_ITEMFACTORY->createDateTime(current_date_time_item,
                                     gmtm.tm_year + 1900,
                                     gmtm.tm_mon + 1,
                                     gmtm.tm_mday, 
                                     gmtm.tm_hour,
                                     gmtm.tm_min,
                                     gmtm.tm_sec + timebuffer.millitm/1000.0,
                                     implicit_timezone/3600);

#if WIN32
    time_t t0;
    time(&t0);
    GENV_ITEMFACTORY->createLong(current_time_millis, t0*1000);
#else
    timeval tv;
    gettimeofday(&tv, 0);
    long long millis = tv.tv_sec;
    millis = millis * 1000 + tv.tv_usec/1000;
    GENV_ITEMFACTORY->createLong(current_time_millis, millis);
#endif
    
	  ctxt_position = 0;
	}
	else
	{
		current_date_time_item = parent->current_date_time_item;
		implicit_timezone = parent->implicit_timezone;
		default_collection_uri = parent->default_collection_uri;

		ctxt_item = parent->ctxt_item;
		ctxt_position = parent->ctxt_position;
	}
}


dynamic_context::~dynamic_context()
{
	///free the pointers from ctx_value_t from keymap
	checked_vector<hashmap<dctx_value_t>::entry>::iterator it;
	const char* keybuff;;

	for(it = keymap.begin(); it != keymap.end(); it++)
	{
		///it is an entry
		keybuff = (*it).key.c_str();
		if(strncmp(keybuff, "var:", 4) == 0)
		{
			destroy_dctx_value(&(*it).val);
		}
	}

  if (theAvailableIndices)
    delete theAvailableIndices;
}


store::Item_t dynamic_context::get_default_collection()
{
	return default_collection_uri;
}


void dynamic_context::set_default_collection(const store::Item_t& default_collection_uri)
{
	this->default_collection_uri = default_collection_uri;
}


void dynamic_context::set_implicit_timezone(int tzone_seconds)
{
	this->implicit_timezone = tzone_seconds;
}


int dynamic_context::get_implicit_timezone()
{
  return implicit_timezone;
}


void	dynamic_context::set_current_date_time( const store::Item_t& aDateTimeItem )
{
  this->current_date_time_item = aDateTimeItem;
}


store::Item_t	dynamic_context::get_current_date_time()
{
	return current_date_time_item;
}


store::Item_t dynamic_context::get_current_time_millis()
{
    return current_time_millis;
}


void dynamic_context::set_context_item(
    const store::Item_t& context_item,
    unsigned long position)
{
	this->ctxt_item = context_item;
	this->ctxt_position = position;
}


store::Item_t dynamic_context::context_item() const 
{
	return ctxt_item;
}


unsigned long dynamic_context::context_position()
{
	return ctxt_position;
}


xqtref_t dynamic_context::context_item_type() const
{
	return GENV_TYPESYSTEM.ITEM_TYPE_STAR;
}


void dynamic_context::set_context_item_type(xqtref_t v)
{
}


void dynamic_context::declare_variable(const std::string& var_name)
{
  if (var_name.empty()) return;

  dctx_value_t v;
  string key = "var:" + var_name;
  if (keymap.get (key, v))
    destroy_dctx_value (&v);

  v.type = dynamic_context::dctx_value_t::no_val;
  v.in_progress = true;
  v.val.var_item = NULL;
  keymap.put(key, v);
}


/*
  var_name is expanded name localname:nsURI
  constructed by static_context::qname_internal_key( .. )
*/
void dynamic_context::set_variable(
    const std::string& var_name,
    store::Iterator_t& var_iterator)
{
  if (var_name.empty()) return;

  string key = "var:" + var_name;
  dctx_value_t v;
  hashmap<dctx_value_t>* map;
  if (! context_value(key, v, &map))
    ZORBA_ASSERT (false);

  var_iterator->open();
  store::TempSeq_t seq = GENV_STORE.createTempSeq(var_iterator.getp());
  var_iterator->close();

  // variables can be set multiple times, so we need to make sure to remove
  // previously set temp sequences
  if (v.type == dynamic_context::dctx_value_t::var_item_val)
  {
    assert(v.val.var_item != NULL);
    RCHelper::removeReference(v.val.var_item);
  }
  else if (v.type == dynamic_context::dctx_value_t::var_temp_seq_val)
  {
    assert(v.val.var_temp_seq != NULL);
    RCHelper::removeReference(v.val.var_temp_seq);
  }

  v.type = dynamic_context::dctx_value_t::var_temp_seq_val;
  v.in_progress = false;
  v.val.var_temp_seq = seq.release();

  map->put(key, v);
}


void dynamic_context::set_variable(
    const std::string& var_name,
    store::Item_t& var_item)
{
  if (var_name.empty()) return;

  string key = "var:" + var_name;
  dctx_value_t v;
  hashmap<dctx_value_t>* map;
  if (! context_value(key, v, &map))
    ZORBA_ASSERT (false);

  // variables can be set multiple times, so we need to make sure to remove
  // previously set temp sequences
  if (v.type == dynamic_context::dctx_value_t::var_item_val)
  {
    assert(v.val.var_item != NULL);
    RCHelper::removeReference(v.val.var_item);
  }
  else if (v.type == dynamic_context::dctx_value_t::var_temp_seq_val)
  {
    assert(v.val.var_temp_seq != NULL);
    RCHelper::removeReference(v.val.var_temp_seq);
  }

  v.type = dynamic_context::dctx_value_t::var_item_val;
  v.in_progress = false;
  v.val.var_item = var_item.release();

  map->put(key, v);
}


void dynamic_context::add_variable(
    const std::string& varname,
    store::Iterator_t& var_iterator) 
{
  declare_variable(varname);
  set_variable(varname, var_iterator);
}


void dynamic_context::add_variable(
    const std::string& varname,
    store::Item_t& var_item) 
{
  declare_variable(varname);
  set_variable(varname, var_item);
}


bool dynamic_context::get_variable(
    const store::Item_t& varname,
    store::Item_t& var_item,
    store::Iterator_t& var_iter)
{
  var_item = NULL;
  var_iter = NULL;
	return lookup_var_value("var:" + varname->getStringValue()->str(),
                          var_item,
                          var_iter);
}


bool dynamic_context::lookup_var_value(
    const std::string& key,
    store::Item_t& var_item,
    store::Iterator_t& var_iter)
{ 
  dctx_value_t val;

  if(!keymap.get(key, val))
	{
    if(parent)
      return parent->lookup_var_value(key, var_item, var_iter);
    else
      return false; // variable not found
	}

  if (val.in_progress)
    ZORBA_ERROR (XQST0054);

  if (val.type == dynamic_context::dctx_value_t::var_item_val)
    var_item = val.val.var_item;
  else
    var_iter = val.val.var_temp_seq->getIterator();

  return true;
}


void dynamic_context::destroy_dctx_value(dctx_value_t* val) 
{
  switch (val->type) 
  {
  case dynamic_context::dctx_value_t::no_val:
    break;
  case dynamic_context::dctx_value_t::var_item_val:
    RCHelper::removeReference(val->val.var_item);
    val->val.var_item = NULL;
    break;
  case dynamic_context::dctx_value_t::var_temp_seq_val:
    RCHelper::removeReference(val->val.var_temp_seq);
    val->val.var_temp_seq = NULL;
    break;
  default:
    ZORBA_ASSERT (false);
  }

  val->type = dynamic_context::dctx_value_t::no_val;
}


void dynamic_context::bind_index(const store::Item* qname, store::Index* index)
{
  if (theAvailableIndices == NULL)
    theAvailableIndices = new IndexMap(0, NULL, 8, false);

  ValueIndexInfo v;
  v.theIndex = index;
  v.theLoadSession = NULL;

  if (!theAvailableIndices->insert(qname, v))
  {
    ZORBA_ERROR_PARAM(XQP0034_INDEX_ALREADY_EXISTS,
                      qname->getStringValue()->c_str(), "");
  }
}


void dynamic_context::unbind_index(const store::Item* qname)
{
  if (!theAvailableIndices->remove(qname))
  {
    if (parent != NULL)
    {
      return parent->unbind_index(qname);
    }
    else
    {
      ZORBA_ERROR_PARAM(XQP0033_INDEX_DOES_NOT_EXIST,
                        qname->getStringValue()->c_str(), "");
    }
  }
}


store::Index* dynamic_context::lookup_index(const store::Item* qname) const
{
  ValueIndexInfo info;

  if (theAvailableIndices->get(qname, info))
  {
    return info.theIndex.getp();
  }
  else if (parent != NULL)
  {
    return parent->lookup_index(qname);
  }
  else
  {
    ZORBA_ERROR_PARAM(XQP0033_INDEX_DOES_NOT_EXIST,
                      qname->getStringValue()->c_str(), "");
  }
}


ValueIndexInsertSession* dynamic_context::get_index_insert_session(
    const store::Item* qname) const
{
  ValueIndexInfo info;

  if (theAvailableIndices->get(qname, info))
  {
    return info.theLoadSession.getp();
  }
  else if (parent != NULL)
  {
    return parent->get_index_insert_session(qname);
  }
  else
  {
    ZORBA_ASSERT(false);
  }
}


void dynamic_context::set_index_insert_session (
    const store::Item* qname,
    ValueIndexInsertSession* s) 
{
  ValueIndexInfo info;

  if (theAvailableIndices->get(qname, info))
  {
    info.theLoadSession = s;
    theAvailableIndices->update(qname, info);
  }
  else if (parent != NULL)
  {
    parent->set_index_insert_session(qname, s);
  }
  else
  {
    ZORBA_ASSERT(false);
  }
}


}	/* namespace zorba */

