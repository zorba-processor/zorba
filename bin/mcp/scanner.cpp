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

#include "scanner.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libxml/parser.h>

#include "scan_preview.h"

namespace zorba {
namespace mcp {

namespace {

// Files listed when a call does not say how many.
std::size_t const default_limit = 100;
// Subdirectories named in an answer that the limit cuts.
std::size_t const max_dirs_shown = 50;
// How long stop() waits for the threads.
std::chrono::seconds const stop_wait( 1 );
// How many entries of a directory are merged at a time.
std::size_t const merge_batch = 256;
std::size_t const read_chunk = 64 * 1024;

std::int64_t mtime_of( struct stat const &st ) {
#ifdef __APPLE__
  struct timespec const &t = st.st_mtimespec;
#else
  struct timespec const &t = st.st_mtim;
#endif
  return static_cast<std::int64_t>( t.tv_sec ) * 1000000000 + t.tv_nsec;
}

// Descriptors here are opened read-only: a failed close loses nothing, and
// there is nothing to do about it.
bool close_fd( int fd ) {
  return close( fd ) == 0;
}

bool close_dir( DIR *dir ) {
  return closedir( dir ) == 0;
}

// Opens rel under root_fd one component at a time, following no link: the
// intermediate components as directories, the last with last_flags. O_NONBLOCK
// because opening a FIFO would otherwise wait for a writer. Returns -1 (and
// errno) on failure.
int open_under( int root_fd, std::string const &rel, int last_flags ) {
  if ( rel.empty() )
    return openat( root_fd, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC );
  int cur = root_fd;
  std::size_t start = 0;
  for ( ;; ) {
    std::size_t const slash = rel.find( '/', start );
    bool const last = slash == std::string::npos;
    std::string const name( rel, start, last ? slash : slash - start );
    int const flags = O_RDONLY | O_NOFOLLOW | O_CLOEXEC |
      ( last ? last_flags | O_NONBLOCK : O_DIRECTORY );
    int const fd = openat( cur, name.c_str(), flags );
    int const saved = errno;
    if ( cur != root_fd )
      close_fd( cur );
    if ( fd < 0 || last ) {
      errno = saved;
      return fd;
    }
    cur = fd;
    start = slash + 1;
  }
}

struct DirEntry {
  std::string name;
  bool is_dir;
  std::uint64_t size;
  std::int64_t mtime;
};

// Lists the regular files and directories directly in rel, in batches to
// fn(batch), which returns whether to go on. Links and dot entries are left
// out; fstatat does not follow links. Returns whether the directory could be
// opened.
template <typename Fn>
bool list_directory( int root_fd, std::string const &rel,
                     std::atomic<bool> const &stopping, Fn fn ) {
  int const fd = open_under( root_fd, rel, O_DIRECTORY );
  if ( fd < 0 )
    return false;
  DIR *const dir = fdopendir( fd );
  if ( !dir ) {
    close_fd( fd );
    return false;
  }
  std::vector<DirEntry> batch;
  while ( struct dirent const *const de = readdir( dir ) ) {
    if ( stopping.load( std::memory_order_relaxed ) )
      break;
    if ( de->d_name[0] == '.' )
      continue;
    struct stat st;
    if ( fstatat( fd, de->d_name, &st, AT_SYMLINK_NOFOLLOW ) != 0 )
      continue;
    bool const is_dir = S_ISDIR( st.st_mode );
    if ( !is_dir && !S_ISREG( st.st_mode ) )
      continue;
    batch.push_back( DirEntry{ de->d_name, is_dir,
                               static_cast<std::uint64_t>( st.st_size ),
                               mtime_of( st ) } );
    if ( batch.size() >= merge_batch ) {
      if ( !fn( batch ) ) {
        close_dir( dir );
        return true;
      }
      batch.clear();
    }
  }
  if ( !batch.empty() )
    fn( batch );
  close_dir( dir );
  return true;
}

// Reads up to cap bytes; *truncated says whether the file goes on.
bool read_capped( int fd, std::size_t cap, std::uint64_t size_hint,
                  std::atomic<bool> const *stop, std::string *data,
                  bool *truncated ) {
  // One byte over the cap shows whether the file goes on.
  std::size_t const want = cap + 1;
  data->resize( static_cast<std::size_t>(
    std::min<std::uint64_t>( size_hint + 1, want ) ) );
  std::size_t got = 0;
  for ( ;; ) {
    if ( stop && stop->load( std::memory_order_relaxed ) )
      return false;
    if ( got == data->size() ) {
      if ( got >= want )
        break;
      data->resize( std::min( want, got + read_chunk ) );
    }
    std::size_t const n = std::min( data->size() - got, read_chunk );
    ssize_t const r = read( fd, &( *data )[ got ], n );
    if ( r < 0 ) {
      if ( errno == EINTR )
        continue;
      return false;
    }
    if ( r == 0 )
      break;
    got += static_cast<std::size_t>( r );
  }
  *truncated = got > cap;
  data->resize( std::min( got, cap ) );
  return true;
}

// The path of a request, with empty and "." segments dropped (like dir:open),
// or false if it is refused.
bool normalize_request_path( std::string const &path, std::string *rel ) {
  rel->clear();
  if ( !path.empty() && path[0] == '/' )
    return false;
  if ( path.find( '\0' ) != std::string::npos )
    return false;
  std::size_t start = 0;
  while ( start <= path.size() ) {
    std::size_t slash = path.find( '/', start );
    if ( slash == std::string::npos )
      slash = path.size();
    std::string const seg( path, start, slash - start );
    start = slash + 1;
    if ( seg.empty() || seg == "." )
      continue;
    if ( seg[0] == '.' )
      return false;                     // "..", and what listings hide
    if ( !rel->empty() )
      *rel += '/';
    *rel += seg;
  }
  return true;
}

std::string error_line( std::string const &status, char const *code,
                        std::string const &message ) {
  std::string out( "{\"error\":{\"code\":" );
  append_json_string( out, code );
  out += ",\"message\":";
  append_json_string( out, scan_valid_utf8( message ) );
  out += "}," + status + "}";
  return out;
}

void append_file_members( std::string &out, std::string const &rel,
                          ScanFormat format, std::uint64_t size ) {
  out += "\"path\":";
  append_json_string( out, scan_valid_utf8( rel ) );
  out += ",\"format\":";
  if ( format == ScanFormat::none )
    out += "null";
  else
    append_json_string( out, scan_format_name( format ) );
  out += ",\"size\":" + std::to_string( size );
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

struct Scanner::Shared {
  struct Entry;
  typedef std::map<std::string,Entry> Entries;

  struct Entry {
    ScanFormat format = ScanFormat::none;
    std::uint64_t size = 0;
    std::int64_t mtime = 0;
    bool queued = false;                // given to the sampler
    bool sampled = false;               // has its preview, or has none to have
    bool has_preview = false;
    std::string preview;
    std::string fields;
    std::size_t preview_cost = 0;       // what preview and fields are charged
    std::list<Entries::iterator>::iterator lru;  // if has_preview
  };

  Shared( std::string const &root, ScanLimits const &limits, ScanHook hook ) :
    theLimits( limits ),
    theHook( std::move( hook ) ),
    theRootFd( open( root.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC ) ) {
    theDirs.push_back( std::string() );
  }

  ~Shared() {
    if ( theRootFd >= 0 )
      close_fd( theRootFd );
  }

  ScanLimits const theLimits;
  ScanHook const theHook;
  int const theRootFd;
  std::atomic<bool> theStopping{ false };

  // All of the following is guarded by theMutex.
  std::mutex theMutex;
  std::condition_variable theCv;
  std::uint32_t theGen = 1;             // refresh adds one
  std::deque<std::string> theDirs;      // directories to walk
  std::deque<std::string> theFiles;     // files to sample
  bool theWalkerBusy = false;
  bool theSamplerBusy = false;
  bool theWalkerDone = false;
  bool theSamplerDone = false;
  Entries theEntries;                   // sorted by path, as listings are
  std::set<std::string> theKnownDirs;   // walked or queued
  std::list<Entries::iterator> theLru;  // entries with previews; front: recent
  std::size_t theSampled = 0;           // entries with sampled set
  std::size_t theBytes = 0;             // charged, all in
  std::size_t thePreviewBytes = 0;      // the part of it that can be evicted
  bool theLimitHit = false;             // the state is memory_limit when idle
  bool theSamplingStopped = false;
  bool theIndexFull = false;            // the listing may miss files

  void hook( char const *stage, std::string const &path ) const {
    if ( theHook )
      theHook( stage, path );
  }

  // What things are charged. It is meant to be on the high side: a tree node
  // and a heap block have overhead the code cannot see, and each path is
  // charged twice because it is also in a queue for a while.
  static std::size_t const heap_block_overhead = 32;

  static std::size_t string_cost( std::size_t length ) {
    return sizeof( std::string ) + length + 1 + heap_block_overhead;
  }

  static std::size_t entry_cost( std::string const &rel ) {
    return sizeof( Entries::value_type ) + heap_block_overhead + 32 +
      2 * string_cost( rel.size() );
  }

  static std::size_t dir_cost( std::string const &rel ) {
    return 32 + heap_block_overhead + 2 * string_cost( rel.size() );
  }

  static std::size_t preview_cost( ScanPreview const &preview ) {
    return string_cost( preview.json.size() ) +
      string_cost( preview.fields.size() ) +
      sizeof( std::list<Entries::iterator>::value_type ) + 16 +
      heap_block_overhead;
  }

  // Guarded by theMutex, from here on.

  bool idle() const {
    return theDirs.empty() && !theWalkerBusy && theFiles.empty() &&
      !theSamplerBusy;
  }

  std::string status() const {
    std::string s( "\"scan\":{\"state\":\"" );
    s += !idle() ? "running" : theLimitHit ? "memory_limit" : "complete";
    s += "\",\"files_found\":" + std::to_string( theEntries.size() ) +
      ",\"files_sampled\":" + std::to_string( theSampled ) + "}";
    return s;
  }

  // Forgets what was read from the file.
  void forget_preview( Entry &e ) {
    if ( e.has_preview ) {
      theLru.erase( e.lru );
      theBytes -= e.preview_cost;
      thePreviewBytes -= e.preview_cost;
      e.preview_cost = 0;
      std::string().swap( e.preview );
      std::string().swap( e.fields );
      e.has_preview = false;
    }
    if ( e.sampled ) {
      --theSampled;
      e.sampled = false;
    }
  }

  void stop_sampling() {
    theLimitHit = true;
    theSamplingStopped = true;
    theFiles.clear();
  }

  // Evicts the least recently used preview.
  void evict_coldest() {
    forget_preview( theLru.back()->second );
    stop_sampling();
  }

  // Makes room for the index, evicting previews. If that is not enough, the
  // walk ends here.
  bool room_in_index( std::size_t cost ) {
    while ( theBytes + cost > theLimits.max_memory ) {
      if ( theLru.empty() ) {
        stop_sampling();
        theIndexFull = true;
        theDirs.clear();
        return false;
      }
      evict_coldest();
    }
    return true;
  }

  void erase( Entries::iterator it ) {
    forget_preview( it->second );
    theBytes -= entry_cost( it->first );
    theEntries.erase( it );
  }

  void erase_dir( std::set<std::string>::iterator it ) {
    theBytes -= dir_cost( *it );
    theKnownDirs.erase( it );
  }

  void erase_tree( std::string const &rel ) {
    Entries::iterator it = theEntries.find( rel );
    if ( it != theEntries.end() )
      erase( it );
    std::string const prefix( rel + "/" );
    it = theEntries.lower_bound( prefix );
    while ( it != theEntries.end() &&
            it->first.compare( 0, prefix.size(), prefix ) == 0 )
      erase( it++ );
    auto d = theKnownDirs.lower_bound( prefix );
    while ( d != theKnownDirs.end() &&
            d->compare( 0, prefix.size(), prefix ) == 0 )
      erase_dir( d++ );
    d = theKnownDirs.find( rel );
    if ( d != theKnownDirs.end() )
      erase_dir( d );
  }

  void queue_sample( std::string const &rel, Entry &e ) {
    if ( e.sampled || e.queued || theSamplingStopped )
      return;
    e.queued = true;
    theFiles.push_back( rel );
  }

  // Records a file that was found. Returns false if the index has no room.
  bool merge_file( std::string const &rel, std::uint64_t size,
                   std::int64_t mtime ) {
    Entries::iterator it = theEntries.find( rel );
    if ( it == theEntries.end() ) {
      std::size_t const cost = entry_cost( rel );
      if ( !room_in_index( cost ) )
        return false;
      theBytes += cost;
      it = theEntries.emplace( rel, Entry() ).first;
      it->second.format = scan_format( rel );
      it->second.size = size;
      it->second.mtime = mtime;
    }
    Entry &e = it->second;
    if ( e.size != size || e.mtime != mtime ) {
      forget_preview( e );
      e.queued = false;
      e.size = size;
      e.mtime = mtime;
    }
    if ( e.format == ScanFormat::none ) {
      if ( !e.sampled ) {
        e.sampled = true;
        ++theSampled;
      }
      return true;
    }
    queue_sample( rel, e );
    return true;
  }

  // Queues a subdirectory unless it has been.
  void merge_dir( std::string const &rel ) {
    if ( theIndexFull || theKnownDirs.count( rel ) )
      return;
    std::size_t const cost = dir_cost( rel );
    if ( !room_in_index( cost ) )
      return;
    theBytes += cost;
    theKnownDirs.insert( rel );
    theDirs.push_back( rel );
  }

  static std::string child( std::string const &dir, std::string const &name ) {
    return dir.empty() ? name : dir + "/" + name;
  }

  void merge_batch_of( std::string const &dir,
                       std::vector<DirEntry> const &batch ) {
    for ( DirEntry const &de : batch ) {
      std::string const rel( child( dir, de.name ) );
      if ( de.is_dir )
        merge_dir( rel );
      else
        merge_file( rel, de.size, de.mtime );
    }
    theCv.notify_all();
  }

  void hold_preview( Entry &e, Entries::iterator it, ScanPreview const &preview,
                     std::size_t cost, bool recent ) {
    e.preview = preview.json;
    e.fields = preview.fields;
    e.has_preview = true;
    e.preview_cost = cost;
    theBytes += cost;
    thePreviewBytes += cost;
    // What a request asked for outlives what the scanner came across.
    e.lru = theLru.insert( recent ? theLru.begin() : theLru.end(), it );
    if ( !e.sampled ) {
      e.sampled = true;
      ++theSampled;
    }
  }

  // What the sampler read. At the limit it stops: a full pass that evicted
  // to go on would drop what requests asked for.
  void store_sampled( std::string const &rel, std::uint32_t gen,
                      std::uint64_t size, std::int64_t mtime,
                      ScanPreview const &preview ) {
    if ( gen != theGen )
      return;                           // read for a scan that was restarted
    Entries::iterator const it = theEntries.find( rel );
    if ( it == theEntries.end() || it->second.sampled )
      return;
    std::size_t const cost = preview_cost( preview );
    if ( theBytes + cost > theLimits.max_memory ) {
      stop_sampling();
      return;
    }
    Entry &e = it->second;
    forget_preview( e );
    e.size = size;
    e.mtime = mtime;
    hold_preview( e, it, preview, cost, false );
  }

  // What a request read: it evicts to fit, unless it cannot fit at all.
  void store_requested( std::string const &rel, ScanPreview const &preview ) {
    Entries::iterator const it = theEntries.find( rel );
    if ( it == theEntries.end() )
      return;                           // the index is full: not kept
    Entry &e = it->second;
    forget_preview( e );
    std::size_t const cost = preview_cost( preview );
    if ( theBytes - thePreviewBytes + cost > theLimits.max_memory ) {
      stop_sampling();
      return;
    }
    while ( theBytes + cost > theLimits.max_memory )
      evict_coldest();
    hold_preview( e, it, preview, cost, true );
  }

  // Unguarded: reading and parsing.

  // Reads and parses the open file; takes over fd.
  ScanPreview preview_of( int fd, ScanFormat format, std::uint64_t size,
                          std::atomic<bool> const *stop ) const {
    std::string data;
    bool truncated = false;
    bool const ok = read_capped( fd, theLimits.max_file_bytes, size, stop,
                                 &data, &truncated );
    int const saved = errno;
    close_fd( fd );
    ScanPreview preview;
    if ( !ok ) {
      preview.json = "{\"error\":";
      append_json_string( preview.json, std::string( "cannot read the file: " ) +
                          std::strerror( saved ) );
      preview.json += '}';
      return preview;
    }
    if ( stop && stop->load( std::memory_order_relaxed ) )
      return preview;                   // not to be used; libxml2 not touched
    return make_scan_preview( format, data, truncated, stop );
  }

  void walker() {
    std::unique_lock<std::mutex> lock( theMutex );
    while ( !theStopping ) {
      if ( theDirs.empty() ) {
        theCv.wait( lock, [this] { return theStopping || !theDirs.empty(); } );
        continue;
      }
      std::string const dir( theDirs.front() );
      theDirs.pop_front();
      std::uint32_t const gen = theGen;
      theWalkerBusy = true;
      lock.unlock();

      hook( "walk", dir );
      list_directory( theRootFd, dir, theStopping,
        [&]( std::vector<DirEntry> const &batch ) {
          std::lock_guard<std::mutex> guard( theMutex );
          if ( gen != theGen || theStopping || theIndexFull )
            return false;               // restarted, or nothing more fits
          merge_batch_of( dir, batch );
          return true;
        } );

      lock.lock();
      theWalkerBusy = false;
      theCv.notify_all();
    }
    theWalkerDone = true;
    theCv.notify_all();
  }

  void sampler() {
    std::unique_lock<std::mutex> lock( theMutex );
    while ( !theStopping ) {
      if ( theFiles.empty() ) {
        theCv.wait( lock, [this] { return theStopping || !theFiles.empty(); } );
        continue;
      }
      std::string const rel( theFiles.front() );
      theFiles.pop_front();
      Entries::iterator const it = theEntries.find( rel );
      if ( it == theEntries.end() || it->second.sampled )
        continue;                       // deleted or explored meanwhile
      ScanFormat const format = it->second.format;
      std::uint32_t const gen = theGen;
      theSamplerBusy = true;
      lock.unlock();

      hook( "sample", rel );
      int const fd = theStopping ? -1 : open_under( theRootFd, rel, 0 );
      int const open_errno = errno;
      struct stat st;
      bool regular = false;
      if ( fd >= 0 )
        regular = fstat( fd, &st ) == 0 && S_ISREG( st.st_mode );
      ScanPreview preview;
      if ( regular )
        preview = preview_of( fd, format,
                              static_cast<std::uint64_t>( st.st_size ),
                              &theStopping );
      else if ( fd >= 0 )
        close_fd( fd );

      lock.lock();
      theSamplerBusy = false;
      if ( !theStopping ) {
        Entries::iterator const e = theEntries.find( rel );
        if ( regular ) {
          store_sampled( rel, gen, static_cast<std::uint64_t>( st.st_size ),
                         mtime_of( st ), preview );
        } else if ( e != theEntries.end() && gen == theGen ) {
          if ( fd < 0 && open_errno != ENOENT && open_errno != ELOOP &&
               open_errno != ENOTDIR ) {
            // Still there, but not readable.
            ScanPreview unreadable;
            unreadable.json = "{\"error\":";
            append_json_string( unreadable.json,
              std::string( "cannot open the file: " ) +
              std::strerror( open_errno ) );
            unreadable.json += '}';
            store_sampled( rel, gen, e->second.size, e->second.mtime,
                           unreadable );
          } else {
            erase( e );                 // gone, or no longer a regular file
          }
        }
      }
      theCv.notify_all();
    }
    theSamplerDone = true;
    theCv.notify_all();
  }

  // Answers, on the thread that calls Scanner::answer().

  void refresh() {
    std::lock_guard<std::mutex> lock( theMutex );
    ++theGen;
    theEntries.clear();
    theLru.clear();
    theKnownDirs.clear();
    theDirs.clear();
    theFiles.clear();
    theDirs.push_back( std::string() );
    theSampled = theBytes = thePreviewBytes = 0;
    theLimitHit = theSamplingStopped = theIndexFull = false;
    theCv.notify_all();
  }

  std::string file_answer( int fd, struct stat const &st,
                           std::string const &rel, std::size_t max_output ) {
    ScanFormat const format = scan_format( rel );
    std::uint64_t const size = static_cast<std::uint64_t>( st.st_size );
    std::int64_t const mtime = mtime_of( st );
    std::string preview( "null" );
    bool known = format == ScanFormat::none;
    if ( !known ) {
      std::lock_guard<std::mutex> lock( theMutex );
      Entries::iterator const it = theEntries.find( rel );
      if ( it != theEntries.end() && it->second.has_preview &&
           it->second.size == size && it->second.mtime == mtime ) {
        preview = it->second.preview;
        theLru.splice( theLru.begin(), theLru, it->second.lru );
        known = true;
      }
    }
    ScanPreview fresh;
    if ( known ) {
      close_fd( fd );
    } else {
      hook( "explore", rel );
      fresh = preview_of( fd, format, size, nullptr );
      preview = fresh.json;
    }
    std::string scan_status;
    {
      std::lock_guard<std::mutex> lock( theMutex );
      merge_file( rel, size, mtime );
      if ( !known )
        store_requested( rel, fresh );
      theCv.notify_all();
      scan_status = status();
    }

    std::string out( "{" + scan_status + "," );
    append_file_members( out, rel, format, size );
    std::size_t const closing = std::strlen( ",\"preview\":}" );
    if ( out.size() + closing + preview.size() > max_output )
      preview = "{\"error\":\"the preview is longer than the output limit\"}";
    out += ",\"preview\":" + preview + "}";
    return out;
  }

  // Makes what is known about rel what is in it now, one level deep.
  bool relist( std::string const &rel, std::vector<DirEntry> *found ) {
    static std::atomic<bool> const never( false );
    bool const opened = list_directory( theRootFd, rel, never,
      [found]( std::vector<DirEntry> const &batch ) {
        found->insert( found->end(), batch.begin(), batch.end() );
        return true;
      } );
    if ( !opened )
      return false;
    std::unordered_set<std::string> files, dirs;
    for ( DirEntry const &de : *found )
      ( de.is_dir ? dirs : files ).insert( de.name );
    std::string const prefix( rel.empty() ? rel : rel + "/" );

    std::lock_guard<std::mutex> lock( theMutex );
    merge_batch_of( rel, *found );
    Entries::iterator it = theEntries.lower_bound( prefix );
    while ( it != theEntries.end() &&
            it->first.compare( 0, prefix.size(), prefix ) == 0 ) {
      std::size_t const slash = it->first.find( '/', prefix.size() );
      if ( slash == std::string::npos ) {
        if ( files.count( it->first.substr( prefix.size() ) ) )
          ++it;
        else
          erase( it++ );
      } else {
        std::string const name(
          it->first.substr( prefix.size(), slash - prefix.size() ) );
        if ( !dirs.count( name ) )
          erase_tree( prefix + name );
        // '0' follows '/': past everything under this subdirectory.
        it = theEntries.lower_bound( prefix + name + "0" );
      }
    }
    auto d = theKnownDirs.lower_bound( prefix );
    while ( d != theKnownDirs.end() &&
            d->compare( 0, prefix.size(), prefix ) == 0 ) {
      std::size_t const slash = d->find( '/', prefix.size() );
      std::string const name(
        d->substr( prefix.size(), slash == std::string::npos
                                  ? slash : slash - prefix.size() ) );
      if ( dirs.count( name ) )
        d = theKnownDirs.lower_bound( prefix + name + "0" );
      else
        erase_dir( d++ );
    }
    return true;
  }

  std::string directory_answer( std::string const &rel, std::size_t limit,
                                std::size_t max_output, bool *is_error ) {
    std::vector<DirEntry> found;
    if ( !relist( rel, &found ) ) {
      *is_error = true;
      std::lock_guard<std::mutex> lock( theMutex );
      erase_tree( rel );
      return error_line( status(), "not-found",
                         "no such file or directory under the root" );
    }
    // More than this cannot fit: do not stat them.
    limit = std::min( limit, max_output / 32 + 1 );

    struct Shown {
      std::string rel;
      ScanFormat format;
      std::uint64_t size;
      std::int64_t mtime;
      std::string fields;
      bool gone;
    };
    std::vector<Shown> shown;
    std::size_t total = 0;
    std::map<std::string,std::size_t> dirs;  // files known per subdirectory
    std::string const prefix( rel.empty() ? rel : rel + "/" );
    {
      std::lock_guard<std::mutex> lock( theMutex );
      std::string last_dir;
      std::size_t *last_count = nullptr;
      for ( Entries::const_iterator it = theEntries.lower_bound( prefix );
            it != theEntries.end() &&
            it->first.compare( 0, prefix.size(), prefix ) == 0; ++it ) {
        ++total;
        if ( limit == 0 )
          continue;
        if ( shown.size() < limit ) {
          Entry const &e = it->second;
          shown.push_back( Shown{ it->first, e.format, e.size, e.mtime,
                                  e.fields, false } );
        }
        std::size_t const slash = it->first.find( '/', prefix.size() );
        if ( slash == std::string::npos )
          continue;
        if ( slash - prefix.size() != last_dir.size() ||
             it->first.compare( prefix.size(), last_dir.size(), last_dir ) != 0 ) {
          last_dir = it->first.substr( prefix.size(), slash - prefix.size() );
          last_count = &dirs[ prefix + last_dir ];
        }
        ++*last_count;
      }
      // The ones the walker has not reached, or that have no files.
      if ( limit > 0 )
        for ( DirEntry const &de : found )
          if ( de.is_dir )
            dirs[ prefix + de.name ];
      // With a full index, the files of this directory that have no room in it
      // are still answered, from the listing just made.
      if ( theIndexFull ) {
        for ( DirEntry const &de : found ) {
          if ( de.is_dir )
            continue;
          std::string const path( child( rel, de.name ) );
          if ( theEntries.count( path ) )
            continue;
          ++total;
          shown.push_back( Shown{ path, scan_format( path ), de.size, de.mtime,
                                  std::string(), false } );
        }
        std::sort( shown.begin(), shown.end(),
                   []( Shown const &a, Shown const &b ) { return a.rel < b.rel; } );
        if ( limit < shown.size() )
          shown.resize( limit );
      }
    }

    // Every file shown is as it is now. Paths are sorted: most share a parent.
    {
      std::string parent;
      int parent_fd = -1;
      std::vector<Shown const*> changed;
      for ( Shown &f : shown ) {
        std::size_t const slash = f.rel.rfind( '/' );
        std::string const dir( slash == std::string::npos
                               ? std::string() : f.rel.substr( 0, slash ) );
        if ( parent_fd < 0 || dir != parent ) {
          if ( parent_fd >= 0 )
            close_fd( parent_fd );
          parent_fd = open_under( theRootFd, dir, O_DIRECTORY );
          parent = dir;
        }
        struct stat st;
        char const *const name =
          f.rel.c_str() + ( slash == std::string::npos ? 0 : slash + 1 );
        if ( parent_fd < 0 ||
             fstatat( parent_fd, name, &st, AT_SYMLINK_NOFOLLOW ) != 0 ||
             !S_ISREG( st.st_mode ) ) {
          f.gone = true;
        } else if ( static_cast<std::uint64_t>( st.st_size ) != f.size ||
                    mtime_of( st ) != f.mtime ) {
          changed.push_back( &f );
          f.size = static_cast<std::uint64_t>( st.st_size );
          f.mtime = mtime_of( st );
          f.fields.clear();
        }
      }
      if ( parent_fd >= 0 )
        close_fd( parent_fd );
      std::lock_guard<std::mutex> lock( theMutex );
      for ( Shown const &f : shown ) {
        if ( !f.gone )
          continue;
        Entries::iterator const it = theEntries.find( f.rel );
        if ( it != theEntries.end() )
          erase( it );
        --total;
      }
      for ( Shown const *const f : changed )
        merge_file( f->rel, f->size, f->mtime );
      if ( !changed.empty() )
        theCv.notify_all();
    }

    std::string scan_status;
    bool incomplete, idle;
    {
      std::lock_guard<std::mutex> lock( theMutex );
      scan_status = status();
      incomplete = theIndexFull;
      idle = this->idle();
    }
    std::string dirs_members;
    if ( !dirs.empty() ) {
      dirs_members = ",\"dirs\":[";
      std::size_t count = 0;
      for ( auto const &d : dirs ) {
        if ( count == max_dirs_shown )
          break;
        if ( count++ )
          dirs_members += ',';
        dirs_members += "{\"path\":";
        append_json_string( dirs_members, scan_valid_utf8( d.first ) );
        dirs_members += ",\"files\":" + std::to_string( d.second ) + "}";
      }
      dirs_members += ']';
      if ( dirs.size() > count )
        dirs_members += ",\"dirs_total\":" + std::to_string( dirs.size() );
      if ( dirs_members.size() > max_output / 2 )
        dirs_members.clear();
    }

    // The first line: its size with the largest counts it can have.
    std::size_t room = max_output;
    std::size_t const first_line = scan_status.size() + dirs_members.size() +
      std::strlen( "{,\"total\":,\"shown\":,\"incomplete\":true}" ) + 2 * 20;
    room = room > first_line ? room - first_line : 0;
    std::string lines;
    std::size_t shown_count = 0;
    for ( Shown const &f : shown ) {
      if ( f.gone )
        continue;
      std::string line( "\n{" );
      append_file_members( line, f.rel, f.format, f.size );
      if ( !f.fields.empty() )
        line += "," + f.fields;
      line += '}';
      if ( lines.size() + line.size() > room )
        break;
      lines += line;
      ++shown_count;
    }
    std::string out( "{" + scan_status + ",\"total\":" +
                     std::to_string( total ) + ",\"shown\":" +
                     std::to_string( shown_count ) );
    if ( incomplete )
      out += ",\"incomplete\":true";
    // The files shown say what is under a directory, unless some are missing.
    if ( limit > 0 && ( shown_count < total || !idle || incomplete ) )
      out += dirs_members;
    out += '}';
    return out + lines;
  }
};

///////////////////////////////////////////////////////////////////////////////

Scanner::Scanner( std::string const &root, ScanLimits const &limits,
                  ScanHook hook ) :
  theShared( std::make_shared<Shared>( root, limits, std::move( hook ) ) ) {
  // libxml2 initializes its global state once, on this thread; the threads
  // then only use contexts of their own. It is cleaned up by Zorba::shutdown().
  xmlInitParser();
  // The threads share ownership: one left behind by stop() keeps the state.
  std::shared_ptr<Shared> const shared( theShared );
  theWalker = std::thread( [shared] { shared->walker(); } );
  try {
    theSampler = std::thread( [shared] { shared->sampler(); } );
  }
  catch ( ... ) {
    {
      std::lock_guard<std::mutex> lock( shared->theMutex );
      shared->theStopping = true;
      shared->theCv.notify_all();
    }
    theWalker.join();
    throw;
  }
}

Scanner::~Scanner() {
  stop();
}

std::string Scanner::cache_summary() const {
  std::lock_guard<std::mutex> lock( theShared->theMutex );
  return std::to_string( theShared->theBytes ) + " bytes of " +
    std::to_string( theShared->theLimits.max_memory ) + ", " +
    std::to_string( theShared->theEntries.size() ) + " files, " +
    std::to_string( theShared->theLru.size() ) + " previews";
}

std::size_t Scanner::cache_bytes() const {
  std::lock_guard<std::mutex> lock( theShared->theMutex );
  return theShared->theBytes;
}

bool Scanner::stop() {
  if ( !theWalker.joinable() && !theSampler.joinable() )
    return true;
  bool walker_done, sampler_done;
  {
    std::unique_lock<std::mutex> lock( theShared->theMutex );
    theShared->theStopping = true;
    theShared->theCv.notify_all();
    Shared *const s = theShared.get();
    theShared->theCv.wait_for( lock, stop_wait, [s] {
      return s->theWalkerDone && s->theSamplerDone;
    } );
    walker_done = s->theWalkerDone;
    sampler_done = s->theSamplerDone;
  }
  if ( walker_done )
    theWalker.join();
  else
    theWalker.detach();
  if ( sampler_done )
    theSampler.join();
  else
    theSampler.detach();
  return walker_done && sampler_done;
}

std::string Scanner::answer( Json const &args, std::size_t max_output,
                             bool *is_error ) {
  Shared &s = *theShared;
  auto status = [&s]() {
    std::lock_guard<std::mutex> lock( s.theMutex );
    return s.status();
  };
  auto fail = [&]( char const *code, std::string const &message ) {
    *is_error = true;
    return error_line( status(), code, message );
  };
  *is_error = false;

  // The arguments.
  for ( auto const &m : args.members() )
    if ( m.first != "path" && m.first != "limit" && m.first != "refresh" )
      return fail( "invalid-arguments",
                   "list_documents has no argument " + m.first );
  std::string path;
  if ( Json const *const p = args.find( "path" ) ) {
    if ( !p->is_string() )
      return fail( "invalid-arguments", "path must be a string" );
    path = p->text();
  }
  std::size_t limit = default_limit;
  if ( Json const *const l = args.find( "limit" ) ) {
    if ( !l->is_integer() || l->text()[0] == '-' )
      return fail( "invalid-arguments", "limit must be an integer of 0 or more" );
    limit = static_cast<std::size_t>( std::strtoull( l->text().c_str(), nullptr, 10 ) );
  }
  bool refresh = false;
  if ( Json const *const r = args.find( "refresh" ) ) {
    if ( !r->is_bool() )
      return fail( "invalid-arguments", "refresh must be a boolean" );
    refresh = r->as_bool();
  }

  std::string rel;
  if ( !normalize_request_path( path, &rel ) )
    return fail( "invalid-path",
                 "a path is relative to the root, without .. or hidden segments" );

  if ( refresh )
    s.refresh();

  if ( rel.empty() )
    return s.directory_answer( rel, limit, max_output, is_error );

  // What is there now decides. A link gets the answer of a missing file: it
  // says nothing about its target.
  int const fd = open_under( s.theRootFd, rel, 0 );
  struct stat st;
  if ( fd >= 0 && fstat( fd, &st ) == 0 ) {
    if ( S_ISREG( st.st_mode ) )
      return s.file_answer( fd, st, rel, max_output );
    if ( S_ISDIR( st.st_mode ) ) {
      close_fd( fd );
      return s.directory_answer( rel, limit, max_output, is_error );
    }
  }
  if ( fd >= 0 )
    close_fd( fd );
  {
    std::lock_guard<std::mutex> lock( s.theMutex );
    s.erase_tree( rel );
  }
  return fail( "not-found", "no such file or directory under the root" );
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
