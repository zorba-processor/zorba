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

// Multithreaded throughput benchmark (bench/threads/run.sh).
// Uses the public API only; links against libzorba.
//
//   mtbench [OPTIONS] QUERY
//
// One engine and one store for the process. QUERY is compiled once; for each
// thread count N, N threads run clones of it (one clone per run) for SECONDS
// and the completed runs are counted. Every run's output must have the
// checksum of a first run made before timing.
//
//   --name NAME          the query's name in the output (default: QUERY)
//   --threads N,N,...    thread counts (default 1,2,4,8,16)
//   --seconds S          time per thread count (default 3)
//   -e NAME:=VALUE       binds the external variable NAME to the string VALUE
//   -x NAME=FILE         binds NAME to the XML document FILE, parsed once and
//                        shared by every thread
//   --private            parses the -x documents once per thread instead
//   --items              consumes the result through the Item API (one
//                        getStringValue() per item) instead of serializing it
//
// Prints one JSON object per thread count:
//   {"query", "threads", "runs", "seconds", "qps", "scaling", "checksum"}
// qps is runs / seconds, where seconds runs from the release of the threads
// to the end of the last run; scaling is qps / qps of the first thread count.
// Exits with status 1 if a run fails or its output differs.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <latch>
#include <mutex>
#include <optional>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <zorba/config.h>
#include <zorba/dynamic_context.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/iterator.h>
#include <zorba/security.h>
#include <zorba/static_context.h>
#include <zorba/store_manager.h>
#include <zorba/xmldatamanager.h>
#include <zorba/xquery.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

using Clock = std::chrono::steady_clock;

// FNV-1a over the bytes written to it.
class HashBuf : public std::streambuf {
public:
  std::uint64_t hash() const { return theHash; }

  void add( char const *s, std::size_t n ) {
    for ( std::size_t i = 0; i < n; ++i ) {
      theHash ^= static_cast<unsigned char>( s[i] );
      theHash *= 0x100000001b3ULL;
    }
  }

protected:
  int_type overflow( int_type c ) override {
    if ( c != traits_type::eof() ) {
      char const ch = traits_type::to_char_type( c );
      add( &ch, 1 );
    }
    return traits_type::not_eof( c );
  }

  std::streamsize xsputn( char const *s, std::streamsize n ) override {
    add( s, static_cast<std::size_t>( n ) );
    return n;
  }

private:
  std::uint64_t theHash = 0xcbf29ce484222325ULL;
};

struct Options {
  std::string query, name;
  std::vector<unsigned> threads{ 1, 2, 4, 8, 16 };
  double seconds = 3;
  std::vector<std::pair<std::string, std::string>> strings;
  std::vector<std::pair<std::string, std::string>> docs;
  bool privateDocs = false;
  bool items = false;
};

[[noreturn]] void usage( char const *msg ) {
  std::cerr << "mtbench: " << msg << "\n"
               "usage: mtbench [--name NAME] [--threads N,N,...] "
               "[--seconds S] [-e NAME:=VALUE] [-x NAME=FILE] [--private] "
               "[--items] QUERY\n";
  std::exit( 2 );
}

Options parse_args( int argc, char **argv ) {
  Options o;
  for ( int i = 1; i < argc; ++i ) {
    std::string const a = argv[i];
    auto value = [&]() -> std::string {
      if ( ++i >= argc )
        usage( ( a + " needs a value" ).c_str() );
      return argv[i];
    };
    if ( a == "--name" )
      o.name = value();
    else if ( a == "--threads" ) {
      o.threads.clear();
      std::stringstream ss( value() );
      std::string n;
      while ( std::getline( ss, n, ',' ) ) {
        int const t = std::atoi( n.c_str() );
        if ( t <= 0 )
          usage( "thread counts must be positive" );
        o.threads.push_back( static_cast<unsigned>( t ) );
      }
    } else if ( a == "--seconds" ) {
      o.seconds = std::atof( value().c_str() );
      if ( o.seconds <= 0 )
        usage( "--seconds must be positive" );
    } else if ( a == "-e" || a == "-x" ) {
      std::string const v = value();
      std::string::size_type const eq = v.find( '=' );
      if ( eq == std::string::npos || eq == 0 )
        usage( ( a + " takes NAME:=VALUE or NAME=FILE" ).c_str() );
      if ( a == "-e" ) {
        if ( v[eq - 1] != ':' )
          usage( "-e takes NAME:=VALUE" );
        o.strings.emplace_back( v.substr( 0, eq - 1 ), v.substr( eq + 1 ) );
      } else
        o.docs.emplace_back( v.substr( 0, eq ), v.substr( eq + 1 ) );
    } else if ( a == "--private" )
      o.privateDocs = true;
    else if ( a == "--items" )
      o.items = true;
    else if ( !a.empty() && a[0] == '-' )
      usage( ( "unknown option " + a ).c_str() );
    else if ( o.query.empty() )
      o.query = a;
    else
      usage( "only one QUERY" );
  }
  if ( o.query.empty() )
    usage( "no QUERY" );
  if ( o.name.empty() )
    o.name = o.query;
#ifdef ZORBA_FOR_ONE_THREAD_ONLY
  for ( unsigned t : o.threads )
    if ( t != 1 )
      usage( "this engine is built for one thread "
             "(ZORBA_FOR_ONE_THREAD_ONLY): use --threads 1" );
#endif
  return o;
}

using Docs = std::vector<std::pair<std::string, Item>>;

Docs parse_docs( Zorba *z, Options const &o ) {
  Docs docs;
  for ( auto const &[name, file] : o.docs ) {
    std::ifstream in( file, std::ios::binary );
    if ( !in )
      throw std::runtime_error( "cannot read " + file );
    docs.emplace_back( name, z->getXmlDataManager()->parseXML( in ) );
  }
  return docs;
}

