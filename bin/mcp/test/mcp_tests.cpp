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

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

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

// AddressSanitizer and ThreadSanitizer need far more address space than the
// process uses: an address-space limit does not work with them.
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
# define ZORBA_TEST_SANITIZER 1
#elif defined(__has_feature)
# if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#  define ZORBA_TEST_SANITIZER 1
# endif
#endif

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

// Bounds the address space of this process while it lives.
class AddressSpaceLimit {
#if defined(__linux__) && !defined(ZORBA_TEST_SANITIZER)
  struct rlimit old_;
  bool set_ = false;
public:
  explicit AddressSpaceLimit( unsigned long long bytes ) {
    if ( getrlimit( RLIMIT_AS, &old_ ) == 0 ) {
      struct rlimit lim = old_;
      lim.rlim_cur = bytes;
      set_ = setrlimit( RLIMIT_AS, &lim ) == 0;
    }
  }
  ~AddressSpaceLimit() {
    if ( set_ )
      check( setrlimit( RLIMIT_AS, &old_ ) == 0,
             "restore the address-space limit" );
  }
#else
public:
  explicit AddressSpaceLimit( unsigned long long ) { }
#endif
};

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

// Runs a query that must fail with the code. A non-empty part must appear in
// the "hint" field and not in the message; an empty part means no hint at all.
void check_error( Server &server, std::string const &query,
                  std::string const &code, std::string const &part,
                  std::string const &what ) {
  Json const r = call_tool( server, "run_query",
                            Json::object( { { "query", query } } ) );
  Json const e( tool_error( r ) );
  Json const *const hint = e.find( "hint" );
  bool ok = is_tool_error( r ) && contains( field( e, "code" ), code );
  if ( part.empty() )
    ok = ok && !hint;
  else
    ok = ok && hint && contains( hint->text(), part ) &&
      !contains( field( e, "message" ), part );
  check( ok, what, r.dump() );
}

// Runs a query that must succeed with this output.
void check_result( Server &server, std::string const &query,
                   std::string const &expected, std::string const &what ) {
  Json const r = call_tool( server, "run_query",
                            Json::object( { { "query", query } } ) );
  std::string text( result_text( r ) );
  while ( !text.empty() && text.back() == '\n' )
    text.pop_back();
  check( !is_tool_error( r ) && text == expected, what, r.dump() );
}

std::string const dir_import(
  "import module namespace dir = \"http://zorba.io/modules/directory\";\n" );
std::string const jsoniq_decl( "jsoniq version \"1.0\";\n" );

