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

#include "mcp_server.h"
#include "query_primer.h"
#include "scan_preview.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <utility>
#include <vector>

#include <zorba/version.h>

namespace zorba {
namespace mcp {

using db::parse_json;

char const protocol_version[] = "2026-07-28";

std::size_t const max_message_size = 4 * 1024 * 1024;

char const *const legacy_protocol_versions[] = {
  "2025-11-25", "2025-06-18", "2025-03-26", nullptr
};

namespace {

// JSON-RPC 2.0 and MCP error codes.
int const PARSE_ERROR = -32700;
int const INVALID_REQUEST = -32600;
int const INTERNAL_ERROR = -32603;
int const METHOD_NOT_FOUND = -32601;
int const INVALID_PARAMS = -32602;
int const UNSUPPORTED_PROTOCOL_VERSION = -32022;


char const meta_version[] = "io.modelcontextprotocol/protocolVersion";
char const meta_capabilities[] = "io.modelcontextprotocol/clientCapabilities";
char const meta_server_info[] = "io.modelcontextprotocol/serverInfo";

char const server_name[] = "zorba-mcp";

// The JSONiq primer is only for a session whose queries are JSONiq by default.
// Its examples mean something else in XQuery, or nothing.
std::string instructions( bool jsoniq_default ) {
  return std::string(
  "Queries a directory of XML, JSON, CSV and HTML documents with JSONiq/XQuery "
  "(Zorba). Start with list_documents. In run_query, import the directory "
  "module (import module namespace dir = \"http://zorba.io/modules/directory\";) "
  "and use $dir:root: dir:documents($dir:root, \"xml\") or "
  "dir:documents($dir:root, (\"json\", \"csv\")) return records "
  "{path, format, content} (or {path, format, error}); "
  "dir:open($dir:root, $path) returns one. " ) +
  ( jsoniq_default ? "Read the JSONiq primer in the run_query description "
                     "before writing a query. " : "" ) +
  "Results are JSON, one item per "
  "line; return JSON objects, arrays or atomic values, not XML nodes "
  "(convert with string() or data()). Only files under the directory can "
  "be read; network access, writes and environment variables are denied. "
  "Predefined query tools, when present, answer common questions directly.";
}

Json server_info() {
  return Json::object( {
    { "name", server_name },
    { "version", Version::getVersion() }
  } );
}

Json error_response( Json const &id, int code, std::string const &message,
                     Json data = Json() ) {
  Json error( Json::object( { { "code", code }, { "message", message } } ) );
  if ( !data.is_null() )
    error.set( "data", std::move( data ) );
  return Json::object( {
    { "jsonrpc", "2.0" }, { "id", id }, { "error", std::move( error ) }
  } );
}

Json all_versions() {
  Json versions( Json::array() );
  versions.push( protocol_version );
  return versions;
}

// Drops an incomplete UTF-8 sequence at the end of a truncated text.
void trim_utf8_tail( std::string *s ) {
  std::size_t const n = s->size();
  for ( std::size_t back = 1; back <= 4 && back <= n; ++back ) {
    unsigned char const b = static_cast<unsigned char>( (*s)[ n - back ] );
    if ( ( b & 0xC0 ) == 0x80 )
      continue;                         // continuation byte
    std::size_t len = 1;
    if ( ( b & 0xE0 ) == 0xC0 ) len = 2;
    else if ( ( b & 0xF0 ) == 0xE0 ) len = 3;
    else if ( ( b & 0xF8 ) == 0xF0 ) len = 4;
    if ( len > back )
      s->resize( n - back );
    return;
  }
}

// Collects a query's output up to a limit, counting what goes beyond it; at the
// end, a truncated text gets a marker.
class CappedSink : public db::ResultSink {
public:
  explicit CappedSink( std::size_t cap ) : theCap( cap ), theTotal( 0 ) { }

  void write( char const *s, std::size_t len ) override {
    // Counted after the append: an append that cannot allocate is not output
    // beyond the limit (and std::bad_alloc fails the query).
    if ( theData.size() < theCap )
      theData.append( s, std::min( len, theCap - theData.size() ) );
    theTotal += len;
  }

