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

// Scripted JSON-RPC exchanges with the zorba-mcp request handler, over the
// directory module's fixture tree.
//
// Usage: mcp_tests <test> <tree-dir> <queries-dir> <join-query-file>
//                  [<zorba-mcp>]
// where <test> is one of the names in the table at the end; the "binary"
// test runs the zorba-mcp executable given as the last argument.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "directory_config.h"
#include "json.h"
#include "mcp_server.h"

using namespace zorba;
using namespace zorba::mcp;

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

struct Fixture {
  Zorba *zorba;
  std::string tree;
  std::string queries;
  std::string join_query;
  std::string binary;
};

std::string read_file( std::string const &path ) {
  std::ifstream in( path.c_str(), std::ios::binary );
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

Json meta() {
  return Json::object( {
    { "io.modelcontextprotocol/protocolVersion", protocol_version },
    { "io.modelcontextprotocol/clientCapabilities", Json::object() },
    { "io.modelcontextprotocol/clientInfo", Json::object( {
        { "name", "mcp_tests" }, { "version", "1" } } ) }
  } );
}

// A request; with modern = true it carries the per-request _meta.
std::string request( Json id, std::string const &method,
                     Json params = Json::object(), bool modern = true ) {
  if ( modern )
    params.set( "_meta", meta() );
  return Json::object( {
    { "jsonrpc", "2.0" }, { "id", id }, { "method", method },
    { "params", params }
  } ).dump();
}

Json roundtrip( Server &server, std::string const &message ) {
  std::string const response( server.handle( message ) );
  Json r;
  std::string why;
  if ( response.empty() )
    return Json();
  if ( !parse_json( response, &r, &why ) ) {
    check( false, "response is JSON", response + " (" + why + ")" );
    return Json();
  }
  if ( response.find( '\n' ) != std::string::npos )
    check( false, "response has no newline", response );
  return r;
}

Json call_tool( Server &server, std::string const &tool, Json args,
                int id = 100 ) {
  return roundtrip( server, request( id, "tools/call", Json::object( {
    { "name", tool }, { "arguments", args } } ) ) );
}

int error_code( Json const &r ) {
  Json const *const e = r.find( "error" );
  Json const *const c = e ? e->find( "code" ) : nullptr;
  return c && c->is_integer() ? std::stoi( c->text() ) : 0;
}

std::string result_text( Json const &r ) {
  Json const *const res = r.find( "result" );
  Json const *const content = res ? res->find( "content" ) : nullptr;
  if ( !content || !content->is_array() || content->items().empty() )
    return std::string();
  Json const *const text = content->items()[0].find( "text" );
  return text && text->is_string() ? text->text() : std::string();
}

bool is_tool_error( Json const &r ) {
  Json const *const res = r.find( "result" );
  Json const *const e = res ? res->find( "isError" ) : nullptr;
  return e && e->is_bool() && e->as_bool();
}

bool contains( std::string const &s, std::string const &part ) {
  return s.find( part ) != std::string::npos;
}

// The error object of a failed tool call (its text is JSON).
Json tool_error( Json const &r ) {
  Json e;
  std::string why;
  if ( !parse_json( result_text( r ), &e, &why ) )
    return Json();
  Json const *const inner = e.find( "error" );
  return inner ? *inner : Json();
}

std::string field( Json const &j, std::string const &name ) {
  Json const *const f = j.find( name );
  return f ? f->text() : std::string();
}

///////////////////////////////////////////////////////////////////////////////

void test_json( Fixture const& ) {
  Json v;
  std::string why;
  check( parse_json( " {\"a\":[1,-0.5e+10,true,null,\"x\\u00e9\\ud83d\\ude00\"]} ",
                     &v, &why ), "parse a valid text", why );
  check( v.dump() == "{\"a\":[1,-0.5e+10,true,null,\"x\xc3\xa9\xf0\x9f\x98\x80\"]}",
         "numbers keep their text; escapes decode to UTF-8", v.dump() );
  check( Json( "a\nb\x01\"" ).dump() == "\"a\\nb\\u0001\\\"\"",
         "control characters are escaped", Json( "a\nb\x01\"" ).dump() );

  char const *const bad[] = {
    "", " ", "{", "}", "[1,]", "[,1]", "{\"a\":}", "{\"a\" 1}", "{\"a\":1,}",
    "[1 2]", "1 2", "01", "1.", "-", "+1", ".5", "1e", "tru", "nul",
    "\"abc", "\"\\x\"", "\"\\u12\"", "\"\\ud800\"", "\"\\udc00\"",
    "\"\\ud800\\u0041\"", "\"a\x01\"", "\"\xff\"", "\"\xc0\xaf\"",
    "\"\xed\xa0\x80\"", "\"\xf4\x90\x80\x80\"", "\"\xe2\x82\"", "{1:2}",
    "[\"a\"]]", "nullx"
  };
  for ( char const *b : bad )
    check( !parse_json( b, &v, &why ), std::string( "reject " ) + b );

  std::string deep( 10000, '[' );
  check( !parse_json( deep, &v, &why ) && contains( why, "deep" ),
         "reject deep nesting without recursing far", why );
  std::string ok_depth( 60, '[' );
  ok_depth += std::string( 60, ']' );
  check( parse_json( ok_depth, &v, &why ), "accept moderate nesting", why );
}

void test_protocol( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  Json r = roundtrip( server, "{not json" );
  check( error_code( r ) == -32700 && r.find( "id" ) && r.find( "id" )->is_null(),
         "parse error: -32700 with id null", r.dump() );

  r = roundtrip( server, "[" + request( 1, "tools/list" ) + "]" );
  check( error_code( r ) == -32600, "batch: -32600", r.dump() );

  r = roundtrip( server, "{\"jsonrpc\":\"2.0\",\"id\":null,\"method\":\"tools/list\"}" );
  check( error_code( r ) == -32600, "null id: -32600", r.dump() );

  r = roundtrip( server, "{\"jsonrpc\":\"1.0\",\"id\":2,\"method\":\"tools/list\"}" );
  check( error_code( r ) == -32600 && field( r, "id" ) == "2",
         "wrong jsonrpc version: -32600, id echoed", r.dump() );

  check( server.handle( "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\","
                        "\"params\":{\"requestId\":1}}" ).empty(),
         "notification: no response" );
  check( server.handle( "{\"jsonrpc\":\"2.0\",\"method\":\"bogus/notification\"}" ).empty(),
         "unknown notification: no response" );

  r = roundtrip( server, request( 3, "tools/list", Json::object(), false ) );
  check( error_code( r ) == -32602 && contains( r.dump(), "2026-07-28" ),
         "request without _meta before initialize: -32602 naming the version",
         r.dump() );

  // Modern (per-request _meta) requests.
  r = roundtrip( server, request( "d1", "server/discover" ) );
  Json const *res = r.find( "result" );
  check( res && field( *res, "resultType" ) == "complete" &&
         contains( r.dump(), "\"supportedVersions\":[\"2026-07-28\"]" ) &&
         contains( r.dump(), "\"tools\":{}" ) &&
         contains( r.dump(), "\"io.modelcontextprotocol/serverInfo\":{\"name\":\"zorba-mcp\"" ) &&
         field( r, "id" ) == "d1",
         "server/discover", r.dump() );

  Json old_meta( meta() );
  old_meta.set( "io.modelcontextprotocol/protocolVersion", "1900-01-01" );
  r = roundtrip( server, Json::object( {
    { "jsonrpc", "2.0" }, { "id", 4 }, { "method", "tools/list" },
    { "params", Json::object( { { "_meta", old_meta } } ) } } ).dump() );
  check( error_code( r ) == -32022 &&
         contains( r.dump(), "\"supported\":[\"2026-07-28\"]" ) &&
         contains( r.dump(), "\"requested\":\"1900-01-01\"" ),
         "unsupported protocol version: -32022", r.dump() );

  Json no_caps( Json::object( {
    { "io.modelcontextprotocol/protocolVersion", protocol_version } } ) );
  r = roundtrip( server, Json::object( {
    { "jsonrpc", "2.0" }, { "id", 5 }, { "method", "tools/list" },
    { "params", Json::object( { { "_meta", no_caps } } ) } } ).dump() );
  check( error_code( r ) == -32602, "missing clientCapabilities: -32602",
         r.dump() );

  r = roundtrip( server, request( 6, "resources/list" ) );
  check( error_code( r ) == -32601, "unknown method: -32601", r.dump() );

  r = roundtrip( server, request( 7, "tools/call", Json::object( {
    { "name", "no_such_tool" }, { "arguments", Json::object() } } ) ) );
  check( error_code( r ) == -32602 && contains( r.dump(), "no_such_tool" ),
         "unknown tool: -32602", r.dump() );

  r = roundtrip( server, request( 8, "tools/call", Json::object( {
    { "name", "run_query" }, { "arguments", "1+1" } } ) ) );
  check( error_code( r ) == -32602, "arguments not an object: -32602",
         r.dump() );

  r = roundtrip( server, request( 9, "ping" ) );
  // ping was removed in 2026-07-28: an unknown method for modern requests.
  check( error_code( r ) == -32601, "modern ping: -32601", r.dump() );

  r = roundtrip( server, request( 10, "ping", Json::object(), false ) );
  check( error_code( r ) == -32602, "ping without _meta or initialize: -32602",
         r.dump() );

  r = call_tool( server, "run_query", Json::object( { { "query", "1 + 1" } } ) );
  res = r.find( "result" );
  check( res && field( *res, "resultType" ) == "complete" &&
         !is_tool_error( r ) && result_text( r ) == "2",
         "modern tools/call", r.dump() );
}

void test_legacy( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  Json r = roundtrip( server, request( 1, "initialize", Json::object( {
    { "protocolVersion", "2025-11-25" },
    { "capabilities", Json::object() },
    { "clientInfo", Json::object( { { "name", "legacy" }, { "version", "1" } } ) }
  } ), false ) );
  Json const *res = r.find( "result" );
  check( res && field( *res, "protocolVersion" ) == "2025-11-25" &&
         contains( r.dump(), "\"serverInfo\":{\"name\":\"zorba-mcp\"" ) &&
         contains( r.dump(), "\"capabilities\":{\"tools\":{}}" ) &&
         res->find( "instructions" ),
         "initialize 2025-11-25", r.dump() );

  check( server.handle( "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}" ).empty(),
         "notifications/initialized: no response" );

  r = roundtrip( server, request( 2, "ping", Json::object(), false ) );
  check( r.find( "result" ) && r.find( "result" )->is_object(), "legacy ping",
         r.dump() );

  r = roundtrip( server, request( 3, "tools/list", Json::object(), false ) );
  check( contains( r.dump(), "\"name\":\"list_documents\"" ) &&
         contains( r.dump(), "\"name\":\"run_query\"" ),
         "legacy tools/list", r.dump() );

  r = roundtrip( server, request( 4, "initialize", Json::object( {
    { "protocolVersion", "2024-11-05" }, { "capabilities", Json::object() },
    { "clientInfo", Json::object( { { "name", "old" }, { "version", "1" } } ) }
  } ), false ) );
  res = r.find( "result" );
  check( res && field( *res, "protocolVersion" ) == "2025-11-25",
         "initialize with an unknown version: server proposes 2025-11-25",
         r.dump() );
}

void test_tools_list( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  config.queries_dir = f.queries;
  Server server( f.zorba, config, std::cerr );

  Json r = roundtrip( server, request( 1, "tools/list" ) );
  Json const *const res = r.find( "result" );
  Json const *const tools = res ? res->find( "tools" ) : nullptr;
  check( tools && tools->is_array() && tools->items().size() == 3,
         "three tools", r.dump() );
  check( res && res->find( "ttlMs" ) && field( *res, "cacheScope" ) == "private",
         "tools/list is cacheable (ttlMs, cacheScope)", r.dump() );
  if ( !tools || tools->items().size() != 3 )
    return;
  Json const &predefined = tools->items()[2];
  check( field( predefined, "name" ) == "order_status", "predefined tool name",
         predefined.dump() );
  check( field( predefined, "description" ) ==
           "Returns the order with the given id (from the XML orders) "
           "together with its payment status (from the JSON payments).",
         "predefined tool description from the leading comment",
         predefined.dump() );
  check( contains( predefined.dump(),
           "\"properties\":{\"order\":{\"type\":\"string\"" ) &&
         contains( predefined.dump(), "\"required\":[\"order\"]" ),
         "predefined tool parameter from the external variable",
         predefined.dump() );
  check( contains( tools->items()[1].dump(), "\"required\":[\"query\"]" ),
         "run_query schema", tools->items()[1].dump() );
}

void test_list_documents( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  config.scan = false;
  Server server( f.zorba, config, std::cerr );

  Json const r = call_tool( server, "list_documents", Json::object() );
  std::string const text( result_text( r ) );
  check( !is_tool_error( r ), "list_documents succeeds", r.dump() );
  check( contains( text, "\"path\" : \"orders/2026/q1.xml\", \"format\" : \"xml\"" ) &&
         contains( text, "\"path\" : \"payments/payments.json\"" ) &&
         contains( text, "\n" ),
         "list_documents lists the files, one per line", text );
  check( !contains( text, "link-out" ) && !contains( text, "hidden" ) &&
         !contains( text, "secret" ),
         "list_documents skips links and hidden entries", text );
}

void test_join( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  Json const r = call_tool( server, "run_query",
    Json::object( { { "query", read_file( f.join_query ) } } ) );
  std::string const text( result_text( r ) );
  check( !is_tool_error( r ) &&
         contains( text, "{ \"order\" : \"o1\", \"file\" : \"orders/2026/q1.xml\", "
                         "\"total\" : 100, \"status\" : \"paid\" }\n"
                         "{ \"order\" : \"o2\", \"file\" : \"orders/2026/q1.xml\", "
                         "\"total\" : 250, \"status\" : \"late\" }" ),
         "run_query joins XML orders with JSON payments", r.dump() );
}

void test_syntax_error( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  Json const r = call_tool( server, "run_query", Json::object( {
    { "query", "let $x := 1\nreturn $x $x" } } ) );
  Json const e( tool_error( r ) );
  Json const *const loc = e.find( "location" );
  check( is_tool_error( r ) && contains( field( e, "code" ), "XPST0003" ) &&
         !field( e, "message" ).empty() &&
         field( e, "namespace" ) == "http://www.w3.org/2005/xqt-errors",
         "syntax error: tool error with the code and message", r.dump() );
  check( loc && field( *loc, "line" ) == "2" && loc->find( "column" ) &&
         field( *loc, "text" ) == "return $x $x",
         "syntax error: line, column and the failing line", r.dump() );
}

void test_escape( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );
  std::string const imp(
    "import module namespace dir = \"http://zorba.io/modules/directory\";\n" );

  Json r = call_tool( server, "run_query", Json::object( {
    { "query", imp + "dir:open($dir:root, \"../outside/secret.xml\")" } } ) );
  check( is_tool_error( r ) &&
         contains( field( tool_error( r ), "code" ), "INVALID_PATH" ) &&
         !contains( r.dump(), "TOP-SECRET" ),
         "dir:open outside the root: INVALID_PATH", r.dump() );

  r = call_tool( server, "run_query", Json::object( {
    { "query", imp + "dir:open($dir:root, \"link-out.xml\")" } } ) );
  check( is_tool_error( r ) && !contains( r.dump(), "TOP-SECRET" ),
         "dir:open of a link out of the root: error", r.dump() );

  r = call_tool( server, "run_query", Json::object( {
    { "query", imp + "doc(concat($dir:root, \"/../outside/secret.xml\"))" } } ) );
  check( is_tool_error( r ) &&
         contains( field( tool_error( r ), "code" ), "FODC0002" ) &&
         contains( r.dump(), "access denied by security policy" ),
         "fn:doc outside the root: denied by the policy", r.dump() );

  r = call_tool( server, "run_query", Json::object( {
    { "query", "doc(\"orders/2026/q1.xml\")/orders/order[1]/string(@id)" } } ) );
  check( !is_tool_error( r ) && contains( result_text( r ), "o1" ),
         "relative URIs resolve against the root", r.dump() );
}

