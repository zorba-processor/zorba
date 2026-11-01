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

#include "directory_config.h"

#include <stdexcept>

#include <zorba/item_factory.h>
#include <zorba/util/fs_util.h>

namespace zorba {
namespace directory_config {

char const module_ns[] = "http://zorba.io/modules/directory";

bool normalize( std::string *dir, std::string *error ) {
  std::string abs( *dir );
  try {
    fs::make_absolute( &abs );
    abs = fs::normalize_path( abs );
  }
  catch ( std::invalid_argument const& ) {
  }
  while ( abs.size() > 1 && abs.back() == '/' )
    abs.pop_back();
  if ( fs::get_type( abs ) != fs::directory ) {
    if ( error )
      *error = "not a directory: " + *dir;
    return false;
  }
  *dir = abs;
  return true;
}

void apply_policy( SecurityPolicy &policy, std::string const &dir ) {
  policy.allow( Capability::FileRead );
  policy.allow( Capability::NativeModules );
  policy.addReadPath( dir );
}

void bind_root( Zorba *zorba, DynamicContext *dctx, std::string const &dir ) {
  dctx->setVariable( module_ns, "root",
    zorba->getItemFactory()->createString( dir ) );
}

std::vector<std::pair<std::string,std::string> > serialization_params() {
  return {
    { "method", "json" },
    { "item-separator", "\n" }
  };
}

} // namespace directory_config
} // namespace zorba
/* vim:set et sw=2 ts=2: */
