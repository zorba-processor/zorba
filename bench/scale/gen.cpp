// Generator of the scale benchmark's synthetic simulation log (see run.sh). Not
// part of the Zorba build: c++ -std=c++23 -O2 -o gen bench/scale/gen.cpp
//
//   gen FORM BYTES OUT
//
// FORM is jsonl (OUT/data.jsonl), array (OUT/data.json), dir (OUT/part-NNNNNN.jsonl,
// 500 records per file) or csv (OUT/data.csv). The record stream depends only on
// BYTES, not on FORM: it stops once the JSON Lines text of the records reaches
// BYTES, so that all forms of a size hold the same records. Prints
// "<records> <bytes written>". The pseudo-random numbers are the program's own, so
// the output is the same on every platform.
//
// A record, with the fields that are optional in brackets:
//   {"seq":N,"ts":"2026-01-01T00:00:00.000Z","run":"r0","node":"n0","event":"step",
//    ["v":12.5|null|"n/a",]
//    ["metrics":{["temp":57.3,]"pressure":101.2,["iter":7]},]
//    ["meta":{"fw":"1.2","tags":["a","b"]},]["extra":[1,2,3],]["note":"..."]}
// About 1 in 7 records has no v, 1 in 11 a null v, 1 in 13 a string v; 1 in 20 has
// no metrics, 1 in 9 of the others no temp, 1 in 5 no iter; 1 in 3 has meta, 1 in
// 17 extra, 1 in 31 note. Timestamps increase by 200 to 1800 ms, by 10 minutes
// about every 5000 records.
//
// The CSV form flattens the objects: metrics.temp is the column metrics_temp, and
// so on; an absent value is an empty field; meta.tags and extra's members are not
// kept (extra is a count). Empty fields are null after csv:parse.

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

namespace {

std::uint64_t state = 0x5ca1eda7a5eed;

std::uint64_t next_random() {  // splitmix64
  std::uint64_t z = ( state += 0x9e3779b97f4a7c15ULL );
  z = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9ULL;
  z = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111ebULL;
  return z ^ ( z >> 31 );
}

bool one_in( unsigned n ) { return next_random() % n == 0; }

struct Record {
  std::uint64_t seq;
  char ts[32];
  unsigned run, node;
  char const *event;
  enum { V_NONE, V_NULL, V_TEXT, V_NUMBER } v_kind;
  unsigned v;       // tenths
  bool metrics, has_temp, has_iter, meta, extra, note;
  unsigned temp, pressure, iter;  // tenths, tenths, count
};

std::int64_t now_ms = 1767225600000LL;  // 2026-01-01T00:00:00Z

void next_record( std::uint64_t seq, Record *r ) {
  r->seq = seq;
  now_ms += 200 + next_random() % 1601;
  if ( one_in( 5000 ) )
    now_ms += 600000;
  std::time_t const secs = now_ms / 1000;
  std::tm tm{};
  gmtime_r( &secs, &tm );
  std::snprintf( r->ts, sizeof r->ts, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                 tm.tm_min, tm.tm_sec, static_cast<int>( now_ms % 1000 ) );
  r->run = static_cast<unsigned>( seq / 100000 % 8 );
  r->node = static_cast<unsigned>( next_random() % 64 );
  std::uint64_t const e = next_random() % 100;
  r->event = e < 90 ? "step" : e < 98 ? "checkpoint" : "warn";
  r->v_kind = one_in( 7 ) ? Record::V_NONE : one_in( 11 ) ? Record::V_NULL
            : one_in( 13 ) ? Record::V_TEXT : Record::V_NUMBER;
  r->v = static_cast<unsigned>( next_random() % 10000 );
  r->metrics = !one_in( 20 );
  r->has_temp = !one_in( 9 );
  r->has_iter = !one_in( 5 );
  r->temp = static_cast<unsigned>( next_random() % 1000 );
  r->pressure = 900 + static_cast<unsigned>( next_random() % 300 );
  r->iter = static_cast<unsigned>( next_random() % 1000 );
  r->meta = one_in( 3 );
  r->extra = one_in( 17 );
  r->note = one_in( 31 );
}

char const note_text[] = "solver retried, step halved";

void append_tenths( std::string *s, unsigned tenths ) {
  *s += std::to_string( tenths / 10 );
  *s += '.';
  *s += static_cast<char>( '0' + tenths % 10 );
}

void to_json( Record const &r, std::string *s ) {
  s->clear();
  *s += "{\"seq\":";
  *s += std::to_string( r.seq );
  *s += ",\"ts\":\"";
  *s += r.ts;
  *s += "\",\"run\":\"r";
  *s += std::to_string( r.run );
  *s += "\",\"node\":\"n";
  *s += std::to_string( r.node );
  *s += "\",\"event\":\"";
  *s += r.event;
  *s += '"';
  switch ( r.v_kind ) {
  case Record::V_NONE: break;
  case Record::V_NULL: *s += ",\"v\":null"; break;
  case Record::V_TEXT: *s += ",\"v\":\"n/a\""; break;
  case Record::V_NUMBER: *s += ",\"v\":"; append_tenths( s, r.v ); break;
  }
  if ( r.metrics ) {
    *s += ",\"metrics\":{";
    if ( r.has_temp ) {
      *s += "\"temp\":";
      append_tenths( s, r.temp );
      *s += ',';
    }
    *s += "\"pressure\":";
    append_tenths( s, r.pressure );
    if ( r.has_iter ) {
      *s += ",\"iter\":";
      *s += std::to_string( r.iter );
    }
    *s += '}';
  }
  if ( r.meta )
    *s += ",\"meta\":{\"fw\":\"1.2\",\"tags\":[\"a\",\"b\"]}";
  if ( r.extra )
    *s += ",\"extra\":[1,2,3]";
  if ( r.note ) {
    *s += ",\"note\":\"";
    *s += note_text;
    *s += '"';
  }
  *s += '}';
}

char const csv_header[] = "seq,ts,run,node,event,v,metrics_temp,metrics_pressure,"
                          "metrics_iter,meta_fw,extra,note\n";

void to_csv( Record const &r, std::string *s ) {
  s->clear();
  *s += std::to_string( r.seq );
  *s += ',';
  *s += r.ts;
  *s += ",r";
  *s += std::to_string( r.run );
  *s += ",n";
  *s += std::to_string( r.node );
  *s += ',';
  *s += r.event;
  *s += ',';
  if ( r.v_kind == Record::V_NUMBER )
    append_tenths( s, r.v );
  else if ( r.v_kind == Record::V_TEXT )
    *s += "n/a";
  *s += ',';
  if ( r.metrics && r.has_temp )
    append_tenths( s, r.temp );
  *s += ',';
  if ( r.metrics )
    append_tenths( s, r.pressure );
  *s += ',';
  if ( r.metrics && r.has_iter )
    *s += std::to_string( r.iter );
  *s += r.meta ? ",1.2," : ",,";
  if ( r.extra )
    *s += '3';
  *s += ',';
  if ( r.note ) {
    *s += '"';
    *s += note_text;
    *s += '"';
  }
}

std::FILE *open_or_die( std::string const &path ) {
  std::FILE *const f = std::fopen( path.c_str(), "w" );
  if ( !f ) {
    std::perror( path.c_str() );
    std::exit( 1 );
  }
  return f;
}

void close_or_die( std::FILE *f ) {
  if ( std::ferror( f ) || std::fclose( f ) != 0 ) {
    std::perror( "write" );
    std::exit( 1 );
  }
}

}  // namespace