// The forms the JSONiq specification defines: they resolve under a JSONiq
// version declaration, and stay errors (with a hint) in XQuery.
void test_hints_jsoniq( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  // $x[]
  check_result( server, jsoniq_decl + "let $x := [1, 2] return $x[]", "1\n2",
                "JSONiq: $x[] unboxes an array" );
  check_error( server, "let $x := [1, 2] return $x[]", "XPST0003",
               "jn:members($x)", "XQuery: $x[] is an error with a hint" );
  check_error( server, "let $x := [1, 2] return $x[ ]", "XPST0003",
               "jn:members($x)", "XQuery: $x[ ] is an error with a hint" );
  check_result( server, "let $x := [1, 2] return $x()", "1\n2",
                "the hint's $x() works" );
  check_result( server, "let $x := [1, 2] return jn:members($x)", "1\n2",
                "the hint's jn:members($x) works" );
  std::string const one_line(
    "import module namespace dir = \"http://zorba.io/modules/directory\"; " );
  check_error( server, one_line + "let $x := [1, 2] return $x[]", "XPST0003",
               "jn:members($x)", "$x[] after a prolog on the same line" );
  check_error( server, one_line + "(1, 2)[$$ eq 2]", "XPST0003", "\".\"",
               "$$ after a prolog on the same line" );
  check_error( server, one_line + "let $a := 3 let $b := 1 return $a-$b",
               "XPST0003", "$a - $b", "$a-$b after a prolog on the same line" );
  check_error( server, "let $x := [1] return $x[[]]", "XPST0003",
               "jn:members($x)", "$x[[]]: hint" );
  check_error( server, "[1, ]", "XPST0003", "",
               "no $x[] hint on another unexpected ]" );
  check_error( server, "let $x := [1] return $x]", "XPST0003", "",
               "no $x[] hint on a stray ]" );
  check_result( server, jsoniq_decl + dir_import +
                "dir:open($dir:root, \"orders/2026/q1.xml\")(\"format\")",
                "\"xml\"", "JSONiq: the declaration works with the module" );

  // $$
  check_result( server, jsoniq_decl + "(1, 2)[$$ eq 2]", "2",
                "JSONiq: $$ is the context item" );
  check_error( server, "(1, 2)[$$ eq 2]", "XPST0003", "\".\"",
               "XQuery: $$ is an error with a hint" );
  check_result( server, "(1, 2)[. eq 2]", "2", "the hint's . works" );
  check_result( server, "({ \"a\" : 1 }, { \"a\" : 2 })[.(\"a\") eq 2](\"a\")",
                "2", "the hint's .(\"key\") works" );

  // Function names without a prefix.
  struct { char const *call, *expected, *fixed; } const fns[] = {
    { "keys({ \"a\" : 1 })", "\"a\"", "jn:keys(" },
    { "size([1, 2])", "2", "jn:size(" },
    { "flatten([1, [2]])", "1\n2", "jn:flatten(" },
    { "members([1, 2])", "1\n2", "jn:members(" }
  };
  for ( auto const &fn : fns ) {
    std::string const call( fn.call );
    check_result( server, jsoniq_decl + call, fn.expected,
                  "JSONiq: " + call + " resolves" );
    check_error( server, call, "XPST0017", fn.fixed,
                 "XQuery: " + call + " is an error with a hint" );
    check_result( server, "jn:" + call, fn.expected,
                  "the hint's jn:" + call + " works" );
  }
  check_result( server, jsoniq_decl + "pi() gt 3", "true",
                "JSONiq: pi() resolves" );
  check_error( server, "pi()", "XPST0017", "math:pi(",
               "XQuery: pi() is an error with a hint" );
  check_error( server, "keys(1, 2)", "XPST0017", "",
               "no hint: jn:keys has no form with two arguments" );
  check_error( server, "nosuchfunction(1)", "XPST0017", "",
               "no hint on an unknown function" );
}

// "-" is a name character: never accepted, the hint gives the expression.
void test_hints_dash( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );

  check_error( server, "let $a2 := 3 let $a1 := 1 return ($a2-$a1)*2",
               "XPST0003", "$a2 - $a1", "$a2-$a1: hint with the expression" );
  check_error( server,
               jsoniq_decl + "let $a2 := 3 let $a1 := 1 return $a2-$a1",
               "XPST0003", "$a2 - $a1", "$a2-$a1 in JSONiq too" );
  check_result( server, "let $a2 := 3 let $a1 := 1 return ($a2 - $a1)*2", "4",
                "the hint's $a2 - $a1 works" );
  check_error( server, "let $a-$b := 1 return 1", "XPST0003", "",
               "no hint where a variable is being bound" );
  check_error( server, "let $a := 3 let $b := 1\nreturn $a-$b\n", "XPST0003",
               "$a - $b", "$a-$b at the end of the query: hint" );
  check_error( server, "let $x := 1 return", "XPST0003", "",
               "no hint on another error at the end of the query" );
  check_error( server, "let $b := 1 return 1 $b", "XPST0003", "",
               "no hint on $ after something else" );

  check_error( server, "let $x := 3 return $x-1", "XPST0008", "$x - 1",
               "$x-1: hint with the expression" );
  check_result( server, "let $x := 3 return $x - 1", "2",
                "the hint's $x - 1 works" );
  check_error( server, "$no-such", "XPST0008", "",
               "no hint when no shorter variable is in scope" );
  check_error( server, "let $x := 3 return $y", "XPST0008", "",
               "no hint on an undeclared name without -" );
}

