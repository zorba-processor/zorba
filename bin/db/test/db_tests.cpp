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

// The database behind zorba-mcp, used directly: plain values in, results in a
// sink and structured errors out, no protocol.
//
// Usage: db_tests <test> <tree-dir> <queries-dir>
// where <test> is one of the names in the table at the end.

#include <chrono>
#include <cstddef>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "database.h"
#include "directory_config.h"
#include "scanner.h"

using namespace zorba;
using namespace zorba::db;

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

struct Fixture {
  Zorba *zorba;
  std::string tree;
  std::string queries;
};

// Keeps every write, as a shell printing the results would see them.
class RecordingSink : public ResultSink {
public:
  void write( char const *data, std::size_t size ) override {
    theText.append( data, size );
    ++theWrites;
  }
  void end() override { ++theEnds; }

  std::string const& text() const { return theText; }
  std::size_t writes() const { return theWrites; }
  std::size_t ends() const { return theEnds; }

private:
  std::string theText;
  std::size_t theWrites = 0;
  std::size_t theEnds = 0;
};

class FailingSink : public ResultSink {
public:
  void write( char const*, std::size_t ) override { throw std::bad_alloc(); }
};

DatabaseConfig config_of( Fixture const &f ) {
  DatabaseConfig config;
  config.root = f.tree;
  return config;
}

std::string const dir_import(
  "import module namespace dir = \"http://zorba.io/modules/directory\";\n" );

void test_run_query( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  RecordingSink sink;
  QueryResult const r( db.run( "1 + 1", sink ) );
  check( r.ok, "1 + 1 succeeds", r.error.message );
  check( sink.text() == "2", "the result goes to the sink", sink.text() );
  check( sink.ends() == 1, "end() once after the last write",
         std::to_string( sink.ends() ) );
}

void test_streaming( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  RecordingSink sink;
  QueryResult const r( db.run( "1 to 1000", sink ) );
  check( r.ok, "the query succeeds", r.error.message );
  check( sink.writes() > 1, "the results arrive in several writes",
         std::to_string( sink.writes() ) );
  std::string const &text = sink.text();
  check( text.compare( 0, 2, "1\n" ) == 0 &&
         contains( text, "\n999\n1000" ), "the whole result, in order",
         text.substr( 0, 200 ) );
}

void test_query_error( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  RecordingSink sink;
  QueryResult const r( db.run( "let $x := 1\nreturn $x $x", sink ) );
  check( !r.ok, "a syntax error fails" );
  check( r.error.kind == QueryError::Kind::query, "an error of the query" );
  check( r.error.code == "err:XPST0003", "its code with the prefix",
         r.error.code );
  check( r.error.ns == "http://www.w3.org/2005/xqt-errors", "its namespace",
         r.error.ns );
  check( !r.error.message.empty(), "a message" );
  check( r.error.location.has_value() && r.error.location->line == 2,
         "the line of the error",
         r.error.location ? std::to_string( r.error.location->line ) : "none" );
  check( r.error.location && r.error.location->text == "return $x $x",
         "the text of the failing line",
         r.error.location ? r.error.location->text : "none" );
  check( sink.ends() == 0, "no end() for a query that failed" );
}

void test_sink_failure( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  FailingSink failing;
  QueryResult const r( db.run( "1 + 1", failing ) );
  check( !r.ok && r.error.kind == QueryError::Kind::memory_limit,
         "a sink out of memory makes a memory_limit error", r.error.message );
  RecordingSink sink;
  QueryResult const next( db.run( "2 + 2", sink ) );
  check( next.ok && sink.text() == "4", "the next query works", sink.text() );
}

void test_predefined( Fixture const &f ) {
  DatabaseConfig config( config_of( f ) );
  config.queries_dir = f.queries;
  std::ostringstream log;
  Database db( f.zorba, config, log );
  std::string names;
  for ( PredefinedQuery const &q : db.predefined() )
    names += q.name + " ";
  check( names == "greet ordered scale ", "the queries, sorted by file name",
         names );
  PredefinedQuery const *const greet = db.find_predefined( "greet" );
  check( greet != nullptr, "found by name" );
  if ( !greet )
    return;
  check( greet->description == "Greets a person; the greeting is optional.",
         "the description is the leading comment", greet->description );
  check( greet->params.size() == 2 &&
         greet->params[0].name == "name" && greet->params[0].required &&
         greet->params[1].name == "greeting" && !greet->params[1].required,
         "the external variables, required unless they have a default" );
  check( db.find_predefined( "nothing" ) == nullptr, "an unknown name" );

  RecordingSink sink;
  QueryResult const r( db.run_predefined( "greet", { { "name", "world" } },
                                          sink ) );
  check( r.ok && sink.text() == "\"hello, world\"", "runs with its variables",
         r.ok ? sink.text() : r.error.message );
}

