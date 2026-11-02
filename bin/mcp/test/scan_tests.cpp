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

// The background scanner of zorba-mcp, through the list_documents tool.
//
// Usage: mcp_scan_tests <test> <zorba-mcp>
//
// A test that needs the scanner in a given state stops it with a Gate (see
// ServerConfig::scan_hook), so what a test sees does not depend on timing.
// wait_complete() polls for a state and fails if it never comes.

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libxml/parser.h>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "json.h"
#include "mcp_server.h"
#include "scan_preview.h"

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
# define ZORBA_TEST_SANITIZER 1
#elif defined(__has_feature)
# if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#  define ZORBA_TEST_SANITIZER 1
# endif
#endif

using namespace zorba;
using namespace zorba::mcp;
namespace stdfs = std::filesystem;

namespace {

int failures = 0;

void check( bool ok, std::string const &what, std::string const &detail = "" ) {
  if ( ok ) {
    std::cout << "ok: " << what << std::endl;
  } else {
    ++failures;
    std::cout << "FAILED: " << what;
    if ( !detail.empty() )
      std::cout << "\n  got: " << detail;
    std::cout << std::endl;
  }
}

bool contains( std::string const &s, std::string const &part ) {
  return s.find( part ) != std::string::npos;
}

std::string dump_of( Json const *j ) {
  return j ? j->dump() : std::string();
}

struct Fixture {
  Zorba *zorba;
  std::string binary;
};

struct Tree {
  std::string base;
  std::string root;

  Tree() {
    std::string tmpl(
      ( stdfs::temp_directory_path() / "mcp-scan-XXXXXX" ).string() );
    char *const made = mkdtemp( &tmpl[0] );
    base = stdfs::canonical( made ? made : "." ).string();
    root = base + "/root";
    stdfs::create_directories( root );
    stdfs::create_directories( base + "/outside" );
  }
  ~Tree() {
    std::error_code ec;
    stdfs::remove_all( base, ec );
  }

  void write( std::string const &rel, std::string const &content ) const {
    stdfs::path const p( root + "/" + rel );
    stdfs::create_directories( p.parent_path() );
    std::ofstream out( p.c_str(), std::ios::binary );
    out << content;
  }
  void outside( std::string const &name, std::string const &content ) const {
    std::ofstream out( ( base + "/outside/" + name ).c_str(),
                       std::ios::binary );
    out << content;
  }
};

// Stops the scanner at one point (a stage and a path; see scan_hook) until
// the test releases it.
class Gate {
public:
  Gate( std::string stage, std::string path ) :
    theStage( std::move( stage ) ), thePath( std::move( path ) ) { }

  void hook( char const *stage, std::string const &path ) {
    if ( theStage != stage || ( thePath != "*" && thePath != path ) )
      return;
    std::unique_lock<std::mutex> lock( theMutex );
    ++theReached;
    theCv.notify_all();
    theCv.wait( lock, [this] { return theOpen; } );
    ++theLeft;
    theCv.notify_all();
  }

  // Whether the scanner got to the gate.
  bool wait_reached() {
    std::unique_lock<std::mutex> lock( theMutex );
    return theCv.wait_for( lock, std::chrono::seconds( 60 ),
                           [this] { return theReached > 0; } );
  }

  void release() {
    std::lock_guard<std::mutex> lock( theMutex );
    theOpen = true;
    theCv.notify_all();
  }

  // Whether everything that reached the gate has left it.
  bool wait_left() {
    std::unique_lock<std::mutex> lock( theMutex );
    return theCv.wait_for( lock, std::chrono::seconds( 60 ),
                           [this] { return theLeft == theReached; } );
  }

private:
  std::string const theStage, thePath;
  std::mutex theMutex;
  std::condition_variable theCv;
  bool theOpen = false;
  int theReached = 0, theLeft = 0;
};

// A config whose scanner stops at \a gate.
ServerConfig gated( std::string const &root,
                    std::shared_ptr<Gate> const &gate ) {
  ServerConfig config;
  config.root = root;
  config.scan_hook = [gate]( char const *stage, std::string const &path ) {
    gate->hook( stage, path );
  };
  return config;
}

Json meta() {
  return Json::object( {
    { "io.modelcontextprotocol/protocolVersion", protocol_version },
    { "io.modelcontextprotocol/clientCapabilities", Json::object() },
    { "io.modelcontextprotocol/clientInfo", Json::object( {
        { "name", "mcp_scan_tests" }, { "version", "1" } } ) }
  } );
}

std::string request( Json id, std::string const &method,
                     Json params = Json::object() ) {
  params.set( "_meta", meta() );
  return Json::object( {
    { "jsonrpc", "2.0" }, { "id", id }, { "method", method },
    { "params", params }
  } ).dump();
}

Json parsed( std::string const &text ) {
  Json r;
  std::string why;
  if ( !parse_json( text, &r, &why ) )
    check( false, "is JSON", text + " (" + why + ")" );
  return r;
}

Json call( Server &server, std::string const &tool, Json args ) {
  return parsed( server.handle( request( 1, "tools/call", Json::object( {
    { "name", tool }, { "arguments", args } } ) ) ) );
}

// One answer of list_documents: its first line, and the files after it.
struct Listing {
  bool is_error = false;
  std::string text;
  Json head;
  std::vector<Json> files;

  std::string state() const { return text_at( "state" ); }
  std::string found() const { return text_at( "files_found" ); }
  std::string sampled() const { return text_at( "files_sampled" ); }
  std::string total() const { return member( "total" ); }
  std::string shown() const { return member( "shown" ); }
  std::string member( std::string const &name ) const {
    Json const *const m = head.find( name );
    return m ? m->text() : std::string();
  }
  bool has_scan() const {
    Json const *const s = head.find( "scan" );
    return s && s->is_object() && s->find( "state" ) &&
      s->find( "files_found" ) && s->find( "files_sampled" );
  }
  bool lists( std::string const &path ) const {
    for ( Json const &f : files )
      if ( f.find( "path" ) && f.find( "path" )->text() == path )
        return true;
    return false;
  }
  std::string error_code() const {
    Json const *const e = head.find( "error" );
    Json const *const c = e ? e->find( "code" ) : nullptr;
    return c ? c->text() : std::string();
  }
  Json const* preview() const { return head.find( "preview" ); }

private:
  std::string text_at( std::string const &name ) const {
    Json const *const s = head.find( "scan" );
    Json const *const v = s ? s->find( name ) : nullptr;
    return v ? v->text() : std::string();
  }
};

Listing list( Server &server, Json args = Json::object() ) {
  Json const r( call( server, "list_documents", std::move( args ) ) );
  Listing l;
  Json const *const res = r.find( "result" );
  Json const *const content = res ? res->find( "content" ) : nullptr;
  Json const *const is_error = res ? res->find( "isError" ) : nullptr;
  l.is_error = is_error && is_error->is_bool() && is_error->as_bool();
  if ( !content || !content->is_array() || content->items().empty() ) {
    check( false, "list_documents gives a text", r.dump() );
    return l;
  }
  l.text = content->items()[0].find( "text" )->text();
  std::istringstream lines( l.text );
  std::string line;
  bool first = true;
  while ( std::getline( lines, line ) ) {
    if ( first )
      l.head = parsed( line );
    else
      l.files.push_back( parsed( line ) );
    first = false;
  }
  return l;
}

Listing list_path( Server &server, std::string const &path ) {
  return list( server, Json::object( { { "path", path } } ) );
}

// Polls until the scan has ended, complete or at its memory limit; fails the
// test if it does not get there (or if the answers have no scan state at all).
Listing wait_settled( Server &server ) {
  Json const args( Json::object( { { "limit", 0 } } ) );
  for ( int i = 0; i < 30000; ++i ) {
    Listing const l( list( server, args ) );
    if ( !l.has_scan() ) {
      check( false, "the answer has a scan state", l.text );
      return l;
    }
    if ( l.state() != "running" )
      return l;
    std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
  }
  check( false, "the scan ends" );
  return Listing();
}

Listing wait_complete( Server &server ) {
  Listing const l( wait_settled( server ) );
  check( l.state() == "complete", "the scan completes", l.text );
  return l;
}

// Names of the same length.
std::string file_name( int i ) {
  std::string n( std::to_string( 1000 + i ) );
  return "f" + n + ".csv";
}

std::string many_json_records( std::size_t bytes ) {
  std::string s( "[" );
  for ( int i = 0; s.size() < bytes; ++i )
    s += std::string( i ? "," : "" ) + "{\"id\":" + std::to_string( i ) + "}";
  return s + "]";
}

///////////////////////////////////////////////////////////////////////////////

// While the walker is stopped the answer says "running" and has what is known;
// afterwards it has everything and says "complete".
void test_status( Fixture const &f ) {
  Tree t;
  t.write( "a.csv", "x,y\n1,2\n" );
  t.write( "sub/b.json", "{\"k\":1}" );
  auto const gate = std::make_shared<Gate>( "walk", "sub" );
  Server server( f.zorba, gated( t.root, gate ), std::cerr );
  check( gate->wait_reached(), "the walker reaches sub" );

  Listing l( list( server ) );
  check( l.has_scan() && l.state() == "running",
         "directory answer while walking: running", l.text );
  check( l.lists( "a.csv" ) && !l.lists( "sub/b.json" ),
         "it has what is known so far", l.text );
  check( l.total() == "1" && l.found() == "1",
         "the total is what is known so far", l.text );
  check( contains( dump_of( l.head.find( "dirs" ) ), "\"path\":\"sub\"" ),
         "while running, the subdirectories are named", l.text );

  gate->release();
  l = wait_complete( server );
  check( l.found() == "2" && l.sampled() == "2",
         "complete: all found and sampled", l.text );
  l = list( server );
  check( l.lists( "a.csv" ) && l.lists( "sub/b.json" ) && l.total() == "2" &&
         l.shown() == "2" && l.state() == "complete",
         "the listing is complete", l.text );
  check( !contains( l.text, "preview" ),
         "a directory answer has no previews", l.text );
  for ( Json const &file : l.files )
    check( file.find( "format" ) && file.find( "size" ) && file.find( "fields" ),
           "per file: path, format, size, fields", file.dump() );
}

// A file with no preview yet is explored at once, known to the walker or not.
void test_immediate( Fixture const &f ) {
  {
    // The sampler is held at this file: the request stops waiting at its bound
    // and says the preview is pending.
    Tree t;
    t.write( "a.csv", "x,y\n1,2\n" );
    auto const gate = std::make_shared<Gate>( "sample", "*" );
    ServerConfig config( gated( t.root, gate ) );
    config.timeout = std::chrono::milliseconds( 300 );
    Server server( f.zorba, config, std::cerr );
    check( gate->wait_reached(), "the sampler reaches the file" );
    auto const t0 = std::chrono::steady_clock::now();
    Listing l( list_path( server, "a.csv" ) );
    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - t0 ).count();
    check( l.state() == "running" && !l.is_error, "file answer has the status",
           l.text );
    check( l.member( "preview_state" ) == "pending" && l.preview() &&
           l.preview()->is_null(), "the preview is pending", l.text );
    check( ms >= 250 && ms < 10000, "the request waited for its bound only",
           std::to_string( ms ) + " ms" );
    gate->release();
    Listing const done( wait_complete( server ) );
    check( done.found() == "1" && done.sampled() == "1",
           "requesting and sampling count the file once", done.text );
    l = list_path( server, "a.csv" );
    check( l.member( "preview_state" ).empty() && l.preview() &&
           contains( l.preview()->dump(), "\"columns\":[\"x\",\"y\"]" ),
           "afterwards it has the preview", l.text );
  }
  {
    // The sampler is free: the request gets the preview, read on its thread and
    // not on the request's, for a file the walker has not found.
    Tree t;
    t.write( "deep/er/b.json", "{\"k\":1}" );
    auto const gate = std::make_shared<Gate>( "walk", "" );
    std::thread::id const request_thread( std::this_thread::get_id() );
    auto const on_request_thread = std::make_shared<std::atomic<int> >( 0 );
    auto const explored = std::make_shared<std::atomic<int> >( 0 );
    ServerConfig config;
    config.root = t.root;
    config.scan_hook = [gate, request_thread, on_request_thread, explored](
        char const *stage, std::string const &path ) {
      if ( std::string( stage ) == "explore" ) {
        ++*explored;
        if ( std::this_thread::get_id() == request_thread )
          ++*on_request_thread;
      }
      gate->hook( stage, path );
    };
    Server server( f.zorba, config, std::cerr );
    check( gate->wait_reached(), "the walker reaches the root" );
    Listing const l( list_path( server, "deep/er/b.json" ) );
    check( l.state() == "running" && l.preview() &&
           contains( l.preview()->dump(), "\"k\":[1]" ),
           "a file the walker has not found is explored too", l.text );
    check( explored->load() == 1 && on_request_thread->load() == 0,
           "the request thread does not parse",
           std::to_string( explored->load() ) + " " +
           std::to_string( on_request_thread->load() ) );
    gate->release();
    wait_complete( server );
  }
}

