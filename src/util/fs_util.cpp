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

#include <cstdlib>                     /* for getenv(3), _dupenv_s() */
#include <climits>                     /* for PATH_MAX */
#include <cstdio>
#include <fcntl.h>                     /* for creat(2) */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>                    /* for chdir(2) */

#include <zorba/internal/cxx_util.h>

#include "diagnostics/xquery_diagnostics.h"
#include "zorbautils/mutex.h"

#include "ascii_util.h"
#include "fs_util.h"
#include "string_util.h"
#include "uri_util.h"

using namespace std;

namespace zorba {
namespace fs {

///////////////////////////////////////////////////////////////////////////////

ostream& operator<<( ostream &o, type t ) {
  static char const *const string_of[] = {
    "non_existant",
    "directory",
    "file",
    "link",
    "volume",
    "other"
  };
  if ( t >= 0 && t <= other )
    o << string_of[ t ];
  else
    o << "<invalid fs::type " << (int)t << '>';
  return o;
}

////////// helper functions ///////////////////////////////////////////////////

static void canonicalize( string *path ) {

  bool added_sentinel;
  if ( !ascii::ends_with( *path, dir_separator ) ) {
    *path += dir_separator;             // add sentinel
    added_sentinel = true;
  } else
    added_sentinel = false;

  ////////// Part 1: replace // by /

  char rbuf[5];
  rbuf[0] = rbuf[1] = dir_separator;

  while ( ascii::replace_all( *path, rbuf, 2, rbuf, 1 ) )
    ;

  ////////// Part 2: remove ./

  // actually look for /./ so as not to interfere with ../ case
  rbuf[0] = rbuf[2] = dir_separator;
  rbuf[1] = '.';

  while ( ascii::replace_all( *path, rbuf, 3, rbuf, 1 ) )
    ;

  ////////// Part 3: remove ../

  rbuf[0] = rbuf[3] = dir_separator;
  rbuf[1] = rbuf[2] = '.';
  rbuf[4] = '\0';

  for ( string::size_type pos = 0;
        (pos = path->find( rbuf, pos )) != string::npos; ) {
    if ( !pos )                         // leading /../
      path->erase( 0, 3 );
    else {
      string::size_type const prev_pos = path->rfind( dir_separator, pos - 1 );
      if ( prev_pos == string::npos ) {
        // "y/../z" => "z"
        path->erase( 0, pos + 4 );
        break;
      } else {
        // "x/y/../z" => "x/z"
        path->erase( prev_pos, pos - prev_pos + 3 );
        pos = prev_pos;
      }
    }
  }

  if ( added_sentinel && path->size() > 1 )
    path->erase( path->size() - 1 );    // remove sentinel
}

inline bool is_dots( char const *s ) {
  return s[0] == '.' && (!s[1] || (s[1] == '.' && !s[2]));
}

template<class PathStringType> inline
typename std::enable_if<ZORBA_IS_STRING(PathStringType),void>::type
replace_foreign( PathStringType *path ) {
  ascii::replace_all( *path, '\\', '/' );
}

static bool parse_file_uri( char const *uri, string *result ) {
  if ( !ZA_BEGINS_WITH( uri, "file://" ) )
    return false;

  using namespace diagnostic;

  *result = uri + 7;
  if ( result->empty() )
    throw invalid_argument( dict::lookup( ZED( EmptyPath ) ) );
  string::size_type slash = result->find( '/' );
  if ( slash == string::npos )
    throw invalid_argument(
      BUILD_STRING( '"', uri, "\": ", dict::lookup( ZED( BadPath ) ) )
    );
  if ( slash > 0 ) {
    string const authority( result->substr( 0, slash ) );
    if ( authority != "localhost" )
      throw invalid_argument(
        BUILD_STRING(
          '"', authority, "\": ", dict::lookup( ZED( NonLocalhostAuthority ) )
        )
      );
  }
  *result = result->substr( slash );
  uri::decode( *result );
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void append( char *path1, char const *path2 ) {
  size_t path1_len = ::strlen( path1 );
  if ( path1_len ) {
    char const path1_last = path1[ path1_len - 1 ];
    if ( path1_last != dir_separator && path2[0] != dir_separator )
      path1[ path1_len++ ] = dir_separator;
    else if ( path1_last == dir_separator && path2[0] == dir_separator )
      ++path2;
  }
  ::strcpy( path1 + path1_len, path2 );
}

#ifdef ZORBA_WITH_FILE_ACCESS

void chdir( char const *path ) {
  if ( ::chdir( path ) != 0 )
    throw fs::exception( "chdir()", path );
}

void create( char const *path ) {
  int const fd = ::creat( path, 0644 );
  if ( fd == -1 )
    throw fs::exception( "creat()", path );
  if ( ::close( fd ) != 0 )
    throw fs::exception( "close()", path );
}

#endif /* ZORBA_WITH_FILE_ACCESS */

string configdir() {
  return getenv( "HOME" );
}

string curdir() {
  static size_t dir_buf_len = PATH_MAX;
  static unique_ptr<char[]> dir_buf( new char[ dir_buf_len ] );

#ifndef ZORBA_FOR_ONE_THREAD_ONLY
  static Mutex mutex;
  AutoMutex const lock( &mutex );
#endif /* ZORBA_FOR_ONE_THREAD_ONLY */

  while ( !::getcwd( dir_buf.get(), dir_buf_len ) ) {
    if ( errno != ERANGE )
      throw ZORBA_IO_EXCEPTION( "getcwd()", "" );
    dir_buf.reset( new char[ dir_buf_len *= 2 ] );
  }
  return dir_buf.get();
}

#ifdef ZORBA_WITH_FILE_ACCESS

zstring get_temp_file() {
  char const *const env_tmp_dir = ::getenv( "TMPDIR" );
  std::string buf( env_tmp_dir && *env_tmp_dir ? env_tmp_dir : "/tmp" );
  if ( buf.back() != dir_separator )
    buf += dir_separator;
  buf += "zorba_tmp.XXXXXXXX";
  if ( !::mkdtemp( buf.data() ) )
    throw fs::exception( "mkdtemp()", buf.c_str() );
  return zstring( buf.c_str() );
}

#endif /* ZORBA_WITH_FILE_ACCESS */

type get_type( char const *path, bool follow_symlink, info *pinfo ) {
  if ( pinfo )
    ::memset( pinfo, 0, sizeof( info ) );
  struct stat st_buf;
  int const status = follow_symlink ?
    ::stat( path, &st_buf ) : ::lstat( path, &st_buf );
  if ( status == -1 ) {
    switch ( errno ) {
      case ENOENT:
        return non_existent;
      case EACCES:
      case ELOOP:
      case ENAMETOOLONG:
      case ENOTDIR:
        throw fs::exception( follow_symlink ? "stat()" : "lstat()", path );
      default:
        throw ZORBA_IO_EXCEPTION( follow_symlink ? "stat()" : "lstat()", path );
    }
  }

  fs::type type;
  if ( S_ISDIR( st_buf.st_mode ) )
    type = directory;
  else if ( S_ISLNK( st_buf.st_mode ) )
    type = link;
  else if ( S_ISREG( st_buf.st_mode ) )
    type = file;
  else
    type = other;

  if ( pinfo ) {
    pinfo->mtime = st_buf.st_mtime;
    pinfo->size = st_buf.st_size;
    pinfo->type = type;
  }
  return type;
}

#ifdef ZORBA_WITH_FILE_ACCESS

void mkdir_impl( char const *path, bool ignore_exists = false ) {
  if ( ::mkdir( path, 0755 ) != 0 &&
       !(ignore_exists && (errno == EEXIST || errno == EISDIR)) ) {
    throw fs::exception( "mkdir()", path );
  }
}

void mkdir( char const *path, bool intermediate ) {
  if ( !intermediate )
    mkdir_impl( path );
  else {
    string const dir( dir_name( path ) );
    if ( dir != path )
      mkdir( dir, true );
    else
      mkdir_impl( dir.c_str(), true );
    mkdir_impl( path, true );
  }
}

string normalize_path( char const *path, char const *base ) {
  if ( !path[0] )
    throw invalid_argument( "empty path" );
  string new_path;
  if ( !parse_file_uri( path, &new_path ) ) {
    //
    // The path is an ordinary path.
    //
    string local_path( path );
    replace_foreign( &local_path );
    if ( !is_absolute( local_path ) && base && base[0] ) {
      new_path = base;
      replace_foreign( &new_path );
      append( new_path, local_path );
    } else
      new_path = local_path;
  }
  canonicalize( &new_path );
  return new_path;
}

void iterator::ctor_impl() {
  make_absolute( &dir_path_ );
  if ( !(dir_ = ::opendir( dir_path_.c_str() )) )
    throw fs::exception( "iterator()", dir_path_.c_str() );
}

iterator::~iterator() {
  // A destructor must not throw (it would call std::terminate), so a failure of
  // closedir() can only be ignored here.
  (void)::closedir( dir_ );
}

bool iterator::next() {
  while ( true ) {
    if ( dirent const *const entry = ::readdir( dir_ ) ) {
      entry_.name = entry->d_name;
      switch ( entry->d_type ) {
        case DT_DIR:
          if ( is_dots( entry_.name ) )
            continue;                   // skip "." and ".." entries
          entry_.type = directory;
          break;
        case DT_LNK:
          entry_.type = link;
          break;
        case DT_REG:
          entry_.type = file;
          break;
        case DT_UNKNOWN: {
          //
          // The d_type member is not supported by all filesystems. If it's
          // not, it will be set to DT_UNKNOWN.  Hence, we're forced to do an
          // explicit stat() to determine the entry type.
          //
          // This check fixes bug #1023862.
          //
          zstring entry_path( dir_path_ );
          append( entry_path, entry_.name );
          entry_.type = get_type( entry_path );
          if ( entry_.type == directory && is_dots( entry_.name ) )
            continue;                   // skip "." and ".." entries
          break;
        }
        default:
          entry_.type = other;
      }
      return true;
    }
    return false;
  } // while
}

void iterator::reset() {
  ::rewinddir( dir_ );
}


bool remove( char const *path, bool ignore_not_found ) {
  if ( ::remove( path ) == 0 )
    return true;
  if ( ignore_not_found && errno == ENOENT )
    return false;
  throw fs::exception( "remove()", path );
}

void rename( char const *from, char const *to ) {
  if ( ::rename( from, to ) != 0 )
    throw fs::exception( "rename()", from );
}

#endif /* ZORBA_WITH_FILE_ACCESS */

///////////////////////////////////////////////////////////////////////////////

} // namespace fs
} // namespace zorba
/* vim:set et sw=2 ts=2: */