void test_network( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  Json r = call_tool( server, "run_query", Json::object( {
    { "query", "import module namespace m = \"http://127.0.0.1:9/remote-module\";\n"
               "m:f()" } } ) );
  check( is_tool_error( r ) &&
         contains( field( tool_error( r ), "code" ), "XQST0059" ) &&
         contains( r.dump(), "access denied by security policy" ),
         "remote module import: denied", r.dump() );

  r = call_tool( server, "run_query", Json::object( {
    { "query", "doc(\"http://127.0.0.1:9/doc.xml\")" } } ) );
  check( is_tool_error( r ) &&
         contains( r.dump(), "access denied by security policy" ),
         "fn:doc over HTTP: denied", r.dump() );
}

void test_native_modules( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  Json r = call_tool( server, "run_query", Json::object( {
    { "query", "import module namespace sleep = \"http://zorba.io/modules/sleep\";\n"
               "\"native-loaded\"" } } ) );
  check( is_tool_error( r ) &&
         contains( field( tool_error( r ), "code" ), "ZXQP0067" ) &&
         contains( r.dump(), "http://zorba.io/modules/sleep" ) &&
         !contains( r.dump(), "native-loaded" ),
         "native module not on the policy's list: ZXQP0067", r.dump() );

  r = call_tool( server, "run_query", Json::object( {
    { "query", "import module namespace file = \"http://expath.org/ns/file\";\n"
               "concat(\"native-\", string-length(file:directory-separator()))" } } ) );
  check( !is_tool_error( r ) && contains( result_text( r ), "native-1" ),
         "file module (on the policy's list): loads", r.dump() );
}

