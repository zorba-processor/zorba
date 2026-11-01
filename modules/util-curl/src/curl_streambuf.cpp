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

// standard
#include <algorithm>
#include <cstdlib>
#include <cstring>                      /* for memcpy(3) */
#include <iostream>
#include <arpa/inet.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

// libcurl
#include <curl/multi.h>

// Zorba
#include "util/curl_streambuf.h"

#define ZORBA_LIBCURL_AT_LEAST(MAJOR,MINOR,PATCH) \
  (LIBCURL_VERSION_MAJOR >= (MAJOR) && LIBCURL_VERSION_MINOR >= (MINOR) && LIBCURL_VERSION_PATCH >= (PATCH))

using namespace std;

namespace zorba {
namespace curl {

///////////////////////////////////////////////////////////////////////////////

exception::exception( char const *function, char const *uri, char const *msg ) :
  std::exception(),
  curl_code_( CURLE_OK ), curlm_code_( CURLM_OK ), msg_( msg )
{
}

exception::exception( char const *function, char const *uri, CURLcode code ) :
  std::exception(),
  curl_code_( code ), curlm_code_( CURLM_OK ),
  msg_( curl_easy_strerror( code ) )
{
  ostringstream oss;
  oss << " (CURLcode " << (int)code << ')';
  msg_ += oss.str();
}

exception::exception( char const *function, char const *uri, CURLMcode code ) :
  std::exception(),
  curl_code_( CURLE_OK ), curlm_code_( code ),
  msg_( curl_multi_strerror( code ) )
{
  ostringstream oss;
  oss << " (CURLMcode " << (int)code << ')';
  msg_ += oss.str();
}

exception::~exception() throw() {
  // out-of-line since it's virtual
}

const char* exception::what() const throw() {
  return msg_.c_str();
}

///////////////////////////////////////////////////////////////////////////////

listener::~listener() {
  // out-of-line since it's virtual
}

///////////////////////////////////////////////////////////////////////////////

CURL* create() {
  //
  // Having cURL initialization wrapped by a class and using a singleton static
  // instance guarantees that cURL is initialized exactly once before use and
  // and also is cleaned-up at program termination (when destructors for static
  // objects are called).
  //
  struct curl_initializer {
    curl_initializer() {
      ZORBA_CURL_ASSERT( curl_global_init( CURL_GLOBAL_ALL ) );
    }
    ~curl_initializer() {
      curl_global_cleanup();
    }
  };
  static curl_initializer initializer;

  if ( CURL *const curl = curl_easy_init() )
    return curl;
  throw exception( "curl_easy_init()", "", "" );
}

namespace {

/**
 * The policy attached to a handle by restrict_to_policy(), reachable from
 * the handle through CURLOPT_PRIVATE and freed by destroy().
 */
struct policy_guard {
  SecurityPolicy policy;
  CURL *curl;
  std::string denied;                   // the first refusal, if any

