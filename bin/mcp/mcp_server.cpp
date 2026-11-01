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

#include <algorithm>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <streambuf>
#include <utility>

#include <zorba/diagnostic.h>
#include <zorba/dynamic_context.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/options.h>
#include <zorba/static_context.h>
#include <zorba/version.h>
#include <zorba/xquery.h>
#include <zorba/xquery_exception.h>
#include <zorba/zorba_exception.h>

#include "directory_config.h"

namespace zorba {
namespace mcp {

char const protocol_version[] = "2026-07-28";

char const *const legacy_protocol_versions[] = {
  "2025-11-25", "2025-06-18", "2025-03-26", nullptr
};

namespace {

// JSON-RPC 2.0 and MCP error codes.
int const PARSE_ERROR = -32700;
int const INVALID_REQUEST = -32600;
int const METHOD_NOT_FOUND = -32601;
int const INVALID_PARAMS = -32602;
int const UNSUPPORTED_PROTOCOL_VERSION = -32022;

// Larger messages are refused before parsing.
std::size_t const max_message_size = 4 * 1024 * 1024;

char const meta_version[] = "io.modelcontextprotocol/protocolVersion";
char const meta_capabilities[] = "io.modelcontextprotocol/clientCapabilities";
char const meta_server_info[] = "io.modelcontextprotocol/serverInfo";

char const server_name[] = "zorba-mcp";

char const instructions[] =
  "Queries a directory of XML, JSON and CSV documents with JSONiq/XQuery "
  "(Zorba). Start with list_documents. In run_query, import the directory "
  "module (import module namespace dir = \"http://zorba.io/modules/directory\";) "
  "and use $dir:root: dir:documents($dir:root, \"xml\") or "
  "dir:documents($dir:root, (\"json\", \"csv\")) return records "
  "{path, format, content} (or {path, format, error}); "
  "dir:open($dir:root, $path) returns one. Results are JSON, one item per "
  "line; return JSON objects, arrays or atomic values, not XML nodes "
  "(convert with string() or data()). Only files under the directory can "
  "be read; network access, writes and environment variables are denied. "
  "Predefined query tools, when present, answer common questions directly.";

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

// Collects a query's output up to a limit, counting what goes beyond it.
class CappedBuf : public std::streambuf {
public:
  explicit CappedBuf( std::size_t cap ) : theCap( cap ), theTotal( 0 ) { }

  std::string const& data() const { return theData; }
  std::size_t total() const { return theTotal; }
  bool truncated() const { return theTotal > theData.size(); }

protected:
  int_type overflow( int_type c ) override {
    if ( !traits_type::eq_int_type( c, traits_type::eof() ) ) {
      char const ch = traits_type::to_char_type( c );
      xsputn( &ch, 1 );
    }
    return traits_type::not_eof( c );
  }

