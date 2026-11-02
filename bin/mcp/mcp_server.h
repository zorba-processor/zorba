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
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <zorba/security.h>
#include <zorba/zorba.h>

#include "document_cache.h"
#include "json.h"
#include "scanner.h"

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
  /**
   * The language of a query that has no version declaration and is not read
   * from a .jq or .xq file: JSONiq if true, else XQuery.
   */
  bool jsoniq_default = false;
  /** The evaluation time limit of one tool call (0: none). */
  std::chrono::milliseconds timeout{ std::chrono::seconds( 30 ) };
  /** The maximum size of a tool's text output, in bytes. */
  std::size_t max_output = 128 * 1024;
  /** The largest file the document cache admits, in KiB (0: no cache). */
  std::size_t cache_max_file_kb = 32;
  /** How many documents the document cache keeps (0: no cache). */
  std::size_t cache_max_files = 10;
  /** Whether to scan the root in the background (see scanner.h). */
  bool scan = true;
  /** The limits of the scanner. */
  ScanLimits scan_limits;
  /** For tests: see ScanHook. */
  ScanHook scan_hook;
  /** The memory limit of the process, in bytes (0: none; see zorba_mcp.cpp). */
  unsigned long long max_memory = 4ULL << 30;
};

/**
 * An MCP server over the Zorba C++ API: answers one JSON-RPC message at a
 * time. Every query runs under SecurityPolicy::secure() narrowed to reading
 * the root directory (see directory_config), with \c $dir:root bound. Unless
 * configured otherwise, list_documents answers from a background scan of the
 * root (see Scanner); queries never depend on it.
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

  ~Server();

  /**
   * Handles one message (one line of the stdio transport).
   *
   * @return the response, without a trailing newline, or the empty string
   * if the message is a notification (no response).
   */
  std::string handle( std::string const &message );

  /** The policy every query runs under. */
  SecurityPolicy const& policy() const { return thePolicy; }

  /** The counters of the document cache. */
  DocumentCache::Stats cache_stats() const { return theCache.stats(); }

  /** The counters as one line for the log; empty without a cache. */
  std::string cache_stats_line() const { return theCache.stats_line(); }
  /**
   * Stops the background scan, if any; the destructor does it too.
   *
   * @return whether every thread was joined.
   */
  bool stop_scan();

  /** The bytes the scan cache accounts for, or 0 without a scan. */
  std::size_t scan_cache_bytes() const;

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

  StaticContext_t new_static_context( std::string const &file_name ) const;

  std::string handle_message( std::string const &message, Json *request_id );
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
  DocumentCache theCache;
  std::string theLegacyVersion;         // set by initialize
  std::unique_ptr<Scanner> theScanner;  // null with --no-scan
};

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_SERVER_H */
/* vim:set et sw=2 ts=2: */
