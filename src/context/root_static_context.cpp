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

#include "context/root_static_context.h"
#include "context/namespace_context.h"
#include "context/default_url_resolvers.h"
#include "context/default_uri_mappers.h"

#include "types/root_typemanager.h"

#include "system/globalenv.h"

#include "compiler/parser/query_loc.h"

#include "common/common.h"
#include "zorbamisc/ns_consts.h"

#include "store/api/item_factory.h"



// DEFINE ENVIRONMENT VARIABLES
namespace zorba 
{


static void append_env_var(
    const char* env_var_name,
    std::vector<zstring>& pathsVector)
{
  char const *const env_value = getenv(env_var_name);
  if (!env_value)
    return;
  // Tokenize a copy: strtok must not modify the process environment.
  std::string dll_path(env_value);
  char* save_ptr = NULL;
  char* str_env = strtok_r(&dll_path[0], ";", &save_ptr);
  while (str_env !=NULL)
  {
    zstring zorba_env_dir(str_env);
    if (str_env[strlen(str_env)-1] != '/')
    {
      zorba_env_dir.append("/");
    }
    pathsVector.push_back(zorba_env_dir);
    str_env = strtok_r(NULL, ";", &save_ptr);
  }
}



root_static_context::root_static_context() 
  :
  static_context()
{
  theTypeManager = &GENV_TYPESYSTEM;
}


void root_static_context::init() 
{
  QueryLoc loc;

  theImplementationBaseUri = "file:///";
  set_entity_retrieval_uri(theImplementationBaseUri);
  compute_base_uri();

  set_language_kind(StaticContextConsts::language_kind_xquery);
  set_xquery_version(StaticContextConsts::xquery_version_3_0);
  set_xpath_compatibility(StaticContextConsts::xpath2_0);

  const char* const default_ns_initializers[] = 
  {
    //"err", XQUERY_ERR_NS,
    "fn", static_context::W3C_FN_NS,
    "jn", static_context::JSONIQ_FN_NS,
    "js", static_context::JSONIQ_DM_NS,
    "local", XQUERY_LOCAL_FN_NS,
    "xml", static_context::W3C_XML_NS,
    "xs", static_context::W3C_XML_SCHEMA_NS,
    "xsi", XSI_NS,
    NULL, NULL
  };

  for (const char* const* p = default_ns_initializers; *p != NULL; p += 2)
  {
    zstring pfx(p[0]);
    zstring ns (p[1]);
    bind_ns(pfx, ns, loc);
  }

  set_default_elem_type_ns(zstring(), true, loc);   

  set_default_function_ns(XQUERY_MATH_FN_NS, false, loc);
  set_default_function_ns(JSONIQ_FN_NS, false, loc);
  set_default_function_ns(W3C_FN_NS, false, loc);

  add_collation(ZORBA_DEF_COLLATION_NS, QueryLoc::null);
  add_collation(W3C_CODEPT_COLLATION_NS, QueryLoc::null);
  add_collation("http://zorba.io/collations/SECONDARY/en/EN", QueryLoc::null);
  set_default_collation(W3C_CODEPT_COLLATION_NS, QueryLoc::null);

  set_construction_mode(StaticContextConsts::cons_preserve);
  set_inherit_ns(true);
  set_preserve_ns(true);

  set_ordering_mode(StaticContextConsts::ordered);

  set_empty_order_mode(StaticContextConsts::empty_greatest);

  set_boundary_space_mode(StaticContextConsts::strip_space);

  set_default_w3c_collection_type(GENV_TYPESYSTEM.ITEM_TYPE_STAR);

  set_context_item_type(GENV_TYPESYSTEM.ITEM_TYPE_ONE, QueryLoc::null);

  // TODO move into globalenv? memory leaks?
  add_url_resolver(new internal::FileURLResolver());
  add_url_resolver(new internal::HTTPURLResolver());
#ifndef ZORBA_NO_FULL_TEXT
  add_url_resolver(new internal::ThesaurusURLResolver());
#endif /* ZORBA_NO_FULL_TEXT */
  add_uri_mapper(new internal::FileizeURIMapper());
  add_uri_mapper(new internal::ModuleVersioningURIMapper());
  add_uri_mapper(new internal::AutoFSURIMapper());
  add_url_resolver(new internal::ZorbaCollectionURLResolver());
  add_uri_mapper(new internal::ZorbaCollectionURIMapper());

  set_validation_mode(StaticContextConsts::lax_validation);

  std::vector<zstring> lRootURIPath;
  std::vector<zstring> lRootLibPath;

  const char ** lURIPathIter = get_builtin_uri_path();
  for (; *lURIPathIter != 0; ++lURIPathIter)
  {
    lRootURIPath.push_back(*lURIPathIter);
  }
  set_uri_path(lRootURIPath);

  const char ** lLibPathIter = get_builtin_lib_path();
  for (; *lLibPathIter != 0; ++lLibPathIter)
  {
    lRootLibPath.push_back(*lLibPathIter);
  }
  set_lib_path(lRootLibPath);

  // by default enabled features
  set_feature( feature::ddl );
  set_feature( feature::scripting );
  set_feature( feature::trace );
  set_feature( feature::http_resolution );

  // Instance default security policy: no capability. Embedders widen it
  // with Zorba::setSecurityPolicy() or StaticContext::setSecurityPolicy().
  set_security_policy(SecurityPolicy::secure());

  // Directories from the environment. They used to be appended to the paths
  // above after those had been set, so they had no effect. They are kept
  // apart now: the full URI / lib path includes them only where the policy
  // allows Capability::Environment.
  std::vector<zstring> lEnvURIPath;
  std::vector<zstring> lEnvLibPath;
  append_env_var ("ZORBA_URI_PATH", lEnvURIPath);
  append_env_var ("ZORBA_LIB_PATH", lEnvLibPath);

  set_env_paths(lEnvURIPath, lEnvLibPath);
} 


root_static_context::~root_static_context()
{
}


void root_static_context::free()
{
}

}
/* vim:set et sw=2 ts=2: */