void test_hints_nodes( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );
  std::string const record( dir_import +
    "let $d := dir:open($dir:root, \"orders/2026/q1.xml\")\n" );

  // JNSE0014: the record of an XML document, and a node.
  check_error( server, record + "return $d", "JNSE0014", "serialize(",
               "a record holding a node: hint" );
  check_error( server, "<a/>", "JNSE0014", "serialize(", "a node: hint" );
  check_error( server, "abs#1", "JNSE0014", "",
               "no node hint on a function item" );
  Json const r = call_tool( server, "run_query",
                            Json::object( { { "query", "<a/>" } } ) );
  check( field( tool_error( r ), "message" ) ==
         "can not serialize a node or function item as JSON",
         "the message is unchanged, the hint is separate", r.dump() );
  Json const s = call_tool( server, "run_query",
    Json::object( { { "query", "serialize(<a>t</a>)" } } ) );
  check( !is_tool_error( s ) && contains( result_text( s ), "<a>t<" ),
         "the hint's serialize() works", s.dump() );
  check_result( server, "string(<a>t</a>)", "\"t\"",
                "the hint's string() works" );

  // XPTY0019: a path over a JSON item.
  check_error( server, record + "return $d//order", "XPTY0019",
               "$d(\"content\")", "a path over an object: hint" );
  check_error( server, "let $a := jn:parse-json('[1]') return $a//b",
               "XPTY0019", "$a()", "a path over an array: hint" );
  check_error( server, "let $n := 1 return $n//b", "XPTY0019", "",
               "no JSON hint on a path over an atomic value" );
  check_result( server, record + "return count($d(\"content\")//order)", "2",
                "the hint's $d(\"content\")//order works" );

  // FORG0001: a string selector on an array.
  std::string const features(
    "let $a := jn:parse-json('[{ \"id\" : 7 }]')\n" );
  check_error( server, features + "return $a(\"id\")", "FORG0001",
               "$x()(\"id\")", "a key on an array: hint" );
  check_error( server, features + "return $a[1](\"id\")", "FORG0001",
               "$x()(\"id\")", "a key after a predicate: hint" );
  check_error( server, "\"abc\" cast as xs:integer", "FORG0001", "",
               "no array hint on another failed cast" );
  check_result( server, features + "return $a()(\"id\")", "7",
                "the hint's $x()(\"id\") works" );
  check_result( server, features + "return $a(1)(\"id\")", "7",
                "the hint's $x(1)(\"id\") works" );
}

void test_hints_names( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );
  std::string const math_ns( "http://www.w3.org/2005/xpath-functions/math" );

  // zorba-mcp predeclares math:, and a query can still override it.
  check_result( server, "math:pi() gt 3", "true", "math: is predeclared" );
  check_result( server, "import module namespace math = \"" + math_ns + "\";\n"
                "math:pi() gt 3", "true", "math: can still be imported" );
  check_result( server, "declare namespace math = \"urn:other\";\n"
                "namespace-uri(<math:a/>)", "\"urn:other\"",
                "math: can be bound to another namespace" );
  check_error( server, "map:merge(())", "XPST0081", "{|",
               "map: hint with the JSON alternative" );
  check_result( server,
                "count(jn:keys({| for $k in (\"a\", \"b\") "
                "return { $k : 1 } |}))", "2", "the hint's {| |} works" );
  check_error( server, "nosuchprefix:f(1)", "XPST0081", "",
               "no hint on an unknown prefix" );

  // Other arities of a known function.
  check_error( server, dir_import + "dir:open(\"a.xml\")", "XPST0017",
               "dir:open($root", "dir:open with one argument: signature" );
  check_error( server, dir_import + "dir:open(\"a.xml\")", "XPST0017",
               "$path", "dir:open with one argument: both parameters" );
  check_error( server, dir_import + "dir:documents(1, 2, 3)", "XPST0017",
               "dir:documents($root", "dir:documents: signature" );
  check_error( server, "fn:substring(1)", "XPST0017", "fn:substring(",
               "a builtin function: signature" );
  check_error( server, dir_import + "dir:nosuch(1)", "XPST0017", "",
               "no hint on an unknown module function" );
  check_error( server, "local:nosuch(1)", "XPST0017", "",
               "no hint on an unknown local function" );
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

  // 2^40 calls in a tree 40 deep. Bounded so that a query that retains the
  // memory of each call fails here instead of exhausting the machine.
  {
    AddressSpaceLimit const limit( 3000000000ULL );
    auto const start = std::chrono::steady_clock::now();
    r = call_tool( server, "run_query", Json::object( { { "query",
      "declare function local:f($n) { if ($n le 0) then 0 else "
      "local:f($n - 1) + local:f($n - 1) }; local:f(40)" } } ) );
    auto const elapsed = std::chrono::steady_clock::now() - start;
    check( is_tool_error( r ) &&
           contains( field( tool_error( r ), "code" ), "ZXQP0068" ),
           "recursion over the time limit: ZXQP0068", r.dump() );
    check( elapsed < std::chrono::seconds( 16 ),
           "recursion stops about when the time limit ends" );
  }

  // The server keeps answering.
  r = call_tool( server, "run_query",
                 Json::object( { { "query", "1 + 1" } } ) );
  check( !is_tool_error( r ) && result_text( r ) == "2",
         "the server answers after a timeout", r.dump() );
}