// One tag with very many attributes takes the parser time that grows with the
// square of their number. It is refused before parsing, so that requests and
// the sampler go on and the threads can be joined.
void test_huge_tag( Fixture const &f ) {
  Tree t;
  auto tag = []( char const *open, char const *close, int attributes ) {
    std::string s( open );
    for ( int i = 0; i < attributes; ++i )
      s += " a" + std::to_string( i ) + close;
    return s + ">";
  };
  std::string const html( tag( "<p", "=1", 120000 ) );
  std::string const xml( tag( "<p", "=\"1\"", 120000 ) + "</p>" );
  t.write( "one.html", html );
  t.write( "two.html", html );
  t.write( "one.xml", xml );
  t.write( "two.xml", xml );
  std::string utf16( "\xFF\xFE" );
  for ( char const c : xml ) {
    utf16 += c;
    utf16 += '\0';
  }
  t.write( "utf16.xml", utf16 );
  t.write( "fine.csv", "x\n1\n" );

  // Limits, and what is not over them.
  t.write( "attrs300.xml", tag( "<p", "=\"1\"", 300 ) + "</p>" );
  t.write( "attrs200.xml", tag( "<p", "=\"1\"", 200 ) + "</p>" );
  t.write( "attrs300.html", tag( "<p", "=1", 300 ) );
  t.write( "attrs200.html", tag( "<p", "=1", 200 ) );
  std::string const name( 20000, 'n' );
  t.write( "name.xml", "<" + name + "/>" );
  t.write( "value.xml", "<p a=\"" + std::string( 200000, 'v' ) + "\"/>" );
  t.write( "value.html", "<p a=\"" + std::string( 200000, 'v' ) + "\">x</p>" );
  t.write( "script.html", "<script>if (a<b) {" + std::string( 100000, ' ' ) +
                          "}</script><table><tr><th>A</th></tr></table>" );

  ServerConfig config;
  config.root = t.root;
  config.timeout = std::chrono::seconds( 5 );
  Server server( f.zorba, config, std::cerr );
  for ( char const *const file : { "one.html", "one.xml", "utf16.xml",
                                   "attrs300.xml", "attrs300.html", "name.xml" } ) {
    auto const t0 = std::chrono::steady_clock::now();
    Listing const l( list_path( server, file ) );
    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - t0 ).count();
    std::cout << file << ": " << ms << " ms" << std::endl;
    check( ms < 8000, std::string( file ) + ": the request stays in its bound",
           std::to_string( ms ) + " ms" );
    check( contains( dump_of( l.preview() ), "\"error\":" ),
           std::string( file ) + ": refused in the preview", l.text );
  }
  Listing l( wait_settled( server ) );
  check( l.state() == "complete" && l.found() == "14" && l.sampled() == "14",
         "the sampler gets through all of them", l.text );
  for ( char const *const file : { "attrs200.xml", "attrs200.html", "value.xml",
                                   "value.html" } ) {
    l = list_path( server, file );
    check( !contains( dump_of( l.preview() ), "\"error\":" ),
           std::string( file ) + ": under the limits", l.text );
  }
  l = list_path( server, "script.html" );
  check( contains( dump_of( l.preview() ), "\"headers\":[[\"A\"]]" ),
         "a script is not taken for a tag", l.text );
  check( server.stop_scan(), "stop joins both threads" );
}