void test_predefined( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  config.queries_dir = f.queries;
  Server server( f.zorba, config, std::cerr );

  Json r = call_tool( server, "order_status",
                      Json::object( { { "order", "o2" } } ) );
  check( !is_tool_error( r ) &&
         result_text( r ) == "{ \"order\" : \"o2\", \"customer\" : \"globex\", "
                             "\"total\" : 250, \"status\" : [ \"late\" ] }",
         "predefined query with a parameter", r.dump() );

  r = call_tool( server, "order_status", Json::object() );
  check( is_tool_error( r ) && contains( r.dump(), "order" ),
         "predefined query without its parameter: tool error", r.dump() );
}

void test_limits( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  config.max_output = 64;
  config.timeout = std::chrono::seconds( 1 );
  Server server( f.zorba, config, std::cerr );

  Json r = call_tool( server, "run_query", Json::object( {
    { "query", "for $i in 1 to 10000 return $i" } } ) );
  std::string const text( result_text( r ) );
  check( !is_tool_error( r ) && text.compare( 0, 8, "1\n2\n3\n4\n" ) == 0 &&
         contains( text, "[zorba-mcp: output truncated after" ) &&
         text.size() < 300,
         "output over the limit is truncated with a marker", text );

  r = call_tool( server, "run_query", Json::object( {
    { "query", "count((1 to 9223372036854775807)[. lt 0])" } } ) );
  check( is_tool_error( r ) &&
         contains( field( tool_error( r ), "code" ), "ZXQP0068" ),
         "evaluation over the time limit: ZXQP0068", r.dump() );
}


