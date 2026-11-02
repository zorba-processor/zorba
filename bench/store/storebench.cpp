/*
 * Copyright 2026 Zorba contributors
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

// Store memory benchmark (bench/store/run.sh; job "store" of
// .github/workflows/directory-scaling.yml).
// Linux/glibc only. Uses the public API only; links against libzorba.
//
//   storebench file FORMAT PATH   parse PATH as dir:documents does
//                                 (FORMAT xml, json or csv) and hold the
//                                 result
//   storebench micro N            parse in-memory documents of N items of one
//                                 kind each (empty elements, text nodes,
//                                 attributes, JSON numbers, strings, ...)
//
// Each measurement prints one JSON object per line on stdout:
//   rss_kb     VmRSS after parsing (result held, query closed) minus before
//   hwm_kb     peak RSS while parsing minus RSS before (the peak is reset
//              through /proc/self/clear_refs first)
//   heap       live malloc bytes (mallinfo2 uordblks + hblkhd) after minus
//              before; includes chunk headers and allocator rounding
//   news       live operator-new allocations after minus before
//   usable     malloc_usable_size() of those allocations after minus before
//   ms         wall time of the parse
// "file" adds the item counts of the parsed result (a walk through the
// public Item API after the measurement) and Item::mem_size(), the store's
// own estimate.

#include <malloc.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <new>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/store_consts.h>
#include <zorba/store_manager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

///////////////////////////////////////////////////////////////////////////////
// Counting operator new/delete. Replacing them in the executable replaces
// them for libzorba too, so every C++ allocation of the store is counted.

namespace {

std::atomic<long> g_news{ 0 };
std::atomic<long> g_usable{ 0 };

inline void *counted( void *p ) {
  if ( p ) {
    g_news.fetch_add( 1, std::memory_order_relaxed );
    g_usable.fetch_add( static_cast<long>( malloc_usable_size( p ) ),
                        std::memory_order_relaxed );
  }
  return p;
}

inline void uncounted( void *p ) {
  if ( p ) {
    g_news.fetch_sub( 1, std::memory_order_relaxed );
    g_usable.fetch_sub( static_cast<long>( malloc_usable_size( p ) ),
                        std::memory_order_relaxed );
    std::free( p );
  }
}

inline void *aligned( std::size_t n, std::align_val_t a ) {
  std::size_t const al = static_cast<std::size_t>( a );
  void *p = nullptr;
  if ( posix_memalign( &p, al < sizeof( void* ) ? sizeof( void* ) : al,
                       n ? n : 1 ) != 0 )
    return nullptr;
  return counted( p );
}

} // namespace

void *operator new( std::size_t n ) {
  if ( void *p = counted( std::malloc( n ? n : 1 ) ) )
    return p;
  throw std::bad_alloc();
}
void *operator new[]( std::size_t n ) { return operator new( n ); }
void *operator new( std::size_t n, std::nothrow_t const& ) noexcept {
  return counted( std::malloc( n ? n : 1 ) );
}
void *operator new[]( std::size_t n, std::nothrow_t const& ) noexcept {
  return counted( std::malloc( n ? n : 1 ) );
}
void *operator new( std::size_t n, std::align_val_t a ) {
  if ( void *p = aligned( n, a ) )
    return p;
  throw std::bad_alloc();
}
void *operator new[]( std::size_t n, std::align_val_t a ) {
  return operator new( n, a );
}
void *operator new( std::size_t n, std::align_val_t a,
                    std::nothrow_t const& ) noexcept {
  return aligned( n, a );
}
void *operator new[]( std::size_t n, std::align_val_t a,
                      std::nothrow_t const& ) noexcept {
  return aligned( n, a );
}
void operator delete( void *p ) noexcept { uncounted( p ); }
void operator delete[]( void *p ) noexcept { uncounted( p ); }
void operator delete( void *p, std::size_t ) noexcept { uncounted( p ); }
void operator delete[]( void *p, std::size_t ) noexcept { uncounted( p ); }
void operator delete( void *p, std::nothrow_t const& ) noexcept {
  uncounted( p );
}
void operator delete[]( void *p, std::nothrow_t const& ) noexcept {
  uncounted( p );
}
void operator delete( void *p, std::align_val_t ) noexcept { uncounted( p ); }
void operator delete[]( void *p, std::align_val_t ) noexcept {
  uncounted( p );
}
void operator delete( void *p, std::size_t, std::align_val_t ) noexcept {
  uncounted( p );
}
void operator delete[]( void *p, std::size_t, std::align_val_t ) noexcept {
  uncounted( p );
}
void operator delete( void *p, std::align_val_t,
                      std::nothrow_t const& ) noexcept {
  uncounted( p );
}
void operator delete[]( void *p, std::align_val_t,
                        std::nothrow_t const& ) noexcept {
  uncounted( p );
}

///////////////////////////////////////////////////////////////////////////////

using namespace zorba;

namespace {

long status_kb( char const *key ) {
  std::ifstream in( "/proc/self/status" );
  std::string line;
  std::size_t const n = std::strlen( key );
  while ( std::getline( in, line ) )
    if ( line.compare( 0, n, key ) == 0 && line.size() > n && line[n] == ':' )
      return std::atol( line.c_str() + n + 1 );
  return -1;
}

void reset_peak() {
  // "5" resets VmHWM to the current RSS (Linux 4.0+).
  std::ofstream out( "/proc/self/clear_refs" );
  out << "5" << std::flush;
}

struct Snap {
  long rss_kb, hwm_kb, news, usable;
  long long heap;
};

Snap snap() {
  struct mallinfo2 const mi = mallinfo2();
  return Snap{ status_kb( "VmRSS" ), status_kb( "VmHWM" ),
               g_news.load(), g_usable.load(),
               static_cast<long long>( mi.uordblks + mi.hblkhd ) };
}

struct Measure {
  long rss_kb, hwm_kb, news, usable;
  long long heap;
  double ms;
};

std::string const prolog =
  "import module namespace file = \"http://expath.org/ns/file\";\n"
  "import module namespace jn = \"http://jsoniq.org/functions\";\n"
  "import module namespace csv = \"http://zorba.io/modules/json-csv\";\n"
  "declare variable $s external;\n";

// The same expressions as dir:parse() in modules/directory/directory.xq;
// SRC is file:read-text($s) for a file and $s for an in-memory document.
std::string parse_query( std::string const &format, bool from_file ) {
  std::string const src = from_file ? "file:read-text( $s )" : "$s";
  if ( format == "xml" )
    return prolog + "parse-xml( " + src + " )";
  if ( format == "json" )
    return prolog + "let $items := jn:parse-json( " + src + " )\n"
      "return if ( count( $items ) eq 1 ) then $items else [ $items ]";
  if ( format == "csv" )
    return prolog + "[ csv:parse( " + src + " ) ]";
  std::cerr << "storebench: unknown format " << format << std::endl;
  std::exit( 2 );
}

// Runs QUERY with $s bound to ARG and appends its result to OUT. The query
// is compiled before the "before" snapshot and closed before the "after"
// one, so only what the result holds (and store-wide caches it filled) is
// counted.
Measure run( Zorba *z, std::string const &query, std::string const &arg,
             std::vector<Item> &out ) {
  XQuery_t q = z->createQuery();
  q->compile( query );
  Item s = z->getItemFactory()->createString( arg );
  q->getDynamicContext()->setVariable( "", "s", s );
  s = Item();

  malloc_trim( 0 );
  reset_peak();
  Snap const before = snap();
  auto const t0 = std::chrono::steady_clock::now();
  {
    Iterator_t it = q->iterator();
    it->open();
    Item i;
    while ( it->next( i ) )
      out.push_back( i );
    it->close();
  }
  auto const t1 = std::chrono::steady_clock::now();
  long const hwm = status_kb( "VmHWM" );
  q->close();
  q = XQuery_t();
  malloc_trim( 0 );
  Snap const after = snap();
  return Measure{
    after.rss_kb - before.rss_kb, hwm - before.rss_kb,
    after.news - before.news, after.usable - before.usable,
    after.heap - before.heap,
    std::chrono::duration<double, std::milli>( t1 - t0 ).count() };
}

void print_measure( std::ostream &o, Measure const &m ) {
  o << "\"rss_kb\":" << m.rss_kb << ",\"hwm_kb\":" << m.hwm_kb
    << ",\"heap\":" << m.heap << ",\"news\":" << m.news
    << ",\"usable\":" << m.usable << ",\"ms\":" << m.ms;
}

///////////////////////////////////////////////////////////////////////////////
// Walk through the parsed result.

struct Counts {
  std::map<std::string, long long> n;
  std::unordered_set<std::string> names, keys;
};

void walk( Item const &i, Counts &c );

void walk_children( Iterator_t it, Counts &c ) {
  it->open();
  Item x;
  while ( it->next( x ) )
    walk( x, c );
  it->close();
}

void walk_node( Item const &i, Counts &c ) {
  switch ( i.getNodeKind() ) {
  case store::StoreConsts::documentNode:
    ++c.n["documents"];
    walk_children( i.getChildren(), c );
    break;
  case store::StoreConsts::elementNode: {
    ++c.n["elements"];
    Item qn;
    i.getNodeName( qn );
    c.names.insert( qn.getStringValue().str() );
    Iterator_t a = i.getAttributes();
    a->open();
    Item at;
    while ( a->next( at ) ) {
      ++c.n["attributes"];
      Item an;
      at.getNodeName( an );
      c.names.insert( "@" + an.getStringValue().str() );
      c.n["attribute_bytes"] += at.getStringValue().length();
    }
    a->close();
    walk_children( i.getChildren(), c );
    break;
  }
  case store::StoreConsts::textNode: {
    std::string const t = i.getStringValue().str();
    ++c.n["texts"];
    c.n["text_bytes"] += t.size();
    if ( t.find_first_not_of( " \t\r\n" ) == std::string::npos )
      ++c.n["whitespace_texts"];
    break;
  }
  case store::StoreConsts::commentNode:
    ++c.n["comments"];
    break;
  case store::StoreConsts::piNode:
    ++c.n["pis"];
    break;
  default:
    ++c.n["other_nodes"];
  }
}

void walk( Item const &i, Counts &c ) {
  if ( i.isNode() ) {
    walk_node( i, c );
    return;
  }
  if ( i.isJSONItem() ) {
    if ( i.getJSONItemKind() == store::StoreConsts::jsonObject ) {
      ++c.n["objects"];
      Iterator_t ks = i.getObjectKeys();
      ks->open();
      Item k;
      while ( ks->next( k ) ) {
        String const key = k.getStringValue();
        ++c.n["pairs"];
        c.n["key_bytes"] += key.length();
        c.keys.insert( key.str() );
        walk( i.getObjectValue( key ), c );
      }
      ks->close();
    } else {
      ++c.n["arrays"];
      uint64_t const size = i.getArraySize();
      c.n["members"] += size;
      for ( uint64_t j = 1; j <= size; ++j )
        walk( i.getArrayValue( static_cast<uint32_t>( j ) ), c );
    }
    return;
  }
  switch ( i.getTypeCode() ) {
  case store::XS_STRING:
    ++c.n["strings"];
    c.n["string_bytes"] += i.getStringValue().length();
    break;
  case store::XS_INTEGER:
  case store::XS_LONG:
  case store::XS_INT:
    ++c.n["integers"];
    break;
  case store::XS_DECIMAL:
    ++c.n["decimals"];
    break;
  case store::XS_DOUBLE:
    ++c.n["doubles"];
    break;
  case store::XS_BOOLEAN:
    ++c.n["booleans"];
    break;
  case store::JS_NULL:
    ++c.n["nulls"];
    break;
  default:
    ++c.n["other_atomics"];
  }
}

long long file_size( char const *path ) {
  std::ifstream in( path, std::ios::binary | std::ios::ate );
  return in ? static_cast<long long>( in.tellg() ) : -1;
}

// Warm-up: compiles and runs each parse once on a tiny document so that the
// modules, the plan caches and the store's pools exist before measuring.
void warm_up( Zorba *z ) {
  struct { char const *format, *doc; } const docs[] = {
    { "xml", "<r a=\"1\"><a>x</a></r>" },
    { "json", "{\"a\":[1,2.5,3e0,\"x\",true,null,{}]}" },
    { "csv", "a,b\n1,x\n" } };
  for ( auto const &d : docs ) {
    std::vector<Item> items;
    run( z, parse_query( d.format, false ), d.doc, items );
  }
}

int file_mode( Zorba *z, std::string const &format, char const *path ) {
  warm_up( z );
  long long const bytes = file_size( path );
  if ( bytes < 0 ) {
    std::cerr << "storebench: cannot read " << path << std::endl;
    return 1;
  }
  std::vector<Item> items;
  Measure const m = run( z, parse_query( format, true ), path, items );
  Counts c;
  std::size_t mem_size = 0;
  for ( Item const &i : items ) {
    walk( i, c );
    mem_size += i.mem_size();
  }
  std::ostringstream o;
  o << "{\"mode\":\"file\",\"format\":\"" << format << "\",\"bytes\":"
    << bytes << ',';
  print_measure( o, m );
  o << ",\"mem_size\":" << mem_size << ",\"items\":" << items.size()
    << ",\"distinct_names\":" << c.names.size()
    << ",\"distinct_keys\":" << c.keys.size();
  for ( auto const &kv : c.n )
    o << ",\"" << kv.first << "\":" << kv.second;
  o << '}';
  std::cout << o.str() << std::endl;
  return 0;
}

// Microbenchmarks: documents made of N items of one kind; the per-item cost
// of a kind is the difference between two cases divided by N (see report.xq).
struct Micro {
  char const *name, *format;
  std::string head, tail;
  std::string ( *item )( long );
  std::string sep;
};

std::string digits64( long i ) {
  // 64 characters that differ between items (no sharing of equal strings).
  std::string s = std::to_string( i );
  return std::string( 64 - s.size(), 'x' ) + s;
}

int micro_mode( Zorba *z, long n ) {
  std::vector<Micro> const cases = {
    { "x_empty", "xml", "<r>", "</r>",
      []( long ) { return std::string( "<a/>" ); }, "" },
    { "x_text1", "xml", "<r>", "</r>",
      []( long ) { return std::string( "<a>x</a>" ); }, "" },
    { "x_text64", "xml", "<r>", "</r>",
      []( long i ) { return "<a>" + digits64( i ) + "</a>"; }, "" },
    { "x_attr1", "xml", "<r>", "</r>",
      []( long ) { return std::string( "<a b=\"x\"/>" ); }, "" },
    { "x_attr64", "xml", "<r>", "</r>",
      []( long i ) { return "<a b=\"" + digits64( i ) + "\"/>"; }, "" },
    { "x_names", "xml", "<r>", "</r>",
      []( long i ) { return "<a" + std::to_string( i ) + "/>"; }, "" },
    { "j_int", "json", "[", "]",
      []( long i ) { return std::to_string( i % 1000 ); }, "," },
    { "j_bigint", "json", "[", "]",
      []( long i ) { return std::to_string( 1000000 + i ); }, "," },
    { "j_dec", "json", "[", "]",
      []( long i ) { return std::to_string( i % 1000 ) + ".25"; }, "," },
    { "j_dbl", "json", "[", "]",
      []( long i ) { return std::to_string( i % 1000 ) + ".5e3"; }, "," },
    { "j_bool", "json", "[", "]",
      []( long ) { return std::string( "true" ); }, "," },
    { "j_null", "json", "[", "]",
      []( long ) { return std::string( "null" ); }, "," },
    { "j_str1", "json", "[", "]",
      []( long ) { return std::string( "\"x\"" ); }, "," },
    { "j_str64", "json", "[", "]",
      []( long i ) { return "\"" + digits64( i ) + "\""; }, "," },
    { "j_obj0", "json", "[", "]",
      []( long ) { return std::string( "{}" ); }, "," },
    { "j_obj1", "json", "[", "]",
      []( long ) { return std::string( "{\"a\":1}" ); }, "," },
    { "j_obj4", "json", "[", "]",
      []( long ) {
        return std::string( "{\"a\":1,\"b\":2,\"c\":3,\"d\":4}" ); }, "," },
    { "j_arr0", "json", "[", "]",
      []( long ) { return std::string( "[]" ); }, "," },
    { "c_row4", "csv", "a,b,c,d\n", "",
      []( long i ) {
        return std::to_string( i % 1000 ) + ",x," +
          std::to_string( i % 7 ) + ",y\n"; }, "" } };
  warm_up( z );
  for ( Micro const &c : cases ) {
    std::string doc = c.head;
    for ( long i = 0; i < n; ++i ) {
      if ( i )
        doc += c.sep;
      doc += c.item( i );
    }
    doc += c.tail;
    long long const bytes = static_cast<long long>( doc.size() );
    std::vector<Item> items;
    Measure const m = run( z, parse_query( c.format, false ), doc, items );
    std::ostringstream o;
    o << "{\"mode\":\"micro\",\"case\":\"" << c.name << "\",\"format\":\""
      << c.format << "\",\"n\":" << n << ",\"bytes\":" << bytes << ',';
    print_measure( o, m );
    o << '}';
    std::cout << o.str() << std::endl;
  }
  return 0;
}

} // namespace

int main( int argc, char **argv ) {
  std::string const mode = argc > 1 ? argv[1] : "";
  if ( !( ( mode == "file" && argc == 4 ) ||
          ( mode == "micro" && argc == 3 ) ) ) {
    std::cerr << "usage: storebench file xml|json|csv PATH\n"
                 "       storebench micro N" << std::endl;
    return 2;
  }
  void *const store = StoreManager::getStore();
  Zorba *const z = Zorba::getInstance( store );
  // file:read-text() needs FileRead, which no default policy grants.
  SecurityPolicy policy( SecurityPolicy::unrestricted() );
  policy.allow( Capability::FileRead );
  z->setSecurityPolicy( policy );
  int status;
  try {
    status = mode == "file" ? file_mode( z, argv[2], argv[3] )
                            : micro_mode( z, std::atol( argv[2] ) );
  }
  catch ( ZorbaException const &e ) {
    std::cerr << "storebench: " << e << std::endl;
    status = 1;
  }
  z->shutdown();
  StoreManager::shutdownStore( store );
  return status;
}