// A sibling whose name sorts between a directory and what is under it.
void test_known_dirs( Fixture const &f ) {
  Tree t;
  t.write( "sub/a.csv", "x\n1\n" );
  stdfs::create_directories( t.root + "/sub-x" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  wait_complete( server );
  stdfs::remove( t.root + "/sub-x" );
  list( server );
  stdfs::create_directories( t.root + "/sub-x" );
  t.write( "sub-x/f.csv", "x\n1\n" );
  list( server );
  wait_complete( server );
  Listing const l( list( server ) );
  check( l.lists( "sub-x/f.csv" ) && l.lists( "sub/a.csv" ) && l.total() == "2",
         "a directory deleted and created again is walked", l.text );
}

// Runs as an ordinary user, also under root in a container, where permissions
// would not stop anything. Destroy it after what it covers.
class OrdinaryUser {
public:
  OrdinaryUser() : theWasRoot( geteuid() == 0 ) {
    if ( theWasRoot )
      check( seteuid( 65534 ) == 0, "become an ordinary user" );
  }
  ~OrdinaryUser() {
    if ( theWasRoot && seteuid( 0 ) != 0 )
      std::cout << "FAILED: back to root" << std::endl;
  }
private:
  bool const theWasRoot;
};

void test_unreadable( Fixture const &f ) {
  OrdinaryUser const user;
  Tree t;
  t.write( "open/a.csv", "x\n1\n" );
  t.write( "locked/b.csv", "x\n1\n" );
  std::string const locked( t.root + "/locked" );
  stdfs::permissions( locked, stdfs::perms::none );
  {
    ServerConfig config;
    config.root = t.root;
    Server server( f.zorba, config, std::cerr );
    Listing l( wait_settled( server ) );
    l = list( server );
    check( contains( l.text, "\"incomplete\":true" ) &&
           l.member( "unreadable_dirs" ) == "1",
           "an unreadable directory is stated", l.text );
    l = list_path( server, "locked" );
    check( l.is_error && l.error_code() == "permission-denied" && l.has_scan(),
           "asking for it: permission-denied", l.text );
    l = list_path( server, "locked/b.csv" );
    check( l.is_error && l.error_code() == "permission-denied",
           "asking for a file in it: permission-denied", l.text );

    // When it can be read again, it is walked and no longer stated.
    stdfs::permissions( locked, stdfs::perms::owner_all );
    l = list_path( server, "locked" );
    check( !l.is_error && l.lists( "locked/b.csv" ),
           "readable again: it is listed", l.text );
    l = list( server );
    check( !contains( l.text, "\"incomplete\"" ) &&
           !contains( l.text, "unreadable_dirs" ),
           "readable again: nothing to state", l.text );

    // A file added below after the walk is seen by the directory's mtime.
    t.write( "open/new.csv", "n\n1\n" );
    auto const stamp = stdfs::last_write_time( t.root + "/open" );
    stdfs::last_write_time( t.root + "/open", stamp + std::chrono::hours( 1 ) );
    list( server );
    wait_settled( server );
    l = list( server );
    check( l.lists( "open/new.csv" ), "a new file below is found", l.text );

    // And a file deleted below goes with the same check.
    stdfs::remove( t.root + "/open/new.csv" );
    stdfs::last_write_time( t.root + "/open", stamp + std::chrono::hours( 2 ) );
    list( server, Json::object( { { "limit", 0 } } ) );
    wait_settled( server );
    l = list( server, Json::object( { { "limit", 0 } } ) );
    check( l.total() == "2", "a deleted file below leaves the totals", l.text );
    check( !list( server ).lists( "open/new.csv" ), "and the listing", l.text );
  }
}

// A tree deeper than the walker goes is stated, and a path still works.
void test_deep_tree( Fixture const &f ) {
  Tree t;
  std::string dir;
  for ( int i = 0; i < 300; ++i )
    dir += "d/";
  t.write( dir + "leaf.csv", "x\n1\n" );
  t.write( "top.csv", "x\n1\n" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  wait_settled( server );
  Listing l( list( server ) );
  check( contains( l.text, "\"incomplete\":true" ) && l.lists( "top.csv" ),
         "the listing says it may be incomplete", l.text );
  l = list_path( server, dir + "leaf.csv" );
  check( !l.is_error && contains( dump_of( l.preview() ), "\"columns\":[\"x\"]" ),
         "a file down there still works by path", l.text );
}

void test_preview_bound( Fixture const &f ) {
  ( void )f;
  std::string const long_prefix( 400000, 'p' );
  std::string const many_ns( "<" + long_prefix + ":r xmlns:" + long_prefix +
                             "=\"u\"/>" );
  // Control characters take six bytes each in JSON.
  std::string const controls( 70, '\x01' );
  std::string csv, csv_row;
  for ( int i = 0; i < 32; ++i ) {
    csv += std::string( i ? "," : "" ) + controls;
    csv_row += std::string( i ? "," : "" ) + std::string( 39, '\x02' );
  }
  csv += "\n" + csv_row + "\n" + csv_row + "\n" + csv_row + "\n";
  std::string json( "{" );
  for ( int i = 0; i < 40; ++i )
    json += std::string( i ? "," : "" ) + "\"" + controls + std::to_string( i ) +
      "\":\"" + std::string( 39, '\x01' ) + "\"";
  std::string json_escaped;
  for ( char const c : json + "}" ) {
    if ( c == '\x01' )
      json_escaped += "\\u0001";
    else
      json_escaped += c;
  }
  std::string xml( "<r>" );
  for ( int i = 0; i < 40; ++i )
    xml += "<" + std::string( 70, 'e' ) + std::to_string( i ) + "/>";
  xml += "</r>";

  // Under the limit of a tag: the prefixes themselves have to be cut.
  std::string const prefix( 3000, 'q' );
  std::string prefixes( "<r" );
  for ( int i = 0; i < 4; ++i )
    prefixes += " xmlns:" + prefix + std::to_string( i ) + "=\"u\"";
  prefixes += "/>";

  struct { char const *what; ScanFormat format; std::string const &data; }
    const cases[] = {
      { "xml with a long prefix", ScanFormat::xml, many_ns },
      { "xml with long prefixes", ScanFormat::xml, prefixes },
      { "xml with many paths", ScanFormat::xml, xml },
      { "csv", ScanFormat::csv, csv },
      { "json", ScanFormat::json, json_escaped } };
  for ( auto const &c : cases ) {
    ScanPreview const p( make_scan_preview( c.format, c.data, false, nullptr ) );
    check( p.json.size() < 8192, std::string( c.what ) + ": under 8 KB",
           std::to_string( p.json.size() ) );
    Json parsed_json;
    std::string why;
    check( parse_json( p.json, &parsed_json, &why ),
           std::string( c.what ) + ": JSON", why );
  }

  // Each prefix once: strict parsers refuse repeated members.
  ScanPreview const p( make_scan_preview( ScanFormat::xml,
    "<a xmlns:p=\"u\" xmlns=\"d\"><b xmlns:p=\"v\" xmlns=\"e\"/></a>", false,
    nullptr ) );
  Json j;
  std::string why;
  check( parse_json( p.json, &j, &why ) && j.find( "namespaces" ) &&
         j.find( "namespaces" )->members().size() == 2,
         "each prefix once", p.json );

  // Arrays too, for a key that is repeated.
  ScanPreview const q( make_scan_preview( ScanFormat::json,
    "{\"a\":[1],\"a\":[1,2]}", false, nullptr ) );
  check( parse_json( q.json, &j, &why ) && j.find( "arrays" ) &&
         j.find( "arrays" )->members().size() == 1, "each array once", q.json );
}

// Names that are not UTF-8 stay apart and can be asked for, where the file
// system has them.
void test_escaped_names( Fixture const &f ) {
  Tree t;
  t.write( "n\xFF.csv", "ff\n1\n" );
  t.write( "n\xFE.csv", "fe\n1\n" );
  t.write( "plain.csv", "p\n1\n" );
  std::size_t files = 0;
  for ( auto const &entry : stdfs::directory_iterator( t.root ) ) {
    ( void )entry;
    ++files;
  }
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  wait_complete( server );
  Listing l( list( server ) );
  check( l.files.size() == files, "all listed", l.text );
  if ( files == 3 ) {
    check( l.lists( "n\\xFF.csv" ) && l.lists( "n\\xFE.csv" ),
           "the names stay apart", l.text );
    l = list_path( server, "n\\xFF.csv" );
    check( contains( dump_of( l.preview() ), "\"columns\":[\"ff\"]" ) &&
           l.member( "path" ) == "n\\xFF.csv",
           "the escaped name can be asked for", l.text );
    l = list_path( server, "n\\xFE.csv" );
    check( contains( dump_of( l.preview() ), "\"columns\":[\"fe\"]" ),
           "the other one too", l.text );
  }
}

// The limit is filled, and dirs is current, after files turn out to be gone.
void test_refill( Fixture const &f ) {
  Tree t;
  t.write( "sub/a.csv", "x\n1\n" );
  t.write( "sub/b.csv", "x\n1\n" );
  t.write( "top.csv", "x\n1\n" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  wait_complete( server );
  stdfs::remove( t.root + "/sub/a.csv" );
  stdfs::remove( t.root + "/sub/b.csv" );
  Listing l( list( server, Json::object( { { "limit", 1 } } ) ) );
  check( l.total() == "1" && l.shown() == "1" && l.lists( "top.csv" ),
         "the limit is filled from what remains", l.text );

  t.write( "two/a.csv", "x\n1\n" );
  t.write( "two/b.csv", "x\n1\n" );
  t.write( "two/c.csv", "x\n1\n" );
  list_path( server, "two" );
  wait_complete( server );
  stdfs::remove( t.root + "/two/a.csv" );
  stdfs::last_write_time( t.root + "/two",
    stdfs::last_write_time( t.root + "/two" ) + std::chrono::hours( 1 ) );
  list( server, Json::object( { { "limit", 0 } } ) );
  wait_complete( server );
  l = list( server, Json::object( { { "limit", 1 } } ) );
  check( contains( dump_of( l.head.find( "dirs" ) ),
                   "{\"path\":\"two\",\"files\":2}" ),
         "dirs counts what remains", l.text );
  stdfs::remove_all( t.root + "/two" );
  l = list( server, Json::object( { { "limit", 1 } } ) );
  check( !contains( l.text, "two" ), "a removed directory leaves dirs", l.text );
}

void test_previews( Fixture const &f ) {
  Tree t;
  t.write( "t.csv", "id,name,qty,price,ok\n1,ann,2,1.5,true\n2,bob,3,2.5,false\n"
                    "3,cy,4,3,true\n4,di,5,4,false\n" );
  t.write( "a.json", "[{\"id\":1,\"who\":{\"name\":\"ann\"},\"tags\":[\"x\"]},"
                     "{\"id\":2,\"who\":{\"name\":\"bob\"}},{\"id\":3}]" );
  t.write( "o.json", "{\"rs\":[{\"id\":1},{\"id\":2}]}" );
  t.write( "l.jsonl", "{\"a\":1}\n{\"a\":2}\n" );
  t.write( "long.json", "{\"s\":\"" + std::string( 500, 'z' ) + "\"}" );
  t.write( "x.xml", "<?xml version=\"1.0\"?><o:orders xmlns:o=\"urn:o\" "
                    "xmlns=\"urn:d\"><order id=\"1\"><total>100</total></order>"
                    "<order id=\"2\"><total>5</total></order></o:orders>" );
  t.write( "p.html", "<html><head><title>T</title></head><body>"
                     "<table><tr><th>A</th><th>B</th></tr></table>"
                     "<table><thead><tr><th>C</th></tr></thead></table>"
                     "<table><tr><td>x</td></tr></table></body></html>" );
  t.write( "big.json", many_json_records( 3 * 1024 * 1024 ) );
  t.write( "big.csv", "id\n" + std::string( 3 * 1024 * 1024, '7' ) + "\n1\n" );
  std::string big_xml( "<r>" );
  for ( int i = 0; big_xml.size() < 3 * 1024 * 1024; ++i )
    big_xml += "<i>" + std::to_string( i ) + "</i>";
  t.write( "big.xml", big_xml + "</r>" );
  t.write( "notes.txt", "plain" );

  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );

  std::string p( dump_of( list_path( server, "t.csv" ).preview() ) );
  check( contains( p, "\"columns\":[\"id\",\"name\",\"qty\",\"price\",\"ok\"]" ),
         "csv: header", p );
  check( contains( p, "\"kinds\":[\"integer\",\"string\",\"integer\",\"decimal\",\"boolean\"]" ),
         "csv: column kinds", p );
  check( contains( p, "[\"3\",\"cy\",\"4\",\"3\",\"true\"]" ) && !contains( p, "\"di\"" ),
         "csv: three rows", p );
  check( contains( p, "\"records\":4" ), "csv: records", p );

  p = dump_of( list_path( server, "a.json" ).preview() );
  check( contains( p, "\"shape\":\"array\"" ) && contains( p, "\"records\":3" ),
         "json: shape and array length", p );
  check( contains( p, "\"[].id\":[1,2]" ) &&
         contains( p, "\"[].who.name\":[\"ann\",\"bob\"]" ) &&
         contains( p, "\"[].tags[]\":[\"x\"]" ),
         "json: key paths with up to two samples", p );

  p = dump_of( list_path( server, "o.json" ).preview() );
  check( contains( p, "\"shape\":\"object\"" ) &&
         contains( p, "\"rs[].id\":[1,2]" ) &&
         contains( p, "\"arrays\":{\"rs\":2}" ),
         "json: nested array, with its length", p );

  p = dump_of( list_path( server, "l.jsonl" ).preview() );
  check( contains( p, "\"shape\":\"lines\"" ) && contains( p, "\"records\":2" ),
         "json: several top-level values", p );

  p = dump_of( list_path( server, "long.json" ).preview() );
  check( contains( p, "zzz" ) && p.size() < 200, "json: a sample is short", p );

  p = dump_of( list_path( server, "x.xml" ).preview() );
  check( contains( p, "\"root\":\"o:orders\"" ), "xml: root name", p );
  check( contains( p, "\"namespaces\":{\"o\":\"urn:o\",\"\":\"urn:d\"}" ),
         "xml: namespaces", p );
  check( contains( p, "\"/o:orders/order\"" ) &&
         contains( p, "\"/o:orders/order/@id\"" ) &&
         contains( p, "\"/o:orders/order/total\"" ),
         "xml: paths seen", p );
  check( contains( p, "\"sample\":{\"path\":\"/o:orders/order/total\",\"value\":\"100\"}" ),
         "xml: one sample", p );
  check( contains( p, "\"records\":2" ), "xml: records", p );

  p = dump_of( list_path( server, "p.html" ).preview() );
  check( contains( p, "\"title\":\"T\"" ) && contains( p, "\"tables\":3" ) &&
         contains( p, "\"headers\":[[\"A\",\"B\"],[\"C\"],[]]" ),
         "html: title, tables, header cells", p );

  for ( char const *const name : { "big.json", "big.csv", "big.xml" } ) {
    Listing const l( list_path( server, name ) );
    p = dump_of( l.preview() );
    check( contains( p, "\"records\":null" ) && contains( p, "\"truncated\":true" ),
           std::string( name ) + " over the cap: no count, truncated", p );
    check( !contains( p, "\"error\"" ),
           std::string( name ) + " over the cap is not malformed", p );
    check( l.text.size() < 4096, std::string( name ) + ": the answer is short",
           l.text );
  }

  Listing l( list_path( server, "notes.txt" ) );
  check( !l.is_error && l.preview() && l.preview()->is_null() &&
         l.head.find( "format" ) && l.head.find( "format" )->is_null(),
         "unknown format: no preview", l.text );

  wait_complete( server );
  l = list( server, Json::object( { { "limit", 100 } } ) );
  bool csv_fields = false;
  for ( Json const &file : l.files )
    if ( file.find( "path" )->text() == "t.csv" )
      csv_fields = contains( file.dump(),
        "\"fields\":[\"id\",\"name\",\"qty\",\"price\",\"ok\"]" );
  check( csv_fields, "directory answer: fields of the csv", l.text );
  check( l.text.size() < 4096, "directory answer: short", l.text );
}

void test_mtime( Fixture const &f ) {
  Tree t;
  t.write( "a.csv", "x\n1\n" );
  t.write( "sub/b.csv", "y\n1\n" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  wait_complete( server );
  check( contains( list_path( server, "a.csv" ).text, "\"columns\":[\"x\"]" ),
         "the first preview" );

  // Same size, other mtime.
  auto const stamp = stdfs::last_write_time( t.root + "/a.csv" );
  t.write( "a.csv", "z\n1\n" );
  stdfs::last_write_time( t.root + "/a.csv", stamp + std::chrono::hours( 1 ) );
  Listing l( list_path( server, "a.csv" ) );
  check( contains( l.text, "\"columns\":[\"z\"]" ), "mtime changed: rescanned",
         l.text );

  // Same mtime, other size.
  auto const stamp2 = stdfs::last_write_time( t.root + "/a.csv" );
  t.write( "a.csv", "zz\n1\n" );
  stdfs::last_write_time( t.root + "/a.csv", stamp2 );
  l = list_path( server, "a.csv" );
  check( contains( l.text, "\"columns\":[\"zz\"]" ), "size changed: rescanned",
         l.text );

  // A directory request re-lists the directory.
  t.write( "sub/new.csv", "n\n1\n" );
  stdfs::remove( t.root + "/sub/b.csv" );
  l = list_path( server, "sub" );
  check( l.lists( "sub/new.csv" ) && !l.lists( "sub/b.csv" ) && l.total() == "1",
         "a directory request re-lists it", l.text );

  // A new subdirectory found by re-listing gets walked.
  t.write( "sub/more/c.csv", "c\n1\n" );
  list_path( server, "sub" );
  l = wait_complete( server );
  l = list_path( server, "sub" );
  check( l.lists( "sub/more/c.csv" ), "a new subdirectory is walked", l.text );

  // A shown file that changed does not keep its old fields.
  auto const stamp3 = stdfs::last_write_time( t.root + "/sub/new.csv" );
  t.write( "sub/new.csv", "changed\n1\n" );
  stdfs::last_write_time( t.root + "/sub/new.csv",
                          stamp3 + std::chrono::hours( 1 ) );
  l = list_path( server, "sub" );
  check( !contains( l.text, "\"fields\":[\"n\"]" ),
         "a changed file does not show its old fields", l.text );
  wait_complete( server );
  l = list_path( server, "sub" );
  check( contains( l.text, "\"fields\":[\"changed\"]" ),
         "a changed file is sampled again", l.text );

  // A deleted file is an error, not its old preview.
  stdfs::remove( t.root + "/a.csv" );
  l = list_path( server, "a.csv" );
  check( l.is_error && l.error_code() == "not-found" && l.has_scan(),
         "a deleted file: not-found, with the status", l.text );
}

void test_refresh( Fixture const &f ) {
  Tree t;
  t.write( "keep/a.csv", "x\n1\n" );
  t.write( "gone/b.csv", "x\n1\n" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  wait_complete( server );

  t.write( "keep/deep/new.csv", "n\n1\n" );
  stdfs::remove_all( t.root + "/gone" );
  // Same size and mtime: only a restart reads it again.
  auto const stamp = stdfs::last_write_time( t.root + "/keep/a.csv" );
  t.write( "keep/a.csv", "y\n1\n" );
  stdfs::last_write_time( t.root + "/keep/a.csv", stamp );

  Listing l( list( server, Json::object( { { "refresh", true } } ) ) );
  check( l.has_scan() && !l.is_error, "refresh answers with the status", l.text );
  l = wait_complete( server );
  check( l.found() == "2" && l.sampled() == "2",
         "refresh: found again, sampled again", l.text );
  l = list( server );
  check( l.lists( "keep/deep/new.csv" ) && l.lists( "keep/a.csv" ) &&
         !l.lists( "gone/b.csv" ), "refresh: new file in, deleted file out",
         l.text );
  check( contains( l.text, "\"fields\":[\"y\"]" ),
         "refresh: files are read again", l.text );

  // A refresh during a scan restarts it.
  auto const gate = std::make_shared<Gate>( "walk", "keep" );
  Server server2( f.zorba, gated( t.root, gate ), std::cerr );
  check( gate->wait_reached(), "the walker reaches keep" );
  l = list( server2, Json::object( { { "refresh", true } } ) );
  check( l.state() == "running", "refresh during a scan: running", l.text );
  gate->release();
  l = wait_complete( server2 );
  check( l.found() == "2" && l.sampled() == "2", "the restarted scan completes",
         l.text );
}

void test_symlink( Fixture const &f ) {
  Tree t;
  t.write( "real/a.csv", "x\n1\n" );
  t.write( "swap.csv", "x\n1\n" );
  t.outside( "secret.csv", "SCAN-OUTSIDE\n1\n" );
  stdfs::create_directories( t.base + "/outside/dir" );
  t.outside( "dir/inner.csv", "SCAN-OUTSIDE\n1\n" );
  stdfs::create_symlink( t.base + "/outside/secret.csv", t.root + "/link.csv" );
  stdfs::create_symlink( t.base + "/outside/dir", t.root + "/linkdir" );
  stdfs::create_symlink( t.root + "/real/a.csv", t.root + "/inlink.csv" );
  stdfs::create_symlink( t.root + "/real", t.root + "/inlinkdir" );

  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  Listing l( wait_complete( server ) );
  check( l.found() == "2", "links are not walked or counted", l.text );
  l = list( server );
  check( !contains( l.text, "link" ) && !contains( l.text, "SCAN-OUTSIDE" ),
         "links are not listed", l.text );

  for ( char const *const p : { "link.csv", "linkdir", "linkdir/inner.csv",
                                "inlink.csv", "inlinkdir", "inlinkdir/a.csv" } ) {
    l = list_path( server, p );
    check( l.is_error && l.error_code() == "not-found" && l.has_scan() &&
           !contains( l.text, "SCAN-OUTSIDE" ) && l.files.empty(),
           std::string( "path through a link refused: " ) + p, l.text );
  }

  // A file replaced by a link after it was scanned.
  check( !list_path( server, "swap.csv" ).is_error, "swap.csv is a file" );
  stdfs::remove( t.root + "/swap.csv" );
  stdfs::create_symlink( t.base + "/outside/secret.csv", t.root + "/swap.csv" );
  l = list_path( server, "swap.csv" );
  check( l.is_error && !contains( l.text, "SCAN-OUTSIDE" ),
         "a file replaced by a link is refused", l.text );
  l = list( server );
  check( !l.lists( "swap.csv" ), "and it leaves the listing", l.text );

  // A directory replaced by a link after it was scanned.
  stdfs::rename( t.root + "/real", t.root + "/real.old" );
  stdfs::create_symlink( t.base + "/outside/dir", t.root + "/real" );
  l = list( server, Json::object( { { "refresh", true } } ) );
  l = wait_complete( server );
  l = list( server );
  check( !contains( l.text, "inner" ) && !contains( l.text, "SCAN-OUTSIDE" ),
         "a directory replaced by a link is not walked", l.text );
}

void test_path( Fixture const &f ) {
  Tree t;
  t.write( "a/b.csv", "x\n1\n" );
  t.write( ".hidden/s.csv", "x\n1\n" );
  t.write( "a/.h.csv", "x\n1\n" );
  t.outside( "o.csv", "SCAN-OUTSIDE\n" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );

  for ( std::string const &p : std::vector<std::string>{
          "..", "../outside/o.csv", "a/../../outside/o.csv", "a/..",
          t.base + "/outside/o.csv", "/etc/passwd", ".hidden",
          ".hidden/s.csv", "a/.h.csv" } ) {
    Listing const l( list_path( server, p ) );
    check( l.is_error && l.error_code() == "invalid-path" && l.has_scan() &&
           !contains( l.text, "SCAN-OUTSIDE" ),
           "path refused: " + p, l.text );
  }
  Listing l( list_path( server, "nosuch/thing.csv" ) );
  check( l.is_error && l.error_code() == "not-found", "no such path", l.text );

  // The spellings the directory module accepts.
  l = list_path( server, "./a//b.csv" );
  check( !l.is_error && l.head.find( "path" ) &&
         l.head.find( "path" )->text() == "a/b.csv",
         "empty and . segments are ignored", l.text );
  l = list_path( server, "a/" );
  check( !l.is_error && l.lists( "a/b.csv" ) && l.total() == "1",
         "a subdirectory, with a trailing separator", l.text );
  l = list_path( server, "." );
  check( !l.is_error && l.lists( "a/b.csv" ), "the root as .", l.text );

  // A FIFO neither blocks the walker nor a request, and is not opened: a writer
  // waiting for a reader stays waiting.
  std::string const fifo( t.root + "/pipe.csv" );
  check( mkfifo( fifo.c_str(), 0600 ) == 0, "mkfifo" );
  pid_t const writer = fork();
  if ( writer == 0 ) {
    int const fd = open( fifo.c_str(), O_WRONLY );
    _exit( fd >= 0 ? 0 : 1 );
  }
  check( writer > 0, "start the writer" );
  list( server, Json::object( { { "refresh", true } } ) );
  l = wait_complete( server );
  check( l.found() == "1", "a FIFO is not listed", l.text );
  l = list_path( server, "pipe.csv" );
  check( l.is_error && l.error_code() == "not-found", "a FIFO is not read",
         l.text );
  list( server );
  std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
  int writer_status = 0;
  check( waitpid( writer, &writer_status, WNOHANG ) == 0,
         "the writer of the FIFO is still waiting" );
  check( kill( writer, SIGKILL ) == 0 &&
         waitpid( writer, &writer_status, 0 ) == writer, "end the writer" );

  // Arguments.
  Json const r1( call( server, "list_documents", Json::object( { { "limit", "x" } } ) ) );
  check( contains( r1.dump(), "invalid-arguments" ), "limit must be a number",
         r1.dump() );
  Json const r2( call( server, "list_documents", Json::object( { { "limit", -1 } } ) ) );
  check( contains( r2.dump(), "invalid-arguments" ), "limit must not be negative",
         r2.dump() );
  Json const r3( call( server, "list_documents", Json::object( { { "refresh", 1 } } ) ) );
  check( contains( r3.dump(), "invalid-arguments" ), "refresh must be a boolean",
         r3.dump() );
  Json const r4( call( server, "list_documents", Json::object( { { "path", 1 } } ) ) );
  check( contains( r4.dump(), "invalid-arguments" ), "path must be a string",
         r4.dump() );
  Json const r5( call( server, "list_documents", Json::object( { { "bogus", 1 } } ) ) );
  check( contains( r5.dump(), "invalid-arguments" ) && contains( r5.dump(), "bogus" ),
         "an undeclared argument is refused", r5.dump() );
}

void test_malformed( Fixture const &f ) {
  Tree t;
  t.write( "bad.json", "{\"a\":}" );
  t.write( "cut.json", "{\"a\":[1,2" );
  t.write( "badutf8.json", "{\"a\":\"\xC3\x28\"}" );
  t.write( "deep.json", std::string( 500000, '[' ) );
  t.write( "bad.xml", "<a><b></a>" );
  t.write( "deep.xml", [] {
    std::string s;
    for ( int i = 0; i < 100000; ++i )
      s += "<a>";
    return s;
  }() );
  t.write( "empty.xml", "" );
  t.write( "open.csv", "a,b\n1,\"oops\n" );
  t.write( "wide.csv", "a,b\n1,2,3\n" );
  t.write( "bin.csv", std::string( "a,b\n\0\0\0\n", 8 ) );
  t.write( "bin.html", std::string( "\0\0\0\0", 4 ) );
  t.write( "empty.html", "" );
  t.write( "open.html", "<table><tr><th>A" );
  // Not every file system takes a name that is not UTF-8.
  t.write( std::string( "name\xFF.csv" ), "a\n1\n" );
  std::size_t files = 0;
  for ( auto const &entry : stdfs::directory_iterator( t.root ) ) {
    ( void )entry;
    ++files;
  }

  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  Listing l( wait_complete( server ) );
  check( l.found() == std::to_string( files ) &&
         l.sampled() == std::to_string( files ),
         "malformed files are sampled and the scan completes", l.text );

  for ( char const *const name : { "bad.json", "cut.json", "badutf8.json",
                                   "deep.json", "bad.xml", "deep.xml",
                                   "empty.xml", "open.csv", "wide.csv",
                                   "bin.csv", "bin.html", "empty.html" } ) {
    l = list_path( server, name );
    std::string const p( dump_of( l.preview() ) );
    check( !l.is_error && contains( p, "\"error\":" ),
           std::string( name ) + ": an error in the preview", l.text );
    check( l.text.size() < 4096, std::string( name ) + ": a short answer", l.text );
  }
  l = list_path( server, "open.html" );
  check( contains( dump_of( l.preview() ), "\"headers\":[[\"A\"]]" ),
         "html is parsed leniently", l.text );

  // Everything above came back as JSON, and so does a listing with a name
  // that is not UTF-8.
  l = list( server, Json::object( { { "limit", 100 } } ) );
  check( l.files.size() == files, "the listing has all files", l.text );
}

// A file nested 100000 levels deep is an error in its preview, sampled or
// asked for, and the server goes on.
void test_deep( Fixture const &f ) {
  constexpr int levels = 100000;
  auto repeat = []( std::string const &part ) {
    std::string s;
    s.reserve( part.size() * static_cast<std::size_t>( levels ) );
    for ( int i = 0; i < levels; ++i )
      s += part;
    return s;
  };
  Tree t;
  t.write( "array.json", repeat( "[" ) + repeat( "]" ) );
  t.write( "object.json", repeat( "{\"a\":" ) + "1" + repeat( "}" ) );
  t.write( "closed.xml", repeat( "<a>" ) + repeat( "</a>" ) );
  t.write( "open.xml", repeat( "<a>" ) );
  t.write( "closed.html", repeat( "<i>" ) + repeat( "</i>" ) );
  t.write( "open.html", repeat( "<div>" ) );
  t.write( "fine.csv", "x\n1\n" );
  std::vector<char const*> const deep{ "array.json", "object.json", "closed.xml",
                                       "open.xml", "closed.html", "open.html" };

  auto check_deep = [&deep]( Server &server, std::string const &how ) {
    for ( char const *const name : deep ) {
      Listing const l( list_path( server, name ) );
      check( !l.is_error && contains( dump_of( l.preview() ), "\"error\":" ),
             how + ": " + name + " reports the depth in its preview", l.text );
      check( l.text.size() < 4096, how + ": " + name + ": a short answer",
             l.text.substr( 0, 400 ) );
    }
  };
  {
    // Sampled in the background.
    ServerConfig config;
    config.root = t.root;
    Server server( f.zorba, config, std::cerr );
    Listing const l( wait_complete( server ) );
    check( l.found() == "7" && l.sampled() == "7",
           "the sampler gets through the deep files", l.text );
    check_deep( server, "sampled" );
    check( contains( list_path( server, "fine.csv" ).text, "\"columns\":[\"x\"]" ),
           "the server goes on" );
  }
  {
    // Asked for before the walker has found them.
    auto const gate = std::make_shared<Gate>( "walk", "" );
    Server server( f.zorba, gated( t.root, gate ), std::cerr );
    check( gate->wait_reached(), "the walker is held" );
    check_deep( server, "explored" );
    gate->release();
    wait_complete( server );
  }
}

std::atomic<int> entity_loads( 0 );

xmlParserInputPtr counting_loader( char const*, char const*, xmlParserCtxtPtr ) {
  ++entity_loads;
  return nullptr;
}

void test_xxe( Fixture const &f ) {
  Tree t;
  t.outside( "secret.txt", "SCAN-OUTSIDE" );
  t.outside( "secret.dtd", "<!ENTITY s \"SCAN-OUTSIDE\">"
                           "<!ATTLIST r d CDATA \"SCAN-OUTSIDE\">" );
  std::string const secret( "file://" + t.base + "/outside/secret.txt" );
  std::string const dtd( "file://" + t.base + "/outside/secret.dtd" );
  t.write( "general.xml", "<!DOCTYPE r [<!ENTITY s SYSTEM \"" + secret +
                          "\">]><r><v>&s;</v></r>" );
  t.write( "subset.xml", "<!DOCTYPE r SYSTEM \"" + dtd + "\"><r><v>&s;</v></r>" );
  t.write( "param.xml", "<!DOCTYPE r [<!ENTITY % p SYSTEM \"" + dtd +
                        "\"> %p;]><r><v>x</v></r>" );
  t.write( "net.xml", "<!DOCTYPE r SYSTEM \"http://127.0.0.1:9/x.dtd\"><r/>" );
  t.write( "internal.xml", "<!DOCTYPE r [<!ENTITY e \"EXPANDED\">]>"
                           "<r><v>&e;</v></r>" );
  t.write( "laughs.xml",
    "<!DOCTYPE r [<!ENTITY a \"aaaaaaaaaa\">"
    "<!ENTITY b \"&a;&a;&a;&a;&a;&a;&a;&a;&a;&a;\">"
    "<!ENTITY c \"&b;&b;&b;&b;&b;&b;&b;&b;&b;&b;\">"
    "<!ENTITY d \"&c;&c;&c;&c;&c;&c;&c;&c;&c;&c;\">"
    "<!ENTITY e \"&d;&d;&d;&d;&d;&d;&d;&d;&d;&d;\">"
    "<!ENTITY f \"&e;&e;&e;&e;&e;&e;&e;&e;&e;&e;\">"
    "<!ENTITY g \"&f;&f;&f;&f;&f;&f;&f;&f;&f;&f;\">"
    "<!ENTITY h \"&g;&g;&g;&g;&g;&g;&g;&g;&g;&g;\">"
    "]><r><v>&h;</v></r>" );

  xmlExternalEntityLoader const previous = xmlGetExternalEntityLoader();
  xmlSetExternalEntityLoader( counting_loader );
  {
    ServerConfig config;
    config.root = t.root;
    Server server( f.zorba, config, std::cerr );
    wait_complete( server );
    for ( char const *const name : { "general.xml", "subset.xml", "param.xml",
                                     "net.xml", "internal.xml", "laughs.xml" } ) {
      Listing const l( list_path( server, name ) );
      check( !l.is_error && l.preview() && !contains( l.text, "SCAN-OUTSIDE" ) &&
             !contains( l.text, "EXPANDED" ) && !contains( l.text, "aaaaaaaaaa" ),
             std::string( name ) + ": nothing read or expanded", l.text );
      check( l.text.size() < 4096, std::string( name ) + ": a short answer", l.text );
    }
    check( entity_loads == 0, "the entity loader is never called",
           std::to_string( entity_loads.load() ) );
  }
  xmlSetExternalEntityLoader( previous );
}

void test_limit( Fixture const &f ) {
  Tree t;
  for ( int i = 100; i < 400; ++i )
    t.write( "d" + std::to_string( i % 3 ) + "/f" + std::to_string( i ) + ".csv",
             "x\n1\n" );
  {
    ServerConfig config;
    config.root = t.root;
    Server server( f.zorba, config, std::cerr );
    wait_complete( server );

    Listing l( list( server ) );
    check( l.total() == "300" && l.shown() == "100" && l.files.size() == 100,
           "the default limit, with the total", l.text.substr( 0, 400 ) );
    check( contains( dump_of( l.head.find( "dirs" ) ),
                     "{\"path\":\"d0\",\"files\":100}" ),
           "an answer cut by the limit has the subdirectories",
           l.text.substr( 0, 400 ) );

    l = list( server, Json::object( { { "limit", 10 } } ) );
    check( l.total() == "300" && l.shown() == "10" && l.files.size() == 10,
           "limit 10", l.text.substr( 0, 400 ) );
    check( l.files.front().find( "path" )->text() == "d0/f102.csv",
           "sorted by path", l.text.substr( 0, 400 ) );

    l = list( server, Json::object( { { "limit", 0 } } ) );
    check( l.total() == "300" && l.shown() == "0" && l.files.empty(),
           "limit 0: only the totals", l.text );

    l = list( server, Json::object( { { "path", "d1" }, { "limit", 1000 } } ) );
    check( l.total() == "100" && l.shown() == "100" && !l.head.find( "dirs" ),
           "a subdirectory; nothing cut, so no dirs", l.text.substr( 0, 400 ) );
  }
  {
    ServerConfig config;
    config.root = t.root;
    config.max_output = 1024;
    Server server( f.zorba, config, std::cerr );
    wait_complete( server );
    Listing const l( list( server, Json::object( { { "limit", 1000 } } ) ) );
    check( l.text.size() <= 1024 && l.total() == "300" &&
           l.shown() == std::to_string( l.files.size() ) && l.files.size() < 300,
           "--max-output also bounds the listing", l.text );
  }
}

// A tree of \a n files, all with a preview of the same size, so that each
// takes the same memory.
void write_equal_files( Tree const &t, int n ) {
  for ( int i = 0; i < n; ++i )
    t.write( file_name( i ), "x,y\n1,2\n" );
}

// Counts what a request had to read (stage "explore") and holds nothing.
class ExploreCounter {
public:
  void hook( char const *stage, std::string const &path ) {
    if ( std::string( stage ) != "explore" )
      return;
    std::lock_guard<std::mutex> lock( theMutex );
    ++theCounts[ path ];
  }
  int count( std::string const &path ) {
    std::lock_guard<std::mutex> lock( theMutex );
    return theCounts[ path ];
  }
private:
  std::mutex theMutex;
  std::map<std::string,int> theCounts;
};

ServerConfig limited( Tree const &t, std::size_t bytes,
                      std::shared_ptr<ExploreCounter> const &counter ) {
  ServerConfig config;
  config.root = t.root;
  config.scan_limits.max_memory = bytes;
  config.scan_hook = [counter]( char const *stage, std::string const &path ) {
    counter->hook( stage, path );
  };
  return config;
}

void test_memory( Fixture const &f ) {
  int const files = 40;
  Tree t;
  write_equal_files( t, files );

  // What all of it takes, with no limit in the way.
  std::size_t all = 0;
  {
    ServerConfig config;
    config.root = t.root;
    Server server( f.zorba, config, std::cerr );
    wait_complete( server );
    all = server.scan_cache_bytes();
    check( all > 0, "the cache accounts for what it holds", std::to_string( all ) );
    std::cout << "accounted for " << files << " files: " << all << " bytes"
              << std::endl;
  }

  // The scanner stops at the limit: the index fits, the previews do not.
  std::size_t const limit = all * 3 / 4;
  auto const counter = std::make_shared<ExploreCounter>();
  Server server( f.zorba, limited( t, limit, counter ), std::cerr );
  Listing l( wait_settled( server ) );
  check( l.state() == "memory_limit", "the scanner stops at the limit", l.text );
  check( l.found() == std::to_string( files ) &&
         std::stoi( l.sampled() ) < files && std::stoi( l.sampled() ) > 1,
         "all files found, some sampled", l.text );
  check( server.scan_cache_bytes() <= limit, "under the limit",
         std::to_string( server.scan_cache_bytes() ) + " of " +
         std::to_string( limit ) );

  // It never evicts to go on: nothing changes by waiting.
  std::string const sampled( l.sampled() );
  l = list( server );
  check( l.sampled() == sampled, "the sampler does not go on", l.text );
  check( !contains( l.text, "\"incomplete\"" ), "the listing is whole", l.text );

  // Requests explore and evict. After this the cache holds the last ones.
  for ( int i = 0; i < files; ++i ) {
    Listing const one( list_path( server, file_name( i ) ) );
    check( one.preview() && contains( one.preview()->dump(), "\"columns\":[\"x\",\"y\"]" ),
           "a request gets its preview: " + file_name( i ), one.text );
    check( server.scan_cache_bytes() <= limit, "under the limit after a request",
           std::to_string( server.scan_cache_bytes() ) );
  }
  l = list( server, Json::object( { { "limit", 0 } } ) );
  int const held = std::stoi( l.sampled() );
  check( held > 2 && held < files, "some previews are held", l.text );
  check( l.state() == "memory_limit", "still at the limit", l.text );

  // The coldest two now; using the coldest makes the next one the coldest.
  std::string const coldest( file_name( files - held ) );
  std::string const next( file_name( files - held + 1 ) );
  int before = counter->count( coldest );
  list_path( server, coldest );
  check( counter->count( coldest ) == before,
         "a held preview is not read again", coldest );

  std::string const evicted( file_name( 0 ) );
  before = counter->count( evicted );
  list_path( server, evicted );
  check( counter->count( evicted ) == before + 1,
         "a preview that was evicted is read again", evicted );

  before = counter->count( coldest );
  list_path( server, coldest );
  check( counter->count( coldest ) == before,
         "the recently used preview stays", coldest );
  before = counter->count( next );
  list_path( server, next );
  check( counter->count( next ) == before + 1,
         "the coldest preview was evicted", next );
  check( server.scan_cache_bytes() <= limit, "under the limit at the end",
         std::to_string( server.scan_cache_bytes() ) );

  // refresh starts over.
  for ( int i = 5; i < files; ++i )
    stdfs::remove( t.root + "/" + file_name( i ) );
  list( server, Json::object( { { "refresh", true } } ) );
  l = wait_settled( server );
  check( l.state() == "complete" && l.found() == "5" && l.sampled() == "5",
         "refresh resets the state", l.text );
  check( server.scan_cache_bytes() <= limit, "under the limit after refresh",
         std::to_string( server.scan_cache_bytes() ) );
}

// A preview that cannot fit is answered and not kept.
void test_memory_big_preview( Fixture const &f ) {
  Tree t;
  std::string wide( "{" );
  for ( int i = 0; i < 32; ++i )
    wide += std::string( i ? "," : "" ) + "\"key" + std::to_string( 100 + i ) +
      "\":\"" + std::string( 30, 'v' ) + "\"";
  t.write( "wide.json", wide + "}" );
  t.write( "a.csv", "x\n1\n" );
  std::size_t const limit = 2048;
  auto const counter = std::make_shared<ExploreCounter>();
  Server server( f.zorba, limited( t, limit, counter ), std::cerr );
  Listing l( wait_settled( server ) );
  check( l.state() == "memory_limit" && l.found() == "2",
         "the preview stops the scanner", l.text );
  for ( int i = 1; i <= 2; ++i ) {
    l = list_path( server, "wide.json" );
    check( !l.is_error && contains( dump_of( l.preview() ), "key131" ),
           "the preview is answered", l.text );
    check( counter->count( "wide.json" ) == i, "it is not kept",
           std::to_string( counter->count( "wide.json" ) ) );
    check( server.scan_cache_bytes() <= limit, "under the limit",
           std::to_string( server.scan_cache_bytes() ) );
  }
  l = list_path( server, "a.csv" );
  check( contains( dump_of( l.preview() ), "\"columns\":[\"x\"]" ),
         "a preview that fits is still answered", l.text );
  check( counter->count( "a.csv" ) >= 0 && server.scan_cache_bytes() <= limit,
         "under the limit", std::to_string( server.scan_cache_bytes() ) );
}

// The index itself does not fit: the walker stops, and requests still work.
void test_memory_walker( Fixture const &f ) {
  Tree t;
  for ( int i = 0; i < 50; ++i )
    t.write( "d" + std::to_string( i ) + "/" + file_name( i ), "x\n1\n" );
  t.write( "d49/z.csv", "z\n1\n" );
  std::size_t const limit = 2048;
  auto const counter = std::make_shared<ExploreCounter>();
  Server server( f.zorba, limited( t, limit, counter ), std::cerr );
  Listing l( wait_settled( server ) );
  check( l.state() == "memory_limit" && std::stoi( l.found() ) < 51,
         "the walker stops at the limit; files_found is a lower bound", l.text );
  check( server.scan_cache_bytes() <= limit, "under the limit",
         std::to_string( server.scan_cache_bytes() ) );
  l = list( server );
  check( contains( l.text, "\"incomplete\":true" ),
         "the listing says it may be incomplete", l.text );

  l = list_path( server, "d49/z.csv" );
  check( !l.is_error && contains( dump_of( l.preview() ), "\"columns\":[\"z\"]" ),
         "a file below still works, by path", l.text );
  l = list_path( server, "d49" );
  check( !l.is_error && l.lists( "d49/z.csv" ) && l.lists( "d49/" + file_name( 49 ) ) &&
         contains( l.text, "\"incomplete\":true" ),
         "a directory below still works, by path", l.text );
  l = list_path( server, "d49/nosuch.csv" );
  check( l.is_error && l.error_code() == "not-found", "a missing file is missing",
         l.text );
  check( server.scan_cache_bytes() <= limit, "under the limit at the end",
         std::to_string( server.scan_cache_bytes() ) );
}

// Eviction while the threads work, for ThreadSanitizer.
void test_memory_stress( Fixture const &f ) {
  int const files = 600;
  Tree t;
  for ( int i = 0; i < files; ++i )
    t.write( "d" + std::to_string( i % 9 ) + "/" + file_name( i ), "x,y\n1,2\n" );
  std::size_t const limit = 64 * 1024;
  auto const counter = std::make_shared<ExploreCounter>();
  Server server( f.zorba, limited( t, limit, counter ), std::cerr );
  for ( int i = 0; i < 400; ++i ) {
    int const n = ( i * 37 ) % files;
    Listing const l( list_path( server, "d" + std::to_string( n % 9 ) + "/" +
                                file_name( n ) ) );
    check( !l.is_error, "request " + std::to_string( i ), l.text );
    if ( i % 50 == 25 )
      list( server, Json::object( { { "refresh", true }, { "limit", 3 } } ) );
    if ( i % 10 == 0 )
      list( server, Json::object( { { "limit", 3 } } ) );
    check( server.scan_cache_bytes() <= limit, "under the limit",
           std::to_string( server.scan_cache_bytes() ) );
  }
  wait_settled( server );
}

// Under --max-memory threads may not start: the server works as with --no-scan.
// In a child process, whose address space is limited to just over what it uses,
// which leaves no room for a thread's stack. The limit is Linux only, and
// sanitizer runtimes cannot run under one.
void test_thread_limit( Fixture const &f ) {
#if defined(__linux__) && !defined(ZORBA_TEST_SANITIZER)
  Tree t;
  t.write( "a.csv", "x\n1\n" );
  pid_t const pid = fork();
  if ( pid == 0 ) {
    std::ifstream statm( "/proc/self/statm" );
    unsigned long pages = 0;
    statm >> pages;
    struct rlimit lim;
    if ( pages == 0 || getrlimit( RLIMIT_AS, &lim ) != 0 )
      _exit( 3 );
    lim.rlim_cur = pages * static_cast<unsigned long>( sysconf( _SC_PAGESIZE ) ) +
      1024 * 1024;
    if ( setrlimit( RLIMIT_AS, &lim ) != 0 )
      _exit( 3 );
    int code = 1;
    try {
      ServerConfig config;
      config.root = t.root;
      Server server( f.zorba, config, std::cerr );
      std::string const tools( server.handle( request( 1, "tools/list" ) ) );
      code = contains( tools, "\"list_documents\"" ) &&
             !contains( tools, "\"refresh\"" ) ? 0 : 2;
    }
    catch ( ... ) {
      code = 4;
    }
    _exit( code );
  }
  check( pid > 0, "start the child" );
  int status = -1;
  check( waitpid( pid, &status, 0 ) == pid, "wait for the child" );
  check( WIFEXITED( status ) && WEXITSTATUS( status ) == 0,
         "no thread can start: the server works without the scan",
         std::to_string( status ) );
#else
  ( void )f;
#endif
}

// An allocation that fails in a scan thread ends the scan, not the server.
void test_thread_failure( Fixture const &f ) {
  Tree t;
  t.write( "a.csv", "x\n1\n" );
  t.write( "sub/b.csv", "y\n1\n" );
  auto const walk_failed = std::make_shared<std::atomic<bool> >( false );
  auto const sample_failed = std::make_shared<std::atomic<bool> >( false );
  ServerConfig config;
  config.root = t.root;
  config.scan_hook = [walk_failed, sample_failed](
      char const *stage, std::string const &path ) {
    std::string const s( stage );
    if ( ( s == "walk" && path == "sub" && !walk_failed->exchange( true ) ) ||
         ( s == "sample" && !sample_failed->exchange( true ) ) )
      throw std::bad_alloc();
  };
  Server server( f.zorba, config, std::cerr );
  Listing l( wait_settled( server ) );
  check( walk_failed->load() && l.state() == "memory_limit",
         "the scan ends at its limit", l.text );
  l = list( server );
  check( contains( l.text, "\"incomplete\":true" ),
         "the listing says it may be incomplete", l.text );
  l = list_path( server, "sub/b.csv" );
  check( contains( dump_of( l.preview() ), "\"columns\":[\"y\"]" ),
         "a request still gets its preview", l.text );
  check( server.stop_scan(), "stop joins both threads" );
}

void test_no_scan( Fixture const &f ) {
  Tree t;
  t.write( "a.csv", "x\n1\n" );
  ServerConfig config;
  config.root = t.root;
  config.scan = false;
  // A scanner must not run: this would stop it, and the test would hang.
  auto const never = std::make_shared<Gate>( "walk", "*" );
  config.scan_hook = [never]( char const *stage, std::string const &path ) {
    never->hook( stage, path );
  };
  Server server( f.zorba, config, std::cerr );

  std::string const tools( server.handle( request( 1, "tools/list" ) ) );
  check( !contains( tools, "\"refresh\"" ) && !contains( tools, "\"limit\"" ),
         "no-scan: list_documents takes no arguments", tools );
  Json const r( call( server, "list_documents", Json::object() ) );
  std::string const d( r.dump() );
  check( contains( d, "\\\"path\\\" : \\\"a.csv\\\"" ) &&
         contains( d, "\\\"modified\\\"" ) && !contains( d, "\\\"scan\\\"" ),
         "no-scan: the listing of dir:list", d );
  Json const r2( call( server, "list_documents",
                       Json::object( { { "path", "a.csv" } } ) ) );
  check( contains( r2.dump(), "invalid-arguments" ),
         "no-scan: path is refused", r2.dump() );
  never->release();
}

// While the scanner stands still, the protocol works and queries see the
// real files.
void test_handshake( Fixture const &f ) {
  Tree t;
  t.write( "a.csv", "x\n1\n" );
  t.write( "sub/b.csv", "x\n1\n" );
  auto const gate = std::make_shared<Gate>( "walk", "" );
  Server server( f.zorba, gated( t.root, gate ), std::cerr );
  check( gate->wait_reached(), "the walker reaches the root" );

  std::string const init( server.handle( Json::object( {
    { "jsonrpc", "2.0" }, { "id", 1 }, { "method", "initialize" },
    { "params", Json::object( {
        { "protocolVersion", "2025-11-25" }, { "capabilities", Json::object() },
        { "clientInfo", Json::object( { { "name", "t" }, { "version", "1" } } ) }
      } ) } } ).dump() ) );
  check( contains( init, "\"result\"" ), "initialize with the walker stopped", init );
  std::string const disc( server.handle( request( 2, "server/discover" ) ) );
  check( contains( disc, "\"supportedVersions\"" ),
         "server/discover with the walker stopped", disc );
  std::string const tools( server.handle( request( 3, "tools/list" ) ) );
  check( contains( tools, "\"list_documents\"" ) && contains( tools, "\"refresh\"" ) &&
         contains( tools, "\"limit\"" ) && contains( tools, "\"path\"" ),
         "tools/list with the walker stopped, with the new arguments", tools );

  Json const r( call( server, "run_query", Json::object( { { "query",
    "import module namespace dir = \"http://zorba.io/modules/directory\";\n"
    "count(dir:list($dir:root))" } } ) ) );
  check( contains( r.dump(), "\"text\":\"2\"" ),
         "queries read the real files, whatever the scan knows", r.dump() );
  gate->release();
  wait_complete( server );
}

// The server stops without waiting for a file that does not come back.
void test_shutdown( Fixture const &f ) {
  Tree t;
  t.write( "a.csv", "x\n1\n" );
  auto const gate = std::make_shared<Gate>( "sample", "*" );
  {
    Server server( f.zorba, gated( t.root, gate ), std::cerr );
    check( gate->wait_reached(), "the sampler is held at a file" );
    auto const t0 = std::chrono::steady_clock::now();
    check( !server.stop_scan(), "the held thread is left behind" );
    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - t0 ).count();
    check( ms < 10000, "stop does not wait for a held file",
           std::to_string( ms ) + " ms" );
    std::string const init( server.handle( request( 2, "tools/list" ) ) );
    check( contains( init, "\"result\"" ), "the server still answers", init );
  }
  gate->release();
  check( gate->wait_left(), "the held thread ends when the file comes back" );
}

struct Run {
  std::string out;
  int status = -1;
};

// Runs the executable over pipes.
Run run_binary( Fixture const &f, std::vector<std::string> const &args,
                std::string const &input ) {
  Run run;
  int to_child[2], from_child[2];
  if ( pipe( to_child ) != 0 || pipe( from_child ) != 0 ) {
    check( false, "pipe" );
    return run;
  }
  pid_t const pid = fork();
  if ( pid < 0 ) {
    check( false, "fork" );
    return run;
  }
  if ( pid == 0 ) {
    if ( dup2( to_child[0], STDIN_FILENO ) < 0 ||
         dup2( from_child[1], STDOUT_FILENO ) < 0 ||
         close( to_child[0] ) != 0 || close( to_child[1] ) != 0 ||
         close( from_child[0] ) != 0 || close( from_child[1] ) != 0 )
      _exit( 127 );
    std::vector<char*> argv;
    argv.push_back( const_cast<char*>( f.binary.c_str() ) );
    for ( std::string const &a : args )
      argv.push_back( const_cast<char*>( a.c_str() ) );
    argv.push_back( nullptr );
    execv( f.binary.c_str(), argv.data() );
    _exit( 127 );
  }
  check( close( to_child[0] ) == 0 && close( from_child[1] ) == 0, "close" );
  std::size_t written = 0;
  while ( written < input.size() ) {
    ssize_t const n = write( to_child[1], input.data() + written,
                             input.size() - written );
    if ( n <= 0 )
      break;
    written += static_cast<std::size_t>( n );
  }
  check( close( to_child[1] ) == 0, "close the input" );
  char buf[4096];
  for ( ;; ) {
    ssize_t const n = read( from_child[0], buf, sizeof buf );
    if ( n <= 0 )
      break;
    run.out.append( buf, static_cast<std::size_t>( n ) );
  }
  check( close( from_child[0] ) == 0, "close the output" );
  check( waitpid( pid, &run.status, 0 ) == pid, "wait" );
  return run;
}

std::string list_call( int id, Json args = Json::object() ) {
  return request( id, "tools/call", Json::object( {
    { "name", "list_documents" }, { "arguments", std::move( args ) } } ) ) + "\n";
}

void test_binary( Fixture const &f ) {
  Tree t;
  // Enough files that EOF comes during the scan on most machines; the test
  // holds either way, and test_shutdown holds a thread deterministically.
  for ( int i = 0; i < 3000; ++i )
    t.write( "d" + std::to_string( i % 50 ) + "/f" + std::to_string( i ) + ".csv",
             "x,y\n1,2\n" );

  Run run( run_binary( f, { "--root", t.root },
    list_call( 1, Json::object( { { "limit", 2 } } ) ) +
    list_call( 2, Json::object( { { "path", "d0/f0.csv" } } ) ) ) );
  check( WIFEXITED( run.status ) && WEXITSTATUS( run.status ) == 0,
         "EOF during a scan: clean exit", run.out );
  check( contains( run.out, "scan" ) && contains( run.out, "preview" ),
         "the binary scans by default", run.out );
  std::istringstream lines( run.out );
  std::string line;
  int n = 0;
  while ( std::getline( lines, line ) ) {
    parsed( line );
    ++n;
  }
  check( n == 2, "one protocol message per line on stdout", run.out );

  run = run_binary( f, { "--root", t.root, "--max-output", "100000",
                         "--max-memory", "0", "--scan-cache-mb", "8" },
                    list_call( 1, Json::object( { { "limit", 1 } } ) ) );
  check( WIFEXITED( run.status ) && WEXITSTATUS( run.status ) == 0 &&
         contains( run.out, "files_found" ),
         "options with values are read as such", run.out );

  run = run_binary( f, { "--root", t.root, "--no-scan" }, list_call( 1 ) );
  check( WIFEXITED( run.status ) && WEXITSTATUS( run.status ) == 0 &&
         contains( run.out, "modified" ) && !contains( run.out, "files_found" ),
         "--no-scan: today's list_documents", run.out.substr( 0, 400 ) );
}

// The option, over a live session.
void test_memory_option( Fixture const &f ) {
  Tree t;
  // Long names: over 1 MiB of index, whatever a preview takes.
  for ( int i = 0; i < 3000; ++i )
    t.write( "d" + std::to_string( i % 20 ) + "/" + std::string( 200, 'n' ) +
             std::to_string( i ) + ".csv", "x\n1\n" );
  t.write( "d0/here.csv", "x\n1\n" );

  int to_child[2], from_child[2];
  check( pipe( to_child ) == 0 && pipe( from_child ) == 0, "pipe" );
  pid_t const pid = fork();
  if ( pid == 0 ) {
    if ( dup2( to_child[0], STDIN_FILENO ) < 0 ||
         dup2( from_child[1], STDOUT_FILENO ) < 0 ||
         close( to_child[0] ) != 0 || close( to_child[1] ) != 0 ||
         close( from_child[0] ) != 0 || close( from_child[1] ) != 0 )
      _exit( 127 );
    execl( f.binary.c_str(), f.binary.c_str(), "--root", t.root.c_str(),
           "--scan-cache-mb", "1", static_cast<char*>( nullptr ) );
    _exit( 127 );
  }
  check( pid > 0 && close( to_child[0] ) == 0 && close( from_child[1] ) == 0,
         "start zorba-mcp" );
  std::FILE *const in = fdopen( to_child[1], "w" );
  std::FILE *const out = fdopen( from_child[0], "r" );
  char *line = nullptr;
  std::size_t cap = 0;
  auto exchange = [&]( std::string const &message ) {
    if ( std::fputs( message.c_str(), in ) == EOF || std::fflush( in ) == EOF )
      return std::string();
    ssize_t const n = getline( &line, &cap, out );
    return n > 0 ? std::string( line, static_cast<std::size_t>( n ) )
                 : std::string();
  };
  std::string answer;
  for ( int i = 0; i < 30000; ++i ) {
    answer = exchange( list_call( 1, Json::object( { { "limit", 0 } } ) ) );
    if ( !contains( answer, "running" ) )
      break;
    std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
  }
  check( contains( answer, "memory_limit" ), "--scan-cache-mb 1: the limit is hit",
         answer );
  answer = exchange( list_call( 2, Json::object( { { "path", "d0/here.csv" } } ) ) );
  check( contains( answer, "columns" ) && contains( answer, "memory_limit" ),
         "and a file still gets its preview, with the status", answer );
  check( std::fclose( in ) == 0, "close the input" );
  std::free( line );
  while ( std::fgetc( out ) != EOF ) { }
  check( std::fclose( out ) == 0, "close the output" );
  int status = -1;
  check( waitpid( pid, &status, 0 ) == pid && WIFEXITED( status ) &&
         WEXITSTATUS( status ) == 0, "clean exit" );

  for ( char const *const bad : { "0", "abc", "99999999999" } ) {
    Run const run( run_binary( f, { "--root", t.root, "--scan-cache-mb", bad }, "" ) );
    check( WIFEXITED( run.status ) && WEXITSTATUS( run.status ) == 2,
           std::string( "--scan-cache-mb " ) + bad + " is refused" );
  }
}

// Everything at once, for ThreadSanitizer: requests, restarts and queries
// (which also use libxml2) while the threads work.
void test_stress( Fixture const &f ) {
  Tree t;
  for ( int i = 0; i < 300; ++i ) {
    std::string const dir( "d" + std::to_string( i % 7 ) + "/" );
    t.write( dir + "f" + std::to_string( i ) + ".csv", "x,y\n1,2\n" );
    t.write( dir + "f" + std::to_string( i ) + ".json", "{\"a\":[1,2]}" );
    t.write( dir + "f" + std::to_string( i ) + ".xml", "<r><v>1</v></r>" );
    t.write( dir + "f" + std::to_string( i ) + ".html",
             "<html><title>t</title></html>" );
  }
  ServerConfig config;
  config.root = t.root;
  config.timeout = std::chrono::milliseconds( 0 );
  Server server( f.zorba, config, std::cerr );
  for ( int i = 0; i < 40; ++i ) {
    Json args( Json::object( { { "limit", 5 } } ) );
    if ( i % 5 == 4 )
      args.set( "refresh", true );
    check( list( server, args ).has_scan(), "list " + std::to_string( i ) );
    check( !list_path( server, "d1/f1.xml" ).is_error, "file " + std::to_string( i ) );
    check( !list_path( server, "d1" ).is_error, "dir " + std::to_string( i ) );
    if ( i % 10 == 0 ) {
      Json const r( call( server, "run_query", Json::object( { { "query",
        "import module namespace dir = \"http://zorba.io/modules/directory\";\n"
        "count(for $d in dir:documents($dir:root, \"xml\")\n"
        "      where exists($d(\"content\")) return $d)" } } ) ) );
      check( contains( r.dump(), "\"text\":\"300\"" ), "query over the tree",
             r.dump() );
    }
    if ( i % 3 == 0 )
      t.write( "d1/f1.xml", "<r><v>" + std::to_string( i ) + "</v></r>" );
  }
  Listing const l( wait_complete( server ) );
  check( l.found() == "1200" && l.sampled() == "1200", "the scan ends whole",
         l.text );
}

// What list_documents costs in every session.
void test_description( Fixture const &f ) {
  Tree t;
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  Json const r( parsed( server.handle( request( 1, "tools/list" ) ) ) );
  Json const *const res = r.find( "result" );
  Json const *const tools = res ? res->find( "tools" ) : nullptr;
  check( tools && tools->is_array() && !tools->items().empty(), "tools" );
  if ( !tools || tools->items().empty() )
    return;
  std::string const tool( tools->items()[0].dump() );
  std::cout << "list_documents definition: " << tool.size() << " bytes"
            << std::endl;
  check( tool.size() <= 700, "the tool definition stays short", tool );
  check( contains( tool, "recursive" ) && contains( tool, "sorted" ) &&
         contains( tool, "dirs" ) && contains( tool, "incomplete" ) &&
         contains( tool, "memory_limit" ),
         "it says what the listing is and what the terms mean", tool );
}

struct Test {
  char const *name;
  void (*fn)( Fixture const& );
};

Test const tests[] = {
  { "scan_status", test_status },
  { "scan_immediate", test_immediate },
  { "scan_huge_tag", test_huge_tag },
  { "scan_known_dirs", test_known_dirs },
  { "scan_unreadable", test_unreadable },
  { "scan_deep_tree", test_deep_tree },
  { "scan_preview_bound", test_preview_bound },
  { "scan_escaped_names", test_escaped_names },
  { "scan_refill", test_refill },
  { "scan_previews", test_previews },
  { "scan_mtime", test_mtime },
  { "scan_refresh", test_refresh },
  { "scan_symlink", test_symlink },
  { "scan_path", test_path },
  { "scan_malformed", test_malformed },
  { "scan_deep", test_deep },
  { "scan_xxe", test_xxe },
  { "scan_limit", test_limit },
  { "scan_memory", test_memory },
  { "scan_memory_big_preview", test_memory_big_preview },
  { "scan_memory_walker", test_memory_walker },
  { "scan_memory_stress", test_memory_stress },
  { "scan_memory_option", test_memory_option },
  { "scan_thread_limit", test_thread_limit },
  { "scan_thread_failure", test_thread_failure },
  { "scan_no_scan", test_no_scan },
  { "scan_handshake", test_handshake },
  { "scan_shutdown", test_shutdown },
  { "scan_binary", test_binary },
  { "scan_stress", test_stress },
  { "scan_description", test_description }
};

} // namespace

int main( int argc, char const *argv[] ) {
  if ( argc != 3 ) {
    std::cerr << "usage: mcp_scan_tests <test> <zorba-mcp>" << std::endl;
    return 2;
  }
  std::string const name( argv[1] );
  Test const *test = nullptr;
  for ( Test const &t : tests )
    if ( name == t.name )
      test = &t;
  if ( !test ) {
    std::cerr << "mcp_scan_tests: unknown test " << name << std::endl;
    return 2;
  }
  void *const store = StoreManager::getStore();
  Zorba *const zorba = Zorba::getInstance( store );
  {
    Fixture f{ zorba, argv[2] };
    test->fn( f );
  }
  zorba->shutdown();
  StoreManager::shutdownStore( store );
  std::cout << ( failures ? "FAILED" : "PASSED" ) << ": " << name << " ("
            << failures << " failures)" << std::endl;
  return failures ? 1 : 0;
}
/* vim:set et sw=2 ts=2: */