// Runs the zorba-mcp executable over a pipe with the given arguments,
// writes the input to it and returns everything it wrote to stdout.
std::string run_binary( Fixture const &f, std::string const &queries,
                        std::string const &input ) {
  int to_child[2], from_child[2];
  if ( pipe( to_child ) != 0 || pipe( from_child ) != 0 ) {
    check( false, "pipe" );
    return std::string();
  }
  pid_t const pid = fork();
  if ( pid < 0 ) {
    check( false, "fork" );
    return std::string();
  }
  if ( pid == 0 ) {
    if ( dup2( to_child[0], STDIN_FILENO ) < 0 ||
         dup2( from_child[1], STDOUT_FILENO ) < 0 ||
         close( to_child[0] ) != 0 || close( to_child[1] ) != 0 ||
         close( from_child[0] ) != 0 || close( from_child[1] ) != 0 )
      _exit( 127 );
    execl( f.binary.c_str(), f.binary.c_str(), "--root", f.tree.c_str(),
           "--queries", queries.c_str(), static_cast<char*>( nullptr ) );
    _exit( 127 );
  }
  check( close( to_child[0] ) == 0, "close the child's input end" );
  check( close( from_child[1] ) == 0, "close the child's output end" );
  std::size_t written = 0;
  while ( written < input.size() ) {
    ssize_t const n = write( to_child[1], input.data() + written,
                             input.size() - written );
    if ( n <= 0 )
      break;
    written += static_cast<std::size_t>( n );
  }
  check( close( to_child[1] ) == 0, "close the child's input" );
  std::string output;
  char buf[4096];
  for ( ;; ) {
    ssize_t const n = read( from_child[0], buf, sizeof buf );
    if ( n <= 0 )
      break;
    output.append( buf, static_cast<std::size_t>( n ) );
  }
  check( close( from_child[0] ) == 0, "close the child's output" );
  int status = 0;
  check( waitpid( pid, &status, 0 ) == pid, "wait for zorba-mcp" );
  check( WIFEXITED( status ) && WEXITSTATUS( status ) == 0,
         "zorba-mcp exits with 0 at end of input",
         std::to_string( status ) );

  return output;
}

// External variables of a predefined query become the tool's parameters: a
// variable with a default value is optional, one without is required. All
// exchanges go over the zorba-mcp executable's stdio.
void test_optional_params( Fixture const &f ) {
  std::string const queries(
    ( std::filesystem::path( f.queries ).parent_path() / "params_queries" )
      .string() );
  auto call = [&]( int id, char const *tool, Json args ) {
    return request( id, "tools/call", Json::object( {
      { "name", tool }, { "arguments", std::move( args ) } } ) ) + "\n";
  };
  auto obj = []( std::initializer_list<std::pair<std::string,Json> > l ) {
    return Json::object( l );
  };

  std::string input;
  input += request( 1, "tools/list" ) + "\n";
  input += call( 2, "greet", obj( { { "name", "Ana" } } ) );
  input += call( 3, "greet", obj( { { "name", "Ana" }, { "greeting", "hi" } } ) );
  input += call( 4, "greet", obj( { { "greeting", "hi" } } ) );
  input += call( 5, "greet", obj( { { "name", Json::array() } } ) );
  input += call( 6, "greet", obj( { { "name", 5 } } ) );
  input += call( 7, "greet", obj( { { "name", Json() } } ) );
  input += call( 8, "scale", obj( { { "value", "21" } } ) );
  input += call( 9, "scale", obj( { { "value", "21" }, { "factor", "3" } } ) );
  input += call( 10, "scale", obj( { { "value", "abc" } } ) );
  input += call( 11, "scale", obj( { { "value", "2" }, { "factor", "x" } } ) );
  input += call( 12, "scale", obj( { { "factor", "3" } } ) );
  input += call( 13, "run_query", obj( {
    { "query", "declare variable $x external := \"dflt\"; $x" } } ) );
  input += call( 14, "run_query", obj( {
    { "query", "declare variable $x external; $x" } } ) );
  input += call( 15, "run_query", obj( {
    { "query", "declare variable $x external := \"dflt\"; $x" },
    { "variables", obj( { { "x", "given" } } ) } } ) );
  input += call( 16, "greet", obj( { { "name", "Ana" } } ) );
  input += call( 17, "greet", obj( { { "name", "Ana" }, { "nmae", "x" } } ) );
  input += call( 18, "list_documents", obj( { { "bogus", "x" } } ) );

  std::string const output( run_binary( f, queries, input ) );
  std::map<std::string, Json> by_id;
  std::istringstream lines( output );
  std::string line;
  while ( std::getline( lines, line ) ) {
    Json r;
    std::string why;
    if ( parse_json( line, &r, &why ) && r.find( "id" ) )
      by_id[ r.find( "id" )->text() ] = r;
  }
  check( by_id.size() == 18, "eighteen responses", output );
  if ( by_id.size() != 18 )
    return;

  // 1. The input schemas.
  Json const *const res1 = by_id["1"].find( "result" );
  Json const *const tools = res1 ? res1->find( "tools" ) : nullptr;
  std::string greet, scale, ordered;
  if ( tools && tools->is_array() )
    for ( Json const &t : tools->items() ) {
      if ( field( t, "name" ) == "ordered" )
        ordered = t.dump();
      else if ( field( t, "name" ) == "greet" )
        greet = t.dump();
      else if ( field( t, "name" ) == "scale" )
        scale = t.dump();
    }
  check( contains( greet, "\"properties\":{\"name\":" ) &&
         contains( greet, "\"greeting\":" ) &&
         contains( greet, "\"required\":[\"name\"]" ),
         "greet: $name required, defaulted $greeting optional", greet );
  check( contains( scale, "\"required\":[\"value\"]" ) &&
         contains( scale, "\"factor\":" ),
         "scale: $value required, defaulted $factor optional", scale );

  // The properties are listed in declaration order, so the tool list is the
  // same on every run.
  {
    size_t pos = 0;
    bool in_order = !ordered.empty();
    for ( char const *n : { "zeta", "alpha", "mid", "beta", "omega", "kappa",
                            "delta", "sigma" } ) {
      size_t const at = ordered.find( std::string( "\"" ) + n + "\":{", pos );
      if ( at == std::string::npos ) {
        in_order = false;
        break;
      }
      pos = at;
    }
    check( in_order, "properties follow declaration order", ordered );
    check( contains( ordered,
             "\"required\":[\"zeta\",\"mid\",\"omega\",\"delta\"]" ),
           "required lists the required parameters in declaration order",
           ordered );
  }

  // 2. Defaults and overrides.
  check( !is_tool_error( by_id["2"] ) &&
         result_text( by_id["2"] ) == "\"hello, Ana\"",
         "optional parameter omitted: the default applies", by_id["2"].dump() );
  check( !is_tool_error( by_id["3"] ) &&
         result_text( by_id["3"] ) == "\"hi, Ana\"",
         "optional parameter supplied: overrides the default",
         by_id["3"].dump() );
  check( !is_tool_error( by_id["16"] ) &&
         result_text( by_id["16"] ) == "\"hello, Ana\"",
         "the override is not retained across calls", by_id["16"].dump() );
  check( !is_tool_error( by_id["8"] ) && result_text( by_id["8"] ) == "42",
         "typed optional parameter omitted: the default applies",
         by_id["8"].dump() );
  check( !is_tool_error( by_id["9"] ) && result_text( by_id["9"] ) == "63",
         "typed optional parameter supplied as text: converted to its type",
         by_id["9"].dump() );

  // 3. A missing required parameter is a tool error naming it.
  for ( char const *const id : { "4", "7", "12" } ) {
    Json const &r = by_id[id];
    check( is_tool_error( r ) &&
           field( tool_error( r ), "code" ) == "invalid-arguments" &&
           contains( field( tool_error( r ), "message" ),
                     std::string( id ) == "12" ? "value" : "name" ),
           std::string( "missing required parameter (request " ) + id +
           "): tool error", r.dump() );
  }

  // 4. A value that does not fit the parameter is a tool error, not a crash.
  check( is_tool_error( by_id["5"] ) &&
         contains( field( tool_error( by_id["5"] ), "message" ), "name" ),
         "array for a string parameter: tool error", by_id["5"].dump() );
  check( !is_tool_error( by_id["6"] ) && result_text( by_id["6"] ) == "\"hello, 5\"",
         "number for a string parameter: its text", by_id["6"].dump() );
  check( is_tool_error( by_id["10"] ) && !tool_error( by_id["10"] ).is_null(),
         "text that is not an xs:integer: tool error with a diagnostic",
         by_id["10"].dump() );
  check( is_tool_error( by_id["11"] ) && !tool_error( by_id["11"] ).is_null(),
         "optional parameter of the wrong type: tool error with a diagnostic",
         by_id["11"].dump() );

  // run_query takes the query only: no variables; the query's own defaults
  // apply and an external variable without one is an error.
  check( !is_tool_error( by_id["13"] ) && result_text( by_id["13"] ) == "\"dflt\"",
         "run_query: a defaulted external variable uses its default",
         by_id["13"].dump() );
  check( is_tool_error( by_id["14"] ) && !tool_error( by_id["14"] ).is_null(),
         "run_query: an external variable without a value is a tool error",
         by_id["14"].dump() );

  // An argument the tool's inputSchema does not declare is rejected, naming
  // it, instead of being ignored.
  struct { char const *id; char const *arg; char const *what; } const unknown[] = {
    { "15", "variables", "run_query" },
    { "17", "nmae", "predefined tool" },
    { "18", "bogus", "list_documents" }
  };
  for ( auto const &u : unknown ) {
    Json const &r = by_id[u.id];
    check( is_tool_error( r ) &&
           field( tool_error( r ), "code" ) == "invalid-arguments" &&
           contains( field( tool_error( r ), "message" ), u.arg ),
           std::string( u.what ) + ": undeclared argument \"" + u.arg +
           "\" is a tool error", r.dump() );
  }
}