  policy_guard( SecurityPolicy const &p, CURL *c ) : policy( p ), curl( c ) { }
};

policy_guard* get_guard( CURL *instance ) {
  char *p = nullptr;
  if ( curl_easy_getinfo( instance, CURLINFO_PRIVATE, &p ) != CURLE_OK )
    return nullptr;
  return reinterpret_cast<policy_guard*>( p );
}

// A URL for a message: without user name and password.
std::string without_userinfo( char const *url ) {
  std::string u( url ? url : "" );
  std::string::size_type const start = u.find( "://" );
  if ( start != std::string::npos ) {
    std::string::size_type const end = u.find_first_of( "/?#", start + 3 );
    std::string::size_type const at = u.rfind( '@', end );
    if ( at != std::string::npos && at > start )
      u.erase( start + 3, at + 1 - ( start + 3 ) );
  }
  return u;
}

bool guard_allows( policy_guard *g, char const *address ) {
  // After a redirect, the effective URL is the redirect target.
  char *url = nullptr;
  curl_easy_getinfo( g->curl, CURLINFO_EFFECTIVE_URL, &url );
  String reason;
  if ( g->policy.checkConnection( url ? url : "", address ? address : "",
                                  &reason ) )
    return true;
  if ( g->denied.empty() ) {
    g->denied = '"';
    g->denied += without_userinfo( url );
    g->denied += "\": access denied by security policy (";
    g->denied += reason.str();
    g->denied += ')';
  }
  return false;
}

// Checks the address before the socket is created, so a denied address is
// never connected to.
curl_socket_t guard_open_socket( void *clientp, curlsocktype,
                                 struct curl_sockaddr *addr ) {
  policy_guard *const g = static_cast<policy_guard*>( clientp );
  char buf[ INET6_ADDRSTRLEN ] = "";
  if ( addr->family == AF_INET )
    inet_ntop( AF_INET,
               &reinterpret_cast<sockaddr_in*>( &addr->addr )->sin_addr,
               buf, sizeof buf );
  else if ( addr->family == AF_INET6 )
    inet_ntop( AF_INET6,
               &reinterpret_cast<sockaddr_in6*>( &addr->addr )->sin6_addr,
               buf, sizeof buf );
  if ( !guard_allows( g, buf ) )
    return CURL_SOCKET_BAD;
  return socket( addr->family, addr->socktype, addr->protocol );
}

#if LIBCURL_VERSION_NUM >= 0x075000     /* 7.80.0 */
// Called for every request, also on a reused connection, before anything is
// sent: checks the URL (e.g. a redirect to another path of the same host)
// and the address connected to.
int guard_prereq( void *clientp, char *conn_primary_ip, char*, int, int ) {
  return guard_allows( static_cast<policy_guard*>( clientp ), conn_primary_ip )
    ? CURL_PREREQFUNC_OK : CURL_PREREQFUNC_ABORT;
}
#endif

bool restricts_nothing( SecurityPolicy const &p ) {
  return p.allows( Capability::Network ) && p.getNetworkPrefixes().empty() &&
         p.allowsPrivateAddresses() &&
         p.allows( Capability::FileRead ) && p.getReadPaths().empty() &&
         p.allows( Capability::FileWrite ) && p.getWritePaths().empty();
}

} // anonymous namespace

void restrict_to_policy( CURL *instance, SecurityPolicy const &policy ) {
  if ( !instance || restricts_nothing( policy ) )
    return;

  delete get_guard( instance );
  policy_guard *const g = new policy_guard( policy, instance );
  if ( curl_easy_setopt( instance, CURLOPT_PRIVATE, g ) != CURLE_OK ) {
    delete g;
    throw exception( "curl_easy_setopt(CURLOPT_PRIVATE)", "" );
  }

#if LIBCURL_VERSION_NUM >= 0x075500     /* 7.85.0 */
  ZORBA_CURL_ASSERT(
    curl_easy_setopt( instance, CURLOPT_PROTOCOLS_STR, "http,https,ftp,ftps" )
  );
  ZORBA_CURL_ASSERT(
    curl_easy_setopt( instance, CURLOPT_REDIR_PROTOCOLS_STR,
                      "http,https,ftp,ftps" )
  );
#else
  long const protocols =
    CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP | CURLPROTO_FTPS;
  ZORBA_CURL_ASSERT( curl_easy_setopt( instance, CURLOPT_PROTOCOLS, protocols ) );
  ZORBA_CURL_ASSERT(
    curl_easy_setopt( instance, CURLOPT_REDIR_PROTOCOLS, protocols )
  );
#endif

  ZORBA_CURL_ASSERT(
    curl_easy_setopt( instance, CURLOPT_OPENSOCKETFUNCTION, guard_open_socket )
  );
  ZORBA_CURL_ASSERT( curl_easy_setopt( instance, CURLOPT_OPENSOCKETDATA, g ) );
#if LIBCURL_VERSION_NUM >= 0x075000     /* 7.80.0 */
  ZORBA_CURL_ASSERT(
    curl_easy_setopt( instance, CURLOPT_PREREQFUNCTION, guard_prereq )
  );
  ZORBA_CURL_ASSERT( curl_easy_setopt( instance, CURLOPT_PREREQDATA, g ) );
#endif
}

bool policy_denied( CURL *instance, std::string *reason ) {
  if ( !instance )
    return false;
  policy_guard const *const g = get_guard( instance );
  if ( !g || g->denied.empty() )
    return false;
  if ( reason )
    *reason = g->denied;
  return true;
}

void destroy( CURL *instance ) {
  if ( instance ) {
    policy_guard *const g = get_guard( instance );
    curl_easy_reset( instance );
    curl_easy_cleanup( instance );
    delete g;
  }
}

///////////////////////////////////////////////////////////////////////////////

streambuf::streambuf() {
  init();
}

streambuf::streambuf( char const *uri ) {
  init();
  open( uri );
}

streambuf::streambuf( CURL *curl ) {
  init();
  curl_ = curl;
  curl_init();
  curlm_init();
}

streambuf::~streambuf() {
  close();
  if ( listener_owner_ )
    delete listener_;
}

void streambuf::close() {
  if ( curl_ ) {
    if ( curlm_ ) {
      curl_multi_remove_handle( curlm_, curl_ );
      curl_multi_cleanup( curlm_ );
      curlm_ = 0;
    }
    curl_destroy();
  }
}

void streambuf::curl_verbose( bool verbose ) {
  verbose_ = verbose;
  if ( curl_ )
    ZORBA_CURL_ASSERT( curl_easy_setopt( curl_, CURLOPT_VERBOSE, verbose_ ? 1 : 0 ) );
}

void streambuf::curl_init() {
#if ZORBA_LIBCURL_AT_LEAST(7,25,0)
  ZORBA_CURL_ASSERT( curl_easy_setopt( curl_, CURLOPT_TCP_KEEPALIVE, 1L ) );
#endif
  ZORBA_CURL_ASSERT( curl_easy_setopt( curl_, CURLOPT_WRITEDATA, this ) );
  ZORBA_CURL_ASSERT( curl_easy_setopt( curl_, CURLOPT_WRITEFUNCTION, curl_write_callback ) );

  if ( verbose_ )
    ZORBA_CURL_ASSERT( curl_easy_setopt( curl_, CURLOPT_VERBOSE, 1 ) );
}

void streambuf::curl_io( size_t *len_ptr ) {
  *len_ptr = 0;
  while ( curl_running_ && !*len_ptr ) {
    fd_set fd_read, fd_write, fd_except;
    FD_ZERO( &fd_read );
    FD_ZERO( &fd_write );
    FD_ZERO( &fd_except );
    int max_fd = -1;
    ZORBA_CURLM_ASSERT(
      curl_multi_fdset( curlm_, &fd_read, &fd_write, &fd_except, &max_fd )
    );

    //
    // Note that the fopen.c sample code is unnecessary at best or wrong at
    // worst; see: http://curl.haxx.se/mail/lib-2011-05/0011.html
    //
    timeval timeout;
    long curl_timeout_ms;
    ZORBA_CURLM_ASSERT( curl_multi_timeout( curlm_, &curl_timeout_ms ) );
    if ( curl_timeout_ms > 0 ) {
      timeout.tv_sec  = curl_timeout_ms / 1000;
      timeout.tv_usec = curl_timeout_ms % 1000 * 1000;
    } else {
      //
      // From curl_multi_timeout(3):
      //
      //    Note: if libcurl returns a -1 timeout here, it just means that
      //    libcurl currently has no stored timeout value. You must not wait
      //    too long (more than a few seconds perhaps) before you call
      //    curl_multi_perform() again.
      //
      // So we just pick some not-too-long default.
      //
      timeout.tv_sec  = 1;
      timeout.tv_usec = 0;
    }

    switch ( select( max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout ) ) {
      case -1:                          // select error
        throw exception( "select()", "", strerror( errno ) );
      case 0:                           // timeout
        // no break;
      default:
        CURLMcode code;
        do {
          code = curl_multi_perform( curlm_, &curl_running_ );
        } while ( code == CURLM_CALL_MULTI_PERFORM );
        ZORBA_CURLM_ASSERT( code );
    } // switch
  } // while
}

CURLcode streambuf::curl_multi_info_read( bool throw_on_error ) {
  underflow();
  CURLMsg *msg;
  int msgs_in_q;
  CURLcode code = CURLE_OK;
  while ( (msg = ::curl_multi_info_read( curlm_, &msgs_in_q )) )
    if ( msg->msg == CURLMSG_DONE )
      code = msg->data.result;
  if ( code && throw_on_error )
    throw exception( "curl_multi_info_read()", "", code );
  return code;
}

size_t streambuf::curl_write_callback( void *ptr, size_t size, size_t nmemb,
                                       void *data ) {
  size *= nmemb;
  streambuf *const that = static_cast<streambuf*>( data );

  if ( that->listener_ )
    that->listener_->curl_read( ptr, size );

  size_t const gbuf_free = that->gbuf_.capacity_ - that->gbuf_.len_;
  if ( size > gbuf_free ) {
    size_t new_capacity = that->gbuf_.capacity_ + size - gbuf_free;
    if ( void *const new_buf = realloc( that->gbuf_.ptr_, new_capacity ) ) {
      that->gbuf_.ptr_ = static_cast<char*>( new_buf );
      that->gbuf_.capacity_ = new_capacity;
    } else
      throw exception( "realloc()", "" );
  }
  ::memcpy( that->gbuf_.ptr_ + that->gbuf_.len_, ptr, size );
  that->gbuf_.len_ += size;
  return size;
}

void streambuf::init() {
  curl_ = 0;
  curlm_ = 0;
  curl_running_ = 0;
  listener_ = 0;
  listener_owner_ = false;
  verbose_ = false;
}

void streambuf::curlm_init() {
  //
  // Lie about cURL running initially so the while-loop in curl_io() will run
  // at least once.
  //
  curl_running_ = 1;

  //
  // Set the "get" pointer to the end (gptr() == egptr()) so a call to
  // underflow() and initial data read will be triggered.
  //
  gbuf_.len_ = gbuf_.capacity_;
  setg( gbuf_.ptr_, gbuf_.ptr_ + gbuf_.len_, gbuf_.ptr_ + gbuf_.len_ );

  //
  // Clean-up has to be done here with try/catch (as opposed to relying on the
  // destructor) because open() can be called from the constructor.  If an
  // exception is thrown, the constructor will not have completed, hence the
  // object will not have been fully constructed; therefore the destructor will
  // not be called.
  //
  try {
    if ( !(curlm_ = curl_multi_init()) )
      throw exception( "curl_multi_init()", "" );
    try {
      ZORBA_CURLM_ASSERT( curl_multi_add_handle( curlm_, curl_ ) );
    }
    catch ( ... ) {
      curl_multi_cleanup( curlm_ );
      curlm_ = 0;
      throw;
    }
  }
  catch ( ... ) {
    curl_destroy();
    throw;
  }
}

void streambuf::open( char const *uri ) {
  if ( !curl_ ) {
    curl_ = create();
    try {
      curl_init();
    }
    catch ( ... ) {
      curl_destroy();
      throw;
    }
    curlm_init();
  }
  ZORBA_CURL_ASSERT( curl_easy_setopt( curl_, CURLOPT_URL, uri ) );
}

void streambuf::set_listener( listener *new_listener, bool take_ownership ) {
  if ( listener_owner_ && new_listener != listener_ )
    delete listener_;
  listener_ = new_listener;
  listener_owner_ = take_ownership;
}

streamsize streambuf::showmanyc() {
  return egptr() - gptr();
}

streambuf::int_type streambuf::underflow() {
  while ( true ) {
    if ( gptr() < egptr() )
      return traits_type::to_int_type( *gptr() );
    curl_io( &gbuf_.len_ );
    if ( !gbuf_.len_ )
      return traits_type::eof();
    setg( gbuf_.ptr_, gbuf_.ptr_, gbuf_.ptr_ + gbuf_.len_ );
  }
}

streamsize streambuf::xsgetn( char_type *to, streamsize size ) {
  streamsize return_size = 0;

  if ( streamsize const gsize = egptr() - gptr() ) {
    streamsize const n = min( gsize, size );
    traits_type::copy( to, gptr(), static_cast<size_t>( n ) );
    gbump( static_cast<int>( n ) );
    to += n;
    size -= n, return_size += n;
  }

  while ( size > 0 ) {
    curl_io( &gbuf_.len_ );
    if ( !gbuf_.len_ )
      break;
    setg( gbuf_.ptr_, gbuf_.ptr_, gbuf_.ptr_ + gbuf_.len_ );
    streamsize const n = min( static_cast<streamsize>(gbuf_.len_), size );
    traits_type::copy( to, gptr(), static_cast<size_t>(n) );
    gbump( static_cast<int>( n ) );
    to += n;
    size -= n, return_size += n;
  }
  return return_size;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace curl
} // namespace zorba
/* vim:set et sw=2 ts=2: */
