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

// zorba-mcp: an MCP server (stdio transport) that lets an AI agent query a
// directory of XML, JSON, CSV and HTML documents with JSONiq/XQuery.

#include <algorithm>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include <sys/resource.h>
#include <unistd.h>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "directory_config.h"
#include "mcp_server.h"

using namespace zorba;

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
# define ZORBA_MCP_SANITIZER 1
#elif defined(__has_feature)
# if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#  define ZORBA_MCP_SANITIZER 1
# endif
#endif

namespace {

char const usage[] =
  "Usage: zorba-mcp --root <dir> [--queries <dir>] [--timeout <seconds>]\n"
  "                 [--max-output <bytes>] [--max-memory <bytes>]\n"
  "                 [--no-scan] [--scan-cache-mb <MiB>]\n"
  "                 [--cache-max-file-kb <KB>] [--cache-max-files <N>]\n"
  "                 [--language xquery|jsoniq]\n"
  "\n"
  "An MCP server (stdio transport) over the Zorba JSONiq/XQuery processor.\n"
  "Queries run under the secure policy, reading only files under --root\n"
  "(bound to $dir:root of http://zorba.io/modules/directory).\n"
  "\n"
  "  --root <dir>         the directory of documents to query (required)\n"
  "  --queries <dir>      each <name>.jq file becomes a tool <name>; its\n"
  "                       leading comment is the description and its\n"
  "                       external variables are string parameters. A .jq\n"
  "                       file is JSONiq unless it declares xquery version;\n"
  "                       .xq files are not loaded\n"
  "  --timeout <seconds>  time limit of one tool call (default 30, 0: none)\n"
  "  --max-output <bytes> limit of one tool call on the query output and on\n"
  "                       the \"message\" of an error (default 131072). Longer\n"
  "                       output is cut with a marker. A longer message\n"
  "                       loses its middle, keeps its end and gets a marker\n"
  "                       with its total size; under a limit smaller than\n"
  "                       the marker, the message is the marker alone. The\n"
  "                       code, the hint and the location are not cut by\n"
  "                       this limit: a hint over 4096 bytes loses its\n"
  "                       middle, and a failing line in \"location\" over 512\n"
  "                       bytes shows about 512 bytes around the column of\n"
  "                       the error, or both ends of a line over 65535 bytes.\n"
  "                       Apart from this limit, a message shortens the data\n"
  "                       it quotes: a value over 100 bytes becomes its head,\n"
  "                       an ellipsis, its tail and its size in bytes. A URI,\n"
  "                       a path, a QName or {uri}local stays whole up to\n"
  "                       4096 bytes, and so does the text of an error, such\n"
  "                       as the message of the parser; longer text keeps\n"
  "                       2000 bytes at each end\n"
  "  --max-memory <bytes> memory limit of the server (default 4294967296,\n"
  "                       0: none; Linux only): a query that needs more\n"
  "                       fails with the tool error memory_limit, and the\n"
  "                       server answers the next call. If memory runs out\n"
  "                       where that cannot be done, for example while the\n"
  "                       server starts, it exits with status 3 and a message\n"
  "                       on stderr, with no response to the call. Libraries\n"
  "                       end it in other ways: abort, with a core file if\n"
  "                       the system keeps them (Xerces-C releasing a schema;\n"
  "                       libxml2 or glibc on the first use by a thread);\n"
  "                       SIGSEGV (ICU compiling a regular expression); status\n"
  "                       2 (the query scanner, while a query compiles)\n"
  "  --no-scan            no background scan of the directory: list_documents\n"
  "                       takes no arguments and lists through dir:list\n"
  "  --scan-cache-mb <MiB> memory for the scan's file index and previews\n"
  "                       (default 64); at the limit the scan stops. This is not\n"
  "                       the document cache: it holds previews, not documents\n"
  "  --cache-max-file-kb <KB>\n"
  "                       the document cache keeps parsed documents across\n"
  "                       calls for dir:open and dir:documents: at most this\n"
  "                       file size (default 16, 0: no document cache). A file\n"
  "                       enters on its second read. It is served only while\n"
  "                       its path, device, inode, size, mtime and ctime stay\n"
  "                       the same. Not kept: larger files, records with an\n"
  "                       error, records over 128 times their file size, files\n"
  "                       changed in the last 20 ms (a later read keeps them;\n"
  "                       the time of a file server is taken to match this\n"
  "                       machine's: with its clock ahead, a file may wait as\n"
  "                       long as the difference).\n"
  "                       Each read gets its own copy of the document.\n"
  "                       Updating and sequential queries do not use it. One\n"
  "                       line of counters goes to stderr at exit\n"
  "  --cache-max-files <N>\n"
  "                       how many documents it keeps, least recently used\n"
  "                       out first (default 10, 0: no document cache); a\n"
  "                       query does not evict a document it has used\n"
  "  --language <lang>    xquery (default) or jsoniq: the language of a\n"
  "                       run_query query, in this order: (1) a jsoniq\n"
  "                       version or xquery version declaration at the very\n"
  "                       start of the query (one preceded by a comment is\n"
  "                       not detected); (2) for a file, its extension (.jq\n"
  "                       is JSONiq); (3) this option. jsoniq also\n"
  "                       puts a JSONiq primer in the run_query description,\n"
  "                       and a pointer to it in the server instructions\n";

bool parse_number( char const *s, unsigned long long *n ) {
  if ( !s || !*s )
    return false;
  char *end = nullptr;
  *n = std::strtoull( s, &end, 10 );
  return *end == '\0' && *s != '-';
}

// The exit status when memory ran out where nothing can recover from it.
int const out_of_memory_status = 3;

std::terminate_handler default_terminate = nullptr;

// std::bad_alloc that nothing can catch ends the process here, without a core
// file of the size of the limit: see --help. Any other exception keeps the
// default behavior, which shows a defect.
[[noreturn]] void on_terminate() {
  bool out_of_memory = false;
  try {
    if ( std::exception_ptr const e = std::current_exception() )
      std::rethrow_exception( e );
  }
  catch ( std::bad_alloc const& ) {
    out_of_memory = true;
  }
  catch ( ... ) {
  }
  if ( !out_of_memory )
    default_terminate();
  static char const msg[] =
    "zorba-mcp: out of memory where it cannot be recovered from; exiting\n";
  // Nothing else can report a failing write to stderr.
  [[maybe_unused]] ssize_t const n = write( STDERR_FILENO, msg, sizeof msg - 1 );
  std::_Exit( out_of_memory_status );
}

#if defined(__linux__) && !defined(ZORBA_MCP_SANITIZER)
// The stack grows on demand, within the address-space limit: at the limit a
// call deeper than any before would get SIGSEGV instead of an allocation that
// fails. So the stack takes its size now, up to 64 MiB, with one page of it in
// memory: a larger limit would use up --max-memory before any query runs. The
// arguments and environment, at most a quarter of its size, are also on it. A
// stack with no size limit reserves what the usual limit would.
__attribute__((noinline)) void reserve_stack() {
  struct rlimit lim;
  if ( getrlimit( RLIMIT_STACK, &lim ) != 0 ) {
    std::perror( "zorba-mcp: getrlimit" );
    return;
  }
  std::uintptr_t const usual = 8 * 1024 * 1024;
  std::uintptr_t const size = lim.rlim_cur == RLIM_INFINITY ?
    usual : static_cast<std::uintptr_t>( lim.rlim_cur );
  std::uintptr_t const margin = size / 4 + 64 * 1024;
  if ( size <= margin )
    return;
  char here;
  *reinterpret_cast<char volatile*>(
    reinterpret_cast<std::uintptr_t>( &here ) -
    std::min<std::uintptr_t>( size - margin, 64 * 1024 * 1024 ) ) = 0;
}
#endif

// A query that needs more memory than the machine has would get the server
// killed, with no answer for the client. With an address-space limit,
// allocation fails first: the query ends with std::bad_alloc, its memory is
// released, and the server goes on. The limit counts address space, which is
// somewhat more than the memory in use. Sanitizer builds cannot have this
// limit: their runtime reserves far more address space than the process uses.
void limit_memory( unsigned long long bytes ) {
#if defined(__linux__) && !defined(ZORBA_MCP_SANITIZER)
  if ( bytes == 0 )
    return;
  reserve_stack();
  struct rlimit lim;
  if ( getrlimit( RLIMIT_AS, &lim ) != 0 ) {
    std::perror( "zorba-mcp: getrlimit" );
    return;
  }
  lim.rlim_cur = lim.rlim_max == RLIM_INFINITY || bytes < lim.rlim_max ?
    bytes : lim.rlim_max;
  if ( setrlimit( RLIMIT_AS, &lim ) != 0 )
    std::perror( "zorba-mcp: setrlimit" );
#else
  (void)bytes;
#endif
}

// Reads one line. Of a line longer than max, only max + 1 characters are kept:
// that much memory is all a line takes, and handle() refuses it by its size.
bool read_line( std::streambuf *in, std::string *line, std::size_t max ) {
  line->clear();
  bool any = false;
  for ( ;; ) {
    int const c = in->sbumpc();
    if ( c == std::char_traits<char>::eof() )
      return any;
    any = true;
    if ( c == '\n' )
      return true;
    if ( line->size() <= max )
      line->push_back( static_cast<char>( c ) );
  }
}

int fail( std::string const &msg ) {
  std::cerr << "zorba-mcp: " << msg << "\n\n" << usage;
  return 2;
}

} // namespace