// Runs the zorba-mcp executable over a pipe: its stdout must carry only
// protocol messages, one per line, even when a query writes a trace.
void test_binary( Fixture const &f ) {
  std::string input;
  input += request( 1, "server/discover" ) + "\n";
  input += request( 2, "initialize", Json::object( {
    { "protocolVersion", "2025-11-25" }, { "capabilities", Json::object() },
    { "clientInfo", Json::object( { { "name", "t" }, { "version", "1" } } ) }
  } ), false ) + "\n";
  input += "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}\n";
  input += request( 3, "tools/call", Json::object( {
    { "name", "run_query" },
    { "arguments", Json::object( { { "query", read_file( f.join_query ) } } ) }
  } ) ) + "\r\n";
  input += "\n";
  input += request( 4, "tools/call", Json::object( {
    { "name", "run_query" },
    { "arguments", Json::object( { { "query", "trace(42, \"label\")" } } ) }
  } ), false ) + "\n";
  input += "this is not json\n";

  std::string const output( run_binary( f, f.queries, input ) );

  std::istringstream lines( output );
  std::string line;
  char const *const ids[] = { "1", "2", "3", "4", "null" };
  std::size_t n = 0;
  bool all_json = true;
  while ( std::getline( lines, line ) ) {
    Json r;
    std::string why;
    if ( !parse_json( line, &r, &why ) || field( r, "jsonrpc" ) != "2.0" ) {
      all_json = false;
      check( false, "stdout line is a JSON-RPC message", line );
      continue;
    }
    Json const *const id = r.find( "id" );
    std::string const got( id ? ( id->is_null() ? "null" : id->text() ) : "" );
    if ( n < 5 && got != ids[n] )
      check( false, "response order", line );
    if ( got == "3" )
      check( contains( result_text( r ),
               "{ \"order\" : \"o1\", \"file\" : \"orders/2026/q1.xml\"" ),
             "join over the binary", line );
    if ( got == "4" )
      check( !is_tool_error( r ) && result_text( r ) == "42",
             "trace output stays off stdout", line );
    if ( got == "null" )
      check( error_code( r ) == -32700, "garbage line: parse error", line );
    ++n;
  }
  check( all_json && n == 5, "five responses, all protocol messages",
         std::to_string( n ) + "\n" + output );
}

///////////////////////////////////////////////////////////////////////////////
// Tests on a fresh tree in a temporary directory: <tmp>/root (the server's
// root) and <tmp>/outside.

struct TempTree {
  std::string base;
  std::string root;

  TempTree() {
    std::string tmpl(
      ( std::filesystem::temp_directory_path() / "mcp-tree-XXXXXX" ).string() );
    char *const made = mkdtemp( &tmpl[0] );
    // The canonical spelling: the policy compares real paths.
    base = std::filesystem::canonical( made ? made : "." ).string();
    root = base + "/root";
    std::filesystem::create_directories( root );
    std::filesystem::create_directories( base + "/outside" );
  }
  ~TempTree() {
    std::error_code ec;
    std::filesystem::remove_all( base, ec );
  }

  void write( std::string const &rel, std::string const &content ) const {
    std::ofstream out( ( root + "/" + rel ).c_str(), std::ios::binary );
    out << content;
  }
};

// The peak resident set size of this process so far, in bytes.
long long peak_rss() {
  struct rusage u;
  getrusage( RUSAGE_SELF, &u );
#ifdef __APPLE__
  return static_cast<long long>( u.ru_maxrss );
#else
  return static_cast<long long>( u.ru_maxrss ) * 1024;
#endif
}