  std::streamsize xsputn( char const *s, std::streamsize n ) override {
    if ( n <= 0 )
      return 0;
    std::size_t const len = static_cast<std::size_t>( n );
    theTotal += len;
    if ( theData.size() < theCap )
      theData.append( s, std::min( len, theCap - theData.size() ) );
    return n;
  }

private:
  std::size_t const theCap;
  std::size_t theTotal;
  std::string theData;
};

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

std::string percent_encode_path( std::string const &path ) {
  static char const hex[] = "0123456789ABCDEF";
  std::string out;
  for ( char const c : path ) {
    unsigned char const u = static_cast<unsigned char>( c );
    if ( ( u >= 'A' && u <= 'Z' ) || ( u >= 'a' && u <= 'z' ) ||
         ( u >= '0' && u <= '9' ) || std::strchr( "-._~/", c ) ) {
      out += c;
    } else {
      out += '%';
      out += hex[ u >> 4 ];
      out += hex[ u & 0x0F ];
    }
  }
  return out;
}

// Line n (1-based) of a text, or the empty string.
std::string source_line( std::string const &source, unsigned long n ) {
  std::istringstream in( source );
  std::string line;
  for ( unsigned long i = 1; std::getline( in, line ); ++i )
    if ( i == n )
      return line;
  return std::string();
}

// The leading comment of a query, flattened to one line: "(: a\n : b :)"
// gives "a b". A version declaration may come first.
std::string leading_comment( std::string const &source ) {
  std::size_t pos = source.find_first_not_of( " \t\r\n" );
  if ( pos == std::string::npos )
    return std::string();
  for ( char const *decl : { "xquery version", "jsoniq version" } ) {
    if ( source.compare( pos, std::strlen( decl ), decl ) == 0 ) {
      std::size_t const semi = source.find( ';', pos );
      if ( semi == std::string::npos )
        return std::string();
      pos = source.find_first_not_of( " \t\r\n", semi + 1 );
      if ( pos == std::string::npos )
        return std::string();
      break;
    }
  }
  if ( source.compare( pos, 2, "(:" ) != 0 )
    return std::string();
  std::size_t const end = source.find( ":)", pos + 2 );
  if ( end == std::string::npos )
    return std::string();
  std::istringstream in( source.substr( pos + 2, end - pos - 2 ) );
  std::string word, out;
  while ( in >> word ) {
    if ( word == ":" )
      continue;                         // the " : " of continuation lines
    if ( !out.empty() )
      out += ' ';
    out += word;
  }
  return out;
}

std::string tool_name( std::string const &stem ) {
  std::string name;
  for ( char const c : stem ) {
    bool const ok = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) ||
      ( c >= '0' && c <= '9' ) || c == '_' || c == '-' || c == '.';
    name += ok ? c : '_';
  }
  if ( name.size() > 128 )
    name.resize( 128 );
  return name;
}

Json diagnostic_json( ZorbaException const &e, std::string const &source,
                      std::string const &file_name ) {
  diagnostic::QName const &qn = e.diagnostic().qname();
  std::string code( qn.localname() );
  if ( qn.prefix() && *qn.prefix() )
    code = std::string( qn.prefix() ) + ':' + code;
  Json err( Json::object( {
    { "code", code },
    { "namespace", qn.ns() ? qn.ns() : "" },
    { "message", e.what() }
  } ) );
  if ( XQueryException const *const xe =
         dynamic_cast<XQueryException const*>( &e ) ) {
    if ( xe->has_source() ) {
      std::string const uri( xe->source_uri() ? xe->source_uri() : "" );
      Json loc( Json::object() );
      if ( !uri.empty() )
        loc.set( "uri", uri );
      loc.set( "line", static_cast<unsigned long long>( xe->source_line() ) );
      loc.set( "column",
               static_cast<unsigned long long>( xe->source_column() ) );
      if ( xe->source_line_end() ) {
        loc.set( "lineEnd",
                 static_cast<unsigned long long>( xe->source_line_end() ) );
        loc.set( "columnEnd",
                 static_cast<unsigned long long>( xe->source_column_end() ) );
      }
      // The text of the failing line, when the error is in the query itself
      // (not in a module it imports).
      bool const in_query = uri.empty() ||
        ( !file_name.empty() && uri.find( file_name ) != std::string::npos );
      if ( in_query ) {
        std::string const text( source_line( source, xe->source_line() ) );
        if ( !text.empty() )
          loc.set( "text", text );
      }
      err.set( "location", std::move( loc ) );
    }
  }
  return Json::object( { { "error", std::move( err ) } } );
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

Server::Server( Zorba *zorba, ServerConfig config, std::ostream &log ) :
  theZorba( zorba ),
  theConfig( std::move( config ) ),
  theLog( log ),
  thePolicy( SecurityPolicy::secure() ) {
  directory_config::apply_policy( thePolicy, theConfig.root );
  if ( theConfig.timeout.count() > 0 )
    thePolicy.setTimeout( theConfig.timeout );
  load_predefined();
}

void Server::load_predefined() {
  if ( theConfig.queries_dir.empty() )
    return;
  namespace stdfs = std::filesystem;
  std::vector<stdfs::path> files;
  std::error_code ec;
  for ( stdfs::directory_iterator i( theConfig.queries_dir, ec ), end;
        !ec && i != end; i.increment( ec ) ) {
    if ( i->path().extension() == ".jq" && i->is_regular_file( ec ) )
      files.push_back( i->path() );
  }
  if ( ec )
    theLog << server_name << ": cannot list " << theConfig.queries_dir
           << ": " << ec.message() << std::endl;
  std::sort( files.begin(), files.end() );
  for ( stdfs::path const &file : files )
    load_predefined( file.string(), tool_name( file.stem().string() ) );
}

bool Server::load_predefined( std::string const &path,
                              std::string const &name ) {
  if ( name.empty() || name == "list_documents" || name == "run_query" ||
       std::any_of( thePredefined.begin(), thePredefined.end(),
                    [&]( Predefined const &p ) { return p.name == name; } ) ) {
    theLog << server_name << ": skipping " << path
           << ": tool name \"" << name << "\" is already taken" << std::endl;
    return false;
  }
  std::ifstream in( path.c_str(), std::ios::binary );
  std::stringstream buf;
  buf << in.rdbuf();
  if ( !in ) {
    theLog << server_name << ": skipping " << path << ": cannot read it"
           << std::endl;
    return false;
  }
  Predefined p;
  p.name = name;
  p.path = path;
  p.source = buf.str();
  p.description = leading_comment( p.source );
  if ( p.description.empty() )
    p.description = "Runs the predefined query " +
      std::filesystem::path( path ).filename().string() + ".";

  // Compile once to find the external variables (in no namespace) that
  // become the tool's parameters.
  try {
    StaticContext_t sctx = theZorba->createStaticContext();
    sctx->setSecurityPolicy( thePolicy );
    XQuery_t query = theZorba->createQuery();
    query->setFileName( path );
    std::istringstream qs( p.source );
    Zorba_CompilerHints_t hints;
    query->compile( qs, sctx, hints );
    Iterator_t vars;
    query->getExternalVariables( vars );
    std::set<std::string> seen;
    Item qname;
    vars->open();
    while ( vars->next( qname ) ) {
      if ( !qname.getNamespace().empty() )
        continue;
      std::string const local( qname.getLocalName().str() );
      if ( !seen.insert( local ).second )
        continue;
      bool const has_default =
        query->getDynamicContext()->isBoundExternalVariable( "", local );
      p.params.push_back( Param{ local, !has_default } );
    }
    vars->close();
    query->close();
  }
  catch ( ZorbaException const &e ) {
    theLog << server_name << ": skipping " << path << ": " << e << std::endl;
    return false;
  }
  theLog << server_name << ": predefined query tool " << p.name << " ("
         << p.params.size() << " parameters)" << std::endl;
  thePredefined.push_back( std::move( p ) );
  return true;
}

Json Server::tools() const {
  Json list( Json::array() );

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
      "as JSON, one item per line. Example: import module namespace dir = "
      "\"http://zorba.io/modules/directory\"; for $d in "
      "dir:documents($dir:root, \"json\") return $d(\"path\"). On failure "
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

  for ( Predefined const &p : thePredefined ) {
    Json props( Json::object() );
    Json required( Json::array() );
    for ( Param const &param : p.params ) {
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
      { "description", p.description },
      { "inputSchema", schema },
      { "annotations", read_only_annotations() }
    } ) );
  }
  return list;
}

Server::QueryOutcome
Server::run( std::string const &source, std::string const &file_name,
             std::vector<std::pair<std::string,std::string> > const &vars ) {
  QueryOutcome outcome;
  try {
    StaticContext_t sctx = theZorba->createStaticContext();
    sctx->setSecurityPolicy( thePolicy );
    // Relative URIs, e.g. doc("orders/q1.xml"), resolve against the root.
    sctx->setBaseURI( "file://" + percent_encode_path( theConfig.root ) + "/" );

    XQuery_t query = theZorba->createQuery();
    if ( !file_name.empty() )
      query->setFileName( file_name );
    std::istringstream qs( source );
    Zorba_CompilerHints_t hints;
    query->compile( qs, sctx, hints );

    DynamicContext *const dctx = query->getDynamicContext();
    directory_config::bind_root( theZorba, dctx, theConfig.root );
    for ( auto const &var : vars )
      dctx->setVariable( "", var.first,
        theZorba->getItemFactory()->createString( var.second ), true );

    Zorba_SerializerOptions opts;
    opts.set( directory_config::serialization_params() );
    CappedBuf buf( theConfig.max_output );
    std::ostream out( &buf );
    query->execute( out, &opts );
    query->close();

    outcome.ok = true;
    outcome.text = buf.data();
    if ( buf.truncated() ) {
      trim_utf8_tail( &outcome.text );
      outcome.text += "\n[zorba-mcp: output truncated after " +
        std::to_string( outcome.text.size() ) + " of " +
        std::to_string( buf.total() ) + " bytes; narrow the query (fewer "
        "fields, a filter, or subsequence()) to see the rest]";
    }
  }
  catch ( ZorbaException const &e ) {
    outcome.text = diagnostic_json( e, source, file_name ).dump();
  }
  catch ( std::exception const &e ) {
    outcome.text = Json::object( { { "error", Json::object( {
      { "code", "internal" }, { "message", e.what() }
    } ) } } ).dump();
  }
  return outcome;
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
  std::vector<std::pair<std::string,std::string> > const no_vars;
  QueryOutcome outcome;
  if ( tool == "list_documents" ) {
    outcome = run(
      std::string( "import module namespace dir = \"" ) +
        directory_config::module_ns + "\";\ndir:list($dir:root)\n",
      std::string(), no_vars );
  } else if ( tool == "run_query" ) {
    Json const *const q = args->find( "query" );
    if ( !q || !q->is_string() ) {
      outcome.text = Json::object( { { "error", Json::object( {
        { "code", "invalid-arguments" },
        { "message", "run_query requires the argument \"query\" (a string)" }
      } ) } } ).dump();
    } else {
      outcome = run( q->text(), std::string(), no_vars );
    }
  } else {
    auto const p = std::find_if( thePredefined.begin(), thePredefined.end(),
      [&]( Predefined const &x ) { return x.name == tool; } );
    if ( p == thePredefined.end() ) {
      *protocol_error = true;
      *error = Json( "Unknown tool: " + tool );
      return Json();
    }
    std::vector<std::pair<std::string,std::string> > vars;
    std::string missing;
    for ( Param const &param : p->params ) {
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
    if ( !missing.empty() ) {
      outcome.text = Json::object( { { "error", Json::object( {
        { "code", "invalid-arguments" },
        { "message", tool + " requires the arguments: " + missing }
      } ) } } ).dump();
    } else {
      outcome = run( p->source, p->path, vars );
    }
  }
  theLog << server_name << ": tools/call " << tool << ": "
         << ( outcome.ok ? "ok" : "error" ) << std::endl;
  return tool_result( outcome );
}

std::string Server::handle( std::string const &message ) {
  Json const null_id;
  if ( message.size() > max_message_size )
    return error_response( null_id, INVALID_REQUEST,
                           "Invalid Request: message too large" ).dump();

  Json msg;
  std::string why;
  if ( !parse_json( message, &msg, &why ) )
    return error_response( null_id, PARSE_ERROR, "Parse error: " + why ).dump();
  if ( !msg.is_object() )
    return error_response( null_id, INVALID_REQUEST,
      "Invalid Request: expected a JSON-RPC object (batches are not "
      "supported)" ).dump();

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
      { "instructions", instructions }
    } );
    return Json::object( {
      { "jsonrpc", "2.0" }, { "id", *id }, { "result", result }
    } ).dump();
  }

  Json const *const meta = p.find( "_meta" );
  Json const *const version =
    meta && meta->is_object() ? meta->find( meta_version ) : nullptr;
  bool const modern = version != nullptr;

  if ( m == "ping" ) {
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
        { "instructions", instructions }
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