int main( int argc, char const *argv[] ) {
  mcp::ServerConfig config;
  for ( int i = 1; i < argc; ++i ) {
    std::string const arg( argv[i] );
    char const *const val = i + 1 < argc ? argv[ i + 1 ] : nullptr;
    unsigned long long n;
    if ( arg == "-h" || arg == "--help" ) {
      std::cerr << usage;
      return 0;
    } else if ( arg == "--root" && val ) {
      config.root = val;
      ++i;
    } else if ( arg == "--queries" && val ) {
      config.queries_dir = val;
      ++i;
    } else if ( arg == "--timeout" && parse_number( val, &n ) ) {
      config.timeout = std::chrono::seconds( n );
      ++i;
    } else if ( arg == "--max-output" && parse_number( val, &n ) && n > 0 ) {
      config.max_output = static_cast<std::size_t>( n );
      ++i;
    } else if ( arg == "--language" && val &&
                ( std::string( val ) == "xquery" ||
                  std::string( val ) == "jsoniq" ) ) {
      config.jsoniq_default = std::string( val ) == "jsoniq";
      ++i;
    } else if ( arg == "--max-memory" && parse_number( val, &n ) ) {
      config.max_memory = n;
      ++i;
    } else if ( arg == "--no-scan" ) {
      config.scan = false;
    } else if ( arg == "--scan-cache-mb" && parse_number( val, &n ) &&
                n > 0 && n <= 1024 * 1024 ) {
      config.scan_limits.max_memory = static_cast<std::size_t>( n ) * 1024 * 1024;
      ++i;
    } else if ( arg == "--cache-max-file-kb" && parse_number( val, &n ) &&
                n <= ( SIZE_MAX >> 10 ) ) {
      config.cache_max_file_kb = static_cast<std::size_t>( n );
      ++i;
    } else if ( arg == "--cache-max-files" && parse_number( val, &n ) ) {
      config.cache_max_files = static_cast<std::size_t>( n );
      ++i;
    } else {
      return fail( "invalid argument: " + arg );
    }
  }
  if ( config.root.empty() )
    return fail( "--root is required" );
  std::string why;
  if ( !directory_config::normalize( &config.root, &why ) )
    return fail( "--root: " + why );
  if ( !config.queries_dir.empty() &&
       !directory_config::normalize( &config.queries_dir, &why ) )
    return fail( "--queries: " + why );

  // Only protocol messages may go to stdout: keep the real stdout for them
  // and send anything else that writes to it (e.g. a library message) to
  // stderr.
  int const proto_fd = dup( STDOUT_FILENO );
  if ( proto_fd < 0 || dup2( STDERR_FILENO, STDOUT_FILENO ) < 0 ) {
    std::perror( "zorba-mcp: dup" );
    return 1;
  }
  std::FILE *const out = fdopen( proto_fd, "w" );
  if ( !out ) {
    std::perror( "zorba-mcp: fdopen" );
    return 1;
  }
  // A client that goes away makes writes fail instead of killing us.
  if ( std::signal( SIGPIPE, SIG_IGN ) == SIG_ERR ) {
    std::perror( "zorba-mcp: signal" );
    return 1;
  }

  default_terminate = std::set_terminate( on_terminate );
  limit_memory( config.max_memory );

  void *const store = StoreManager::getStore();
  Zorba *const zorba = Zorba::getInstance( store );
  int status = 0;
  {
    mcp::Server server( zorba, config, std::cerr );
    zorba->setSecurityPolicy( server.policy() );
    std::cerr << "zorba-mcp: serving " << config.root << " (MCP "
              << mcp::protocol_version << ")" << std::endl;

    std::string line;
    line.reserve( mcp::max_message_size + 1 );
    while ( read_line( std::cin.rdbuf(), &line, mcp::max_message_size ) ) {
      if ( !line.empty() && line.back() == '\r' )
        line.pop_back();
      if ( line.find_first_not_of( " \t" ) == std::string::npos )
        continue;
      std::string response;
      try {
        response = server.handle( line );
      }
      catch ( std::bad_alloc const& ) {
        // Not even the error response of handle() could be made. The memory is
        // back by now if it was the call that held it. This allocates nothing.
        if ( std::fprintf( out, "{\"jsonrpc\":\"2.0\",\"id\":%s,\"error\":{\"code\":-32603,"
                           "\"message\":\"Internal error: out of memory\"}}\n",
                           server.current_id() ) < 0 ||
             std::fflush( out ) == EOF ) {
          std::cerr << "zorba-mcp: cannot write to stdout" << std::endl;
          status = 1;
          break;
        }
        continue;
      }
      if ( response.empty() )
        continue;
      if ( std::fputs( response.c_str(), out ) == EOF ||
           std::fputc( '\n', out ) == EOF || std::fflush( out ) == EOF ) {
        std::cerr << "zorba-mcp: cannot write to stdout" << std::endl;
        status = 1;
        break;
      }
    }
    std::string const cache_line( server.cache_stats_line() );
    if ( !cache_line.empty() )
      std::cerr << cache_line << std::endl;
    // Zorba::shutdown() cleans up libxml2, which a thread left behind could be
    // in, so in that case the process ends here.
    if ( !server.stop_scan() ) {
      std::cerr << "zorba-mcp: a scan thread did not stop in time" << std::endl;
      if ( std::fclose( out ) == EOF && status == 0 )
        status = 1;
      std::_Exit( status );
    }
  }
  if ( std::fclose( out ) == EOF && status == 0 ) {
    std::cerr << "zorba-mcp: cannot write to stdout" << std::endl;
    status = 1;
  }
  zorba->shutdown();
  StoreManager::shutdownStore( store );
  return status;
}
/* vim:set et sw=2 ts=2: */
