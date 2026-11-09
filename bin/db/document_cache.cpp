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
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iterator>
#include <sstream>
#include <utility>

#include <sys/stat.h>

#include <zorba/dynamic_context.h>
#include <zorba/empty_sequence.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/uri_resolvers.h>
#include <zorba/zorba_exception.h>

namespace zorba {
namespace db {

namespace {

char const cache_ns[] = "http://zorba.io/modules/directory-cache";
char const cache_url[] = "urn:zorba-mcp:directory-cache";

// The module text served instead of the installed (non-caching) module:
// the same function, implemented by DocumentCache.
char const cache_module_source[] =
  "xquery version \"3.0\";\n"
  "module namespace dc = \"http://zorba.io/modules/directory-cache\";\n"
  "declare namespace an = \"http://zorba.io/annotations\";\n"
  "declare namespace ver = \"http://zorba.io/options/versioning\";\n"
  "declare option ver:module-version \"1.0\";\n"
  "declare %an:nondeterministic function dc:get(\n"
  "  $root as xs:string, $path as xs:string ) as object()? external;\n";

// The query that builds a record on a miss. Its static context does not
// attach the cache, so the directory module's hook is the installed one and
// dir:open reads and parses the file itself, with every check it applies to
// a user's dir:open, under the server's policy.
char const reader_source[] =
  "import module namespace dir = \"http://zorba.io/modules/directory\";\n"
  "declare variable $root as xs:string external;\n"
  "declare variable $path as xs:string external;\n"
  "dir:open( $root, $path )\n";

// A copy of a record, for the query that reads it.
char const copier_source[] =
  "declare variable $record external;\n"
  "copy $copy := $record modify () return $copy\n";

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

} // namespace

///////////////////////////////////////////////////////////////////////////////

class DocumentCache::Module : public ExternalModule {
public:
  explicit Module( DocumentCache *cache ) : theGet( cache ) { }

  String getURI() const override { return cache_ns; }

