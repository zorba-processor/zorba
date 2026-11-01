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
// directory of XML, JSON and CSV documents with JSONiq/XQuery.

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include <unistd.h>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "directory_config.h"
#include "mcp_server.h"

using namespace zorba;

namespace {

char const usage[] =
  "Usage: zorba-mcp --root <dir> [--queries <dir>] [--timeout <seconds>]\n"
  "                 [--max-output <bytes>]\n"
  "\n"
  "An MCP server (stdio transport) over the Zorba JSONiq/XQuery processor.\n"
  "Queries run under the secure policy, reading only files under --root\n"
  "(bound to $dir:root of http://zorba.io/modules/directory).\n"
  "\n"
  "  --root <dir>         the directory of documents to query (required)\n"
  "  --queries <dir>      each <name>.jq file becomes a tool <name>; its\n"
  "                       leading comment is the description and its\n"
  "                       external variables are string parameters\n"
  "  --timeout <seconds>  time limit of one tool call (default 30, 0: none)\n"
  "  --max-output <bytes> output size limit of one tool call (default\n"
  "                       131072); longer output is cut with a marker\n";

bool parse_number( char const *s, unsigned long long *n ) {
  if ( !s || !*s )
    return false;
  char *end = nullptr;
  *n = std::strtoull( s, &end, 10 );
  return *end == '\0' && *s != '-';
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

  void *const store = StoreManager::getStore();
  Zorba *const zorba = Zorba::getInstance( store );
  int status = 0;
  {
    mcp::Server server( zorba, config, std::cerr );
    zorba->setSecurityPolicy( server.policy() );
    std::cerr << "zorba-mcp: serving " << config.root << " (MCP "
              << mcp::protocol_version << ")" << std::endl;

    std::string line;
    while ( std::getline( std::cin, line ) ) {
      if ( !line.empty() && line.back() == '\r' )
        line.pop_back();
      if ( line.find_first_not_of( " \t" ) == std::string::npos )
        continue;
      std::string const response( server.handle( line ) );
      if ( response.empty() )
        continue;
      if ( std::fputs( response.c_str(), out ) == EOF ||
           std::fputc( '\n', out ) == EOF || std::fflush( out ) == EOF ) {
        std::cerr << "zorba-mcp: cannot write to stdout" << std::endl;
        status = 1;
        break;
      }
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
