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

#include <sstream>

#include <cstdio>

#include <zorba/internal/unique_ptr.h>
#include <zorba/util/error_util.h>

#include "diagnostics/dict.h"
#include "diagnostics/diagnostic.h"

#include "stl_util.h"
#include "string_util.h"

namespace zorba {
namespace os_error {

using namespace std;

////////// exception //////////////////////////////////////////////////////////

static string make_what( char const *function, char const *path,
                         char const *err_string ) {
  ostringstream oss;
  if ( path && *path )
    oss << '"' << path << "\": ";
  if ( err_string )
    oss << format_err_string( function, err_string );
  else
    oss << get_err_string( function );
  return oss.str();
}

exception::exception( char const *function, char const *path,
                      char const *err_string ) :
  message_( make_what( function, path, err_string ) ),
  function_( function ), path_( path )
{
}

exception::~exception() throw() {
  // out-of-line since it's virtual
}

char const* exception::what() const throw() {
  return message_.c_str();
}

///////////////////////////////////////////////////////////////////////////////

string format_err_string( char const *function, char const *err_string ) {
  if ( function && *function ) {
    using namespace internal::diagnostic;
    parameters::value_type result =
      diagnostic::dict::lookup( ZED( FunctionFailed_12o ) );
    parameters const params( ERROR_PARAMS( function, err_string ) );
    params.substitute( &result );
    return result;
  }
  return err_string;
}

string format_err_string( char const *function, code_type code,
                          char const *err_string ) {
  using namespace internal::diagnostic;
  parameters params;
  parameters::value_type result;
  if ( function && *function ) {
    result = diagnostic::dict::lookup(
      ZED( FunctionFailedErrorCodeMessage_123 )
    );
    params = ERROR_PARAMS( function, code, err_string );
  } else {
    result = diagnostic::dict::lookup( ZED( ErrorCodeMessage_12 ) );
    params = ERROR_PARAMS( code, err_string );
  }
  params.substitute( &result );
  return result;
}

// strerror_r() comes in two flavors: the XSI one returns an int and fills the
// buffer; the GNU one returns a char* that may or may not be the buffer.
inline char const* strerror_result( int rc, char const *buf ) {
  return rc == 0 ? buf : "unknown error";
}

inline char const* strerror_result( char const *result, char const* ) {
  return result;
}

string get_err_string( char const *function, code_type code ) {
  char err_buf[ 128 ];
  char const *const err_string =
    strerror_result( ::strerror_r( code, err_buf, sizeof err_buf ), err_buf );
  return format_err_string( function, code, err_string );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace os_error
} // namespace zorba
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