// dir:documents parses a document when the caller asks for it, and a
// document the caller is done with can be freed: a scan of the whole tree
// keeps one parsed document in memory, not all of them. The test scans a
// tree of 8 files of 2 MB, adds 32 more and scans again. Holding every
// document, the second scan raises the peak RSS by 4 times what the first
// one did (a parsed JSON document takes about 22 bytes per byte of file);
// streaming, it does not raise it by as much as the first scan did. The
// first scan's growth is the scale, so allocator overhead and a
// sanitizer's quarantine of freed memory (256 MiB in AddressSanitizer,
// filled by the first scan) count on both sides.
void test_documents_stream( Fixture const &f ) {
  TempTree t;
  std::string content( "{ \"rs\" : [" );
  for ( int i = 0; content.size() < 2000000; ++i ) {
    if ( i )
      content += ",";
    content += "{\"id\":" + std::to_string( i ) + ",\"cat\":\"c" +
      std::to_string( i % 4 ) + "\",\"v\":" + std::to_string( i % 100 ) +
      ",\"n\":\"name" + std::to_string( i ) + "\"}";
  }
  content += "] }";
  ServerConfig config;
  config.root = t.root;
  config.timeout = std::chrono::milliseconds( 0 );
  Server server( f.zorba, config, std::cerr );
  std::string const scan(
    "import module namespace dir = \"http://zorba.io/modules/directory\";\n"
    "count(for $d in dir:documents($dir:root)\n"
    "      where exists($d(\"content\")(\"rs\")) return $d)" );

  int files = 0;
  long long peak[3];
  peak[0] = peak_rss();
  for ( int step = 1; step <= 2; ++step ) {
    for ( int const target = step == 1 ? 8 : 40; files < target; ++files )
      t.write( "d" + std::to_string( 100 + files ) + ".json", content );
    Json const r = call_tool( server, "run_query",
                              Json::object( { { "query", scan } } ) );
    peak[step] = peak_rss();
    check( !is_tool_error( r ) && result_text( r ) == std::to_string( files ),
           "dir:documents parses all " + std::to_string( files ) +
           " documents", r.dump() );
  }
  long long const first = peak[1] - peak[0], second = peak[2] - peak[1];
  check( second < first,
         "a scan with dir:documents does not hold every parsed document",
         "peak RSS grew by " + std::to_string( first >> 20 ) +
         " MiB scanning 8 files, then by " + std::to_string( second >> 20 ) +
         " MiB scanning 40" );
}

///////////////////////////////////////////////////////////////////////////////
// Files that take a parser to its limits: each gets a result or an error of
// its own, and the server keeps answering.

// The limit on the nesting depth of JSON values, zerr:ZXQP0071.
std::size_t const json_limit = 1500;

std::string repeated( std::string const &s, std::size_t n ) {
  std::string r;
  r.reserve( s.size() * n );
  for ( std::size_t i = 0; i < n; ++i )
    r += s;
  return r;
}

std::string deep_array( std::size_t n ) {
  return repeated( "[", n ) + "1" + repeated( "]", n );
}

std::string deep_object( std::size_t n ) {
  return repeated( "{\"a\":", n ) + "1" + repeated( "}", n );
}

std::string const dir_import(
  "import module namespace dir = \"http://zorba.io/modules/directory\";\n" );

// The files in the root that have an error record, with its code.
std::string const error_records(
  dir_import +
  "string-join(for $d in dir:documents($dir:root)\n"
  "            order by $d(\"path\")\n"
  "            return concat($d(\"path\"), \"=\", \n"
  "              if (exists($d(\"error\"))) then $d(\"error\")(\"code\")\n"
  "              else \"ok\"), \" \")" );

Json run_query( Server &server, std::string const &query ) {
  return call_tool( server, "run_query",
                    Json::object( { { "query", query } } ) );
}

void check_alive( Server &server ) {
  Json const r = run_query( server, "1 + 1" );
  check( !is_tool_error( r ) && result_text( r ) == "2",
         "the server still answers a normal query", r.dump() );
}

void write_deep_tree( TempTree const &t ) {
  t.write( "good.json", "{ \"a\" : [ 1, 2 ] }" );
  t.write( "bad.json", "{ \"a\" : " );
  // The record around a file's content is one more level.
  t.write( "under.json", deep_array( json_limit - 1 ) );
  t.write( "at-limit.json", deep_array( json_limit ) );
  t.write( "over.json", deep_array( json_limit + 1 ) );
  t.write( "deep-array.json", deep_array( 200000 ) );
  t.write( "deep-object.json", deep_object( 200000 ) );
}

std::string const deep_tree_records(
  "at-limit.json=ZXQP0071 bad.json=JNDY0021 deep-array.json=ZXQP0071 "
  "deep-object.json=ZXQP0071 good.json=ok over.json=ZXQP0071 under.json=ok" );

void test_deep_json( Fixture const &f ) {
  TempTree t;
  write_deep_tree( t );
  ServerConfig config;
  config.root = t.root;
  config.timeout = std::chrono::milliseconds( 0 );
  Server server( f.zorba, config, std::cerr );

  Json r = run_query( server, error_records );
  check( !is_tool_error( r ) && contains( result_text( r ), deep_tree_records ),
         "every too-deep file has its error record, the others are not "
         "affected", r.dump().substr( 0, 2000 ) );
  check_alive( server );

  // The file that just fits is served, and serializes.
  r = run_query( server,
    dir_import + "dir:open($dir:root, \"under.json\")" );
  check( !is_tool_error( r ) && contains( result_text( r ), "content" ) &&
         !contains( result_text( r ), "ZXQP0071" ),
         "the file that fits is served", r.dump().substr( 0, 2000 ) );
  check_alive( server );

  r = run_query( server,
    dir_import + "dir:open($dir:root, \"deep-array.json\")" );
  check( !is_tool_error( r ) && contains( result_text( r ), "ZXQP0071" ) &&
         contains( result_text( r ), std::to_string( json_limit ) ),
         "dir:open of a too-deep file: the error record says the limit",
         r.dump().substr( 0, 2000 ) );
  check_alive( server );

  // Serializing everything dir:documents returns.
  r = run_query( server, dir_import + "dir:documents($dir:root)" );
  check( !is_tool_error( r ) && contains( result_text( r ), "good.json" ),
         "dir:documents serializes with too-deep files in the tree",
         r.dump().substr( 0, 2000 ) );
  check_alive( server );

  // A query that builds such a value.
  r = run_query( server,
    "import module namespace jn = \"http://jsoniq.org/functions\";\n"
    "jn:parse-json(concat(string-join(for $i in 1 to 200000 return \"[\"),\n"
    "                     string-join(for $i in 1 to 200000 return \"]\")))" );
  check( is_tool_error( r ) &&
         contains( field( tool_error( r ), "code" ), "ZXQP0071" ),
         "a query that parses a too-deep value: tool error",
         r.dump().substr( 0, 2000 ) );
  check_alive( server );
}

