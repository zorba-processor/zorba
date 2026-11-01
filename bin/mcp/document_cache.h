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

#ifndef ZORBA_MCP_DOCUMENT_CACHE_H
#define ZORBA_MCP_DOCUMENT_CACHE_H

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <zorba/item.h>
#include <zorba/static_context.h>

namespace zorba {
namespace mcp {

/**
 * A cache of the document records that the directory module parses
 * (http://zorba.io/modules/directory), kept across the queries of one
 * zorba-mcp server.
 *
 * The directory module asks the module http://zorba.io/modules/directory-cache
 * for a record before it reads and parses a file, and hands it every record
 * it builds. By default that module caches nothing; attach() makes a static
 * context resolve it to external functions backed by this cache.
 *
 * A record is keyed by the root and the path it was opened with, and is
 * valid while the canonical path of its file and the file's device, inode,
 * size, modification time and change time stay the same: the file is
 * stat()ed on every lookup, so a modified or deleted file is never served
 * from the cache (and its entry is dropped). Only files under the root are cached, and a record is only
 * stored after the module has read the file through the file module, under
 * the query's security policy. Records are bounded by a byte budget, each
 * accounted by an estimate of its in-memory size (the file size times a
 * per-format factor measured with bench/directory), and evicted least
 * recently used first; an entry used by the current query is not evicted
 * (a record that does not fit then is not stored), so a query that reads
 * more than the budget keeps the documents cached first.
 */
class DocumentCache {
public:
  struct Stats {
    std::size_t hits = 0;
    std::size_t misses = 0;
    std::size_t stores = 0;
    std::size_t evictions = 0;
    std::size_t entries = 0;
    std::size_t bytes = 0;      // sum of the estimates of the entries
  };

  /**
   * @param root the root directory (absolute, normalized).
   * @param budget the byte budget; 0 disables the cache.
   */
  DocumentCache( std::string const &root, std::size_t budget );
  ~DocumentCache();

  DocumentCache( DocumentCache const& ) = delete;
  DocumentCache& operator=( DocumentCache const& ) = delete;

  bool enabled() const { return theBudget > 0; }

  /**
   * Makes queries compiled with \a sctx use this cache. A no-op if the cache
   * is disabled.
   */
  void attach( StaticContext_t const &sctx );

  /**
   * Whether the current query may use the cache. A query that can apply
   * updates (updating or sequential) does not: it could modify the cached
   * documents. Called before each query; when \a active, also drops the
   * entries of files deleted or modified since they were stored.
   */
  void set_active( bool active );

  /** Drops every entry (the items must go before the store shuts down). */
  void clear();

  Stats stats() const;

  /** The estimated in-memory size of a parsed file of \a size bytes. */
  static std::size_t estimate( std::string const &path, std::uintmax_t size );

  // Called by the external functions.
  Item get( std::string const &root, std::string const &path );
  Item put( std::string const &root, std::string const &path,
            Item const &record );

private:
  struct Signature {
    std::string canonical;
    std::uintmax_t dev = 0, ino = 0, size = 0;
    std::int64_t mtime_ns = 0, ctime_ns = 0;
    bool operator==( Signature const& ) const = default;
  };

  struct Entry {
    Signature sig;
    Item record;
    std::size_t bytes;
    std::uint64_t query;                // the last query that used it
    std::list<std::string>::iterator lru;
  };

  bool key_of( std::string const &root, std::string const &path,
               std::string *key, Signature *sig ) const;
  bool signature_of( std::string const &full, Signature *sig ) const;
  void erase( std::unordered_map<std::string,Entry>::iterator i );

  std::string theRoot;                  // canonical, with a trailing '/'
  std::size_t const theBudget;
  bool theActive = true;
  std::uint64_t theQuery = 0;           // incremented by set_active()

  mutable std::mutex theMutex;
  std::unordered_map<std::string,Entry> theEntries;
  std::list<std::string> theLru;        // most recently used first
  std::map<std::string,Signature> thePending;   // looked up, not stored yet
  Stats theStats;

  class Module;
  class Mapper;
  class Resolver;
  std::unique_ptr<Module> theModule;
  std::unique_ptr<Mapper> theMapper;
  std::unique_ptr<Resolver> theResolver;
};

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_DOCUMENT_CACHE_H */
/* vim:set et sw=2 ts=2: */