  void end() override {
    if ( theTotal <= theData.size() )
      return;
    trim_utf8_tail( &theData );
    theData += "\n[zorba-mcp: output truncated after " +
      std::to_string( theData.size() ) + " of " +
      std::to_string( theTotal ) + " bytes; narrow the query (fewer "
      "fields, a filter, or subsequence()) to see the rest]";
  }

  std::string take() { return std::move( theData ); }

private:
  std::size_t const theCap;
  std::size_t theTotal;
  std::string theData;
};

inline bool is_utf8_continuation( char c ) {
  return ( static_cast<unsigned char>( c ) & 0xC0 ) == 0x80;
}

// A text with its middle replaced by a marker, within \a cap bytes if the
// marker fits. The cuts fall between UTF-8 characters.
std::string cut_middle( std::string const &text, std::size_t cap,
                        char const *what ) {
  if ( text.size() <= cap )
    return text;
  std::string const marker( std::string( "[zorba-mcp: " ) + what + " cut, " +
                            std::to_string( text.size() ) + " bytes in all]" );
  std::size_t const room = cap > marker.size() ? cap - marker.size() : 0;
  std::size_t head = room / 2;
  std::size_t tail = text.size() - ( room - head );
  while ( head && is_utf8_continuation( text[ head ] ) )
    --head;
  while ( tail < text.size() && is_utf8_continuation( text[ tail ] ) )
    ++tail;
  return text.substr( 0, head ) + marker + text.substr( tail );
}

// A line of a query as it goes in an error: a long one, as the bytes around the
// column of the error and a marker with its size. A column is 16 bits in the
// engine: past that it says nothing about the place, so a longer line shows its
// two ends.
std::string line_excerpt( std::string const &line, unsigned long column ) {
  std::size_t const around = 256;
  if ( line.size() <= 2 * around )
    return line;
  std::string const marker( " [zorba-mcp: line of " +
                            std::to_string( line.size() ) + " bytes]" );
  std::string const dots( "\xE2\x80\xA6" );
  if ( line.size() > 65535 ) {
    std::size_t head = around, tail = line.size() - around;
    while ( head && is_utf8_continuation( line[ head ] ) )
      --head;
    while ( tail < line.size() && is_utf8_continuation( line[ tail ] ) )
      ++tail;
    return line.substr( 0, head ) + dots + line.substr( tail ) + marker;
  }
  std::size_t const at =
    std::min( static_cast<std::size_t>( column ? column - 1 : 0 ), line.size() );
  std::size_t begin = at > around ? at - around : 0;
  std::size_t end = std::min( at + around, line.size() );
  while ( begin < end && is_utf8_continuation( line[ begin ] ) )
    ++begin;
  while ( end > begin && end < line.size() &&
          is_utf8_continuation( line[ end ] ) )
    --end;
  return ( begin ? dots : std::string() ) + line.substr( begin, end - begin ) +
    ( end < line.size() ? dots : std::string() ) + marker;
}

std::string out_of_memory_text( unsigned long long max_memory ) {
  return Json::object( { { "error", Json::object( {
    { "code", "memory_limit" },
    { "message", "the query ran out of memory (server limit --max-memory: " +
      ( max_memory ? std::to_string( max_memory ) + " bytes" :
                     std::string( "none" ) ) +
      "); narrow the query, or start the server with a higher limit" }
  } ) } } ).dump();
}

// The longest hint that goes whole. A hint can repeat names from the query.
std::size_t const max_hint_bytes = 4096;

// The text of a tool error for a query that failed.
std::string error_text( db::QueryError const &e, std::size_t max_output,
                        unsigned long long max_memory ) {
  if ( e.kind == db::QueryError::Kind::memory_limit )
    return out_of_memory_text( max_memory );
  if ( e.kind == db::QueryError::Kind::internal )
    return Json::object( { { "error", Json::object( {
      { "code", "internal" },
      { "message", cut_middle( e.message, max_output, "message" ) }
    } ) } } ).dump();
  Json err( Json::object( {
    { "code", e.code },
    { "namespace", e.ns },
    { "message", cut_middle( e.message, max_output, "message" ) }
  } ) );
  if ( !e.hint.empty() )
    err.set( "hint", cut_middle( e.hint, max_hint_bytes, "hint" ) );
  if ( e.location ) {
    db::SourceLocation const &l = *e.location;
    Json loc( Json::object() );
    if ( !l.uri.empty() )
      loc.set( "uri", l.uri );
    loc.set( "line", static_cast<unsigned long long>( l.line ) );
    loc.set( "column", static_cast<unsigned long long>( l.column ) );
    if ( l.line_end ) {
      loc.set( "lineEnd", static_cast<unsigned long long>( l.line_end ) );
      loc.set( "columnEnd", static_cast<unsigned long long>( l.column_end ) );
    }
    if ( !l.text.empty() )
      loc.set( "text", line_excerpt( l.text, l.column ) );
    err.set( "location", std::move( loc ) );
  }
  return Json::object( { { "error", std::move( err ) } } ).dump();
}

// The names of the arguments that the tool's inputSchema does not declare
// ("additionalProperties": false), as "a, b"; empty when there are none.
std::string undeclared_arguments( Json const &args,
                                  std::vector<std::string> const &declared ) {
  std::string names;
  for ( auto const &member : args.members() ) {
    if ( std::find( declared.begin(), declared.end(), member.first ) ==
         declared.end() )
      names += ( names.empty() ? "" : ", " ) + member.first;
  }
  return names;
}

std::string invalid_arguments_text( std::string const &message ) {
  return Json::object( { { "error", Json::object( {
    { "code", "invalid-arguments" }, { "message", message }
  } ) } } ).dump();
}

Json text_content( std::string const &text ) {
  Json content( Json::array() );
  content.push( Json::object( { { "type", "text" }, { "text", text } } ) );
  return content;
}

Json read_only_annotations() {
  return Json::object( {
    { "readOnlyHint", true },
    { "destructiveHint", false },
    { "idempotentHint", true },
    { "openWorldHint", false }
  } );
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

namespace {

db::DatabaseConfig database_config( ServerConfig const &config ) {
  db::DatabaseConfig c;
  c.root = config.root;
  c.queries_dir = config.queries_dir;
  c.jsoniq_default = config.jsoniq_default;
  c.timeout = config.timeout;
  c.cache_max_file_kb = config.cache_max_file_kb;
  c.cache_max_files = config.cache_max_files;
  c.before_execute = config.before_execute;
  c.cache_after_read = config.cache_after_read;
  return c;
}

} // namespace

Server::Server( Zorba *zorba, ServerConfig config, std::ostream &log ) :
  theConfig( std::move( config ) ),
  theLog( log ),
  theDatabase( zorba, database_config( theConfig ), log ) {
  if ( theConfig.scan ) {
    try {
      theScanner = std::make_unique<db::Scanner>(
        theConfig.root, theConfig.scan_limits, theConfig.scan_hook );
    }
    // Threads may not start under --max-memory, which is applied before this:
    // the server then works as with --no-scan.
    catch ( std::exception const &e ) {
      theLog << server_name << ": no background scan: " << e.what() << std::endl;
    }
  }
}

Server::~Server() {
  (void)stop_scan();
  if ( theScanner )
    theLog << server_name << ": the scan cache holds "
           << theScanner->cache_summary() << std::endl;
}

bool Server::stop_scan() {
  return !theScanner || theScanner->stop();
}

std::size_t Server::scan_cache_bytes() const {
  return theScanner ? theScanner->cache_bytes() : 0;
}

Json Server::tools() const {
  Json list( Json::array() );

  if ( theScanner ) {
    list.push( Json::object( {
      { "name", "list_documents" },
      { "title", "List documents" },
      { "description",
        "Lists files, recursive, sorted by path: format, size, top-level "
        "fields. Line 1: scan state, totals, dirs (subdirectories, if cut or "
        "scanning), incomplete (may miss files). memory_limit: scan stopped; "
        "ask by path. A file as path adds a preview (nested fields)." },
      { "inputSchema", Json::object( {
          { "type", "object" },
          { "properties", Json::object( {
              { "path", Json::object( {
                  { "type", "string" },
                  { "description", "Subdirectory or file." } } ) },
              { "limit", Json::object( {
                  { "type", "integer" },
                  { "description", "Max files; default 100." } } ) },
              { "refresh", Json::object( {
                  { "type", "boolean" },
                  { "description", "Restart the scan." } } ) }
            } ) },
          { "additionalProperties", false }
        } ) },
      { "annotations", read_only_annotations() }
    } ) );
  } else {
    Json no_args( Json::object( {
      { "type", "object" }, { "additionalProperties", false }
    } ) );
    list.push( Json::object( {
      { "name", "list_documents" },
      { "title", "List documents" },
      { "description",
        "Lists the files under the directory, recursively and sorted by path: "
        "one JSON object per line, {path, format, size, modified}. The format "
        "(xml, json, csv, html or null) says how run_query's dir:documents "
        "and dir:open parse the file." },
      { "inputSchema", no_args },
      { "annotations", read_only_annotations() }
    } ) );
  }

  Json query_props( Json::object( {
    { "query", Json::object( {
        { "type", "string" },
        { "description",
          "A JSONiq/XQuery main module. Import the directory module "
          "(http://zorba.io/modules/directory) and use $dir:root." }
      } ) }
  } ) );
  Json query_required( Json::array() );
  query_required.push( "query" );
  list.push( Json::object( {
    { "name", "run_query" },
    { "title", "Run a JSONiq/XQuery query" },
    { "description",
      "Runs a JSONiq/XQuery query over the directory and returns its result "
      "as JSON, one item per line. " + std::string(
        theConfig.jsoniq_default
          ? "Queries are JSONiq by default; start with `xquery version "
            "\"3.0\";` for XQuery."
          : "Queries are XQuery by default; start with `jsoniq version "
            "\"1.0\";` for JSONiq." ) +
      " Example: import module namespace dir = "
      "\"http://zorba.io/modules/directory\"; for $d in "
      "dir:documents($dir:root, \"json\") return $d(\"path\"). " +
      ( theConfig.jsoniq_default ? std::string( query_primer ) + " "
                                 : std::string() ) +
      "On failure "
      "the result is an error object with the Zorba error code, message "
      "and the line/column of the failing expression." },
    { "inputSchema", Json::object( {
        { "type", "object" },
        { "properties", query_props },
        { "required", query_required },
        { "additionalProperties", false }
      } ) },
    { "annotations", read_only_annotations() }
  } ) );

  for ( db::PredefinedQuery const &p : theDatabase.predefined() ) {
    Json props( Json::object() );
    Json required( Json::array() );
    for ( db::PredefinedQuery::Param const &param : p.params ) {
      props.set( param.name, Json::object( {
        { "type", "string" },
        { "description", "The value of $" + param.name + " (a string)." }
      } ) );
      if ( param.required )
        required.push( param.name );
    }
    Json schema( Json::object( {
      { "type", "object" }, { "properties", props }
    } ) );
    if ( !required.items().empty() )
      schema.set( "required", required );
    schema.set( "additionalProperties", false );
    list.push( Json::object( {
      { "name", p.name },
      { "description", p.description.empty()
          ? "Runs the predefined query " +
            std::filesystem::path( p.path ).filename().string() + "."
          : p.description },
      { "inputSchema", schema },
      { "annotations", read_only_annotations() }
    } ) );
  }
  return list;
}

template<typename Run>
Server::QueryOutcome Server::run( Run const &query ) {
  CappedSink sink( theConfig.max_output );
  db::QueryResult const result( query( sink ) );
  QueryOutcome outcome;
  outcome.ok = result.ok;
  outcome.text = result.ok ? sink.take()
    : error_text( result.error, theConfig.max_output, theConfig.max_memory );
  return outcome;
}

std::string Server::list_documents( db::Scanner &scanner, Json const &args,
                                    bool *is_error ) {
  // The shape of the scanner's own errors, with the status after the error.
  auto fail = [&]( std::string const &message ) {
    *is_error = true;
    std::string out( "{\"error\":{\"code\":\"invalid-arguments\",\"message\":" );
    db::append_json_string( out, db::scan_valid_utf8( message ) );
    return out + "}," + scanner.status() + "}";
  };
  for ( auto const &m : args.members() )
    if ( m.first != "path" && m.first != "limit" && m.first != "refresh" )
      return fail( "list_documents has no argument " + m.first );
  db::ListRequest request;
  if ( Json const *const p = args.find( "path" ) ) {
    if ( !p->is_string() )
      return fail( "path must be a string" );
    request.path = p->text();
  }
  if ( Json const *const l = args.find( "limit" ) ) {
    if ( !l->is_integer() || l->text()[0] == '-' )
      return fail( "limit must be an integer of 0 or more" );
    request.limit =
      static_cast<std::size_t>( std::strtoull( l->text().c_str(), nullptr, 10 ) );
  }
  if ( Json const *const r = args.find( "refresh" ) ) {
    if ( !r->is_bool() )
      return fail( "refresh must be a boolean" );
    request.refresh = r->as_bool();
  }
  // A request does not wait past its time limit, nor long without one.
  std::chrono::milliseconds const longest( std::chrono::seconds( 10 ) );
  std::chrono::milliseconds const wait(
    theConfig.timeout.count() > 0 ? std::min( theConfig.timeout, longest )
                                  : longest );
  return scanner.answer( request, theConfig.max_output, wait, is_error );
}

Json Server::tool_result( QueryOutcome const &outcome ) const {
  return Json::object( {
    { "content", text_content( outcome.text ) },
    { "isError", !outcome.ok }
  } );
}

Json Server::call_tool( Json const &params, bool *protocol_error,
                        Json *error ) {
  *protocol_error = false;
  Json const *const name = params.find( "name" );
  if ( !name || !name->is_string() ) {
    *protocol_error = true;
    *error = Json( "Invalid params: \"name\" must be a string" );
    return Json();
  }
  Json const *args = params.find( "arguments" );
  Json const empty( Json::object() );
  if ( !args || args->is_null() )
    args = &empty;
  if ( !args->is_object() ) {
    *protocol_error = true;
    *error = Json( "Invalid params: \"arguments\" must be an object" );
    return Json();
  }

  std::string const &tool = name->text();
  QueryOutcome outcome;
  std::string unexpected;
  if ( tool == "list_documents" && theScanner ) {
    bool is_error = false;
    outcome.text = list_documents( *theScanner, *args, &is_error );
    outcome.ok = !is_error;
  } else if ( tool == "list_documents" ) {
    unexpected = undeclared_arguments( *args, {} );
    if ( !unexpected.empty() ) {
      outcome.text = invalid_arguments_text(
        "list_documents takes no arguments; unexpected: " + unexpected );
    } else {
      outcome = run( [&]( db::ResultSink &sink ) {
        return theDatabase.list_files( sink );
      } );
    }
  } else if ( tool == "run_query" ) {
    Json const *const q = args->find( "query" );
    unexpected = undeclared_arguments( *args, { "query" } );
    if ( !unexpected.empty() ) {
      outcome.text = invalid_arguments_text(
        "run_query takes only the argument \"query\"; unexpected: " +
        unexpected );
    } else if ( !q || !q->is_string() ) {
      outcome.text = Json::object( { { "error", Json::object( {
        { "code", "invalid-arguments" },
        { "message", "run_query requires the argument \"query\" (a string)" }
      } ) } } ).dump();
    } else {
      outcome = run( [&]( db::ResultSink &sink ) {
        return theDatabase.run( q->text(), sink );
      } );
    }
  } else {
    db::PredefinedQuery const *const p = theDatabase.find_predefined( tool );
    if ( !p ) {
      *protocol_error = true;
      *error = Json( "Unknown tool: " + tool );
      return Json();
    }
    db::Variables vars;
    std::string missing;
    std::vector<std::string> declared;
    for ( db::PredefinedQuery::Param const &param : p->params )
      declared.push_back( param.name );
    unexpected = undeclared_arguments( *args, declared );
    for ( db::PredefinedQuery::Param const &param : p->params ) {
      Json const *const v = args->find( param.name );
      if ( !v || v->is_null() ) {
        if ( param.required )
          missing += ( missing.empty() ? "" : ", " ) + param.name;
        continue;
      }
      if ( v->is_string() || v->is_number() )
        vars.emplace_back( param.name, v->text() );
      else if ( v->is_bool() )
        vars.emplace_back( param.name, v->as_bool() ? "true" : "false" );
      else
        missing += ( missing.empty() ? "" : ", " ) + param.name +
          " (must be a string)";
    }
    if ( !unexpected.empty() ) {
      outcome.text = invalid_arguments_text(
        tool + " has no argument(s): " + unexpected );
    } else if ( !missing.empty() ) {
      outcome.text = invalid_arguments_text(
        tool + " requires the arguments: " + missing );
    } else {
      outcome = run( [&]( db::ResultSink &sink ) {
        return theDatabase.run_predefined( tool, vars, sink );
      } );
    }
  }
  theLog << server_name << ": tools/call " << tool << ": "
         << ( outcome.ok ? "ok" : "error" ) << std::endl;
  return tool_result( outcome );
}

std::string Server::handle( std::string const &message ) {
  Json request_id;
  theCurrentId = "null";
  theCurrentKind = MessageKind::unknown;
  try {
    return handle_message( message, &request_id );
  }
  catch ( std::exception const &e ) {
    // Whatever it was (out of memory, most likely), the next request may work.
    // If it came before or while the id was read, the message is read once more
    // for it: the memory of the first attempt is back by now.
    if ( theCurrentKind == MessageKind::unknown ||
         theCurrentKind == MessageKind::id_lost ) {
      try {
        Json msg;
        std::string why;
        if ( message.size() <= max_message_size &&
             parse_json( message, &msg, &why ) && msg.is_object() )
          identify( msg, &request_id );
        else
          theCurrentKind = MessageKind::without_id;
      }
      catch ( std::bad_alloc const& ) {
        theCurrentKind = MessageKind::id_lost;
      }
    }
    if ( theCurrentKind == MessageKind::notification )
      return std::string();
    // An id that could not be read is null in the response (JSON-RPC 2.0, section
    // 5); theCurrentId is still "null" then.
    if ( theCurrentKind == MessageKind::id_lost ||
         theCurrentKind == MessageKind::unknown )
      request_id = Json();
    return error_response( request_id, INTERNAL_ERROR,
      std::string( "Internal error: " ) + e.what() ).dump();
  }
}

void Server::identify( Json const &msg, Json *request_id ) {
  Json const *const id = msg.find( "id" );
  if ( !id ) {
    // A notification, or a response from the client: never answered.
    theCurrentKind = MessageKind::notification;
    return;
  }
  if ( !id->is_string() && !id->is_integer() ) {
    theCurrentKind = MessageKind::without_id;
    return;
  }
  theCurrentKind = MessageKind::id_lost;   // until the id is copied
  *request_id = *id;
  theCurrentId = id->dump();
  theCurrentKind = MessageKind::request;
}

Server::AfterFailure Server::after_failure() const {
  return theCurrentKind == MessageKind::notification ? AfterFailure::nothing
                                                     : AfterFailure::answer;
}

std::string Server::handle_message( std::string const &message,
                                    Json *request_id ) {
  Json const null_id;
  if ( message.size() > max_message_size ) {
    theCurrentKind = MessageKind::without_id;
    return error_response( null_id, INVALID_REQUEST,
                           "Invalid Request: message too large" ).dump();
  }

  Json msg;
  std::string why;
  if ( !parse_json( message, &msg, &why ) ) {
    theCurrentKind = MessageKind::without_id;
    return error_response( null_id, PARSE_ERROR, "Parse error: " + why ).dump();
  }
  if ( !msg.is_object() ) {
    theCurrentKind = MessageKind::without_id;
    return error_response( null_id, INVALID_REQUEST,
      "Invalid Request: expected a JSON-RPC object (batches are not "
      "supported)" ).dump();
  }

  identify( msg, request_id );
  Json const *const id = msg.find( "id" );
  Json const *const method = msg.find( "method" );
  if ( !method ) {
    // A response from the client (we send no requests): nothing to answer.
    if ( !id || msg.find( "result" ) || msg.find( "error" ) )
      return std::string();
  }
  bool const valid_id = id && ( id->is_string() || id->is_integer() );
  if ( id && !valid_id )
    return error_response( null_id, INVALID_REQUEST,
      "Invalid Request: id must be a string or an integer" ).dump();
  Json const *const jsonrpc = msg.find( "jsonrpc" );
  bool const well_formed = jsonrpc && jsonrpc->is_string() &&
    jsonrpc->text() == "2.0" && method && method->is_string();
  if ( !id ) {
    // A notification (e.g. notifications/initialized, notifications/
    // cancelled): never answered, even if malformed.
    return std::string();
  }
  if ( !well_formed )
    return error_response( *id, INVALID_REQUEST,
      "Invalid Request: expected jsonrpc \"2.0\" and a string method" ).dump();

  std::string const &m = method->text();
  Json const *const params = msg.find( "params" );
  if ( params && !params->is_object() )
    return error_response( *id, INVALID_PARAMS,
                           "Invalid params: params must be an object" ).dump();
  Json const empty( Json::object() );
  Json const &p = params ? *params : empty;

  Json result;

  // Legacy (handshake-based) revisions.
  if ( m == "initialize" ) {
    Json const *const v = p.find( "protocolVersion" );
    if ( !v || !v->is_string() )
      return error_response( *id, INVALID_PARAMS,
        "Invalid params: initialize requires a string protocolVersion; this "
        "server implements MCP " + std::string( protocol_version ) +
        " (server/discover) and 2025-11-25" ).dump();
    std::string negotiated( legacy_protocol_versions[0] );
    for ( char const *const *lv = legacy_protocol_versions; *lv; ++lv )
      if ( v->text() == *lv )
        negotiated = *lv;
    theLegacyVersion = negotiated;
    result = Json::object( {
      { "protocolVersion", negotiated },
      { "capabilities", Json::object( { { "tools", Json::object() } } ) },
      { "serverInfo", server_info() },
      { "instructions", instructions( theConfig.jsoniq_default ) }
    } );
    return Json::object( {
      { "jsonrpc", "2.0" }, { "id", *id }, { "result", result }
    } ).dump();
  }

  Json const *const meta = p.find( "_meta" );
  Json const *const version =
    meta && meta->is_object() ? meta->find( meta_version ) : nullptr;
  bool const modern = version != nullptr;

  // ping exists in the legacy revisions only (removed in 2026-07-28).
  if ( m == "ping" && !modern && !theLegacyVersion.empty() ) {
    result = Json::object();
  } else {
    if ( modern ) {
      if ( !version->is_string() )
        return error_response( *id, INVALID_PARAMS,
          std::string( "Invalid params: " ) + meta_version +
          " must be a string" ).dump();
      if ( version->text() != protocol_version )
        return error_response( *id, UNSUPPORTED_PROTOCOL_VERSION,
          "Unsupported protocol version",
          Json::object( {
            { "supported", all_versions() },
            { "requested", version->text() }
          } ) ).dump();
      Json const *const caps = meta->find( meta_capabilities );
      if ( !caps || !caps->is_object() )
        return error_response( *id, INVALID_PARAMS,
          std::string( "Invalid params: missing _meta " ) +
          meta_capabilities ).dump();
    } else if ( theLegacyVersion.empty() ) {
      return error_response( *id, INVALID_PARAMS,
        std::string( "Invalid params: missing _meta " ) + meta_version +
        "; this server implements MCP " + protocol_version +
        " (clients of 2025-11-25 and earlier must send initialize first)"
      ).dump();
    }

    if ( m == "server/discover" && modern ) {
      result = Json::object( {
        { "supportedVersions", all_versions() },
        { "capabilities", Json::object( { { "tools", Json::object() } } ) },
        { "instructions", instructions( theConfig.jsoniq_default ) }
      } );
    } else if ( m == "tools/list" ) {
      result = Json::object( { { "tools", tools() } } );
    } else if ( m == "tools/call" ) {
      bool protocol_error;
      Json error;
      result = call_tool( p, &protocol_error, &error );
      if ( protocol_error )
        return error_response( *id, INVALID_PARAMS, error.text() ).dump();
    } else {
      return error_response( *id, METHOD_NOT_FOUND,
                             "Method not found: " + m ).dump();
    }
    // Cacheable results (server/discover, tools/list): the tools do not
    // change, but the answer names the user's files, so keep it private.
    if ( modern && ( m == "server/discover" || m == "tools/list" ) ) {
      result.set( "ttlMs", 0 );
      result.set( "cacheScope", "private" );
    }
  }

  if ( modern ) {
    result.set( "resultType", "complete" );
    result.set( "_meta", Json::object( { { meta_server_info, server_info() } } ) );
  }
  return Json::object( {
    { "jsonrpc", "2.0" }, { "id", *id }, { "result", std::move( result ) }
  } ).dump();
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