// The same with the executable: it must survive and answer later calls.
void test_deep_json_binary( Fixture const &f ) {
  TempTree t;
  write_deep_tree( t );
  Fixture g( f );
  g.tree = t.root;
  auto call = [&]( int id, std::string const &query ) {
    return request( id, "tools/call", Json::object( {
      { "name", "run_query" },
      { "arguments", Json::object( { { "query", query } } ) }
    } ) ) + "\n";
  };
  std::string input;
  input += call( 1, error_records );
  input += call( 2, "1 + 1" );
  input += call( 3, dir_import + "dir:documents($dir:root)" );
  input += call( 4, "2 + 2" );
  input += call( 5,
    "import module namespace jn = \"http://jsoniq.org/functions\";\n"
    "jn:parse-json(concat(string-join(for $i in 1 to 200000 return \"[\"),\n"
    "                     string-join(for $i in 1 to 200000 return \"]\")))" );
  input += call( 6, "3 + 3" );

  std::string const output( run_binary( g, f.queries, input ) );
  std::map<std::string, Json> by_id;
  std::istringstream lines( output );
  std::string line;
  while ( std::getline( lines, line ) ) {
    Json r;
    std::string why;
    if ( parse_json( line, &r, &why ) && r.find( "id" ) )
      by_id[ r.find( "id" )->text() ] = r;
  }
  check( by_id.size() == 6, "six responses", output.substr( 0, 2000 ) );
  check( contains( result_text( by_id["1"] ), deep_tree_records ),
         "the error records", by_id["1"].dump().substr( 0, 2000 ) );
  check( result_text( by_id["2"] ) == "2", "a normal query after the scan",
         by_id["2"].dump() );
  check( !is_tool_error( by_id["3"] ), "dir:documents serializes",
         by_id["3"].dump().substr( 0, 2000 ) );
  check( result_text( by_id["4"] ) == "4", "a normal query after serializing",
         by_id["4"].dump() );
  check( is_tool_error( by_id["5"] ) &&
         contains( field( tool_error( by_id["5"] ), "code" ), "ZXQP0071" ),
         "a query that parses a too-deep value: tool error",
         by_id["5"].dump().substr( 0, 2000 ) );
  check( result_text( by_id["6"] ) == "6", "a normal query after the error",
         by_id["6"].dump() );
}

// Internal entities that expand to far more than the document: libxml2 as
// built in this environment refuses them, quickly, and the file gets its
// error record. Runs under the production libxml2 of every CI job.
void test_entity_expansion( Fixture const &f ) {
  TempTree t;
  std::string laughs( "<?xml version=\"1.0\"?>\n<!DOCTYPE lolz [\n"
                      "<!ENTITY lol0 \"lol\">\n" );
  for ( int i = 1; i < 10; ++i ) {
    laughs += "<!ENTITY lol" + std::to_string( i ) + " \"";
    for ( int j = 0; j < 10; ++j )
      laughs += "&lol" + std::to_string( i - 1 ) + ";";
    laughs += "\">\n";
  }
  laughs += "]>\n<lolz>&lol9;</lolz>";
  t.write( "laughs.xml", laughs );

  // One long entity referenced many times: no nesting, the same growth.
  std::string quadratic( "<?xml version=\"1.0\"?>\n<!DOCTYPE r [\n"
                         "<!ENTITY a \"" + std::string( 100000, 'a' ) +
                         "\">\n]>\n<r>" );
  quadratic += repeated( "&a;", 100000 );
  quadratic += "</r>";
  t.write( "quadratic.xml", quadratic );

  t.write( "good.xml", "<r>fine</r>" );
  ServerConfig config;
  config.root = t.root;
  config.timeout = std::chrono::milliseconds( 0 );
  Server server( f.zorba, config, std::cerr );

  auto const start = std::chrono::steady_clock::now();
  long long const rss = peak_rss();
  Json const r = run_query( server, error_records );
  auto const seconds = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::steady_clock::now() - start ).count();
  long long const growth = peak_rss() - rss;
  std::cout << "entity expansion: " << result_text( r ) << " in " << seconds
            << " s, peak RSS grew by " << ( growth >> 20 ) << " MiB"
            << std::endl;
  check( !is_tool_error( r ) && contains( result_text( r ), "good.xml=ok" ) &&
         !contains( result_text( r ), "laughs.xml=ok" ) &&
         !contains( result_text( r ), "quadratic.xml=ok" ),
         "the expansions get an error record, the other file does not",
         r.dump().substr( 0, 2000 ) );
  check( seconds < 30, "refused quickly", std::to_string( seconds ) + " s" );
  check( growth < 1024LL * 1024 * 1024, "refused before using much memory",
         std::to_string( growth >> 20 ) + " MiB" );
  check_alive( server );
}

// Paths that differ from the root in case only, for the case-insensitive file
// systems (macOS by default; elsewhere these spell other, absent paths). A
// different spelling of the root leads to no file outside of it, and a sibling
// whose name starts with the root's stays denied under every spelling.
void test_case_paths( Fixture const &f ) {
  TempTree t;
  std::filesystem::create_directories( t.root + "/Sub" );
  t.write( "Sub/File.json", "{ \"inside\" : \"INSIDE-VALUE\" }" );
  std::string const sibling( t.base + "/root-sibling" );
  std::filesystem::create_directories( sibling );
  {
    std::ofstream out( ( sibling + "/secret.json" ).c_str() );
    out << "{ \"secret\" : \"TOP-SECRET\" }";
  }
  bool const insensitive =
    std::filesystem::exists( t.base + "/ROOT/Sub/File.json" );
  std::cout << "file system is case-" << ( insensitive ? "in" : "" )
            << "sensitive" << std::endl;

  std::string const read(
    "import module namespace file = \"http://expath.org/ns/file\";\n"
    "file:read-text(\"" );
  std::string const denied[] = {
    t.base + "/root-sibling/secret.json",
    t.base + "/ROOT-SIBLING/secret.json",
    t.base + "/Root-Sibling/secret.json",
    t.root + "/../root-sibling/secret.json",
    t.base + "/ROOT/../root-sibling/secret.json",
    t.base + "/ROOT/../ROOT-SIBLING/secret.json"
  };

  auto check_server = [&]( std::string const &root, char const *what ) {
    ServerConfig config;
    config.root = root;
    std::string why;
    check( directory_config::normalize( &config.root, &why ) || !insensitive,
           std::string( what ) + ": the root is usable", why );
    if ( !why.empty() )
      return;
    Server server( f.zorba, config, std::cerr );
    std::cout << what << ": root " << config.root << std::endl;

    for ( std::string const &path : denied ) {
      Json r = run_query( server, read + path + "\")" );
      check( is_tool_error( r ) && !contains( r.dump(), "TOP-SECRET" ),
             std::string( what ) + ": sibling stays denied: " + path,
             r.dump() );
      r = run_query( server, "doc(\"file://" + path + "\")" );
      check( is_tool_error( r ) && !contains( r.dump(), "TOP-SECRET" ),
             std::string( what ) + ": fn:doc of the sibling: " + path,
             r.dump() );
    }

    // dir:open with a relative path: nothing leaves the root.
    char const *const relative[] = {
      "../root-sibling/secret.json", "../ROOT-SIBLING/secret.json",
      "SUB/../../root-sibling/secret.json", "Sub/../../ROOT-SIBLING/secret.json"
    };
    for ( char const *const path : relative ) {
      Json const r = run_query( server,
        dir_import + "dir:open($dir:root, \"" + path + "\")" );
      check( is_tool_error( r ) && !contains( r.dump(), "TOP-SECRET" ),
             std::string( what ) + ": dir:open outside the root: " + path,
             r.dump() );
    }

    // A different-case spelling inside the root reads that file or is refused;
    // on a case-sensitive file system there is no such file.
    char const *const inside[] = { "Sub/File.json", "SUB/FILE.JSON", "sub/file.json" };
    for ( char const *const path : inside ) {
      Json const r = run_query( server,
        dir_import + "dir:open($dir:root, \"" + path + "\")" );
      std::cout << what << ": dir:open " << path << ": "
                << r.dump().substr( 0, 400 ) << std::endl;
      check( !contains( r.dump(), "TOP-SECRET" ),
             std::string( what ) + ": inside the root: " + path, r.dump() );
    }
    Json const r = run_query( server, read + t.base + "/ROOT/Sub/File.json\")" );
    std::cout << what << ": read through the upper-case root: "
              << r.dump().substr( 0, 400 ) << std::endl;
    check( !contains( r.dump(), "TOP-SECRET" ),
           std::string( what ) + ": read through the upper-case root", r.dump() );

    check_alive( server );
  };
  check_server( t.root, "root as created" );
  if ( insensitive )
    check_server( t.base + "/ROOT", "root spelled in upper case" );
}