// Runs the zorba-mcp executable over a pipe with the given arguments,
// writes the input to it and returns everything it wrote to stdout.
std::string run_binary( Fixture const &f, std::string const &queries,
                        std::string const &input,
                        std::vector<std::string> const &extra_args = {} ) {
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
    std::vector<char const*> argv = {
      f.binary.c_str(), "--root", f.tree.c_str(), "--queries", queries.c_str()
    };
    for ( std::string const &arg : extra_args )
      argv.push_back( arg.c_str() );
    argv.push_back( nullptr );
    execv( f.binary.c_str(), const_cast<char *const*>( argv.data() ) );
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

// A query that runs out of memory under --max-memory is a tool error, and
// the server keeps answering. The limit is the address-space limit of the
// process, so the test needs Linux and no sanitizer.
void test_memory_limit( Fixture const &f ) {
#if defined(__linux__) && !defined(ZORBA_TEST_SANITIZER)
  auto call = [&]( int id, char const *query ) {
    return request( id, "tools/call", Json::object( {
      { "name", "run_query" },
      { "arguments", Json::object( { { "query", query } } ) } } ) ) + "\n";
  };
  char const *const hungry =
    "declare variable $m external := 4000000000;\n"
    "let $x := for $i in 1 to $m return string($i)\n"
    "return count($x) + count($x)";

  std::string input;
  input += call( 1, "1 + 1" );
  input += call( 2, hungry );
  input += call( 3, "1 + 2" );
  input += call( 4, hungry );
  input += call( 5, "1 + 3" );
  std::string const output( run_binary( f, f.queries, input, {
    "--max-memory", "1000000000", "--timeout", "60" } ) );

  std::map<std::string, Json> by_id;
  std::istringstream lines( output );
  std::string line;
  while ( std::getline( lines, line ) ) {
    Json r;
    std::string why;
    if ( parse_json( line, &r, &why ) && r.find( "id" ) )
      by_id[ r.find( "id" )->text() ] = r;
  }
  check( by_id.size() == 5, "five responses", output );
  if ( by_id.size() != 5 )
    return;
  check( !is_tool_error( by_id["1"] ) && result_text( by_id["1"] ) == "2",
         "a query under the limit runs", by_id["1"].dump() );
  for ( char const *const id : { "2", "4" } )
    check( is_tool_error( by_id[id] ) &&
           field( tool_error( by_id[id] ), "code" ) == "memory_limit" &&
           contains( field( tool_error( by_id[id] ), "message" ),
                     "--max-memory" ),
           "a query over the limit is a tool error", by_id[id].dump() );
  check( !is_tool_error( by_id["3"] ) && result_text( by_id["3"] ) == "3",
         "the server answers after the memory limit", by_id["3"].dump() );
  check( !is_tool_error( by_id["5"] ) && result_text( by_id["5"] ) == "4",
         "and again after a second time", by_id["5"].dump() );
#else
  (void)f;
  std::cout << "--max-memory has no effect in this build" << std::endl;
#endif
}

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

///////////////////////////////////////////////////////////////////////////////
// The language of a query: its own version declaration; else the extension of
// the file it is read from (.jq is JSONiq); else the configured default
// (XQuery unless --language jsoniq).

// A query that runs only in JSONiq: $x[] unboxes an array.
std::string const jsoniq_only( "let $x := [1, 2] return $x[]" );
std::string const xquery_decl( "xquery version \"3.0\";\n" );

std::string language_queries( Fixture const &f ) {
  return ( std::filesystem::path( f.queries ).parent_path() /
           "language_queries" ).string();
}

std::string run_query_description( Json const &list ) {
  Json const *const res = list.find( "result" );
  Json const *const tools = res ? res->find( "tools" ) : nullptr;
  if ( tools && tools->is_array() )
    for ( Json const &t : tools->items() )
      if ( field( t, "name" ) == "run_query" )
        return field( t, "description" );
  return std::string();
}

// The line of the error of a failed run_query.
std::string error_line( Server &server, std::string const &query ) {
  Json const r = call_tool( server, "run_query",
                            Json::object( { { "query", query } } ) );
  Json const e( tool_error( r ) );
  Json const *const loc = e.find( "location" );
  return loc ? field( *loc, "line" ) : std::string();
}

void test_language( Fixture const &f ) {
  for ( bool const jsoniq : { false, true } ) {
    ServerConfig config;
    config.root = f.tree;
    config.queries_dir = language_queries( f );
    config.jsoniq_default = jsoniq;
    Server server( f.zorba, config, std::cerr );
    std::string const mode( jsoniq ? "--language jsoniq: " : "default: " );

    // No declaration, no file: the configured default.
    if ( jsoniq )
      check_result( server, jsoniq_only, "1\n2", mode + "run_query is JSONiq" );
    else
      check_error( server, jsoniq_only, "XPST0003", "jsoniq version",
                   mode + "run_query is XQuery" );
    if ( !jsoniq )
      check_result( server, "(1, 2)[. eq 2]", "2",
                    mode + "run_query: the XQuery context item" );

    // The declaration wins, in both directions.
    check_result( server, jsoniq_decl + jsoniq_only, "1\n2",
                  mode + "jsoniq version wins" );
    check_error( server, xquery_decl + jsoniq_only, "XPST0003", "jn:members",
                 mode + "xquery version wins" );
    check_error( server, xquery_decl + "keys({ \"a\" : 1 })", "XPST0017",
                 "jn:keys(", mode + "xquery version: names resolve as XQuery" );
    check_result( server, jsoniq_decl + "keys({ \"a\" : 1 })", "\"a\"",
                  mode + "jsoniq version: names resolve as JSONiq" );

    // Error lines are those of the query, whatever the language.
    check( error_line( server, "(: one :)\n(: two :)\n1 div 0" ) == "3",
           mode + "runtime error line" );
    check( error_line( server, "(: one :)\n(: two :)\nlet $x := 1 return $x $x" )
             == "3", mode + "syntax error line" );
    check( error_line( server, jsoniq_decl + "(: one :)\n1 div 0" ) == "3",
           mode + "runtime error line after a declaration" );

    // A .jq file is JSONiq whatever the default; its declaration wins.
    Json r = call_tool( server, "jq_plain", Json::object() );
    check( !is_tool_error( r ) && result_text( r ) == "1\n2",
           mode + "a .jq predefined query is JSONiq", r.dump() );
    r = call_tool( server, "jq_jsoniq_header", Json::object() );
    check( !is_tool_error( r ) && result_text( r ) == "1\n2",
           mode + "a .jq predefined query with jsoniq version", r.dump() );
    r = call_tool( server, "jq_xquery_header", Json::object() );
    check( !is_tool_error( r ) && result_text( r ) == "2",
           mode + "a .jq predefined query with xquery version is XQuery",
           r.dump() );

    // The description of run_query says which language applies.
    std::string const description(
      run_query_description( roundtrip( server, request( 1, "tools/list" ) ) ) );
    std::string const expected( jsoniq
      ? "Queries are JSONiq by default; start with `xquery version \"3.0\";` "
        "for XQuery."
      : "Queries are XQuery by default; start with `jsoniq version \"1.0\";` "
        "for JSONiq." );
    check( contains( description, expected ),
           mode + "run_query describes the language", description );
    std::cout << "run_query description (" << description.size()
              << " bytes): " << description << std::endl;
  }
}

// The same over the zorba-mcp executable and its --language option.
void test_language_binary( Fixture const &f ) {
  auto call = [&]( int id, std::string const &tool, Json args ) {
    return request( id, "tools/call", Json::object( {
      { "name", tool }, { "arguments", std::move( args ) } } ) ) + "\n";
  };
  std::string input;
  input += call( 1, "run_query",
                 Json::object( { { "query", jsoniq_only } } ) );
  input += call( 2, "run_query", Json::object( {
    { "query", xquery_decl + jsoniq_only } } ) );
  input += call( 3, "jq_plain", Json::object() );
  input += request( 4, "tools/list" ) + "\n";

  for ( bool const jsoniq : { false, true } ) {
    std::string const output( run_binary( f, language_queries( f ), input,
      jsoniq ? std::vector<std::string>{ "--language", "jsoniq" }
             : std::vector<std::string>{ "--language", "xquery" } ) );
    std::map<std::string,Json> by_id;
    std::istringstream lines( output );
    std::string line;
    while ( std::getline( lines, line ) ) {
      Json r;
      std::string why;
      if ( parse_json( line, &r, &why ) && r.find( "id" ) )
        by_id[ r.find( "id" )->text() ] = r;
    }
    std::string const mode( jsoniq ? "--language jsoniq: " : "--language xquery: " );
    check( is_tool_error( by_id["1"] ) != jsoniq &&
           ( !jsoniq || result_text( by_id["1"] ) == "1\n2" ),
           mode + "run_query follows the option", by_id["1"].dump() );
    check( is_tool_error( by_id["2"] ) &&
           contains( result_text( by_id["2"] ), "XPST0003" ),
           mode + "xquery version wins over the option", by_id["2"].dump() );
    check( !is_tool_error( by_id["3"] ) && result_text( by_id["3"] ) == "1\n2",
           mode + "a .jq predefined query is JSONiq", by_id["3"].dump() );
    check( contains( run_query_description( by_id["4"] ),
             jsoniq ? "Queries are JSONiq by default;"
                    : "Queries are XQuery by default;" ),
           mode + "run_query describes the language", by_id["4"].dump() );
  }
}

// The README states what a query cannot do under zorba-mcp. These tests hold
// each statement: no write, no read outside the root through any function,
// no network, and no native module other than the file module.
std::string const file_import(
  "import module namespace file = \"http://expath.org/ns/file\";\n" );

// Names of the entries of a directory, sorted, with their contents.
std::string snapshot( std::string const &dir ) {
  std::vector<std::string> entries;
  for ( auto const &e : std::filesystem::recursive_directory_iterator( dir ) ) {
    std::string s( e.path().string() );
    if ( e.is_regular_file() && !e.is_symlink() )
      s += "=" + read_file( e.path().string() );
    entries.push_back( s );
  }
  std::sort( entries.begin(), entries.end() );
  std::string out;
  for ( std::string const &s : entries )
    out += s + "\n";
  return out;
}

void test_no_write( Fixture const &f ) {
  TempTree t;
  t.write( "keep.txt", "KEEP" );
  {
    std::ofstream out( ( t.base + "/outside/other.txt" ).c_str() );
    out << "OTHER";
  }
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  std::string const before( snapshot( t.base ) );
  std::string const r( t.root ), o( t.base + "/outside" );

  std::string const queries[] = {
    file_import + "file:write-text(\"" + r + "/new.txt\", \"x\")",
    file_import + "file:write-text(\"" + o + "/new.txt\", \"x\")",
    file_import + "file:write-text(\"" + r + "/keep.txt\", \"CHANGED\")",
    file_import + "file:write-text-lines(\"" + r + "/new.txt\", (\"a\", \"b\"))",
    file_import + "file:write-binary(\"" + r + "/new.bin\", xs:base64Binary(\"AAAA\"))",
    file_import + "file:append-text(\"" + r + "/keep.txt\", \"x\")",
    file_import + "file:append-text-lines(\"" + r + "/keep.txt\", \"x\")",
    file_import + "file:append-binary(\"" + r + "/keep.txt\", xs:base64Binary(\"AAAA\"))",
    file_import + "file:delete(\"" + r + "/keep.txt\")",
    file_import + "file:delete(\"" + o + "/other.txt\")",
    file_import + "file:create-directory(\"" + r + "/newdir\")",
    file_import + "file:copy(\"" + r + "/keep.txt\", \"" + r + "/copy.txt\")",
    file_import + "file:copy(\"" + r + "/keep.txt\", \"" + o + "/copy.txt\")",
    file_import + "file:move(\"" + r + "/keep.txt\", \"" + r + "/moved.txt\")",
    file_import + "file:move(\"" + r + "/keep.txt\", \"" + o + "/moved.txt\")",
    "put(<a/>, \"file://" + r + "/put.xml\")",
    "put(<a/>, \"file://" + o + "/put.xml\")",
    "put(<a/>, \"put.xml\")"
  };
  for ( std::string const &q : queries ) {
    Json const res = run_query( server, q );
    // fn:put stores the document in the engine's memory, not on disk, so it
    // is accepted; the disk check below is what holds for it.
    if ( q.compare( 0, 3, "put" ) != 0 )
      check( is_tool_error( res ), "write is refused: " + q, res.dump() );
    check( snapshot( t.base ) == before, "nothing changed on disk: " + q );
  }
  check_alive( server );
}

void test_read_functions( Fixture const &f ) {
  TempTree t;
  t.write( "inside.txt", "INSIDE-VALUE" );
  {
    std::ofstream out( ( t.base + "/outside/secret.txt" ).c_str() );
    out << "TOP-SECRET";
  }
  std::filesystem::create_symlink( t.base + "/outside/secret.txt",
                                   t.root + "/link.txt" );
  ServerConfig config;
  config.root = t.root;
  Server server( f.zorba, config, std::cerr );
  std::string const r( t.root ), o( t.base + "/outside" );
  std::string const fetch_import(
    "import module namespace fetch = \"http://zorba.io/modules/fetch\";\n" );

  // Controls: the same functions read a file under the root.
  std::string const allowed[] = {
    "unparsed-text(\"file://" + r + "/inside.txt\")",
    "unparsed-text(\"inside.txt\")",
    file_import + "file:read-text(\"" + r + "/inside.txt\")"
  };
  for ( std::string const &q : allowed ) {
    Json const res = run_query( server, q );
    check( !is_tool_error( res ) && contains( result_text( res ), "INSIDE-VALUE" ),
           "reads under the root: " + q, res.dump() );
  }

  std::string const denied[] = {
    "unparsed-text(\"file://" + o + "/secret.txt\")",
    "unparsed-text(\"../outside/secret.txt\")",
    "unparsed-text-lines(\"file://" + o + "/secret.txt\")",
    "unparsed-text(\"file://" + r + "/link.txt\")",
    "doc(\"file://" + o + "/secret.txt\")",
    "doc(\"file://" + r + "/link.txt\")",
    "doc(\"../outside/secret.txt\")",
    file_import + "file:read-text(\"" + o + "/secret.txt\")",
    file_import + "file:read-text(\"" + r + "/link.txt\")",
    file_import + "file:read-text(\"../outside/secret.txt\")",
    file_import + "file:read-text-lines(\"" + o + "/secret.txt\")",
    file_import + "file:read-binary(\"" + o + "/secret.txt\")",
    file_import + "file:list(\"" + o + "\")",
    file_import + "file:size(\"" + o + "/secret.txt\")",
    file_import + "file:copy(\"" + o + "/secret.txt\", \"" + r + "/c.txt\")",
    fetch_import + "fetch:content(\"file://" + o + "/secret.txt\")",
    fetch_import + "fetch:content(\"file://" + r + "/link.txt\")",
    fetch_import + "fetch:content-binary(\"file://" + o + "/secret.txt\")"
  };
  for ( std::string const &q : denied ) {
    Json const res = run_query( server, q );
    check( is_tool_error( res ) && !contains( res.dump(), "TOP-SECRET" ),
           "read outside the root is refused: " + q, res.dump() );
  }
  check( !std::filesystem::exists( t.root + "/c.txt" ), "no copy was made" );

  // unparsed-text-available answers false, and leaks nothing.
  Json const res = run_query( server,
    "unparsed-text-available(\"file://" + o + "/secret.txt\")" );
  check( !contains( res.dump(), "TOP-SECRET" ) &&
         ( is_tool_error( res ) || result_text( res ) == "false" ),
         "unparsed-text-available outside the root", res.dump() );

  // No network from the functions that read a URI.
  for ( char const *const q : {
          "unparsed-text(\"http://127.0.0.1:9/a.txt\")",
          "unparsed-text(\"https://127.0.0.1:9/a.txt\")",
          "unparsed-text-lines(\"http://127.0.0.1:9/a.txt\")",
          "doc(\"https://127.0.0.1:9/a.xml\")",
          "doc(\"ftp://127.0.0.1:9/a.xml\")",
          "import module namespace fetch = \"http://zorba.io/modules/fetch\";\n"
          "fetch:content(\"http://127.0.0.1:9/a.txt\")" } ) {
    Json const n = run_query( server, q );
    check( is_tool_error( n ) &&
           contains( n.dump(), "access denied by security policy" ),
           std::string( "network read is refused: " ) + q, n.dump() );
  }
  check_alive( server );
}

// The native modules other than the file module do not load: the HTTP and
// FTP clients cannot be reached. (util-curl declares no function; fetch,
// random and reflection are implemented in the engine, not in a library, and
// check the policy when called: see read_functions.)
void test_native_import( Fixture const &f ) {
  ServerConfig config;
  config.root = f.tree;
  Server server( f.zorba, config, std::cerr );
  for ( char const *const ns : {
          "http://zorba.io/modules/http-client",
          "http://www.zorba-xquery.com/modules/http-client",
          "http://zorba.io/modules/http-client-wrapper",
          "http://zorba.io/modules/ftp-client",
          "http://zorba.io/modules/sleep" } ) {
    Json const r = run_query( server, std::string( "import module namespace m = \"" ) +
                                      ns + "\";\n\"loaded\"" );
    check( is_tool_error( r ) && !contains( result_text( r ), "loaded" ),
           std::string( "module import is refused: " ) + ns, r.dump() );
    check( is_tool_error( r ) &&
           ( contains( field( tool_error( r ), "code" ), "ZXQP0067" ) ||
             contains( r.dump(), "access denied by security policy" ) ),
           std::string( "refused by the policy: " ) + ns, r.dump() );
  }
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
  { "memory_limit", test_memory_limit },
  { "optional_params", test_optional_params },
  { "documents_stream", test_documents_stream },
  { "hints_jsoniq", test_hints_jsoniq },
  { "hints_dash", test_hints_dash },
  { "hints_nodes", test_hints_nodes },
  { "hints_names", test_hints_names },
  { "deep_json", test_deep_json },
  { "deep_json_binary", test_deep_json_binary },
  { "entity_expansion", test_entity_expansion },
  { "case_paths", test_case_paths },
  { "html_edge_cases", test_html_edge_cases },
  { "language", test_language },
  { "language_binary", test_language_binary },
  { "no_write", test_no_write },
  { "read_functions", test_read_functions },
  { "native_import", test_native_import }
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
