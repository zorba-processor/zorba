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

#include "mcp_server.h"

#include "directory_config.h"

namespace zorba {
namespace mcp {

char const protocol_version[] = "2026-07-28";

char const *const legacy_protocol_versions[] = {
  "2025-11-25", "2025-06-18", "2025-03-26", nullptr
};

// Not implemented yet: answers nothing.
Server::Server( Zorba *zorba, ServerConfig config, std::ostream &log ) :
  theZorba( zorba ),
  theConfig( std::move( config ) ),
  theLog( log ),
  thePolicy( SecurityPolicy::secure() ) {
  directory_config::apply_policy( thePolicy, theConfig.root );
}

std::string Server::handle( std::string const& ) {
  return std::string();
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
