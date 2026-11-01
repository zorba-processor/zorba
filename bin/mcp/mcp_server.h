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

#ifndef ZORBA_MCP_SERVER_H
#define ZORBA_MCP_SERVER_H

#include <chrono>
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include <zorba/security.h>
#include <zorba/zorba.h>

#include "json.h"

namespace zorba {
namespace mcp {

/** The MCP revision this server implements per request (stateless). */
extern char const protocol_version[];

/** The handshake-based revisions served after an \c initialize request. */
extern char const *const legacy_protocol_versions[];

struct ServerConfig {
  /** The directory the queries read; absolute and normalized. */
  std::string root;
  /** A directory of predefined queries (*.jq), or empty. */
  std::string queries_dir;
  /** The evaluation time limit of one tool call (0: none). */
  std::chrono::milliseconds timeout{ std::chrono::seconds( 30 ) };
  /** The maximum size of a tool's text output, in bytes. */
  std::size_t max_output = 128 * 1024;
};

/**
 * An MCP server over the Zorba C++ API: answers one JSON-RPC message at a
 * time. Every query runs under SecurityPolicy::secure() narrowed to reading
 * the root directory (see directory_config), with \c $dir:root bound.
 *
 * Serves the modern revision (per-request \c _meta, \c server/discover) and,
 * once a client sends \c initialize, the legacy handshake-based revisions.
 */
class Server {
public:
  /**
   * @param zorba the engine.
   * @param config the configuration.
   * @param log where diagnostics go (never the protocol stream).
   */
  Server( Zorba *zorba, ServerConfig config, std::ostream &log );

  /**
   * Handles one message (one line of the stdio transport).
   *
   * @return the response, without a trailing newline, or the empty string
   * if the message is a notification (no response).
   */
  std::string handle( std::string const &message );

  /** The policy every query runs under. */
  SecurityPolicy const& policy() const { return thePolicy; }

private:
  struct Param {
    std::string name;
    bool required;
  };

  struct Predefined {
    std::string name;
    std::string description;
    std::string path;
    std::string source;
    std::vector<Param> params;
  };

  struct QueryOutcome {
    bool ok = false;
    std::string text;
  };

  void load_predefined();
  bool load_predefined( std::string const &path, std::string const &name );

  Json tools() const;
  Json call_tool( Json const &params, bool *protocol_error, Json *error );
  QueryOutcome run( std::string const &source, std::string const &file_name,
                    std::vector<std::pair<std::string,std::string> > const
                      &vars );
  Json tool_result( QueryOutcome const &outcome ) const;

  Zorba *const theZorba;
  ServerConfig const theConfig;
  std::ostream &theLog;
  SecurityPolicy thePolicy;
  std::vector<Predefined> thePredefined;
  std::string theLegacyVersion;         // set by initialize
};

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_SERVER_H */
/* vim:set et sw=2 ts=2: */
