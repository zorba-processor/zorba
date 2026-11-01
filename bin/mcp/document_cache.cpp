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

#include "document_cache.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <iterator>
#include <sstream>
#include <vector>

#include <sys/stat.h>

#include <zorba/empty_sequence.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/iterator.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/uri_resolvers.h>

namespace zorba {
namespace mcp {

namespace {

char const cache_ns[] = "http://zorba.io/modules/directory-cache";
char const cache_url[] = "urn:zorba-mcp:directory-cache";

// The module text served instead of the installed (non-caching) module:
// the same functions, implemented by DocumentCache.
char const cache_module_source[] =
  "module namespace dc = \"http://zorba.io/modules/directory-cache\";\n"
  "declare namespace an = \"http://zorba.io/annotations\";\n"
  "declare namespace ver = \"http://zorba.io/options/versioning\";\n"
  "declare option ver:module-version \"1.0\";\n"
  "declare %an:nondeterministic function dc:get(\n"
  "  $root as xs:string, $path as xs:string ) as object()? external;\n"
  "declare %an:nondeterministic function dc:put(\n"
  "  $root as xs:string, $path as xs:string, $record as object() )\n"
  "  as object() external;\n";

void release_stream( std::istream *s ) {
  delete s;
}

std::string string_arg( ExternalFunction::Arguments_t const &args,
                        std::size_t i ) {
  Item item;
  Iterator_t it( args[i]->getIterator() );
  it->open();
  it->next( item );
  it->close();
  return item.isNull() ? std::string() : item.getStringValue().str();
}

Item item_arg( ExternalFunction::Arguments_t const &args, std::size_t i ) {
  Item item;
  Iterator_t it( args[i]->getIterator() );
  it->open();
  it->next( item );
  it->close();
  return item;
}

std::int64_t nanoseconds( struct timespec const &t ) {
  return static_cast<std::int64_t>( t.tv_sec ) * 1000000000 + t.tv_nsec;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

class DocumentCache::Module : public ExternalModule {
public:
  explicit Module( DocumentCache *cache ) : theGet( cache ), thePut( cache ) { }

  String getURI() const override { return cache_ns; }

  ExternalFunction* getExternalFunction( String const &name ) override {
    if ( name == "get" )
      return &theGet;
    if ( name == "put" )
      return &thePut;
    return nullptr;
  }

private:
  class Get : public NonContextualExternalFunction {
  public:
    explicit Get( DocumentCache *cache ) : theCache( cache ) { }
    String getURI() const override { return cache_ns; }
    String getLocalName() const override { return "get"; }
    ItemSequence_t evaluate( Arguments_t const &args ) const override {
      Item const record(
        theCache->get( string_arg( args, 0 ), string_arg( args, 1 ) ) );
      if ( record.isNull() )
        return ItemSequence_t( new EmptySequence() );
      return ItemSequence_t( new SingletonItemSequence( record ) );
    }
  private:
    DocumentCache *const theCache;
  };

  class Put : public NonContextualExternalFunction {
  public:
    explicit Put( DocumentCache *cache ) : theCache( cache ) { }
    String getURI() const override { return cache_ns; }
    String getLocalName() const override { return "put"; }
    ItemSequence_t evaluate( Arguments_t const &args ) const override {
      return ItemSequence_t( new SingletonItemSequence(
        theCache->put( string_arg( args, 0 ), string_arg( args, 1 ),
                       item_arg( args, 2 ) ) ) );
    }
  private:
    DocumentCache *const theCache;
  };

  Get theGet;
  Put thePut;
};

class DocumentCache::Mapper : public URIMapper {
public:
  void mapURI( String const aUri, EntityData const *aEntityData,
               std::vector<String> &oUris ) override {
    if ( aEntityData->getKind() == EntityData::MODULE && aUri == cache_ns )
      oUris.push_back( cache_url );
  }
};

class DocumentCache::Resolver : public URLResolver {
public:
  Resource* resolveURL( String const &aUrl,
                        EntityData const *aEntityData ) override {
    if ( aEntityData->getKind() != EntityData::MODULE || aUrl != cache_url )
      return nullptr;
    return StreamResource::create(
      new std::istringstream( cache_module_source ), &release_stream );
  }
};

///////////////////////////////////////////////////////////////////////////////

DocumentCache::DocumentCache( std::string const &root, std::size_t budget ) :
  theBudget( budget ),
  theModule( new Module( this ) ),
  theMapper( new Mapper() ),
  theResolver( new Resolver() ) {
  char buf[ PATH_MAX ];
  theRoot = realpath( root.c_str(), buf ) ? buf : root;
  if ( theRoot.empty() || theRoot.back() != '/' )
    theRoot += '/';
}

DocumentCache::~DocumentCache() {
  clear();
}

void DocumentCache::attach( StaticContext_t const &sctx ) {
  if ( !enabled() )
    return;
  sctx->registerURIMapper( theMapper.get() );
  sctx->registerURLResolver( theResolver.get() );
  sctx->registerModule( theModule.get() );
}

void DocumentCache::set_active( bool active ) {
  std::lock_guard<std::mutex> lock( theMutex );
  theActive = active;
  thePending.clear();
  if ( !active )
    return;
  // Drop the entries of deleted or modified files (a query only looks up
  // the files it opens).
  for ( auto i = theEntries.begin(); i != theEntries.end(); ) {
    auto const next = std::next( i );
    Signature now;
    if ( !signature_of( i->first, &now ) || !( now == i->second.sig ) )
      erase( i );
    i = next;
  }
}

void DocumentCache::clear() {
  std::lock_guard<std::mutex> lock( theMutex );
  theEntries.clear();
  theLru.clear();
  thePending.clear();
  theStats.entries = 0;
  theStats.bytes = 0;
}

DocumentCache::Stats DocumentCache::stats() const {
  std::lock_guard<std::mutex> lock( theMutex );
  return theStats;
}

// Bytes of memory per byte of file, measured with bench/directory (peak RSS
// of a query holding every parsed document, over the size of the files).
std::size_t DocumentCache::estimate( std::string const &path,
                                     std::uintmax_t size ) {
  std::string ext;
  std::string::size_type const dot = path.rfind( '.' );
  if ( dot != std::string::npos && path.find( '/', dot ) == std::string::npos )
    ext = path.substr( dot + 1 );
  std::transform( ext.begin(), ext.end(), ext.begin(),
                  []( unsigned char c ) { return std::tolower( c ); } );
  std::uintmax_t factor = 25;
  if ( ext == "xml" || ext == "xhtml" )
    factor = 25;
  else if ( ext == "json" || ext == "jsonl" || ext == "ndjson" )
    factor = 25;
  else if ( ext == "csv" )
    factor = 25;
  // Every record also has a fixed part (the record object, its path).
  return static_cast<std::size_t>( 1024 + size * factor );
}

bool DocumentCache::key_of( std::string const &root, std::string const &path,
                            std::string *key, Signature *sig ) const {
  if ( root.empty() || path.empty() )
    return false;
  // The record carries the path relative to the root it was opened with,
  // so the key is the path as the module spells it.
  *key = root + "/" + path;
  return signature_of( *key, sig );
}

bool DocumentCache::signature_of( std::string const &full,
                                  Signature *sig ) const {
  char buf[ PATH_MAX ];
  if ( !realpath( full.c_str(), buf ) )
    return false;
  sig->canonical = buf;
  // Only files under the root: the server's policy reads nothing else.
  if ( sig->canonical.compare( 0, theRoot.size(), theRoot ) != 0 )
    return false;
  struct stat st;
  if ( stat( buf, &st ) != 0 || !S_ISREG( st.st_mode ) )
    return false;
  sig->dev = static_cast<std::uintmax_t>( st.st_dev );
  sig->ino = static_cast<std::uintmax_t>( st.st_ino );
  sig->size = static_cast<std::uintmax_t>( st.st_size );
#ifdef __APPLE__
  sig->mtime_ns = nanoseconds( st.st_mtimespec );
  sig->ctime_ns = nanoseconds( st.st_ctimespec );
#else
  sig->mtime_ns = nanoseconds( st.st_mtim );
  sig->ctime_ns = nanoseconds( st.st_ctim );
#endif
  return true;
}

void DocumentCache::erase( std::unordered_map<std::string,Entry>::iterator i ) {
  theStats.bytes -= i->second.bytes;
  theLru.erase( i->second.lru );
  theEntries.erase( i );
  theStats.entries = theEntries.size();
}

Item DocumentCache::get( std::string const &root, std::string const &path ) {
  std::string key;
  Signature sig;
  bool const ok = key_of( root, path, &key, &sig );
  std::lock_guard<std::mutex> lock( theMutex );
  if ( !theActive || key.empty() )
    return Item();
  auto const i = theEntries.find( key );
  if ( !ok ) {
    // Deleted, or not a regular file under the root.
    if ( i != theEntries.end() )
      erase( i );
    return Item();
  }
  if ( i != theEntries.end() ) {
    if ( i->second.sig == sig ) {
      ++theStats.hits;
      theLru.splice( theLru.begin(), theLru, i->second.lru );
      return i->second.record;
    }
    erase( i );                         // the file changed
  }
  ++theStats.misses;
  // put() stores the record under the signature seen before the read, so a
  // file modified while it is read is parsed again next time.
  thePending[ key ] = sig;
  return Item();
}

Item DocumentCache::put( std::string const &root, std::string const &path,
                         Item const &record ) {
  std::string key;
  Signature now;
  bool const ok = key_of( root, path, &key, &now );
  std::lock_guard<std::mutex> lock( theMutex );
  if ( !theActive || !ok || record.isNull() )
    return record;
  auto const p = thePending.find( key );
  if ( p == thePending.end() )
    return record;
  Signature const sig( p->second );
  thePending.erase( p );
  if ( !( sig == now ) )
    return record;                      // modified while it was read
  std::size_t const bytes = estimate( path, sig.size );
  if ( bytes > theBudget )
    return record;
  auto const old = theEntries.find( key );
  if ( old != theEntries.end() )
    erase( old );
  while ( theStats.bytes + bytes > theBudget && !theLru.empty() ) {
    erase( theEntries.find( theLru.back() ) );
    ++theStats.evictions;
  }
  theLru.push_front( key );
  theEntries.emplace( key, Entry{ sig, record, bytes, theLru.begin() } );
  theStats.bytes += bytes;
  theStats.entries = theEntries.size();
  ++theStats.stores;
  return record;
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
