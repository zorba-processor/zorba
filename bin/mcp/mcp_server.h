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
#include <functional>
#include <memory>
#include <ostream>
#include <string>

#include <zorba/security.h>
#include <zorba/zorba.h>

#include "database.h"
#include "json.h"
#include "scanner.h"

namespace zorba {
namespace mcp {

using db::Json;

/** The MCP revision this server implements per request (stateless). */
extern char const protocol_version[];

/** Larger messages are refused before parsing. */
extern std::size_t const max_message_size;

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
  std::size_t cache_max_file_kb = 16;
  /** How many documents the document cache keeps (0: no cache). */
  std::size_t cache_max_files = 10;
  /** Whether to scan the root in the background (see scanner.h). */
  bool scan = true;
  /** The limits of the scanner. */
  db::ScanLimits scan_limits;
  /** For tests: called when a query is compiled, before it runs. */
  std::function<void()> before_execute;
  /** For tests: see the after_read parameter of db::DocumentCache. */
  std::function<void()> cache_after_read;
  /** For tests: see db::ScanHook. */
  db::ScanHook scan_hook;
  /** The memory limit of the process, in bytes (0: none; see zorba_mcp.cpp). */
  unsigned long long max_memory = 4ULL << 30;
};

/**
 * An MCP server over a db::Database: answers one JSON-RPC message at a time.
 * The tools run the database's queries; unless configured otherwise,
 * list_documents answers from a db::Scanner of the root, which queries never
 * depend on. This class checks the arguments of the tools and turns results
 * and errors into tool results.
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

  /**
   * The id of the request being handled, as JSON ("null" if it has none or is
   * not known yet), for a response made when handle() itself runs out of memory.
   */
  char const* current_id() const { return theCurrentId.c_str(); }

  /**
   * What to do when handle() threw: no response for a notification, else an
   * error response with current_id(), which is "null" if the id could not be
   * read (JSON-RPC 2.0, section 5).
   */
  enum class AfterFailure {
    nothing,  // the message was a notification
    answer    // an error response with current_id()
  };
  AfterFailure after_failure() const;

  /** The policy every query runs under. */
  SecurityPolicy const& policy() const { return theDatabase.policy(); }

  /** The counters of the document cache. */
  db::DocumentCache::Stats cache_stats() const {
    return theDatabase.cache_stats();
  }

  /** The counters as one line for the log; empty without a cache. */
  std::string cache_stats_line() const {
    return theDatabase.cache_stats_line();
  }
  /**
   * Stops the background scan, if any; the destructor does it too.
   *
   * @return whether every thread was joined.
   */
  bool stop_scan();

  /** The bytes the scan cache accounts for, or 0 without a scan. */
  std::size_t scan_cache_bytes() const;

private:
  struct QueryOutcome {
    bool ok = false;
    std::string text;
  };

  std::string handle_message( std::string const &message, Json *request_id );
  // Sets theCurrentKind, and the id of a request.
  void identify( Json const &msg, Json *request_id );

  Json tools() const;
  Json call_tool( Json const &params, bool *protocol_error, Json *error );
  std::string list_documents( db::Scanner &scanner, Json const &args,
                              bool *is_error );
  // Runs a query of the database with the output capped at --max-output.
  template<typename Run> QueryOutcome run( Run const &query );
  Json tool_result( QueryOutcome const &outcome ) const;

  ServerConfig const theConfig;
  std::ostream &theLog;
  db::Database theDatabase;
  std::string theLegacyVersion;         // set by initialize
  std::string theCurrentId{ "null" };   // see current_id()
  enum class MessageKind {
    unknown,       // not read yet
    notification,  // no id: never answered
    request,       // theCurrentId is its id
    without_id,    // answered with id null (unreadable, or an invalid id)
    id_lost        // memory ran out while its id was read: answered with null
  };
  MessageKind theCurrentKind = MessageKind::unknown;
  std::unique_ptr<db::Scanner> theScanner;  // null with --no-scan
};

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_SERVER_H */
/* vim:set et sw=2 ts=2: */
