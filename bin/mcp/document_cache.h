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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <zorba/item.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>

namespace zorba {
namespace mcp {

/**
 * A small cache of the document records that the directory module parses
 * (http://zorba.io/modules/directory), kept across the queries of one
 * zorba-mcp server. It keeps a few small documents that are read repeatedly;
 * it is not meant to hold a tree.
 *
 * The directory module asks the module http://zorba.io/modules/directory-cache
 * (dc:get) for the record of a file before it reads and parses the file
 * itself. By default that module returns nothing; attach() makes a static
 * context resolve it to an external function backed by this cache.
 *
 * Admission: a file larger than the size limit gets one stat() and nothing
 * else. The first read of a smaller file only records a hash of its path and
 * metadata in a fixed table, and the module reads the file as it does without
 * a cache. The second read admits it: the cache runs dir:open in a query of
 * its own, compiled under the server's security policy with the installed
 * (non-caching) hook, and keeps the record that query returns. Records enter
 * only that way: a user query cannot hand the cache a record. A record with
 * an error is not kept, and neither is one whose file changed during the read.
 *
 * Eviction: at most \c max_files records stay, least recently used out first.
 * A record that the current query used is not evicted, and a document is then
 * not admitted, so a query pays for at most \c max_files admissions however
 * many documents it reads.
 *
 * Validity: a record is served only while the canonical path of its file and
 * the file's device, inode, size, modification time and change time are the
 * ones seen before it was read. Only files under the root are cached.
 */
class DocumentCache {
public:
  struct Stats {
    std::size_t hits = 0;
    std::size_t misses = 0;
    std::size_t admissions = 0;
    std::size_t evictions = 0;
    std::size_t entries = 0;
    std::size_t bytes = 0;      // the memory of the records (Item::mem_size)
  };

  /**
   * @param zorba the engine.
   * @param root the root directory (absolute, normalized).
   * @param max_file_bytes the largest file admitted; 0 disables the cache.
   * @param max_files the number of records kept; 0 disables the cache.
   * @param policy the policy the records are read under (read when the
   * first record is built, so it may be set up after construction).
   */
  DocumentCache( Zorba *zorba, std::string const &root,
                 std::uintmax_t max_file_bytes, std::size_t max_files,
                 SecurityPolicy const *policy );
  ~DocumentCache();

  DocumentCache( DocumentCache const& ) = delete;
  DocumentCache& operator=( DocumentCache const& ) = delete;

  bool enabled() const { return theMaxFileBytes > 0 && theMaxFiles > 0; }

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

  /**
   * Drops every entry and the reading query (the items must go before the
   * store shuts down).
   */
  void clear();

  Stats const& stats() const { return theStats; }

  /** The counters as one line for the log; empty if the cache is disabled. */
  std::string stats_line() const;

  /** The memory of the hit tracker. */
  static std::size_t tracker_bytes();

  /**
   * The record of \a path under \a root (dc:get): the cached record if its
   * file is unchanged, the record dir:open builds if this read admits the
   * document, else null (the directory module then reads the file itself).
   */
  Item get( std::string const &root, std::string const &path );

private:
  struct FileId {
    std::uintmax_t dev = 0, ino = 0, size = 0;
    std::int64_t mtime_ns = 0, ctime_ns = 0;
    bool operator==( FileId const& ) const = default;
  };

  struct Entry {
    std::string canonical;
    FileId id;
    Item record;
    std::size_t bytes;
    std::uint64_t query;                // the last query that used it
    std::list<std::string>::iterator lru;
  };

  // The hit tracker: one tag per slot, like a direct-mapped CPU cache. 32 KiB.
  static constexpr std::size_t tracker_slots = 4096;

  static bool file_id( std::string const &full, FileId *id );
  static std::uint64_t tag_of( std::string const &key, FileId const &id );
  bool canonical_under_root( std::string const &full,
                             std::string *canonical ) const;
  void erase( std::unordered_map<std::string,Entry>::iterator i );
  Item read( std::string const &root, std::string const &path,
             std::size_t *bytes );
  Item admit( std::string const &root, std::string const &path,
              std::string const &key, std::string const &full,
              FileId const &id, std::uint64_t *seen );

  Zorba *const theZorba;
  std::string theRoot;                  // canonical, with a trailing '/'
  std::uintmax_t const theMaxFileBytes;
  std::size_t const theMaxFiles;
  SecurityPolicy const *const thePolicy;
  XQuery_t theReader;                   // dir:open($root, $path)
  bool theActive = true;
  std::uint64_t theQuery = 0;           // incremented by set_active()

  std::vector<std::uint64_t> theSeen;   // the hit tracker
  std::unordered_map<std::string,Entry> theEntries;
  std::list<std::string> theLru;        // most recently used first
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
