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
//
//   dirbench soak FORMAT SCENARIO FILES KB CALLS OUT -- CMD ARGS...
//     One long session against the MCP server CMD, whose --root is the current
//     directory: writes FILES files of about KB KiB in FORMAT (xml, json, csv
//     or html) and makes CALLS run_query calls, changing the files between
//     calls as SCENARIO says. Writes the last response to OUT. Linux only.
//       hits      a scan of all the files
//       turnover  one file per query, each read twice, in rotation
//       rewrite   a scan; before it, one file is rewritten with another size
//       touch     a scan; before it, one file gets a new modification time
//       delete    a scan; before it, one file is deleted and written again
//       refused   a scan of files that cannot be parsed
//     Prints "<calls> <VmRSS KiB> <VmHWM KiB> <failed calls>" after call 1 and
//     every tenfold of calls, and when the session ends.
//
//   dirbench scan OUT -- CMD ARGS...
//     Starts the MCP server CMD, initializes a session, calls list_documents
//     once and writes its response to OUT. If the answer has a scan state,
//     then polls list_documents (limit 0) until it is no longer running. Prints
//     "<handshake ms> <first list ms> <first list bytes> <ms until it ended or
//     -> <final state or -> <VmRSS KiB after the handshake> <VmRSS KiB at the end> <peak RSS KiB>",
//     with all times but the first list counted from the start of CMD. Linux
//     only (VmRSS comes from /proc).

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
#include <sys/stat.h>
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
               "       dirbench mcp CALLS QUERY OUT -- CMD...\n"
               "       dirbench soak FORMAT SCENARIO FILES KB CALLS OUT -- CMD...\n"
               "       dirbench scan OUT -- CMD...\n";
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

long proc_status_kib( pid_t pid, std::string const &field ) {
  std::ifstream status( "/proc/" + std::to_string( pid ) + "/status" );
  std::string word;
  while ( status >> word )
    if ( word == field ) {
      long kib = 0;
      status >> kib;
      return kib;
    }
  return 0;
}

long vm_rss_kib( pid_t pid ) {
  return proc_status_kib( pid, "VmRSS:" );
}

// A document of about \a bytes in \a format; \a tag varies its content.
std::string soak_document( std::string const &format, std::size_t bytes,
                           int tag ) {
  std::string const t( std::to_string( tag ) );
  std::string r;
  if ( format == "xml" ) {
    r = "<rs>";
    while ( r.size() < bytes )
      r += "<r id=\"" + t + "\" cat=\"c1\"><v>17</v></r>";
    r += "</rs>";
  } else if ( format == "html" ) {
    r = "<html><body>";
    while ( r.size() < bytes )
      r += "<p class=\"c1\">" + t + " text</p>";
    r += "</body></html>";
  } else if ( format == "csv" ) {
    r = "id,cat,v\n";
    while ( r.size() < bytes )
      r += t + ",c1,17\n";
  } else {
    r = "{\"rs\":[{\"id\":0}";
    while ( r.size() < bytes )
      r += ",{\"id\":" + t + ",\"cat\":\"c1\",\"v\":17}";
    r += "]}";
  }
  return r;
}

int soak( std::string const &format, std::string const &scenario, int files,
          std::size_t kb, int calls, char const *out, char **cmd ) {
  auto name = [&]( int i ) {
    std::string n( std::to_string( i ) );
    n.insert( 0, n.size() < 4 ? 4 - n.size() : 0, '0' );
    return "f" + n + "." + format;
  };
  std::size_t const bytes = kb * 1024;
  auto write_file = [&]( int i, std::size_t size, int tag ) {
    std::string const text( scenario == "refused"
      ? "<r><unclosed>" + std::string( size, 'x' )
      : soak_document( format, size, tag ) );
    std::ofstream( name( i ), std::ios::binary ) << text;
  };
  for ( int i = 0; i < files; ++i )
    write_file( i, bytes, i );

  std::string const import(
    "import module namespace dir = \"http://zorba.io/modules/directory\";\n" );
  std::string const scan_query( json_string( import +
    "count(for $d in dir:documents($dir:root) "
    "where exists($d(\"content\")) or exists($d(\"error\")) return $d)" ) );

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
  std::string response;
  auto exchange = [&]( std::string const &request ) {
    std::fputs( request.c_str(), w );
    std::fputc( '\n', w );
    std::fflush( w );
    ssize_t const n = getline( &line, &cap, r );
    if ( n <= 0 )
      return false;
    response.assign( line, static_cast<std::size_t>( n ) );
    return true;
  };
  if ( !exchange( "{\"jsonrpc\":\"2.0\",\"id\":0,\"method\":\"initialize\","
                  "\"params\":{\"protocolVersion\":\"2025-11-25\","
                  "\"capabilities\":{},\"clientInfo\":{\"name\":\"dirbench\","
                  "\"version\":\"1\"}}}" ) ||
       response.find( "\"result\"" ) == std::string::npos ) {
    std::cerr << "dirbench: initialize failed: " << response << "\n";
    return 1;
  }
  std::fputs( "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}\n",
              w );

  int failed = 0;
  int next_report = 1;
  for ( int i = 1; i <= calls; ++i ) {
    int const which = ( i - 1 ) % files;
    std::string query( scan_query );
    if ( scenario == "turnover" ) {
      query = json_string( import + "count(dir:open($dir:root, \"" +
        name( ( ( i - 1 ) / 2 ) % files ) + "\")(\"content\"))" );
    } else if ( scenario == "rewrite" ) {
      // Sizes that take turns: a record of another size replaces the old one.
      write_file( which, i % 2 ? bytes : bytes / 2, i % 10 );
    } else if ( scenario == "touch" ) {
      struct timespec const times[2] = { { 0, UTIME_NOW }, { 0, UTIME_NOW } };
      utimensat( AT_FDCWD, name( which ).c_str(), times, 0 );
    } else if ( scenario == "delete" ) {
      unlink( name( which ).c_str() );
      write_file( which, bytes, which );
    }
    std::string const request(
      "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string( i ) +
      ",\"method\":\"tools/call\",\"params\":{\"name\":\"run_query\","
      "\"arguments\":{\"query\":" + query + "}}}" );
    if ( !exchange( request ) ) {
      std::cerr << "dirbench: the server closed the session at call " << i
                << "\n";
      ++failed;
      break;
    }
    if ( response.find( "\"result\"" ) == std::string::npos ||
         response.find( "\"isError\":true" ) != std::string::npos )
      ++failed;
    if ( i == next_report || i == calls ) {
      std::printf( "%d %ld %ld %d\n", i, vm_rss_kib( pid ),
                   proc_status_kib( pid, "VmHWM:" ), failed );
      std::fflush( stdout );
      if ( i == next_report )
        next_report *= 10;
    }
  }
  std::ofstream( out ) << response;

  std::fclose( w );
  int status = 0;
  if ( waitpid( pid, &status, 0 ) < 0 ) {
    std::perror( "waitpid" );
    return 1;
  }
  std::fclose( r );
  std::free( line );
  if ( !WIFEXITED( status ) || WEXITSTATUS( status ) != 0 ) {
    std::cerr << "dirbench: the server failed\n";
    return 1;
  }
  return failed ? 1 : 0;
}

