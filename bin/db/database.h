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

#ifndef ZORBA_DB_DATABASE_H
#define ZORBA_DB_DATABASE_H

#include <chrono>
#include <cstddef>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <zorba/security.h>
#include <zorba/zorba.h>

#include "document_cache.h"

namespace zorba {
namespace db {

struct DatabaseConfig {
  /** The directory the queries read; absolute and normalized. */
  std::string root;
  /** A directory of predefined queries (*.jq), or empty. */
  std::string queries_dir;
  /**
   * The language of a query that has no version declaration and is not read
   * from a .jq or .xq file: JSONiq if true, else XQuery.
   */
  bool jsoniq_default = false;
  /** The evaluation time limit of one query (0: none). */
  std::chrono::milliseconds timeout{ std::chrono::seconds( 30 ) };
  /** The largest file the document cache admits, in KiB (0: no cache). */
  std::size_t cache_max_file_kb = 16;
  /** How many documents the document cache keeps (0: no cache). */
  std::size_t cache_max_files = 10;
  /** For tests: called when a query is compiled, before it runs. */
  std::function<void()> before_execute;
};

/**
 * Where the serialized result of a query goes, as the engine produces it.
 *
 * An exception from write() or end() fails the query, and reaches the caller
 * as its QueryError: memory_limit if it is std::bad_alloc, or if the engine
 * reports it as zerr:ZXQP0003 with "bad_alloc" in the message; otherwise
 * ZXQP0003 (kind query) with the exception's what() in the message, or
 * internal. When a query fails, writes made before the failure have already
 * reached the sink.
 */
class ResultSink {
public:
  virtual ~ResultSink() = default;
  /** The next bytes of the result. */
  virtual void write( char const *data, std::size_t size ) = 0;
  /** Called once after the last write of a query that completed. */
  virtual void end() { }
};

/** Where in a query an error was raised. */
struct SourceLocation {
  /** The module, if the engine names one. */
  std::string uri;
  unsigned long line = 0;
  unsigned long column = 0;
  /** 0 if the engine gives no end. */
  unsigned long line_end = 0;
  unsigned long column_end = 0;
  /**
   * The whole failing line, when the error is in the query itself (not in a
   * module it imports); else empty.
   */
  std::string text;
};

struct QueryError {
  enum class Kind {
    query,         // raised by the engine or the query: all members below
    memory_limit,  // memory ran out: no members
    internal       // anything else: message
  };
  Kind kind = Kind::internal;
  /** The QName of the error, with its prefix ("err:XPST0003"). */
  std::string code;
  std::string ns;
  std::string message;
  /** Empty if the engine has none. */
  std::string hint;
  std::optional<SourceLocation> location;
};

struct QueryResult {
  bool ok = false;
  /** Why the query failed, if not ok; the sink then holds nothing useful. */
  QueryError error;
};

/** A query read from the queries directory, run by name. */
struct PredefinedQuery {
  struct Param {
    std::string name;
    bool required;
  };
  /** The file name without .jq, made of [A-Za-z0-9_.-] (others become _). */
  std::string name;
  /** The leading comment, flattened to one line; may be empty. */
  std::string description;
  std::string path;
  std::string source;
  /** Its external variables in no namespace; required unless they have a default. */
  std::vector<Param> params;
};

/** The values of external variables, by name, as strings. */
typedef std::vector<std::pair<std::string,std::string> > Variables;

/**
 * Queries over a directory with the Zorba C++ API, one at a time. Every query
 * runs under SecurityPolicy::secure() narrowed to reading the root directory
 * (see directory_config), with \c $dir:root bound and the document cache
 * attached. It starts no threads: the background scan (Scanner) is separate.
 *
 * Callers pass plain values that they have checked; results go to a
 * ResultSink and failures come back as a QueryError.
 */
class Database {
public:
  /**
   * Compiles the predefined queries. Call on the thread that runs the queries.
   *
   * @param zorba the engine.
   * @param config the configuration.
   * @param log where diagnostics go.
   */
  Database( Zorba *zorba, DatabaseConfig config, std::ostream &log );

  Database( Database const& ) = delete;
  Database& operator=( Database const& ) = delete;

  /** Runs a query given as text. */
  QueryResult run( std::string const &source, ResultSink &sink );

  /**
   * Runs a predefined query with the values of its parameters. An unknown
   * name gives an internal error.
   */
  QueryResult run_predefined( std::string const &name, Variables const &vars,
                              ResultSink &sink );

  /** Lists the files under the root with dir:list, without the scan. */
  QueryResult list_files( ResultSink &sink );

  /** The predefined queries, sorted by file name. */
  std::vector<PredefinedQuery> const& predefined() const {
    return thePredefined;
  }

  /** The predefined query of that name, or nullptr. */
  PredefinedQuery const* find_predefined( std::string const &name ) const;

  /** The policy every query runs under. */
  SecurityPolicy const& policy() const { return thePolicy; }

  /** The counters of the document cache. */
  DocumentCache::Stats cache_stats() const { return theCache.stats(); }

  /** The counters as one line for the log; empty without a cache. */
  std::string cache_stats_line() const { return theCache.stats_line(); }

private:
  StaticContext_t new_static_context( std::string const &file_name ) const;
  void load_predefined();
  bool load_predefined( std::string const &path, std::string const &name );
  QueryResult execute( std::string const &source, std::string const &file_name,
                       Variables const &vars, ResultSink &sink );

  Zorba *const theZorba;
  DatabaseConfig const theConfig;
  std::ostream &theLog;
  SecurityPolicy thePolicy;
  std::vector<PredefinedQuery> thePredefined;
  DocumentCache theCache;
};

} // namespace db
} // namespace zorba

#endif /* ZORBA_DB_DATABASE_H */
/* vim:set et sw=2 ts=2: */