int main( int argc, char **argv ) {
  if ( argc != 4 ) {
    std::fprintf( stderr, "usage: gen jsonl|array|dir|csv BYTES OUT\n" );
    return 2;
  }
  std::string const form( argv[1] ), out( argv[3] );
  std::uint64_t const target = std::strtoull( argv[2], nullptr, 10 );
  bool const dir = form == "dir", csv = form == "csv", array = form == "array";
  if ( !dir && !csv && !array && form != "jsonl" ) {
    std::fprintf( stderr, "gen: unknown form %s\n", form.c_str() );
    return 2;
  }
  unsigned const per_file = 500;

  std::FILE *f = nullptr;
  if ( !dir )
    f = open_or_die( out + ( csv ? "/data.csv" : array ? "/data.json"
                                                       : "/data.jsonl" ) );
  std::uint64_t written = 0;
  auto const put = [&]( std::string const &text ) {
    std::fwrite( text.data(), 1, text.size(), f );
    written += text.size();
  };
  if ( csv )
    put( csv_header );
  if ( array )
    put( "[\n" );

  std::uint64_t seq = 0, json_bytes = 0;
  std::string json, text;
  Record r;
  // A file of one record would be an object, not an array, in dir:parse: the
  // stream does not stop just after the first record of a file.
  while ( json_bytes < target || seq % per_file == 1 ) {
    next_record( seq + 1, &r );
    to_json( r, &json );
    json_bytes += json.size() + 1;
    if ( dir && seq % per_file == 0 ) {
      if ( f )
        close_or_die( f );
      char name[32];
      std::snprintf( name, sizeof name, "/part-%06" PRIu64 ".jsonl",
                     seq / per_file );
      f = open_or_die( out + name );
    }
    ++seq;
    if ( csv ) {
      to_csv( r, &text );
      text += '\n';
      put( text );
    } else if ( array ) {
      if ( seq > 1 )
        put( ",\n" );
      put( json );
    } else {
      json += '\n';
      put( json );
    }
  }
  if ( array )
    put( "\n]\n" );
  close_or_die( f );
  std::printf( "%" PRIu64 " %" PRIu64 "\n", seq, written );
  return 0;
}