  ExternalFunction* getExternalFunction( String const &name ) override {
    if ( name == "get" )
      return &theGet;
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

  Get theGet;
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

/////////////////////////////////////////////////////////////////////////////

bool DocumentCache::file_id( std::string const &full, FileId *id ) {
  struct stat st;
  if ( stat( full.c_str(), &st ) != 0 || !S_ISREG( st.st_mode ) )
    return false;
  id->dev = static_cast<std::uintmax_t>( st.st_dev );
  id->ino = static_cast<std::uintmax_t>( st.st_ino );
  id->size = static_cast<std::uintmax_t>( st.st_size );
#ifdef __APPLE__
  struct timespec const &modified = st.st_mtimespec, &changed = st.st_ctimespec;
#else
  struct timespec const &modified = st.st_mtim, &changed = st.st_ctim;
#endif
  id->mtime_sec = modified.tv_sec;
  id->mtime_nsec = modified.tv_nsec;
  id->ctime_sec = changed.tv_sec;
  id->ctime_nsec = changed.tv_nsec;
  return true;
}

// Whether the change time is less than min_change_age ago, or ahead of this
// machine's clock. No arithmetic on whole times: they can be far off.
bool DocumentCache::changed_just_now( FileId const &id ) {
  struct timespec now;
  if ( clock_gettime( CLOCK_REALTIME, &now ) != 0 )
    return true;
  std::int64_t const now_sec = now.tv_sec;
  if ( id.ctime_sec > now_sec )
    return true;
  if ( id.ctime_sec < now_sec - 1 )
    return false;
  std::int64_t const age = ( now_sec - id.ctime_sec ) * 1000000000 +
    ( now.tv_nsec - id.ctime_nsec );
  return age < min_change_age;
}

// FNV-1a over the key and the metadata. A collision at worst admits a
// document on its first hit; what is kept is still read from the file.
std::uint64_t DocumentCache::tag_of( std::string const &key, FileId const &id ) {
  std::uint64_t h = 14695981039346656037u;
  auto mix = [&h]( void const *data, std::size_t size ) {
    unsigned char const *const p = static_cast<unsigned char const*>( data );
    for ( std::size_t i = 0; i < size; ++i )
      h = ( h ^ p[i] ) * 1099511628211u;
  };
  mix( key.data(), key.size() );
  mix( &id.dev, sizeof id.dev );
  mix( &id.ino, sizeof id.ino );
  mix( &id.size, sizeof id.size );
  mix( &id.mtime_sec, sizeof id.mtime_sec );
  mix( &id.mtime_nsec, sizeof id.mtime_nsec );
  mix( &id.ctime_sec, sizeof id.ctime_sec );
  mix( &id.ctime_nsec, sizeof id.ctime_nsec );
  return h ? h : 1;                     // 0 marks an empty slot
}

DocumentCache::DocumentCache( Zorba *zorba, std::string const &root,
                              std::uintmax_t max_file_bytes,
                              std::size_t max_files,
                              SecurityPolicy const *policy,
                              std::string const &base_uri,
                              std::function<void()> after_read ) :
  theZorba( zorba ),
  theGivenRoot( root ),
  theBaseUri( base_uri ),
  theMaxFileBytes( max_file_bytes ),
  theMaxFiles( max_files ),
  thePolicy( policy ),
  theAfterRead( std::move( after_read ) ),
  theModule( new Module( this ) ),
  theMapper( new Mapper() ),
  theResolver( new Resolver() ) {
  char buf[ PATH_MAX ];
  theRoot = realpath( root.c_str(), buf ) ? buf : root;
  if ( theRoot.empty() || theRoot.back() != '/' )
    theRoot += '/';
  if ( enabled() )
    theSeen.resize( tracker_slots );
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
  theActive = active;
  ++theQuery;
  if ( !active )
    return;
  // Drop the entries of deleted or modified files: a query only looks up the
  // files it opens, and they should not hold memory.
  for ( auto i = theEntries.begin(); i != theEntries.end(); ) {
    auto const next = std::next( i );
    std::string full( i->first );
    full[ full.find( '\0' ) ] = '/';
    FileId now;
    std::string canonical;
    if ( !file_id( full, &now ) || !( now == i->second.id ) ||
         !canonical_under_root( full, &canonical ) ||
         canonical != i->second.canonical )
      erase( i );
    i = next;
  }
}

void DocumentCache::clear() {
  theEntries.clear();
  theLru.clear();
  theStats.entries = 0;
  theStats.bytes = 0;
  if ( theReader ) {
    theReader->close();
    theReader = nullptr;
  }
  if ( theCopier ) {
    theCopier->close();
    theCopier = nullptr;
  }
}

std::string DocumentCache::stats_line() const {
  if ( !enabled() )
    return std::string();
  return "zorba-mcp: cache: hits=" + std::to_string( theStats.hits ) +
    " misses=" + std::to_string( theStats.misses ) +
    " admissions=" + std::to_string( theStats.admissions ) +
    " evictions=" + std::to_string( theStats.evictions ) +
    " bytes_retained=" + std::to_string( theStats.bytes );
}

std::size_t DocumentCache::tracker_bytes() {
  return tracker_slots * sizeof( std::uint64_t );
}

bool DocumentCache::canonical_under_root( std::string const &full,
                                          std::string *canonical ) const {
  char buf[ PATH_MAX ];
  if ( !realpath( full.c_str(), buf ) )
    return false;
  // Only files under the root: the server's policy reads nothing else.
  if ( std::strncmp( buf, theRoot.data(), theRoot.size() ) != 0 )
    return false;
  *canonical = buf;
  return true;
}

// The root of a key: another spelling of a root is the same root.
bool DocumentCache::root_key( std::string const &root,
                              std::string *key ) const {
  if ( root == theGivenRoot ) {
    key->assign( theRoot, 0, theRoot.size() - 1 );
    return true;
  }
  char buf[ PATH_MAX ];
  if ( !realpath( root.c_str(), buf ) )
    return false;
  *key = buf;
  return true;
}

std::uint64_t* DocumentCache::tracker_slot( std::uint64_t tag ) {
  std::uint64_t *const set =
    &theSeen[ ( tag % ( tracker_slots / tracker_ways ) ) * tracker_ways ];
  std::uint64_t *empty = nullptr;
  for ( std::size_t way = 0; way < tracker_ways; ++way ) {
    if ( set[way] == tag || set[way] == refused_tag( tag ) )
      return &set[way];
    if ( !set[way] && !empty )
      empty = &set[way];
  }
  if ( empty )
    return empty;
  theVictim ^= theVictim << 13;
  theVictim ^= theVictim >> 7;
  theVictim ^= theVictim << 17;
  return &set[ theVictim >> 61 ];       // the high bits: tracker_ways is 8
}

void DocumentCache::erase( std::unordered_map<std::string,Entry>::iterator i ) {
  theStats.bytes -= i->second.bytes;
  theLru.erase( i->second.lru );
  theEntries.erase( i );
  theStats.entries = theEntries.size();
}

Item DocumentCache::get( std::string const &root, std::string const &path ) {
  if ( !theActive || root.empty() || path.empty() )
    return Item();
  std::string const full( root + "/" + path );
  FileId id;
  // Deleted, or not a regular file: the next set_active() drops its record.
  if ( !file_id( full, &id ) )
    return Item();
  // Larger than the limit: nothing but the stat. If it was cached when it
  // was smaller, the next set_active() drops it.
  if ( id.size > theMaxFileBytes ) {
    ++theStats.misses;
    return Item();
  }

  std::string key;
  if ( !root_key( root, &key ) )
    return Item();
  key += '\0';
  key += path;
  auto const i = theEntries.find( key );
  if ( i != theEntries.end() ) {
    std::string canonical;
    if ( id == i->second.id && canonical_under_root( full, &canonical ) &&
         canonical == i->second.canonical ) {
      ++theStats.hits;
      i->second.query = theQuery;
      theLru.splice( theLru.begin(), theLru, i->second.lru );
      return copy_of( i->second.record );
    }
    erase( i );                         // the file changed
  }

  ++theStats.misses;
  std::uint64_t const tag = tag_of( key, id );
  std::uint64_t &seen = *tracker_slot( tag );
  if ( seen == refused_tag( tag ) )
    return Item();                      // not worth another try
  if ( seen != tag ) {
    seen = tag;                         // the first hit: nothing else to do
    return Item();
  }
  return admit( root, path, key, full, id, &seen );
}

Item DocumentCache::admit( std::string const &root, std::string const &path,
                           std::string const &key, std::string const &full,
                           FileId const &id, std::uint64_t *seen ) {
  // Full of documents this query has used: reading one more would only
  // replace them. A query thus admits at most theMaxFiles documents.
  if ( theEntries.size() >= theMaxFiles &&
       theEntries.at( theLru.back() ).query == theQuery )
    return Item();

  // Changed a moment ago: a write now could leave the same timestamps. The
  // next read tries again.
  if ( changed_just_now( id ) )
    return Item();

  std::string canonical;
  if ( !canonical_under_root( full, &canonical ) )
    return Item();
  // A read that yields nothing to keep is not tried again until the file
  // changes: a tree of such files must not pay for a second query per read.
  ++theStats.refused;
  std::size_t bytes = 0;
  Item const record( read( root, path, &bytes ) );
  if ( record.isNull() )
    return Item();
  if ( theAfterRead )
    theAfterRead();

  // Kept only if it holds no error (which can come from the moment rather
  // than the file), is not a small file that expands far beyond what files of
  // its size take, and the file still is what it was before the read (a file
  // modified meanwhile is parsed again next time).
  FileId now;
  std::string canonical_now;
  if ( !record.getObjectValue( "error" ).isNull() ||
       bytes > max_expansion * std::max<std::uintmax_t>( id.size, 1024 ) ||
       !file_id( full, &now ) ||
       !( now == id ) || !canonical_under_root( full, &canonical_now ) ||
       canonical_now != canonical )
  {
    // Null above, or running out of memory here, says nothing about the file:
    // only this marks it as not to be tried again.
    *seen = refused_tag( *seen );
    return record;
  }

  while ( theEntries.size() >= theMaxFiles ) {
    erase( theEntries.find( theLru.back() ) );
    ++theStats.evictions;
  }
  // The map first: a key in the list and not in the map would break eviction.
  auto const added = theEntries.emplace( key,
    Entry{ canonical, id, record, bytes, theQuery, theLru.end() } );
  try {
    theLru.push_front( key );
  }
  catch ( ... ) {
    theEntries.erase( added.first );
    throw;
  }
  added.first->second.lru = theLru.begin();
  theStats.bytes += bytes;
  theStats.entries = theEntries.size();
  --theStats.refused;
  ++theStats.admissions;
  // After an eviction, the document needs two hits again.
  *seen = 0;
  return copy_of( record );
}

// Null if the engine reports an error, as when it runs out of memory; the query
// has no policy, so no time limit stops it. The directory module then reads the
// file itself and reports what happens.
Item DocumentCache::copy_of( Item const &record ) {
  try {
    if ( !theCopier ) {
      StaticContext_t sctx = theZorba->createStaticContext();
      XQuery_t copier = theZorba->createQuery();
      std::istringstream qs( copier_source );
      Zorba_CompilerHints_t hints;
      copier->compile( qs, sctx, hints );
      theCopier = copier;
    }
    DynamicContext *const dctx = theCopier->getDynamicContext();
    dctx->setVariable( "", "record", record );
    Item copy;
    Iterator_t it( theCopier->iterator() );
    it->open();
    if ( !it->next( copy ) )
      copy = Item();
    it->close();
    // The variable must not hold a record that is evicted.
    dctx->setVariable( "", "record",
                       theZorba->getItemFactory()->createString( "" ) );
    return copy;
  }
  catch ( ZorbaException const& ) {
    if ( theCopier ) {
      theCopier->close();
      theCopier = nullptr;
    }
    return Item();
  }
}

Item DocumentCache::read( std::string const &root, std::string const &path,
                          std::size_t *bytes ) {
  try {
    if ( !theReader ) {
      StaticContext_t sctx = theZorba->createStaticContext();
      sctx->setSecurityPolicy( *thePolicy );
      sctx->setBaseURI( theBaseUri );
      XQuery_t reader = theZorba->createQuery();
      std::istringstream qs( reader_source );
      Zorba_CompilerHints_t hints;
      reader->compile( qs, sctx, hints );
      theReader = reader;
    }
    DynamicContext *const dctx = theReader->getDynamicContext();
    ItemFactory *const f = theZorba->getItemFactory();
    dctx->setVariable( "", "root", f->createString( root ) );
    dctx->setVariable( "", "path", f->createString( path ) );
    Item record;
    Iterator_t it( theReader->iterator() );
    it->open();
    if ( !it->next( record ) )
      record = Item();
    it->close();
    if ( !record.isNull() )
      *bytes = record.mem_size();
    return record;
  }
  catch ( ZorbaException const& ) {
    // The time limit, for one: nothing is kept, and the directory module
    // reads the file itself and reports what happens.
    if ( theReader ) {
      theReader->close();
      theReader = nullptr;
    }
    return Item();
  }
}

} // namespace db
} // namespace zorba
/* vim:set et sw=2 ts=2: */
