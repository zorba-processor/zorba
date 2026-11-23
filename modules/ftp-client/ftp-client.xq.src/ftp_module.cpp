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

// Zorba
#include <zorba/zorba.h>

// local
#include "ftp_functions.h"
#include "ftp_module.h"

namespace zorba {
namespace ftp_client {

///////////////////////////////////////////////////////////////////////////////

module::module() :
  item_factory_( Zorba::getInstance(0)->getItemFactory() )
{
  func_map_[ "connect" ] = std::make_unique<connect_function>( this );
  func_map_[ "delete" ] = std::make_unique<delete_function>( this );
  func_map_[ "get-binary" ] = std::make_unique<get_binary_function>( this );
  func_map_[ "get-text" ] = std::make_unique<get_text_function>( this );
  func_map_[ "list" ] = std::make_unique<list_function>( this );
  func_map_[ "mkdir" ] = std::make_unique<mkdir_function>( this );
  func_map_[ "put-binary" ] = std::make_unique<put_binary_function>( this );
  func_map_[ "put-text" ] = std::make_unique<put_text_function>( this );
  func_map_[ "rename" ] = std::make_unique<rename_function>( this );
  func_map_[ "rmdir" ] = std::make_unique<rmdir_function>( this );
}

void module::destroy() {
  delete this;
}

ExternalFunction* module::getExternalFunction( String const &local_name ) {
  func_map_type::const_iterator const i = func_map_.find( local_name );
  return i == func_map_.end() ? nullptr : i->second.get();
}

ItemFactory* module::getItemFactory() const {
  return item_factory_;
}

String module::getURI() const {
  static String const uri( "http://zorba.io/modules/ftp-client" );
  return uri;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ftp_client
} // namespace zorba

#define DLL_EXPORT __attribute__ ((visibility("default")))

extern "C" DLL_EXPORT zorba::ExternalModule* createModule() {
  return new zorba::ftp_client::module();
}

/* vim:set et sw=2 ts=2: */
