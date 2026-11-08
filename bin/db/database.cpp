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

#include "database.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <new>
#include <set>
#include <sstream>
#include <streambuf>
#include <utility>

#include <libxml/parser.h>

#include <zorba/diagnostic.h>
#include <zorba/dynamic_context.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/options.h>
#include <zorba/static_context.h>
#include <zorba/user_exception.h>
#include <zorba/xquery.h>
#include <zorba/xquery_exception.h>
#include <zorba/zorba_exception.h>

#include "directory_config.h"

namespace zorba {
namespace db {

namespace {

// The program these log lines come from, as in DocumentCache::stats_line().
char const log_name[] = "zorba-mcp";

// Passes what the serializer writes to a sink, with no buffer of its own.
class SinkBuf : public std::streambuf {
public:
  explicit SinkBuf( ResultSink &sink ) : theSink( sink ) { }

protected:
  int_type overflow( int_type c ) override {
    if ( !traits_type::eq_int_type( c, traits_type::eof() ) ) {
      char const ch = traits_type::to_char_type( c );
      theSink.write( &ch, 1 );
    }
    return traits_type::not_eof( c );
  }

  std::streamsize xsputn( char const *s, std::streamsize n ) override {
    if ( n <= 0 )
      return 0;
    theSink.write( s, static_cast<std::size_t>( n ) );
    return n;
  }

private:
  ResultSink &theSink;
};

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

// The base URI of the queries, and of what the directory module parses for them.
std::string base_uri_of( std::string const &root ) {
  return "file://" + percent_encode_path( root ) + "/";
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

std::string query_name( std::string const &stem ) {
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

QueryError error_of( ZorbaException const &e, std::string const &source,
                     std::string const &file_name ) {
  QueryError err;
  err.kind = QueryError::Kind::query;
  diagnostic::QName const &qn = e.diagnostic().qname();
  err.code = qn.localname();
  if ( qn.prefix() && *qn.prefix() )
    err.code = std::string( qn.prefix() ) + ':' + err.code;
  err.ns = qn.ns() ? qn.ns() : "";
  err.message = e.what();
  err.hint = e.hint();
  if ( XQueryException const *const xe =
         dynamic_cast<XQueryException const*>( &e ) ) {
    if ( xe->has_source() ) {
      SourceLocation loc;
      loc.uri = xe->source_uri() ? xe->source_uri() : "";
      loc.line = xe->source_line();
      loc.column = xe->source_column();
      loc.line_end = xe->source_line_end();
      loc.column_end = xe->source_column_end();
      bool const in_query = loc.uri.empty() ||
        ( !file_name.empty() && loc.uri.find( file_name ) != std::string::npos );
      if ( in_query )
        loc.text = source_line( source, loc.line );
      err.location = std::move( loc );
    }
  }
  return err;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

Database::Database( Zorba *zorba, DatabaseConfig config, std::ostream &log ) :
  theZorba( zorba ),
  theConfig( std::move( config ) ),
  theLog( log ),
  thePolicy( SecurityPolicy::secure() ),
  theCache( zorba, theConfig.root,
            std::uintmax_t( theConfig.cache_max_file_kb ) * 1024,
            theConfig.cache_max_files, &thePolicy, base_uri_of( theConfig.root ) ) {
  // libxml2 ends the process if it cannot allocate its state for a thread, the
  // first time the thread uses it: this thread does so now, before any query.
  xmlDictFree( xmlDictCreate() );
  directory_config::apply_policy( thePolicy, theConfig.root );
  if ( theConfig.timeout.count() > 0 )
    thePolicy.setTimeout( theConfig.timeout );
  load_predefined();
  if ( theConfig.scan ) {
    try {
      theScanner = std::make_unique<Scanner>(
        theConfig.root, theConfig.scan_limits, theConfig.scan_hook );
    }
    // Threads may not start under --max-memory, which is applied before this:
    // the database then works as with --no-scan.
    catch ( std::exception const &e ) {
      theLog << log_name << ": no background scan: " << e.what() << std::endl;
    }
  }
}

Database::~Database() {
  (void)stop_scan();
  if ( theScanner )
    theLog << log_name << ": the scan cache holds "
           << theScanner->cache_summary() << std::endl;
}

bool Database::stop_scan() {
  return !theScanner || theScanner->stop();
}

std::size_t Database::scan_cache_bytes() const {
  return theScanner ? theScanner->cache_bytes() : 0;
}

// The static context a query is compiled in. The language of a query without
// a version declaration is JSONiq for a .jq file, else the configured default;
// a declaration in the query wins.
StaticContext_t Database::new_static_context( std::string const &file_name )
    const {
  StaticContext_t sctx = theZorba->createStaticContext();
  sctx->setSecurityPolicy( thePolicy );
  std::string const ext( std::filesystem::path( file_name ).extension() );
  if ( ext == ".jq" || ( ext != ".xq" && theConfig.jsoniq_default ) )
    sctx->setJSONiqVersion( jsoniq_version_1_0 );
  return sctx;
}

void Database::load_predefined() {
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
    theLog << log_name << ": cannot list " << theConfig.queries_dir
           << ": " << ec.message() << std::endl;
  std::sort( files.begin(), files.end() );
  for ( stdfs::path const &file : files )
    load_predefined( file.string(), query_name( file.stem().string() ) );
}

bool Database::load_predefined( std::string const &path,
                                std::string const &name ) {
  // The names of the built-in tools of zorba-mcp are not taken either.
  if ( name.empty() || name == "list_documents" || name == "run_query" ||
       find_predefined( name ) ) {
    theLog << log_name << ": skipping " << path
           << ": tool name \"" << name << "\" is already taken" << std::endl;
    return false;
  }
  std::ifstream in( path.c_str(), std::ios::binary );
  if ( !in ) {
    theLog << log_name << ": skipping " << path << ": cannot read it"
           << std::endl;
    return false;
  }
  PredefinedQuery p;
  p.name = name;
  p.path = path;
  // Not a stream insertion, which leaves a shorter text when it cannot grow:
  // std::bad_alloc goes to the caller.
  p.source.assign( std::istreambuf_iterator<char>( in ),
                   std::istreambuf_iterator<char>() );
  p.description = leading_comment( p.source );

  // Compile once to find the external variables (in no namespace) that
  // become the parameters.
  try {
    StaticContext_t sctx = new_static_context( path );
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
      p.params.push_back( PredefinedQuery::Param{ local, !has_default } );
    }
    vars->close();
    query->close();
  }
  catch ( ZorbaException const &e ) {
    theLog << log_name << ": skipping " << path << ": " << e << std::endl;
    return false;
  }
  theLog << log_name << ": predefined query tool " << p.name << " ("
         << p.params.size() << " parameters)" << std::endl;
  thePredefined.push_back( std::move( p ) );
  return true;
}

PredefinedQuery const* Database::find_predefined( std::string const &name )
    const {
  auto const p = std::find_if( thePredefined.begin(), thePredefined.end(),
    [&]( PredefinedQuery const &x ) { return x.name == name; } );
  return p == thePredefined.end() ? nullptr : &*p;
}

QueryResult Database::run( std::string const &source, ResultSink &sink ) {
  return execute( source, std::string(), Variables(), sink );
}

QueryResult Database::run_predefined( std::string const &name,
                                      Variables const &vars,
                                      ResultSink &sink ) {
  PredefinedQuery const *const p = find_predefined( name );
  if ( !p ) {
    QueryResult result;
    result.error.message = "no predefined query " + name;
    return result;
  }
  return execute( p->source, p->path, vars, sink );
}

QueryResult Database::list_files( ResultSink &sink ) {
  return execute(
    std::string( "import module namespace dir = \"" ) +
      directory_config::module_ns + "\";\ndir:list($dir:root)\n",
    std::string(), Variables(), sink );
}

std::string Database::list_documents( ListRequest const &request,
                                      std::size_t max_output, bool *is_error ) {
  // A request does not wait past its time limit, nor long without one.
  std::chrono::milliseconds const longest( std::chrono::seconds( 10 ) );
  std::chrono::milliseconds const wait(
    theConfig.timeout.count() > 0 ? std::min( theConfig.timeout, longest )
                                  : longest );
  return theScanner->answer( request, max_output, wait, is_error );
}

std::string Database::list_documents_error( char const *code,
                                            std::string const &message ) {
  return theScanner->error_answer( code, message );
}

QueryResult Database::execute( std::string const &source,
                               std::string const &file_name,
                               Variables const &vars, ResultSink &sink ) {
  QueryResult result;
  try {
    StaticContext_t sctx = new_static_context( file_name );
    // Relative URIs, e.g. doc("orders/q1.xml"), resolve against the root.
    sctx->setBaseURI( base_uri_of( theConfig.root ) );
    theCache.attach( sctx );
    // An implementation may add predeclared namespace prefixes (XQuery 3.0,
    // 4.13); a query can still bind this one to something else.
    sctx->addNamespace( "math", "http://www.w3.org/2005/xpath-functions/math" );

    XQuery_t query = theZorba->createQuery();
    if ( !file_name.empty() )
      query->setFileName( file_name );
    std::istringstream qs( source );
    Zorba_CompilerHints_t hints;
    query->compile( qs, sctx, hints );
    // A query that can apply updates could modify cached documents.
    theCache.set_active( !query->isUpdating() && !query->isSequential() );

    DynamicContext *const dctx = query->getDynamicContext();
    directory_config::bind_root( theZorba, dctx, theConfig.root );
    for ( auto const &var : vars )
      dctx->setVariable( "", var.first,
        theZorba->getItemFactory()->createString( var.second ), true );

    Zorba_SerializerOptions opts;
    opts.set( directory_config::serialization_params() );
    SinkBuf buf( sink );
    if ( theConfig.before_execute )
      theConfig.before_execute();
    std::ostream out( &buf );
    // An exception from the sink (std::bad_alloc) goes through the stream to
    // here, instead of a bad stream and a shorter output.
    out.exceptions( std::ios::badbit );
    query->execute( out, &opts );
    query->close();
    sink.end();
    result.ok = true;
  }
  // The query is gone by now, and with it the memory it used.
  catch ( ZorbaException const &e ) {
    // Zorba reports std::bad_alloc as an internal error; fn:error() can raise
    // that code too, as a UserException.
    if ( !dynamic_cast<UserException const*>( &e ) &&
         std::strcmp( e.diagnostic().qname().localname(), "ZXQP0003" ) == 0 &&
         std::strstr( e.what(), "bad_alloc" ) )
      result.error.kind = QueryError::Kind::memory_limit;
    else
      result.error = error_of( e, source, file_name );
  }
  catch ( std::bad_alloc const& ) {
    result.error.kind = QueryError::Kind::memory_limit;
  }
  catch ( std::exception const &e ) {
    result.error.message = e.what();
  }
  return result;
}

} // namespace db
} // namespace zorba
/* vim:set et sw=2 ts=2: */