// One run of a clone of aMaster; returns the checksum of its output.
std::uint64_t run_once( XQuery_t const &aMaster, ItemFactory *aFactory,
                        Options const &o, Docs const &aDocs ) {
  XQuery_t q = aMaster->clone();
  DynamicContext *dctx = q->getDynamicContext();
  for ( auto const &[name, value] : o.strings )
    dctx->setVariable( name, aFactory->createString( value ) );
  for ( auto const &[name, doc] : aDocs )
    dctx->setVariable( name, doc );
  HashBuf buf;
  if ( o.items ) {
    Iterator_t it = q->iterator();
    it->open();
    Item item;
    while ( it->next( item ) ) {
      String const s = item.getStringValue();
      buf.add( s.data(), s.size() );
      buf.add( "\n", 1 );
    }
    it->close();
  } else {
    std::ostream out( &buf );
    q->execute( out );
    out.flush();
  }
  return buf.hash();
}

std::string hex( std::uint64_t h ) {
  char s[17];
  std::snprintf( s, sizeof s, "%016llx", static_cast<unsigned long long>( h ) );
  return s;
}

std::string json_string( std::string const &s ) {
  std::string r = "\"";
  for ( char c : s ) {
    if ( c == '"' || c == '\\' )
      r += '\\';
    r += c;
  }
  return r + '"';
}

struct Point {
  long runs = 0;
  double seconds = 0;
};

Point measure( unsigned aThreads, XQuery_t const &aMaster, ItemFactory *aFactory,
               Zorba *z, Options const &o, Docs const &aShared,
               std::uint64_t aExpected, std::string &aError ) {
  std::latch ready( aThreads + 1 );
  std::latch go( 1 );
  std::atomic<long> runs{ 0 };
  std::mutex errorMutex;
  std::atomic<bool> stop{ false };
  Clock::time_point start;
  std::vector<std::thread> threads;
  threads.reserve( aThreads );

  for ( unsigned t = 0; t < aThreads; ++t ) {
    threads.emplace_back( [&]() {
      std::optional<Docs> own;
      try {
        if ( o.privateDocs )
          own = parse_docs( z, o );
      } catch ( std::exception const &e ) {
        std::lock_guard<std::mutex> g( errorMutex );
        aError = e.what();
        stop = true;
      }
      Docs const &docs = own ? *own : aShared;
      ready.count_down();
      go.wait();
      auto const deadline =
          start + std::chrono::duration_cast<Clock::duration>(
                      std::chrono::duration<double>( o.seconds ) );
      long mine = 0;
      while ( !stop && Clock::now() < deadline ) {
        try {
          if ( run_once( aMaster, aFactory, o, docs ) != aExpected ) {
            std::lock_guard<std::mutex> g( errorMutex );
            aError = "output differs from the first run";
            stop = true;
          }
        } catch ( std::exception const &e ) {
          std::lock_guard<std::mutex> g( errorMutex );
          aError = e.what();
          stop = true;
        }
        ++mine;
      }
      runs += mine;
    } );
  }
  ready.arrive_and_wait();
  start = Clock::now();
  go.count_down();
  for ( std::thread &t : threads )
    t.join();
  Point p;
  p.runs = runs;
  p.seconds = std::chrono::duration<double>( Clock::now() - start ).count();
  return p;
}

} // namespace

int main( int argc, char **argv ) {
  Options const o = parse_args( argc, argv );
  void *store = StoreManager::getStore();
  Zorba *z = Zorba::getInstance( store );
  // file:read-text() and schema imports need FileRead, which no default
  // policy grants.
  SecurityPolicy policy( SecurityPolicy::unrestricted() );
  policy.allow( Capability::FileRead );
  z->setSecurityPolicy( policy );
  int status = 0;
  try {
    std::ifstream in( o.query, std::ios::binary );
    if ( !in )
      throw std::runtime_error( "cannot read " + o.query );
    StaticContext_t sctx = z->createStaticContext();
    std::string const path = std::filesystem::absolute( o.query ).string();
    sctx->setBaseURI( "file://" + path );
    XQuery_t master = z->compileQuery( in, sctx );
    ItemFactory *factory = z->getItemFactory();
    Docs const shared = o.privateDocs ? Docs() : parse_docs( z, o );
    Docs const first = o.privateDocs ? parse_docs( z, o ) : Docs();
    std::uint64_t const expected =
        run_once( master, factory, o, o.privateDocs ? first : shared );

    double base = 0;
    for ( unsigned n : o.threads ) {
      std::string error;
      Point const p =
          measure( n, master, factory, z, o, shared, expected, error );
      if ( !error.empty() ) {
        std::cerr << "mtbench: " << o.name << ", " << n
                  << " threads: " << error << "\n";
        status = 1;
        break;
      }
      double const qps = p.runs / p.seconds;
      if ( base == 0 )
        base = qps;
      std::printf( "{\"query\": %s, \"threads\": %u, \"runs\": %ld, "
                   "\"seconds\": %.3f, \"qps\": %.3f, \"scaling\": %.3f, "
                   "\"checksum\": \"%s\"}\n",
                   json_string( o.name ).c_str(), n, p.runs, p.seconds, qps,
                   qps / base, hex( expected ).c_str() );
      std::fflush( stdout );
    }
  } catch ( std::exception const &e ) {
    std::cerr << "mtbench: " << o.name << ": " << e.what() << "\n";
    status = 1;
  }
  z->shutdown();
  StoreManager::shutdownStore( store );
  return status;
}