int scan( char const *out, char **cmd ) {
  int to_child[2], from_child[2];
  if ( pipe( to_child ) < 0 || pipe( from_child ) < 0 ) {
    std::perror( "pipe" );
    return 1;
  }
  auto const t0 = clock_type::now();
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
  std::string response;
  auto exchange = [&]( std::string const &request ) {
    std::fputs( request.c_str(), w );
    std::fputc( '\n', w );
    std::fflush( w );
    ssize_t const n = getline( &line, &cap, r );
    if ( n <= 0 )
      return false;
    response.assign( line, static_cast<std::size_t>( n ) );
    return true;
  };

  if ( !exchange( "{\"jsonrpc\":\"2.0\",\"id\":0,\"method\":\"initialize\","
                  "\"params\":{\"protocolVersion\":\"2025-11-25\","
                  "\"capabilities\":{},\"clientInfo\":{\"name\":\"dirbench\","
                  "\"version\":\"1\"}}}" ) ||
       response.find( "\"result\"" ) == std::string::npos ) {
    std::cerr << "dirbench: initialize failed: " << response << "\n";
    return 1;
  }
  double const handshake = ms_since( t0 );
  long const rss_handshake = vm_rss_kib( pid );
  std::fputs( "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}\n",
              w );

  std::string const list(
    "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":"
    "{\"name\":\"list_documents\",\"arguments\":{}}}" );
  auto const t1 = clock_type::now();
  if ( !exchange( list ) || response.find( "\"isError\":false" ) == std::string::npos ) {
    std::cerr << "dirbench: list_documents failed: " << response << "\n";
    return 1;
  }
  double const first_list = ms_since( t1 );
  std::size_t const first_bytes = response.size();
  std::ofstream( out ) << response;

  char const running[] = "\\\"state\\\":\\\"running\\\"";
  char const state[] = "\\\"state\\\":";
  std::string done( "-" ), final_state( "-" );
  if ( response.find( state ) != std::string::npos ) {
    std::string const poll(
      "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\",\"params\":"
      "{\"name\":\"list_documents\",\"arguments\":{\"limit\":0}}}" );
    while ( response.find( running ) != std::string::npos ) {
      usleep( 2000 );
      if ( !exchange( poll ) ) {
        std::cerr << "dirbench: the server closed the session\n";
        return 1;
      }
    }
    done = std::to_string( ms_since( t0 ) );
    final_state = response.find( "memory_limit" ) != std::string::npos
      ? "memory_limit" : "complete";
  }
  long const rss_end = vm_rss_kib( pid );

  std::fclose( w );
  int status = 0;
  rusage ru{};
  if ( wait4( pid, &status, 0, &ru ) < 0 ) {
    std::perror( "wait4" );
    return 1;
  }
  std::fclose( r );
  std::free( line );
  if ( !WIFEXITED( status ) || WEXITSTATUS( status ) != 0 ) {
    std::cerr << "dirbench: the server failed\n";
    return 1;
  }
  std::printf( "%.1f %.1f %zu %s %s %ld %ld %ld\n", handshake, first_list,
               first_bytes, done.c_str(), final_state.c_str(), rss_handshake,
               rss_end, ru.ru_maxrss );
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
  if ( argc >= 10 && !std::strcmp( argv[1], "soak" ) &&
       !std::strcmp( argv[8], "--" ) ) {
    int const files = std::atoi( argv[4] );
    int const kb = std::atoi( argv[5] );
    int const calls = std::atoi( argv[6] );
    if ( files <= 0 || kb <= 0 || calls <= 0 )
      return usage();
    return soak( argv[2], argv[3], files, static_cast<std::size_t>( kb ), calls,
                 argv[7], argv + 9 );
  }
  if ( argc >= 5 && !std::strcmp( argv[1], "scan" ) &&
       !std::strcmp( argv[3], "--" ) )
    return scan( argv[2], argv + 4 );
  return usage();
}