void test_list_files( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  RecordingSink sink;
  QueryResult const r( db.list_files( sink ) );
  check( r.ok, "the listing succeeds", r.error.message );
  check( contains( sink.text(), "customers.csv" ) &&
         contains( sink.text(), "payments/late.json" ),
         "the files under the root", sink.text() );
  check( !contains( sink.text(), "secret" ), "no hidden files", sink.text() );
}

void test_policy( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  // Both files exist; only the one under the root may be read.
  RecordingSink inside;
  QueryResult const ok( db.run( dir_import +
    "name(doc(concat($dir:root, \"/orders/2026/q1.xml\"))/*)", inside ) );
  check( ok.ok && !inside.text().empty(), "a file under the root",
         ok.ok ? inside.text() : ok.error.message );
  RecordingSink outside;
  QueryResult const denied( db.run( dir_import +
    "doc(concat($dir:root, \"/../outside/secret.xml\"))", outside ) );
  check( !denied.ok && denied.error.kind == QueryError::Kind::query &&
         contains( denied.error.code, "FODC0002" ) &&
         contains( denied.error.message, "access denied by security policy" ),
         "a file outside the root: denied by the policy",
         denied.error.code + " " + denied.error.message );
  check( !contains( outside.text(), "SECRET" ), "nothing of it in the sink",
         outside.text() );
}

// The scan needs no Database: it never touches the engine.
void test_scan_listing( Fixture const &f ) {
  Scanner scanner( f.tree, ScanLimits(), ScanHook() );
  std::chrono::milliseconds const wait( std::chrono::seconds( 10 ) );
  bool is_error = true;
  std::string const all( scanner.answer( ListRequest(), 1 << 20, wait,
                                         &is_error ) );
  check( !is_error && contains( all, "\"scan\"" ) &&
         contains( all, "customers.csv" ), "the files, after a status line",
         all );
  ListRequest one;
  one.path = "customers.csv";
  std::string const file( scanner.answer( one, 1 << 20, wait, &is_error ) );
  check( !is_error && contains( file, "customers.csv" ), "one file", file );
  ListRequest bad;
  bad.path = "../outside";
  std::string const refused( scanner.answer( bad, 1 << 20, wait, &is_error ) );
  check( is_error && contains( refused, "invalid-path" ),
         "a path out of the root", refused );
  std::string const status( scanner.status() );
  check( status.compare( 0, 8, "\"scan\":{" ) == 0 && status.back() == '}',
         "the status as a member", status );
  check( scanner.stop(), "the scan stops" );
}

void test_document_cache( Fixture const &f ) {
  std::ostringstream log;
  Database db( f.zorba, config_of( f ), log );
  for ( int i = 0; i < 3; ++i ) {
    RecordingSink sink;
    QueryResult const r( db.run( dir_import +
      "dir:open($dir:root, \"customers.csv\")(\"path\")", sink ) );
    check( r.ok, "read " + std::to_string( i + 1 ), r.error.message );
  }
  check( db.cache_stats().hits >= 1, "the third read is a hit",
         db.cache_stats_line() );
}

struct Test {
  char const *name;
  void (*fn)( Fixture const& );
};

Test const tests[] = {
  { "run_query", test_run_query },
  { "streaming", test_streaming },
  { "query_error", test_query_error },
  { "sink_failure", test_sink_failure },
  { "predefined", test_predefined },
  { "list_files", test_list_files },
  { "policy", test_policy },
  { "scan_listing", test_scan_listing },
  { "document_cache", test_document_cache }
};

} // namespace

int main( int argc, char const *argv[] ) {
  if ( argc != 4 ) {
    std::cerr << "usage: db_tests <test> <tree-dir> <queries-dir>" << std::endl;
    return 2;
  }
  std::string const name( argv[1] );
  Test const *test = nullptr;
  for ( Test const &t : tests )
    if ( name == t.name )
      test = &t;
  if ( !test ) {
    std::cerr << "db_tests: unknown test " << name << std::endl;
    return 2;
  }

  void *const store = StoreManager::getStore();
  Zorba *const zorba = Zorba::getInstance( store );
  {
    Fixture f{ zorba, argv[2], argv[3] };
    std::string why;
    if ( !directory_config::normalize( &f.tree, &why ) ||
         !directory_config::normalize( &f.queries, &why ) ) {
      std::cerr << "db_tests: " << why << std::endl;
      return 2;
    }
    test->fn( f );
  }
  zorba->shutdown();
  StoreManager::shutdownStore( store );
  std::cout << ( failures ? "FAILED" : "PASSED" ) << ": " << name << " ("
            << failures << " failures)" << std::endl;
  return failures ? 1 : 0;
}
/* vim:set et sw=2 ts=2: */
