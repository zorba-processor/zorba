// dirbench: timing driver of the directory scaling benchmark
// (.github/workflows/directory-scaling.yml). Not part of the Zorba build:
// the workflow compiles it with  c++ -std=c++23 -O2 -o dirbench dirbench.cpp
//
//   dirbench run OUT -- CMD ARGS...
//     Runs CMD once with stdout sent to the file OUT. Prints
//     "<wall ms> <peak RSS KiB> <exit status>".
//
//   dirbench mcp CALLS QUERY OUT -- CMD ARGS...
//     Starts the MCP server CMD (stdio transport), initializes a session
//     (MCP 2025-11-25 handshake) and calls the tool run_query CALLS times in
//     that session with the text of the file QUERY. The last response is
//     written to OUT. Prints "<ms of call 1> ... <ms of call N>", then
//     "<peak RSS KiB> <failed calls>" on a second line. A call fails when its
//     response is not a result or has "isError":true.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

using clock_type = std::chrono::steady_clock;

double ms_since( clock_type::time_point t0 ) {
  return std::chrono::duration<double, std::milli>( clock_type::now() - t0 )
    .count();
}

int usage() {
  std::cerr << "usage: dirbench run OUT -- CMD...\n"
               "       dirbench mcp CALLS QUERY OUT -- CMD...\n";
  return 2;
}

[[noreturn]] void exec_or_die( char **argv ) {
  execvp( argv[0], argv );
  std::perror( argv[0] );
  _exit( 127 );
}

std::string json_string( std::string const &s ) {
  std::string r( "\"" );
  for ( unsigned char c : s ) {
    switch ( c ) {
    case '"': r += "\\\""; break;
    case '\\': r += "\\\\"; break;
    case '\n': r += "\\n"; break;
    case '\r': r += "\\r"; break;
    case '\t': r += "\\t"; break;
    default:
      if ( c < 0x20 ) {
        char buf[8];
        std::snprintf( buf, sizeof buf, "\\u%04x", c );
        r += buf;
      } else {
        r += static_cast<char>( c );
      }
    }
  }
  return r + "\"";
}

int run( char const *out, char **cmd ) {
  auto const t0 = clock_type::now();
  pid_t const pid = fork();
  if ( pid < 0 ) {
    std::perror( "fork" );
    return 1;
  }
  if ( pid == 0 ) {
    int const fd = open( out, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
    if ( fd < 0 || dup2( fd, STDOUT_FILENO ) < 0 ) {
      std::perror( out );
      _exit( 127 );
    }
    exec_or_die( cmd );
  }
  int status = 0;
  rusage ru{};
  if ( wait4( pid, &status, 0, &ru ) < 0 ) {
    std::perror( "wait4" );
    return 1;
  }
  double const ms = ms_since( t0 );
  int const code = WIFEXITED( status ) ? WEXITSTATUS( status )
                                       : 128 + WTERMSIG( status );
  std::printf( "%.1f %ld %d\n", ms, ru.ru_maxrss, code );
  return 0;
}

int mcp( int calls, char const *query_file, char const *out, char **cmd ) {
  std::ifstream qf( query_file );
  if ( !qf ) {
    std::perror( query_file );
    return 1;
  }
  std::stringstream qs;
  qs << qf.rdbuf();
  std::string const query( json_string( qs.str() ) );

  int to_child[2], from_child[2];
  if ( pipe( to_child ) < 0 || pipe( from_child ) < 0 ) {
    std::perror( "pipe" );
    return 1;
  }
  pid_t const pid = fork();
  if ( pid < 0 ) {
    std::perror( "fork" );
    return 1;
  }
  if ( pid == 0 ) {
    dup2( to_child[0], STDIN_FILENO );
    dup2( from_child[1], STDOUT_FILENO );
    close( to_child[0] ); close( to_child[1] );
    close( from_child[0] ); close( from_child[1] );
    exec_or_die( cmd );
  }
  close( to_child[0] );
  close( from_child[1] );
  std::FILE *const w = fdopen( to_child[1], "w" );
  std::FILE *const r = fdopen( from_child[0], "r" );
  if ( !w || !r ) {
    std::perror( "fdopen" );
    return 1;
  }

  char *line = nullptr;
  std::size_t cap = 0;
  auto exchange = [&]( std::string const &request, std::string *response ) {
    std::fputs( request.c_str(), w );
    std::fputc( '\n', w );
    std::fflush( w );
    ssize_t const n = getline( &line, &cap, r );
    if ( n <= 0 )
      return false;
    response->assign( line, static_cast<std::size_t>( n ) );
    return true;
  };

  std::string response;
  if ( !exchange( "{\"jsonrpc\":\"2.0\",\"id\":0,\"method\":\"initialize\","
                  "\"params\":{\"protocolVersion\":\"2025-11-25\","
                  "\"capabilities\":{},\"clientInfo\":{\"name\":\"dirbench\","
                  "\"version\":\"1\"}}}", &response ) ||
       response.find( "\"result\"" ) == std::string::npos ) {
    std::cerr << "dirbench: initialize failed: " << response << "\n";
    return 1;
  }
  std::fputs( "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}\n",
              w );

  std::vector<double> times;
  int failed = 0;
  for ( int i = 1; i <= calls; ++i ) {
    std::string const request(
      "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string( i ) +
      ",\"method\":\"tools/call\",\"params\":{\"name\":\"run_query\","
      "\"arguments\":{\"query\":" + query + "}}}" );
    auto const t0 = clock_type::now();
    bool const ok = exchange( request, &response );
    times.push_back( ms_since( t0 ) );
    if ( !ok ) {
      std::cerr << "dirbench: the server closed the session at call " << i
                << "\n";
      ++failed;
      break;
    }
    if ( response.find( "\"result\"" ) == std::string::npos ||
         response.find( "\"isError\":true" ) != std::string::npos )
      ++failed;
  }
  std::ofstream( out ) << response;

  std::fclose( w );
  int status = 0;
  rusage ru{};
  if ( wait4( pid, &status, 0, &ru ) < 0 ) {
    std::perror( "wait4" );
    return 1;
  }
  std::fclose( r );
  std::free( line );
  if ( !WIFEXITED( status ) || WEXITSTATUS( status ) != 0 )
    ++failed;

  for ( std::size_t i = 0; i < times.size(); ++i )
    std::printf( "%s%.1f", i ? " " : "", times[i] );
  std::printf( "\n%ld %d\n", ru.ru_maxrss, failed );
  return 0;
}

} // namespace

int main( int argc, char **argv ) {
  if ( argc >= 5 && !std::strcmp( argv[1], "run" ) &&
       !std::strcmp( argv[3], "--" ) )
    return run( argv[2], argv + 4 );
  if ( argc >= 7 && !std::strcmp( argv[1], "mcp" ) &&
       !std::strcmp( argv[5], "--" ) ) {
    int const calls = std::atoi( argv[2] );
    if ( calls <= 0 )
      return usage();
    return mcp( calls, argv[3], argv[4], argv + 6 );
  }
  return usage();
}
