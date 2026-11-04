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
#include <exception>
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
// Known directories whose mtime one directory request checks.
std::size_t const max_dir_checks = 1024;
// Files whose preview requests are waiting for, at most.
std::size_t const max_requests = 64;
// How deep the walker goes: opening a directory takes one openat per level, so
// a walk takes time in the square of the depth. A path below still works.
std::size_t const max_walk_depth = 256;

typedef std::chrono::steady_clock scan_clock;

std::int64_t mtime_of( struct stat const &st ) {
#ifdef __APPLE__
  struct timespec const &t = st.st_mtimespec;
#else
  struct timespec const &t = st.st_mtim;
#endif
  // Only compared with another such value: it may wrap for times past 2262.
  return static_cast<std::int64_t>(
    static_cast<std::uint64_t>( t.tv_sec ) * 1000000000u +
    static_cast<std::uint64_t>( t.tv_nsec ) );
}

// Descriptors here are opened read-only: a failed close loses nothing, and
// there is nothing to do about it.
bool close_fd( int fd ) {
  return close( fd ) == 0;
}

bool close_dir( DIR *dir ) {
  return closedir( dir ) == 0;
}

bool is_denied( int error ) {
  return error == EACCES || error == EPERM;
}

// Opens the directory rel under root_fd one component at a time, following no
// link. Returns -1 (and errno) on failure.
int open_directory( int root_fd, std::string const &rel ) {
  int const flags = O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC;
  if ( rel.empty() )
    return openat( root_fd, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC );
  int cur = root_fd;
  std::size_t start = 0;
  for ( ;; ) {
    std::size_t const slash = rel.find( '/', start );
    bool const last = slash == std::string::npos;
    std::string const name( rel, start, last ? slash : slash - start );
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

char const* base_name( std::string const &rel ) {
  std::size_t const slash = rel.rfind( '/' );
  return rel.c_str() + ( slash == std::string::npos ? 0 : slash + 1 );
}

std::string parent_of( std::string const &rel ) {
  std::size_t const slash = rel.rfind( '/' );
  return slash == std::string::npos ? std::string() : rel.substr( 0, slash );
}

// The status of rel, not following a link; 0 or an errno. Nothing is opened
// but the directories on the way, so a FIFO or a device sees nothing of it.
int stat_under( int root_fd, std::string const &rel, struct stat *st ) {
  int const parent = open_directory( root_fd, parent_of( rel ) );
  if ( parent < 0 )
    return errno;
  int const failed = fstatat( parent, base_name( rel ), st, AT_SYMLINK_NOFOLLOW );
  int const error = failed ? errno : 0;
  close_fd( parent );
  return error;
}

// Opens a regular file: only after seeing that it is one, and only if what got
// opened is what was seen. Returns -1 (and errno) otherwise.
int open_regular( int root_fd, std::string const &rel, struct stat *st ) {
  int const parent = open_directory( root_fd, parent_of( rel ) );
  if ( parent < 0 )
    return -1;
  struct stat before;
  int fd = -1;
  int error = 0;
  if ( fstatat( parent, base_name( rel ), &before, AT_SYMLINK_NOFOLLOW ) != 0 ) {
    error = errno;
  } else if ( !S_ISREG( before.st_mode ) ) {
    error = ENOENT;
  } else {
    fd = openat( parent, base_name( rel ),
                 O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_NOCTTY | O_CLOEXEC );
    if ( fd < 0 ) {
      error = errno;
    } else if ( fstat( fd, st ) != 0 ) {
      error = errno;
      close_fd( fd );
      fd = -1;
    } else if ( st->st_dev != before.st_dev || st->st_ino != before.st_ino ) {
      error = ENOENT;
      close_fd( fd );
      fd = -1;
    }
  }
  close_fd( parent );
  errno = error;
  return fd;
}

struct DirEntry {
  std::string name;
  bool is_dir;
  std::uint64_t size;
  std::int64_t mtime;
};

// Lists the regular files and directories directly in rel, in batches to
// fn(batch), which returns whether to go on; nothing holds the whole directory.
// Links and dot entries are left out; fstatat does not follow links. Returns 0,
// or an errno if rel could not be listed in full, and *whole says whether all of
// it went through fn.
template <typename Fn>
int list_directory( int root_fd, std::string const &rel,
                    std::atomic<bool> const &stopping, Fn fn, bool *whole ) {
  *whole = false;
  int const fd = open_directory( root_fd, rel );
  if ( fd < 0 )
    return errno;
  DIR *const dir = fdopendir( fd );
  if ( !dir ) {
    int const error = errno;
    close_fd( fd );
    return error;
  }
  std::vector<DirEntry> batch;
  int error = 0;
  bool done = true;
  for ( ;; ) {
    errno = 0;
    struct dirent const *const de = readdir( dir );
    if ( !de ) {
      error = errno;
      break;
    }
    if ( stopping.load( std::memory_order_relaxed ) ) {
      done = false;
      break;
    }
    if ( de->d_name[0] == '.' )
      continue;
    struct stat st;
    if ( fstatat( fd, de->d_name, &st, AT_SYMLINK_NOFOLLOW ) != 0 ) {
      if ( is_denied( errno ) )
        error = errno;                  // names can be read, but not what they are
      continue;
    }
    bool const is_dir = S_ISDIR( st.st_mode );
    if ( !is_dir && !S_ISREG( st.st_mode ) )
      continue;
    batch.push_back( DirEntry{ de->d_name, is_dir,
                               static_cast<std::uint64_t>( st.st_size ),
                               mtime_of( st ) } );
    if ( batch.size() >= merge_batch ) {
      if ( !fn( batch ) ) {
        done = false;
        break;
      }
      batch.clear();
    }
  }
  if ( done && !batch.empty() && !fn( batch ) )
    done = false;
  close_dir( dir );
  *whole = done && error == 0;
  return error;
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
  append_json_string( out, scan_path_text( rel ) );
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
    std::uint64_t seen = 0;             // the stamp of the last listing with it
    bool queued = false;                // given to the sampler
    bool sampled = false;               // has its preview, or has none to have
    bool has_preview = false;
    std::string preview;
    std::string fields;
    std::size_t preview_cost = 0;       // what preview and fields are charged
    std::list<Entries::iterator>::iterator lru;  // if has_preview
  };

  struct KnownDir {
    std::int64_t mtime = 0;
    std::uint64_t seen = 0;
  };

  // A file a request wants the preview of, read by the sampler.
  struct Request {
    explicit Request( std::string const &path ) : rel( path ) { }
    std::string const rel;
    bool done = false;
    std::string preview;
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
  std::uint64_t theStamp = 0;           // each listing adds one
  std::deque<std::string> theDirs;      // directories to walk
  std::deque<std::string> theFiles;     // files to sample
  std::deque<std::shared_ptr<Request> > theRequests;  // before theFiles
  std::shared_ptr<Request> theCurrentRequest;         // being read
  bool theWalkerBusy = false;
  bool theSamplerBusy = false;
  bool theWalkerDone = false;
  bool theSamplerDone = false;
  Entries theEntries;                   // sorted by path, as listings are
  std::map<std::string,KnownDir> theKnownDirs;  // walked or queued
  std::set<std::string> theUnreadable;  // directories that could not be listed
  std::string theDirCursor;             // where the mtime checks go on from
  std::list<Entries::iterator> theLru;  // entries with previews; front: recent
  std::size_t theSampled = 0;           // entries with sampled set
  std::size_t theBytes = 0;             // charged, all in
  std::size_t thePreviewBytes = 0;      // the part of it that can be evicted
  bool theLimitHit = false;             // the state is memory_limit when idle
  bool theSamplingStopped = false;
  bool theIndexFull = false;            // the listing may miss files
  bool theTooDeep = false;              // the walk left out what is too deep

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
    return sizeof( KnownDir ) + 32 + heap_block_overhead +
      2 * string_cost( rel.size() );
  }

  static std::size_t preview_cost( ScanPreview const &preview ) {
    return string_cost( preview.json.size() ) +
      string_cost( preview.fields.size() ) +
      sizeof( std::list<Entries::iterator>::value_type ) + 16 +
      heap_block_overhead;
  }

  static bool has_prefix( std::string const &s, std::string const &prefix ) {
    return s.compare( 0, prefix.size(), prefix ) == 0;
  }

  // Guarded by theMutex, from here on.

  bool idle() const {
    return theDirs.empty() && !theWalkerBusy && theFiles.empty() &&
      theRequests.empty() && !theSamplerBusy;
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

  void forget_unreadable( std::string const &rel ) {
    std::set<std::string>::iterator const it = theUnreadable.find( rel );
    if ( it != theUnreadable.end() ) {
      theBytes -= dir_cost( *it );
      theUnreadable.erase( it );
    }
  }

  void note_unreadable( std::string const &rel ) {
    if ( !theUnreadable.count( rel ) && room_in_index( dir_cost( rel ) ) ) {
      theBytes += dir_cost( rel );
      theUnreadable.insert( rel );
    }
  }

  void erase_tree( std::string const &rel ) {
    Entries::iterator it = theEntries.find( rel );
    if ( it != theEntries.end() )
      erase( it );
    std::string const prefix( rel + "/" );
    it = theEntries.lower_bound( prefix );
    while ( it != theEntries.end() && has_prefix( it->first, prefix ) )
      erase( it++ );
    auto d = theKnownDirs.lower_bound( prefix );
    while ( d != theKnownDirs.end() && has_prefix( d->first, prefix ) ) {
      theBytes -= dir_cost( d->first );
      d = theKnownDirs.erase( d );
    }
    d = theKnownDirs.find( rel );
    if ( d != theKnownDirs.end() ) {
      theBytes -= dir_cost( d->first );
      theKnownDirs.erase( d );
    }
    forget_unreadable( rel );
    auto u = theUnreadable.lower_bound( prefix );
    while ( u != theUnreadable.end() && has_prefix( *u, prefix ) ) {
      theBytes -= dir_cost( *u );
      u = theUnreadable.erase( u );
    }
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
      it = theEntries.emplace( rel, Entry() ).first;
      theBytes += cost;
      it->second.format = scan_format( rel );
      it->second.size = size;
      it->second.mtime = mtime;
    }
    Entry &e = it->second;
    e.seen = theStamp;
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

  // Records a subdirectory: queued if new or changed.
  void merge_dir( std::string const &rel, std::int64_t mtime ) {
    auto const it = theKnownDirs.find( rel );
    if ( it != theKnownDirs.end() ) {
      it->second.seen = theStamp;
      if ( it->second.mtime != mtime ) {
        it->second.mtime = mtime;
        if ( !theIndexFull )
          theDirs.push_back( rel );
      }
      return;
    }
    if ( theIndexFull )
      return;
    if ( static_cast<std::size_t>( std::count( rel.begin(), rel.end(), '/' ) ) >=
         max_walk_depth ) {
      theTooDeep = true;
      return;
    }
    std::size_t const cost = dir_cost( rel );
    if ( !room_in_index( cost ) )
      return;
    KnownDir known;
    known.mtime = mtime;
    known.seen = theStamp;
    theKnownDirs.emplace( rel, known );
    theBytes += cost;
    theDirs.push_back( rel );
  }

  static std::string child( std::string const &dir, std::string const &name ) {
    return dir.empty() ? name : dir + "/" + name;
  }

  // What a listing that went through a whole directory did not see is gone.
  void remove_unseen( std::string const &dir, std::uint64_t start ) {
    std::string const prefix( dir.empty() ? dir : dir + "/" );
    Entries::iterator it = theEntries.lower_bound( prefix );
    while ( it != theEntries.end() && has_prefix( it->first, prefix ) ) {
      std::size_t const slash = it->first.find( '/', prefix.size() );
      if ( slash == std::string::npos ) {
        if ( it->second.seen < start )
          erase( it++ );
        else
          ++it;
      } else {
        // '0' follows '/': past everything under this subdirectory.
        it = theEntries.lower_bound( it->first.substr( 0, slash ) + "0" );
      }
    }
    std::vector<std::string> gone;
    for ( auto d = theKnownDirs.lower_bound( prefix );
          d != theKnownDirs.end() && has_prefix( d->first, prefix ); ++d )
      if ( d->second.seen < start &&
           d->first.find( '/', prefix.size() ) == std::string::npos )
        gone.push_back( d->first );
    for ( std::string const &rel : gone )
      erase_tree( rel );
  }

  // Files that have no room in the index, for an answer about their directory.
  struct Unindexed {
    std::size_t keep = 0;
    std::size_t total = 0;
    std::vector<DirEntry> files;

    void add( DirEntry const &de ) {
      ++total;
      if ( keep == 0 )
        return;
      files.push_back( de );
      if ( files.size() >= 2 * keep + merge_batch )
        trim();
    }

    void trim() {
      std::sort( files.begin(), files.end(),
                 []( DirEntry const &a, DirEntry const &b ) {
                   return a.name < b.name;
                 } );
      if ( files.size() > keep )
        files.resize( keep );
    }
  };

  // Lists one directory into what is known, in batches. A walk ends at a full
  // index; a request goes on, for its unindexed files. Returns 0 or an errno.
  int list_level( std::string const &dir, std::uint32_t gen,
                  std::atomic<bool> const &stop,
                  scan_clock::time_point const *deadline, Unindexed *unindexed ) {
    std::uint64_t start;
    {
      std::lock_guard<std::mutex> lock( theMutex );
      start = ++theStamp;
    }
    bool whole = false;
    int const error = list_directory( theRootFd, dir, stop,
      [&]( std::vector<DirEntry> const &batch ) {
        std::lock_guard<std::mutex> guard( theMutex );
        if ( gen != theGen || theStopping || ( !unindexed && theIndexFull ) )
          return false;                 // restarted, or nothing more fits
        for ( DirEntry const &de : batch ) {
          std::string const rel( child( dir, de.name ) );
          if ( de.is_dir )
            merge_dir( rel, de.mtime );
          else if ( !merge_file( rel, de.size, de.mtime ) && unindexed )
            unindexed->add( de );
        }
        theCv.notify_all();
        return !deadline || scan_clock::now() <= *deadline;
      }, &whole );

    std::lock_guard<std::mutex> lock( theMutex );
    if ( gen != theGen )
      return error;
    if ( is_denied( error ) ) {
      note_unreadable( dir );
    } else if ( whole ) {
      forget_unreadable( dir );
      remove_unseen( dir, start );
    }
    return error;
  }

  // Finds changes further down than a request lists: a directory whose mtime
  // changed is walked again. At most max_dir_checks per request, going round.
  void check_known_dirs( std::string const &prefix ) {
    std::vector<std::pair<std::string,std::int64_t> > known;
    {
      std::lock_guard<std::mutex> lock( theMutex );
      auto d = theKnownDirs.lower_bound( std::max( theDirCursor, prefix ) );
      for ( ; d != theKnownDirs.end() && has_prefix( d->first, prefix ) &&
              known.size() < max_dir_checks; ++d )
        known.emplace_back( d->first, d->second.mtime );
      theDirCursor = known.size() == max_dir_checks ? known.back().first
                                                    : std::string();
    }
    std::vector<std::pair<std::string,std::int64_t> > changed;
    std::vector<std::string> gone;
    for ( auto const &k : known ) {
      struct stat st;
      // A link on the way is harmless: only the time is used, and the walker
      // opens the directory without following any.
      if ( fstatat( theRootFd, k.first.c_str(), &st, AT_SYMLINK_NOFOLLOW ) != 0 ) {
        if ( errno == ENOENT || errno == ENOTDIR )
          gone.push_back( k.first );
      } else if ( !S_ISDIR( st.st_mode ) ) {
        gone.push_back( k.first );
      } else if ( mtime_of( st ) != k.second ) {
        changed.emplace_back( k.first, mtime_of( st ) );
      }
    }
    if ( changed.empty() && gone.empty() )
      return;
    std::lock_guard<std::mutex> lock( theMutex );
    for ( std::string const &rel : gone )
      erase_tree( rel );
    for ( auto const &c : changed ) {
      auto const it = theKnownDirs.find( c.first );
      if ( it != theKnownDirs.end() ) {
        it->second.mtime = c.second;
        if ( !theIndexFull )
          theDirs.push_back( c.first );
      }
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
  ScanPreview preview_of( int fd, ScanFormat format, std::uint64_t size ) const {
    std::string data;
    bool truncated = false;
    bool const ok = read_capped( fd, theLimits.max_file_bytes, size, &theStopping,
                                 &data, &truncated );
    int const saved = errno;
    close_fd( fd );
    ScanPreview preview;
    if ( !ok ) {
      preview.json = error_preview_text( "cannot read the file", saved );
      return preview;
    }
    if ( theStopping )
      return preview;                   // not to be used; libxml2 not touched
    return make_scan_preview( format, data, truncated, &theStopping );
  }

  static std::string error_preview_text( char const *what, int error ) {
    std::string json( "{\"error\":" );
    append_json_string( json, std::string( what ) + ": " + std::strerror( error ) );
    return json + "}";
  }

  // An allocation can fail in these threads (under --max-memory, while a query
  // holds most of it). The scan then ends where it is, as at its own limit,
  // and the server goes on.
  void walker() {
    for ( ;; ) {
      try {
        walk();
        break;
      }
      catch ( std::exception const& ) {
        std::lock_guard<std::mutex> lock( theMutex );
        theWalkerBusy = false;
        stop_sampling();
        theIndexFull = true;
        theDirs.clear();
        theCv.notify_all();
      }
    }
    std::lock_guard<std::mutex> lock( theMutex );
    theWalkerDone = true;
    theCv.notify_all();
  }

  void sampler() {
    // See Server::Server. The scan of a root starts with the server, before any
    // query can have taken the memory.
    xmlFreeDict( xmlDictCreate() );
    for ( ;; ) {
      try {
        sample();
        break;
      }
      catch ( std::exception const& ) {
        std::lock_guard<std::mutex> lock( theMutex );
        theSamplerBusy = false;
        stop_sampling();
        // A request being read stays pending; asking again sends a new one.
        theCurrentRequest.reset();
        theCv.notify_all();
      }
    }
    std::lock_guard<std::mutex> lock( theMutex );
    theSamplerDone = true;
    theCv.notify_all();
  }

  void walk() {
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
      list_level( dir, gen, theStopping, nullptr, nullptr );

      lock.lock();
      theWalkerBusy = false;
      theCv.notify_all();
    }
  }

  void sample() {
    std::unique_lock<std::mutex> lock( theMutex );
    while ( !theStopping ) {
      std::shared_ptr<Request> request;
      std::string rel;
      if ( !theRequests.empty() ) {
        request = theRequests.front();
        theRequests.pop_front();
        theCurrentRequest = request;
        rel = request->rel;
      } else if ( !theFiles.empty() ) {
        rel = theFiles.front();
        theFiles.pop_front();
        Entries::iterator const it = theEntries.find( rel );
        if ( it == theEntries.end() || it->second.sampled )
          continue;                     // deleted or asked for meanwhile
      } else {
        theCv.wait( lock, [this] {
          return theStopping || !theRequests.empty() || !theFiles.empty();
        } );
        continue;
      }
      std::uint32_t const gen = theGen;
      theSamplerBusy = true;
      lock.unlock();

      hook( request ? "explore" : "sample", rel );
      struct stat st;
      std::memset( &st, 0, sizeof st );
      int const fd = theStopping ? -1 : open_regular( theRootFd, rel, &st );
      int const open_errno = errno;
      ScanPreview preview;
      if ( fd >= 0 )
        preview = preview_of( fd, scan_format( rel ),
                              static_cast<std::uint64_t>( st.st_size ) );

      lock.lock();
      theSamplerBusy = false;
      if ( !theStopping ) {
        bool const gone = open_errno == ENOENT || open_errno == ELOOP ||
          open_errno == ENOTDIR;
        if ( fd < 0 )
          preview.json = error_preview_text( "cannot open the file", open_errno );
        if ( request ) {
          if ( fd >= 0 && gen == theGen &&
               merge_file( rel, static_cast<std::uint64_t>( st.st_size ),
                           mtime_of( st ) ) )
            store_requested( rel, preview );
          request->preview = preview.json;
          request->done = true;
          theCurrentRequest.reset();
        } else if ( fd >= 0 ) {
          store_sampled( rel, gen, static_cast<std::uint64_t>( st.st_size ),
                         mtime_of( st ), preview );
        } else if ( gen == theGen ) {
          Entries::iterator const e = theEntries.find( rel );
          if ( e != theEntries.end() ) {
            if ( gone )
              erase( e );               // or no longer a regular file
            else
              store_sampled( rel, gen, e->second.size, e->second.mtime, preview );
          }
        }
      }
      theCv.notify_all();
    }
  }

  // Answers, on the thread that calls Scanner::answer().

  void refresh() {
    std::lock_guard<std::mutex> lock( theMutex );
    ++theGen;
    theEntries.clear();
    theLru.clear();
    theKnownDirs.clear();
    theUnreadable.clear();
    theDirs.clear();
    theFiles.clear();
    theRequests.clear();
    theDirCursor.clear();
    theDirs.push_back( std::string() );
    theSampled = theBytes = thePreviewBytes = 0;
    theLimitHit = theSamplingStopped = theIndexFull = theTooDeep = false;
    theCv.notify_all();
  }

  // The request for this file that is on its way, or a new one if there is room.
  std::shared_ptr<Request> request_for( std::string const &rel ) {
    if ( theCurrentRequest && theCurrentRequest->rel == rel )
      return theCurrentRequest;
    for ( std::shared_ptr<Request> const &r : theRequests )
      if ( r->rel == rel )
        return r;
    if ( theRequests.size() >= max_requests )
      return std::shared_ptr<Request>();
    theRequests.push_back( std::make_shared<Request>( rel ) );
    theCv.notify_all();
    return theRequests.back();
  }

  // The request thread does not parse: the sampler does, with this file first,
  // and the request waits for that up to its bound.
  std::string file_answer( struct stat const &st, std::string const &rel,
                           std::size_t max_output,
                           std::chrono::milliseconds wait ) {
    ScanFormat const format = scan_format( rel );
    std::uint64_t const size = static_cast<std::uint64_t>( st.st_size );
    std::string preview( "null" );
    bool pending = false;
    std::string scan_status;
    {
      std::unique_lock<std::mutex> lock( theMutex );
      merge_file( rel, size, mtime_of( st ) );
      theCv.notify_all();
      if ( format != ScanFormat::none ) {
        Entries::iterator const it = theEntries.find( rel );
        if ( it != theEntries.end() && it->second.has_preview ) {
          preview = it->second.preview;
          theLru.splice( theLru.begin(), theLru, it->second.lru );
        } else {
          std::shared_ptr<Request> const request( request_for( rel ) );
          bool const done = request && theCv.wait_for( lock, wait, [&] {
            return request->done || theStopping;
          } ) && request->done;
          if ( done )
            preview = request->preview;
          else
            pending = true;
        }
      }
      scan_status = status();
    }

    std::string out( "{" + scan_status + "," );
    append_file_members( out, rel, format, size );
    std::size_t const closing = std::strlen( ",\"preview\":}" );
    if ( out.size() + closing + preview.size() > max_output )
      preview = "{\"error\":\"the preview is longer than the output limit\"}";
    out += ",\"preview\":" + preview;
    if ( pending )
      out += ",\"preview_state\":\"pending\"";
    out += "}";
    return out;
  }

  std::string directory_answer( std::string const &rel, std::size_t limit,
                                std::size_t max_output,
                                std::chrono::milliseconds wait, bool *is_error ) {
    // More than this cannot fit: do not read them.
    limit = std::min( limit, max_output / 32 + 1 );
    scan_clock::time_point const deadline = scan_clock::now() + wait;
    static std::atomic<bool> const never( false );

    Unindexed unindexed;
    unindexed.keep = limit;
    std::uint32_t gen;
    {
      std::lock_guard<std::mutex> lock( theMutex );
      gen = theGen;
    }
    int const error = list_level( rel, gen, never, &deadline, &unindexed );
    if ( error && !is_denied( error ) && error != EINTR ) {
      std::lock_guard<std::mutex> lock( theMutex );
      if ( error == ENOENT || error == ENOTDIR || error == ELOOP ) {
        *is_error = true;
        erase_tree( rel );
        return error_line( status(), "not-found",
                           "no such file or directory under the root" );
      }
    }
    if ( is_denied( error ) ) {
      *is_error = true;
      std::lock_guard<std::mutex> lock( theMutex );
      return error_line( status(), "permission-denied",
                         "the directory cannot be read" );
    }
    std::string const prefix( rel.empty() ? rel : rel + "/" );
    check_known_dirs( prefix );

    struct Shown {
      std::string rel;
      ScanFormat format;
      std::uint64_t size;
      std::int64_t mtime;
      std::string fields;
    };
    std::vector<Shown> shown;

    // Every file shown is as it is now. Those that are gone make room for the
    // next ones. Paths are sorted: most share a parent.
    {
      std::string after;
      bool started = false;
      std::string parent;
      int parent_fd = -1;
      while ( shown.size() < limit ) {
        std::vector<Shown> batch;
        {
          std::lock_guard<std::mutex> lock( theMutex );
          Entries::const_iterator it = started ? theEntries.upper_bound( after )
                                               : theEntries.lower_bound( prefix );
          for ( ; it != theEntries.end() && has_prefix( it->first, prefix ) &&
                  batch.size() < limit - shown.size(); ++it ) {
            Entry const &e = it->second;
            batch.push_back( Shown{ it->first, e.format, e.size, e.mtime, e.fields } );
          }
        }
        if ( batch.empty() )
          break;
        started = true;
        after = batch.back().rel;

        std::vector<std::string> gone;
        std::vector<Shown> changed;
        for ( Shown &f : batch ) {
          std::string const dir( parent_of( f.rel ) );
          if ( parent_fd < 0 || dir != parent ) {
            if ( parent_fd >= 0 )
              close_fd( parent_fd );
            parent_fd = open_directory( theRootFd, dir );
            parent = dir;
          }
          struct stat st;
          if ( parent_fd < 0 ||
               fstatat( parent_fd, base_name( f.rel ), &st,
                        AT_SYMLINK_NOFOLLOW ) != 0 ||
               !S_ISREG( st.st_mode ) ) {
            gone.push_back( f.rel );
            continue;
          }
          if ( static_cast<std::uint64_t>( st.st_size ) != f.size ||
               mtime_of( st ) != f.mtime ) {
            f.size = static_cast<std::uint64_t>( st.st_size );
            f.mtime = mtime_of( st );
            f.fields.clear();
            changed.push_back( f );
          }
          shown.push_back( f );
        }
        {
          std::lock_guard<std::mutex> lock( theMutex );
          for ( std::string const &path : gone ) {
            Entries::iterator const it = theEntries.find( path );
            if ( it != theEntries.end() )
              erase( it );
          }
          for ( Shown const &f : changed )
            merge_file( f.rel, f.size, f.mtime );
          if ( !changed.empty() )
            theCv.notify_all();
        }
        if ( scan_clock::now() > deadline )
          break;
      }
      if ( parent_fd >= 0 )
        close_fd( parent_fd );
    }

    // With a full index, the files of this directory that have no room in it
    // are still answered, from the listing just made.
    unindexed.trim();
    for ( DirEntry const &de : unindexed.files ) {
      std::string const path( child( rel, de.name ) );
      shown.push_back( Shown{ path, scan_format( path ), de.size, de.mtime,
                              std::string() } );
    }
    std::sort( shown.begin(), shown.end(),
               []( Shown const &a, Shown const &b ) { return a.rel < b.rel; } );
    if ( shown.size() > limit )
      shown.resize( limit );

    // What the answer states comes after all of that, so it is current.
    std::size_t total = unindexed.total;
    std::size_t dirs_total = 0;
    std::map<std::string,std::size_t> dirs;  // files known per subdirectory
    std::string scan_status;
    bool incomplete, is_idle;
    std::size_t unreadable;
    {
      std::lock_guard<std::mutex> lock( theMutex );
      if ( limit > 0 ) {
        for ( auto d = theKnownDirs.lower_bound( prefix );
              d != theKnownDirs.end() && has_prefix( d->first, prefix ); ++d ) {
          if ( d->first.find( '/', prefix.size() ) != std::string::npos )
            continue;
          ++dirs_total;
          if ( dirs.size() < max_dirs_shown )
            dirs[ d->first ] = 0;
        }
      }
      std::string last_dir;
      std::size_t *last_count = nullptr;
      for ( Entries::const_iterator it = theEntries.lower_bound( prefix );
            it != theEntries.end() && has_prefix( it->first, prefix ); ++it ) {
        ++total;
        if ( limit == 0 )
          continue;
        std::size_t const slash = it->first.find( '/', prefix.size() );
        if ( slash == std::string::npos )
          continue;
        if ( slash + 1 != last_dir.size() ||
             it->first.compare( 0, slash + 1, last_dir ) != 0 ) {
          last_dir = it->first.substr( 0, slash + 1 );
          auto const found = dirs.find( it->first.substr( 0, slash ) );
          last_count = found == dirs.end() ? nullptr : &found->second;
        }
        if ( last_count )
          ++*last_count;
      }
      scan_status = status();
      unreadable = theUnreadable.size();
      incomplete = theIndexFull || theTooDeep || unreadable > 0;
      is_idle = idle();
    }

    std::string dirs_members;
    if ( !dirs.empty() ) {
      dirs_members = ",\"dirs\":[";
      bool first = true;
      for ( auto const &d : dirs ) {
        if ( !first )
          dirs_members += ',';
        first = false;
        dirs_members += "{\"path\":";
        append_json_string( dirs_members, scan_path_text( d.first ) );
        dirs_members += ",\"files\":" + std::to_string( d.second ) + "}";
      }
      dirs_members += ']';
      if ( dirs_total > dirs.size() )
        dirs_members += ",\"dirs_total\":" + std::to_string( dirs_total );
      if ( dirs_members.size() > max_output / 2 )
        dirs_members.clear();
    }

    // The first line: its size with the largest counts it can have.
    std::size_t room = max_output;
    std::size_t const first_line = scan_status.size() + dirs_members.size() +
      std::strlen( "{,\"total\":,\"shown\":,\"incomplete\":true,\"unreadable_dirs\":}" ) +
      3 * 20;
    room = room > first_line ? room - first_line : 0;
    std::string lines;
    std::size_t shown_count = 0;
    for ( Shown const &f : shown ) {
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
    if ( unreadable > 0 )
      out += ",\"unreadable_dirs\":" + std::to_string( unreadable );
    // The files shown say what is under a directory, unless some are missing.
    if ( limit > 0 && ( shown_count < total || !is_idle || incomplete ) )
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
                             std::chrono::milliseconds wait, bool *is_error ) {
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
    return s.directory_answer( rel, limit, max_output, wait, is_error );

  // What is there now decides, and only a stat looks at it. A link gets the
  // answer of a missing file: it says nothing about its target.
  struct stat st;
  std::memset( &st, 0, sizeof st );
  int error = stat_under( s.theRootFd, rel, &st );
  if ( error == ENOENT && rel.find( "\\x" ) != std::string::npos ) {
    // A name that is not UTF-8, as answers write it.
    std::string const bytes( scan_path_from_text( rel ) );
    if ( stat_under( s.theRootFd, bytes, &st ) == 0 ) {
      rel = bytes;
      error = 0;
    }
  }
  if ( error == 0 && S_ISREG( st.st_mode ) )
    return s.file_answer( st, rel, max_output, wait );
  if ( error == 0 && S_ISDIR( st.st_mode ) )
    return s.directory_answer( rel, limit, max_output, wait, is_error );
  if ( is_denied( error ) )
    return fail( "permission-denied", "the path cannot be read" );
  {
    std::lock_guard<std::mutex> lock( s.theMutex );
    s.erase_tree( rel );
  }
  return fail( "not-found", "no such file or directory under the root" );
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