// HTML files a parser has to survive: every one gives a result or an error
// record, through dir:open, dir:documents and x:parse-html.
void test_html_edge_cases( Fixture const &f ) {
  TempTree t;
  char const nul[] = "<html>\0<body>a\0b<div \0x=\"\0\">c</div></body></html>";
  struct { char const *name; std::string content; } const files[] = {
    { "deep.html", repeated( "<div>", 200000 ) },
    { "deep-closed.html", repeated( "<div>", 200000 ) +
                          repeated( "</div>", 200000 ) },
    { "attributes.html", "<html><body><a href=\"" +
                         std::string( 12 * 1024 * 1024, 'x' ) +
                         "\">x</a></body></html>" },
    { "many-attributes.html", "<html><body><a " +
                              [] {
                                std::string r;
                                for ( int i = 0; i < 20000; ++i )
                                  r += "a" + std::to_string( i ) + "=\"1\" ";
                                return r;
                              }() + ">x</a></body></html>" },
    { "truncated.html", "<html><body><div class=\"unterminated" },
    { "truncated-tag.html", "<html><body><" },
    { "nul.html", std::string( nul, sizeof nul - 1 ) },
    { "only-nul.html", std::string( 1024, '\0' ) },
    { "empty.html", "" },
    { "good.html", "<html><body><p>fine</p></body></html>" }
  };
  for ( auto const &file : files )
    t.write( file.name, file.content );

  ServerConfig config;
  config.root = t.root;
  config.timeout = std::chrono::milliseconds( 0 );
  Server server( f.zorba, config, std::cerr );

  std::string const parse( dir_import +
    "import module namespace x = \"http://zorba.io/modules/xml\";\n"
    "import module namespace file = \"http://expath.org/ns/file\";\n" );

  // Builds without XML Schema support have no HTML parser.
  bool const has_parser = !contains(
    run_query( server,
      dir_import + "dir:open($dir:root, \"good.html\")" ).dump(),
    "UNSUPPORTED_FORMAT" );
  std::cout << "HTML parser in this build: " << has_parser << std::endl;

  for ( auto const &file : files ) {
    std::string const name( file.name );
    auto const start = std::chrono::steady_clock::now();
    Json const r = run_query( server, dir_import +
      "let $d := dir:open($dir:root, \"" + name + "\")\n"
      "return if (exists($d(\"error\"))) then $d(\"error\")(\"code\")\n"
      "       else concat(\"ok \", count($d(\"content\")//*))" );
    auto const seconds = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - start ).count();
    std::cout << name << ": dir:open: " << r.dump().substr( 0, 400 ) << " in "
              << seconds << " s" << std::endl;
    check( !r.is_null(), name + ": dir:open answers" );
    check( seconds < 120, name + ": dir:open in reasonable time",
           std::to_string( seconds ) + " s" );

    if ( has_parser ) {
      Json const p = run_query( server, parse +
        "count(x:parse-html(file:read-text(concat($dir:root, \"/" + name +
        "\")))//*)" );
      std::cout << name << ": x:parse-html: " << p.dump().substr( 0, 400 )
                << std::endl;
      check( !p.is_null() && !contains( p.dump(), "XPST" ) &&
             !contains( p.dump(), "XQST" ),
             name + ": x:parse-html runs and answers",
             p.dump().substr( 0, 400 ) );
    }

    check_alive( server );
  }

  Json const r = run_query( server, error_records );
  std::cout << "records: " << result_text( r ) << std::endl;
  check( !is_tool_error( r ) && contains( result_text( r ), "good.html=" ),
         "dir:documents over all the files", r.dump().substr( 0, 2000 ) );
  check( !has_parser || contains( result_text( r ), "good.html=ok" ),
         "the good file is not affected", r.dump().substr( 0, 2000 ) );
  check_alive( server );
}

struct Test {
  char const *name;
  void (*fn)( Fixture const& );
};

Test const tests[] = {
  { "json", test_json },
  { "protocol", test_protocol },
  { "legacy", test_legacy },
  { "tools_list", test_tools_list },
  { "list_documents", test_list_documents },
  { "join", test_join },
  { "syntax_error", test_syntax_error },
  { "escape", test_escape },
  { "network", test_network },
  { "native_modules", test_native_modules },
  { "predefined", test_predefined },
  { "limits", test_limits },
  { "binary", test_binary },
  { "optional_params", test_optional_params },
  { "documents_stream", test_documents_stream },
  { "deep_json", test_deep_json },
  { "deep_json_binary", test_deep_json_binary },
  { "entity_expansion", test_entity_expansion },
  { "case_paths", test_case_paths },
  { "html_edge_cases", test_html_edge_cases }
};

} // namespace

int main( int argc, char const *argv[] ) {
  if ( argc != 5 && argc != 6 ) {
    std::cerr << "usage: mcp_tests <test> <tree-dir> <queries-dir> "
                 "<join-query-file> [<zorba-mcp>]" << std::endl;
    return 2;
  }
  std::string const name( argv[1] );
  Test const *test = nullptr;
  for ( Test const &t : tests )
    if ( name == t.name )
      test = &t;
  if ( !test ) {
    std::cerr << "mcp_tests: unknown test " << name << std::endl;
    return 2;
  }

  void *const store = StoreManager::getStore();
  Zorba *const zorba = Zorba::getInstance( store );
  {
    Fixture f{ zorba, argv[2], argv[3], argv[4], argc == 6 ? argv[5] : "" };
    std::string why;
    if ( !directory_config::normalize( &f.tree, &why ) ||
         !directory_config::normalize( &f.queries, &why ) ) {
      std::cerr << "mcp_tests: " << why << std::endl;
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
