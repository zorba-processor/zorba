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

#ifndef ZORBA_DB_SCANNER_H
#define ZORBA_DB_SCANNER_H

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace zorba {
namespace db {

/** What the scanner may use. */
struct ScanLimits {
  /** Bytes read from one file. */
  std::size_t max_file_bytes = 1024 * 1024;
  /**
   * Bytes the cache may take: the index of files and directories, and the
   * previews (see --scan-cache-mb). Only what the threads hold on to counts,
   * not the transient buffer of a file being read.
   */
  std::size_t max_memory = 64 * 1024 * 1024;
};

/**
 * Called by the scanner's threads before they list a directory (stage "walk")
 * and before they read a file (stage "sample"), and by a request before it reads
 * a file (stage "explore"), with the path relative to the root. For tests, to
 * hold a thread at a known point or count reads.
 */
typedef std::function<void( char const *stage, std::string const &path )>
  ScanHook;

/** What a listing asks for (see Scanner::answer()). */
struct ListRequest {
  /** A subdirectory or a file, relative to the root; empty: the root. */
  std::string path;
  /** The most files listed. */
  std::size_t limit = 100;
  /** Whether to restart the scan first. */
  bool refresh = false;
};

/**
 * The background scan of a directory for list_documents: one thread walks the
 * tree, another reads the head of each file for its preview, and a request
 * answers from what they have found so far, plus a status.
 *
 * These threads must not touch the Zorba engine, which is single-threaded (see
 * doc/zorba/threading.dox): this file and its .cpp files include nothing of
 * it. Only the thread that calls answer() writes answers; the scanner's
 * threads write to nothing but their shared state.
 *
 * Confinement: everything is opened from a descriptor of the root, one path
 * component at a time, without following symbolic links, so that no path
 * leads out of the root even if the tree changes during the scan. Entries
 * whose name starts with a dot are skipped, like in dir:list.
 *
 * Memory: everything held counts against ScanLimits::max_memory, under one
 * mutex with the threads. At the limit the sampler stops (it never evicts to go
 * on), and the state becomes memory_limit. A request evicts the least recently
 * used previews to keep what it read. Previews go before the index: the walker
 * evicts them to go on, and when there are none left it stops too, and listings
 * say they may be incomplete.
 *
 * Queries do not use any of this: they read the files themselves.
 */
class Scanner {
public:
  /**
   * Starts the threads. Call on the main thread: it initializes libxml2's
   * global state, and the scanner must be stopped before that is cleaned up
   * (Zorba::shutdown()).
   *
   * @param root the root directory: absolute and normalized.
   * @throw std::system_error if the threads cannot be started.
   */
  Scanner( std::string const &root, ScanLimits const &limits, ScanHook hook );

  /** Calls stop(). */
  ~Scanner();

  Scanner( Scanner const& ) = delete;
  Scanner& operator=( Scanner const& ) = delete;

  /**
   * Stops the threads. A thread that does not end soon, because the file
   * system does not return from a read, is left behind (detached): once it
   * returns, it ends without using anything but its shared state. If a thread
   * was left behind, do not call Zorba::shutdown(), which cleans up libxml2:
   * the thread could be in it. End the process instead.
   *
   * @return whether both threads were joined.
   */
  bool stop();

  /**
   * Answers one listing request. The first line is an object with the member
   * \c scan (the status) and either \c total, \c shown, maybe \c incomplete and
   * \c dirs, followed by a line per file (path, format, size and the top-level
   * \c fields, nothing nested), or a file's members with \c preview (the whole
   * preview), or \c error.
   *
   * @param request what to list.
   * @param max_output the maximum size of the answer, in bytes (the first
   * line can go over it, if smaller than that).
   * @param wait how long to wait for the sampler to read a file asked for, and
   * for a large directory to be listed; then the answer has what is known, and
   * \\c "preview_state":"pending" if the preview is not there yet.
   * @param is_error set to whether the answer is an error.
   * @return the answer.
   */
  std::string answer( ListRequest const &request, std::size_t max_output,
                      std::chrono::milliseconds wait, bool *is_error );

  /**
   * The status as answers carry it: the member \c "scan":{...}, as JSON text
   * without braces around it.
   */
  std::string status() const;

  /** The bytes the cache accounts for (see ScanLimits::max_memory). */
  std::size_t cache_bytes() const;

  /** One line on the cache, for the log. */
  std::string cache_summary() const;

private:
  struct Shared;

  std::shared_ptr<Shared> theShared;
  std::thread theWalker;
  std::thread theSampler;
};

} // namespace db
} // namespace zorba

#endif /* ZORBA_DB_SCANNER_H */
/* vim:set et sw=2 ts=2: */
