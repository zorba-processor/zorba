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

// Previews of XML, JSON, CSV and HTML files. This code runs on the scanner's
// thread: it must not use anything of the Zorba engine (see scanner.h).

#include "scan_preview.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlversion.h>

#include "json.h"

namespace zorba {
namespace mcp {

namespace {

// How much of a file goes into a preview.
std::size_t const max_value_bytes = 40;   // a sample value or cell
std::size_t const max_name_bytes = 80;    // a key path, element path, column
std::size_t const max_columns = 32;       // CSV columns
std::size_t const max_rows = 3;           // CSV rows
std::size_t const max_paths = 32;         // JSON key paths, XML paths
std::size_t const max_samples = 2;        // JSON samples per key
std::size_t const max_arrays = 8;         // JSON array lengths
std::size_t const max_namespaces = 8;     // XML namespace declarations
std::size_t const max_tables = 8;         // HTML tables with their headers
std::size_t const max_header_cells = 16;  // HTML header cells per table
std::size_t const max_fields = 8;         // fields per file in a listing
std::size_t const max_error_bytes = 160;
std::size_t const max_depth = 64;         // JSON nesting
std::size_t const max_xml_depth = 256;    // XML nesting
std::size_t const max_html_depth = 200;   // HTML nesting
std::size_t const max_preview_bytes = 8 * 1024;  // a preview, whole
std::size_t const preview_budget = 6 * 1024;     // what the lists of one take
std::size_t const max_field_bytes = 1024;        // the fields of a listing
std::size_t const max_tag_bytes = 16 * 1024;     // a tag, without quoted values
std::size_t const max_tag_attributes = 256;
std::chrono::seconds const max_parse_time( 5 );

char const ellipsis[] = "\xE2\x80\xA6";
char const replacement[] = "\xEF\xBF\xBD";

// The length of the UTF-8 sequence at p, or 0 if it is invalid or cut off.
std::size_t utf8_length( char const *p, char const *end ) {
  unsigned char const b = static_cast<unsigned char>( *p );
  if ( b < 0x80 )
    return 1;
  std::size_t len;
  unsigned char lo = 0x80, hi = 0xBF;
  if ( b >= 0xC2 && b <= 0xDF ) {
    len = 2;
  } else if ( b >= 0xE0 && b <= 0xEF ) {
    len = 3;
    if ( b == 0xE0 ) lo = 0xA0;
    if ( b == 0xED ) hi = 0x9F;
  } else if ( b >= 0xF0 && b <= 0xF4 ) {
    len = 4;
    if ( b == 0xF0 ) lo = 0x90;
    if ( b == 0xF4 ) hi = 0x8F;
  } else {
    return 0;
  }
  if ( static_cast<std::size_t>( end - p ) < len )
    return 0;
  for ( std::size_t i = 1; i < len; ++i ) {
    unsigned char const c = static_cast<unsigned char>( p[i] );
    if ( c < lo || c > hi )
      return 0;
    lo = 0x80;
    hi = 0xBF;
  }
  return len;
}

// At most max bytes of valid UTF-8 from the text, marked if it was cut.
std::string short_text( char const *s, std::size_t n, std::size_t max ) {
  std::string out;
  char const *p = s;
  char const *const end = s + n;
  while ( p < end ) {
    std::size_t const len = utf8_length( p, end );
    if ( out.size() + ( len ? len : 3 ) > max ) {
      out += ellipsis;
      break;
    }
    if ( len ) {
      out.append( p, len );
      p += len;
    } else {
      out += replacement;
      ++p;
    }
  }
  return out;
}

std::string short_text( std::string const &s, std::size_t max ) {
  return short_text( s.data(), s.size(), max );
}

std::string json_quoted( std::string const &s ) {
  std::string out;
  append_json_string( out, s );
  return out;
}

// Adds "name":<value> to the JSON text of an object under construction.
void member( std::string &out, char const *name, std::string const &value ) {
  out += out.size() > 1 ? ",\"" : "\"";
  out += name;
  out += "\":";
  out += value;
}

// The "error" and "records"/"truncated" members every format ends with.
void finish( std::string &out, bool truncated, bool failed,
             std::size_t records, std::string const &error ) {
  if ( truncated ) {
    member( out, "records", "null" );
    member( out, "truncated", "true" );
  } else {
    member( out, "records", failed ? "null" : std::to_string( records ) );
  }
  if ( failed )
    member( out, "error", json_quoted( short_text( error, max_error_bytes ) ) );
  out += '}';
}

std::string error_preview( std::string const &error ) {
  std::string out( "{" );
  member( out, "error", json_quoted( short_text( error, max_error_bytes ) ) );
  out += '}';
  return out;
}

std::string fields_members( std::vector<std::string> const &names, bool more ) {
  if ( names.empty() )
    return std::string();
  std::string out( "\"fields\":[" );
  std::size_t shown = 0;
  for ( ; shown < names.size() && shown < max_fields; ++shown ) {
    std::string const name( json_quoted( names[ shown ] ) );
    if ( out.size() + name.size() > max_field_bytes )
      break;
    if ( shown )
      out += ',';
    out += name;
  }
  out += ']';
  if ( more || names.size() > shown )
    out += ",\"fields_more\":true";
  return out;
}

std::string string_array( std::vector<std::string> const &items ) {
  std::string out( "[" );
  for ( std::size_t i = 0; i < items.size(); ++i ) {
    if ( i )
      out += ',';
    append_json_string( out, items[i] );
  }
  return out + "]";
}

bool has_nul( std::string const &data ) {
  return data.find( '\0' ) != std::string::npos;
}

std::size_t after_bom( std::string const &data ) {
  return data.compare( 0, 3, "\xEF\xBB\xBF" ) == 0 ? 3 : 0;
}

///////////////////////////////////////////////////////////////////////////////
// CSV

class CsvColumn {
public:
  void add( std::string const &cell, bool quoted_cell, bool cut ) {
    if ( cell.empty() )
      return;
    if ( quoted_cell || cut )
      theString = true;
    else if ( cell == "true" || cell == "false" )
      theBoolean = true;
    else if ( is_integer( cell ) )
      theInteger = true;
    else if ( is_decimal( cell ) )
      theDecimal = true;
    else
      theString = true;
  }

  char const* kind() const {
    if ( theString || ( theBoolean && ( theInteger || theDecimal ) ) )
      return "string";
    if ( theBoolean )
      return "boolean";
    if ( theDecimal )
      return "decimal";
    return theInteger ? "integer" : "empty";
  }

private:
  static std::size_t digits( std::string const &s, std::size_t i ) {
    std::size_t n = 0;
    while ( i + n < s.size() &&
            std::isdigit( static_cast<unsigned char>( s[ i + n ] ) ) )
      ++n;
    return n;
  }

  static bool is_integer( std::string const &s ) {
    std::size_t const i = s[0] == '-' || s[0] == '+' ? 1 : 0;
    std::size_t const n = digits( s, i );
    return n > 0 && i + n == s.size();
  }

  static bool is_decimal( std::string const &s ) {
    std::size_t i = s[0] == '-' || s[0] == '+' ? 1 : 0;
    std::size_t n = digits( s, i );
    i += n;
    if ( i < s.size() && s[i] == '.' ) {
      ++i;
      std::size_t const frac = digits( s, i );
      n += frac;
      i += frac;
    }
    if ( n == 0 )
      return false;
    if ( i < s.size() && ( s[i] == 'e' || s[i] == 'E' ) ) {
      ++i;
      if ( i < s.size() && ( s[i] == '-' || s[i] == '+' ) )
        ++i;
      std::size_t const exp = digits( s, i );
      if ( exp == 0 )
        return false;
      i += exp;
    }
    return i == s.size();
  }

  bool theString = false, theBoolean = false, theInteger = false,
       theDecimal = false;
};

ScanPreview csv_preview( std::string const &data, bool truncated ) {
  ScanPreview result;
  if ( has_nul( data ) ) {
    result.json = error_preview( "binary data: a NUL byte" );
    return result;
  }

  std::size_t const cell_cap = 64;
  std::vector<std::string> columns;
  std::size_t column_count = 0;
  std::size_t column_bytes = 0;
  bool have_header = false;
  std::vector<CsvColumn> kinds;
  std::vector<std::string> rows;
  std::size_t records = 0;
  std::string error;

  std::string cell;
  bool cell_quoted = false, cell_cut = false, in_quotes = false;
  std::vector<std::string> row;
  std::size_t fields = 0;

  auto end_cell = [&]() {
    if ( !have_header ) {
      std::string const column( short_text( cell, max_name_bytes ) );
      std::size_t const size = json_quoted( column ).size();
      if ( columns.size() < max_columns &&
           column_bytes + size <= preview_budget / 2 ) {
        columns.push_back( column );
        column_bytes += size;
      }
    } else if ( fields < kinds.size() ) {
      kinds[ fields ].add( cell, cell_quoted, cell_cut );
      if ( records < max_rows )
        row.push_back( short_text( cell, max_value_bytes ) );
    }
    ++fields;
    cell.clear();
    cell_quoted = cell_cut = false;
  };
  auto end_row = [&]() {
    if ( fields == 0 && cell.empty() && !cell_quoted )
      return;                           // an empty line
    end_cell();
    if ( !have_header ) {
      have_header = true;
      column_count = fields;
      kinds.resize( columns.size() );
    } else {
      if ( fields > column_count && error.empty() )
        error = "row " + std::to_string( records + 1 ) + " has " +
          std::to_string( fields ) + " fields, the header " +
          std::to_string( column_count );
      if ( records < max_rows )
        rows.push_back( string_array( row ) );
      row.clear();
      ++records;
    }
    fields = 0;
  };
  auto add_char = [&]( char c ) {
    if ( cell.size() < cell_cap )
      cell += c;
    else
      cell_cut = true;
  };

  std::size_t i = after_bom( data );
  std::size_t const n = data.size();
  // The offset of the first byte that is not valid UTF-8.
  std::size_t bad_utf8 = std::string::npos;
  for ( std::size_t k = i; k < n; ) {
    std::size_t const len = utf8_length( data.data() + k, data.data() + n );
    if ( len == 0 ) {
      if ( !truncated || n - k >= 4 )
        bad_utf8 = k;
      break;
    }
    k += len;
  }
  while ( i < n ) {
    char const c = data[i];
    if ( in_quotes ) {
      if ( c == '"' ) {
        if ( i + 1 < n && data[ i + 1 ] == '"' ) {
          add_char( '"' );
          i += 2;
          continue;
        }
        in_quotes = false;
      } else {
        add_char( c );
      }
      ++i;
      continue;
    }
    switch ( c ) {
      case ',':
        end_cell();
        break;
      case '\r':
        if ( i + 1 < n && data[ i + 1 ] == '\n' )
          ++i;
        end_row();
        break;
      case '\n':
        end_row();
        break;
      case '"':
        if ( cell.empty() && !cell_quoted ) {
          in_quotes = true;
          cell_quoted = true;
        } else {
          add_char( c );
        }
        break;
      default:
        add_char( c );
    }
    ++i;
  }
  if ( in_quotes && !truncated && error.empty() )
    error = "a quoted field is not closed";
  // A last line without a newline is a row, unless the cap cut it.
  if ( !truncated && !in_quotes )
    end_row();
  if ( bad_utf8 != std::string::npos && error.empty() )
    error = "not valid UTF-8 at byte " + std::to_string( bad_utf8 );

  std::string out( "{" );
  member( out, "columns", string_array( columns ) );
  if ( column_count > columns.size() )
    member( out, "columns_total", std::to_string( column_count ) );
  std::string k( "[" );
  for ( std::size_t c = 0; c < kinds.size(); ++c ) {
    if ( c )
      k += ',';
    k += '"';
    k += kinds[c].kind();
    k += '"';
  }
  member( out, "kinds", k + "]" );
  std::string r( "[" );
  for ( std::size_t c = 0; c < rows.size(); ++c ) {
    if ( c )
      r += ',';
    r += rows[c];
  }
  member( out, "rows", r + "]" );
  finish( out, truncated, !error.empty(), records, error );
  result.json = std::move( out );
  result.fields = fields_members( columns, column_count > columns.size() );
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// JSON

// Reads the structure of a JSON text strictly, as far as the bytes go. When
// the bytes end inside a value of a file that goes on, it stops without an
// error (cut()); anywhere else that is an error.
class JsonReader {
public:
  JsonReader( std::string const &data, bool truncated ) :
    theData( data.data() ),
    theP( data.data() + after_bom( data ) ),
    theEnd( data.data() + data.size() ),
    theTruncated( truncated ) { }

  ScanPreview run() {
    std::size_t values = 0;
    std::string path;
    char first = 0;
    for ( ;; ) {
      skip_space();
      if ( theP == theEnd )
        break;
      if ( values == 0 )
        first = *theP;
      if ( values )
        ++theNotFirst;
      bool const ok = value( path, 0 );
      if ( values )
        --theNotFirst;
      if ( !ok )
        break;
      ++values;
    }
    if ( values == 0 && !theCut && theError.empty() )
      theError = "no JSON value";

    std::string out( "{" );
    char const *shape;
    if ( values > 1 )
      shape = "lines";
    else if ( first == '[' )
      shape = "array";
    else if ( first == '{' )
      shape = "object";
    else
      shape = first ? "scalar" : "empty";
    member( out, "shape", json_quoted( shape ) );

    std::string keys( "{" );
    std::vector<std::string> names;
    for ( Key const &k : theKeys ) {
      if ( keys.size() > 1 )
        keys += ',';
      append_json_string( keys, k.path );
      keys += ":[";
      for ( std::size_t i = 0; i < k.samples.size(); ++i ) {
        if ( i )
          keys += ',';
        keys += k.samples[i];
      }
      keys += ']';
      names.push_back( k.path );
    }
    member( out, "keys", keys + "}" );
    if ( theMoreKeys )
      member( out, "keys_more", "true" );
    if ( !theArrays.empty() ) {
      std::string arrays( "{" );
      for ( auto const &a : theArrays ) {
        if ( arrays.size() > 1 )
          arrays += ',';
        append_json_string( arrays, a.first );
        arrays += ':';
        arrays += std::to_string( a.second );
      }
      member( out, "arrays", arrays + "}" );
    }
    std::size_t const records =
      values == 1 && first == '[' ? theTopLength : values;
    finish( out, theCut, !theError.empty(), records, theError );

    ScanPreview result;
    result.json = std::move( out );
    result.fields = fields_members( names, theMoreKeys );
    return result;
  }

private:
  struct Key {
    std::string path;
    std::vector<std::string> samples;
  };

  bool cut() {
    if ( theTruncated ) {
      theCut = true;
      return false;
    }
    return fail( "unexpected end of the file" );
  }

  bool fail( std::string const &what ) {
    theError = what + " at byte " + std::to_string( theP - theData );
    return false;
  }

  void skip_space() {
    while ( theP < theEnd && ( *theP == ' ' || *theP == '\n' || *theP == '\t' ||
                               *theP == '\r' ) )
      ++theP;
  }

  // Registers a key of the first record; adds a sample to any known key.
  void add_key( std::string const &path, std::string const *sample ) {
    auto const known = theKeyIndex.find( path );
    std::size_t index;
    if ( known != theKeyIndex.end() ) {
      index = known->second;
    } else {
      if ( theNotFirst > 0 )
        return;
      std::size_t const size = json_quoted( path ).size();
      if ( theKeys.size() >= max_paths || path.size() > max_name_bytes ||
           theBytes + size > preview_budget ) {
        theMoreKeys = true;
        return;
      }
      theBytes += size;
      index = theKeys.size();
      theKeyIndex.emplace( path, index );
      theKeys.push_back( Key{ path, {} } );
    }
    if ( sample && theKeys[ index ].samples.size() < max_samples &&
         theBytes + sample->size() <= preview_budget ) {
      theBytes += sample->size();
      theKeys[ index ].samples.push_back( *sample );
    }
  }

  bool value( std::string &path, std::size_t depth ) {
    skip_space();
    if ( theP == theEnd )
      return cut();
    if ( depth > max_depth )
      return fail( "nesting deeper than " + std::to_string( max_depth ) );
    switch ( *theP ) {
      case '{': return object( path, depth );
      case '[': return array( path, depth );
      case '"': {
        std::string text;
        if ( !string( &text ) )
          return false;
        std::string const sample( json_quoted( short_text( text, max_value_bytes ) ) );
        add_key( path, &sample );
        return true;
      }
      case 't': return literal( path, "true" );
      case 'f': return literal( path, "false" );
      case 'n': return literal( path, "null" );
      default:  return number( path );
    }
  }

  bool literal( std::string const &path, char const *word ) {
    std::size_t const len = std::strlen( word );
    std::size_t const left = static_cast<std::size_t>( theEnd - theP );
    if ( std::strncmp( theP, word, std::min( len, left ) ) != 0 )
      return fail( "unexpected character" );
    if ( left < len )
      return cut();
    theP += len;
    std::string const sample( word );
    add_key( path, &sample );
    return true;
  }

  bool number( std::string const &path ) {
    char const *const start = theP;
    if ( theP < theEnd && *theP == '-' )
      ++theP;
    auto digits = [&]() {
      char const *const d = theP;
      while ( theP < theEnd && *theP >= '0' && *theP <= '9' )
        ++theP;
      return theP != d;
    };
    if ( theP == theEnd )
      return cut();
    char const lead = *theP;
    if ( !digits() ) {
      theP = start;
      return fail( "unexpected character" );
    }
    if ( lead == '0' && theP - start > ( *start == '-' ? 2 : 1 ) ) {
      theP = start;
      return fail( "a number with a leading zero" );
    }
    if ( theP < theEnd && *theP == '.' ) {
      ++theP;
      if ( !digits() )
        return theP == theEnd ? cut() : fail( "a malformed number" );
    }
    if ( theP < theEnd && ( *theP == 'e' || *theP == 'E' ) ) {
      ++theP;
      if ( theP < theEnd && ( *theP == '+' || *theP == '-' ) )
        ++theP;
      if ( !digits() )
        return theP == theEnd ? cut() : fail( "a malformed number" );
    }
    if ( theP == theEnd && theTruncated )
      return cut();                     // it may go on
    std::size_t const len = static_cast<std::size_t>( theP - start );
    std::string const sample( len <= max_value_bytes
      ? std::string( start, len )
      : json_quoted( short_text( start, len, max_value_bytes ) ) );
    add_key( path, &sample );
    return true;
  }

  static void append_code_point( std::string *out, unsigned long c ) {
    if ( c < 0x80 ) {
      *out += static_cast<char>( c );
    } else if ( c < 0x800 ) {
      *out += static_cast<char>( 0xC0 | ( c >> 6 ) );
      *out += static_cast<char>( 0x80 | ( c & 0x3F ) );
    } else if ( c < 0x10000 ) {
      *out += static_cast<char>( 0xE0 | ( c >> 12 ) );
      *out += static_cast<char>( 0x80 | ( ( c >> 6 ) & 0x3F ) );
      *out += static_cast<char>( 0x80 | ( c & 0x3F ) );
    } else {
      *out += static_cast<char>( 0xF0 | ( c >> 18 ) );
      *out += static_cast<char>( 0x80 | ( ( c >> 12 ) & 0x3F ) );
      *out += static_cast<char>( 0x80 | ( ( c >> 6 ) & 0x3F ) );
      *out += static_cast<char>( 0x80 | ( c & 0x3F ) );
    }
  }

  // Four hex digits at theP.
  bool hex4( unsigned long *c ) {
    if ( theEnd - theP < 4 ) {
      for ( char const *q = theP; q < theEnd; ++q )
        if ( !std::isxdigit( static_cast<unsigned char>( *q ) ) )
          return fail( "a malformed \\u escape" );
      return cut();
    }
    *c = 0;
    for ( int i = 0; i < 4; ++i, ++theP ) {
      unsigned char const h = static_cast<unsigned char>( *theP );
      if ( !std::isxdigit( h ) )
        return fail( "a malformed \\u escape" );
      *c = *c * 16 + ( std::isdigit( h ) ? h - '0'
                                         : std::tolower( h ) - 'a' + 10 );
    }
    return true;
  }

  // The string at theP; only the first bytes of it go to *out.
  bool string( std::string *out ) {
    std::size_t const keep = 2 * max_name_bytes;
    ++theP;
    for ( ;; ) {
      if ( theP == theEnd )
        return cut();
      unsigned char const c = static_cast<unsigned char>( *theP );
      if ( c == '"' ) {
        ++theP;
        return true;
      }
      if ( c < 0x20 )
        return fail( "a control character in a string" );
      if ( c == '\\' ) {
        ++theP;
        if ( theP == theEnd )
          return cut();
        char const e = *theP++;
        unsigned long code = 0;
        switch ( e ) {
          case '"': case '\\': case '/': code = static_cast<unsigned char>( e ); break;
          case 'b': code = '\b'; break;
          case 'f': code = '\f'; break;
          case 'n': code = '\n'; break;
          case 'r': code = '\r'; break;
          case 't': code = '\t'; break;
          case 'u':
            if ( !hex4( &code ) )
              return false;
            if ( code >= 0xD800 && code <= 0xDFFF )
              code = 0xFFFD;            // the sample need not be exact
            break;
          default:
            --theP;
            return fail( "a malformed escape" );
        }
        if ( out->size() < keep )
          append_code_point( out, code );
        continue;
      }
      std::size_t const len = utf8_length( theP, theEnd );
      if ( len == 0 ) {
        if ( theEnd - theP < 4 && theTruncated )
          return cut();
        return fail( "not valid UTF-8" );
      }
      if ( out->size() < keep )
        out->append( theP, len );
      theP += len;
    }
  }

  bool object( std::string &path, std::size_t depth ) {
    ++theP;
    skip_space();
    if ( theP == theEnd )
      return cut();
    if ( *theP == '}' ) {
      ++theP;
      add_key( path, nullptr );
      return true;
    }
    std::size_t const base = path.size();
    for ( ;; ) {
      skip_space();
      if ( theP == theEnd )
        return cut();
      if ( *theP != '"' )
        return fail( "expected a member name" );
      std::string key;
      if ( !string( &key ) )
        return false;
      skip_space();
      if ( theP == theEnd )
        return cut();
      if ( *theP != ':' )
        return fail( "expected ':'" );
      ++theP;
      if ( base )
        path += '.';
      path += short_text( key, max_value_bytes );
      bool const ok = value( path, depth + 1 );
      path.resize( base );
      if ( !ok )
        return false;
      skip_space();
      if ( theP == theEnd )
        return cut();
      if ( *theP == ',' ) {
        ++theP;
        continue;
      }
      if ( *theP == '}' ) {
        ++theP;
        return true;
      }
      return fail( "expected ',' or '}'" );
    }
  }

  bool array( std::string &path, std::size_t depth ) {
    ++theP;
    skip_space();
    if ( theP == theEnd )
      return cut();
    std::size_t length = 0;
    if ( *theP == ']' ) {
      ++theP;
      add_key( path, nullptr );
    } else {
      std::size_t const base = path.size();
      path += "[]";
      for ( ;; ) {
        if ( length )
          ++theNotFirst;
        bool const ok = value( path, depth + 1 );
        if ( length )
          --theNotFirst;
        if ( !ok ) {
          path.resize( base );
          return false;
        }
        ++length;
        skip_space();
        if ( theP == theEnd ) {
          path.resize( base );
          return cut();
        }
        if ( *theP == ',' ) {
          ++theP;
          continue;
        }
        if ( *theP == ']' ) {
          ++theP;
          break;
        }
        path.resize( base );
        return fail( "expected ',' or ']'" );
      }
      path.resize( base );
    }
    if ( depth == 0 ) {
      theTopLength = length;
    } else if ( theNotFirst == 0 && theArrays.size() < max_arrays &&
                path.size() <= max_name_bytes &&
                std::none_of( theArrays.begin(), theArrays.end(),
                  [&path]( std::pair<std::string,std::size_t> const &a ) {
                    return a.first == path;
                  } ) ) {
      theBytes += json_quoted( path ).size();
      theArrays.emplace_back( path, length );
    }
    return true;
  }

  char const *const theData;
  char const *theP;
  char const *const theEnd;
  bool const theTruncated;
  bool theCut = false;
  std::string theError;
  std::size_t theNotFirst = 0;
  std::size_t theTopLength = 0;
  std::vector<Key> theKeys;
  std::unordered_map<std::string,std::size_t> theKeyIndex;
  bool theMoreKeys = false;
  std::size_t theBytes = 0;
  std::vector<std::pair<std::string,std::size_t> > theArrays;
};

///////////////////////////////////////////////////////////////////////////////
// Markup, before it is parsed

char const* find_text( char const *p, char const *end, char const *text ) {
  char const *const found = std::search( p, end, text, text + std::strlen( text ) );
  return found == end ? end : found + std::strlen( text );
}

bool starts_with( char const *p, char const *end, char const *text ) {
  std::size_t const n = std::strlen( text );
  return static_cast<std::size_t>( end - p ) >= n && std::memcmp( p, text, n ) == 0;
}

bool is_markup_space( char c ) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

// Past a DOCTYPE, whose internal subset can hold '>'.
char const* past_doctype( char const *p, char const *end ) {
  int depth = 0;
  for ( ; p < end; ++p ) {
    if ( *p == '[' )
      ++depth;
    else if ( *p == ']' )
      --depth;
    else if ( *p == '>' && depth <= 0 )
      return p + 1;
  }
  return end;
}

// Why the parser should not get this markup, or the empty string. Time in
// libxml2's start tag parsing grows with the square of the attributes of one
// tag (libxml2 2.9.14: 167 s in HTML and 5 s in XML for 120000 attributes), and
// neither parser can be interrupted inside a tag, so such a tag is refused here,
// in a time linear in the bytes. Quoted values do not count: those parse in
// linear time, and data URIs make them long.
std::string markup_refusal( std::string const &data, bool html ) {
  std::string stripped;
  char const *p = data.data();
  char const *end = p + data.size();
  // UTF-16 and UTF-32: the ASCII of the markup without its NUL bytes.
  if ( !html && has_nul( data ) ) {
    stripped.reserve( data.size() );
    for ( char const c : data )
      if ( c != '\0' )
        stripped += c;
    p = stripped.data();
    end = p + stripped.size();
  }

  while ( p < end ) {
    p = std::find( p, end, '<' );
    if ( p == end )
      break;
    ++p;
    if ( p == end )
      break;
    if ( *p == '!' || *p == '?' ) {
      if ( starts_with( p, end, "!--" ) )
        p = find_text( p, end, "-->" );
      else if ( html )
        p = find_text( p, end, ">" );   // anything else ends at the first '>'
      else if ( *p == '?' )
        p = find_text( p, end, "?>" );
      else if ( starts_with( p, end, "![CDATA[" ) )
        p = find_text( p, end, "]]>" );
      else
        p = past_doctype( p, end );
      continue;
    }
    if ( *p == '/' )
      ++p;
    if ( p == end )
      break;
    unsigned char const lead = static_cast<unsigned char>( *p );
    if ( !std::isalpha( lead ) && lead != '_' && lead != ':' && lead < 0x80 )
      continue;                         // not a tag, as in "a < b"

    // The name.
    char const *const name_start = p;
    while ( p < end && !is_markup_space( *p ) && *p != '>' && *p != '/' )
      ++p;
    std::size_t bytes = static_cast<std::size_t>( p - name_start );
    std::size_t attributes = 0;
    std::string name;
    if ( html && bytes <= 6 ) {
      for ( char const *q = name_start; q < p; ++q )
        name += static_cast<char>( std::tolower( static_cast<unsigned char>( *q ) ) );
    }
    bool const closing = name_start[-1] == '/';

    // The attributes.
    while ( p < end && *p != '>' ) {
      if ( bytes > max_tag_bytes )
        return "a tag longer than " + std::to_string( max_tag_bytes ) + " bytes";
      if ( is_markup_space( *p ) || *p == '/' ) {
        ++p;
        ++bytes;
        continue;
      }
      if ( ++attributes > max_tag_attributes )
        return "a tag with more than " + std::to_string( max_tag_attributes ) +
          " attributes";
      char const *const attribute = p;
      // A first '=' is part of the name, as in HTML.
      ++p;
      while ( p < end && !is_markup_space( *p ) && *p != '=' && *p != '>' &&
              *p != '/' )
        ++p;
      bytes += static_cast<std::size_t>( p - attribute );
      while ( p < end && is_markup_space( *p ) ) {
        ++p;
        ++bytes;
      }
      if ( p == end || *p != '=' )
        continue;
      ++p;
      while ( p < end && is_markup_space( *p ) ) {
        ++p;
        ++bytes;
      }
      if ( p < end && ( *p == '"' || *p == '\'' ) ) {
        char const *const quote_end = std::find( p + 1, end, *p );
        p = quote_end == end ? end : quote_end + 1;
      } else {
        char const *const value = p;
        while ( p < end && !is_markup_space( *p ) && *p != '>' )
          ++p;
        bytes += static_cast<std::size_t>( p - value );
      }
    }
    if ( bytes > max_tag_bytes )
      return "a tag longer than " + std::to_string( max_tag_bytes ) + " bytes";
    if ( p < end )
      ++p;                              // the '>'

    // In HTML, what a script or a style holds is not markup.
    if ( !closing && ( name == "script" || name == "style" ) ) {
      std::string const close_tag( "</" + name );
      while ( p < end ) {
        p = std::find( p, end, '<' );
        if ( static_cast<std::size_t>( end - p ) < close_tag.size() ) {
          p = end;
          break;
        }
        bool same = true;
        for ( std::size_t i = 0; i < close_tag.size() && same; ++i )
          same = std::tolower( static_cast<unsigned char>( p[i] ) ) ==
            static_cast<unsigned char>( close_tag[i] );
        if ( same )
          break;
        ++p;
      }
    }
  }
  return std::string();
}

// Declared encodings other than these are refused: with one like UTF-7 the
// markup that the parser sees is not the markup that was checked.
std::string encoding_refusal( std::string const &data ) {
  if ( starts_with( data.data(), data.data() + data.size(), "\x4C\x6F\xA7\x94" ) )
    return "an EBCDIC encoding";
  std::string head;
  for ( std::size_t i = 0; i < data.size() && head.size() < 256; ++i )
    if ( data[i] != '\0' )
      head += data[i];
  std::size_t const declaration = head.find( "<?xml" );
  if ( declaration == std::string::npos )
    return std::string();
  std::size_t const declaration_end = head.find( "?>", declaration );
  std::size_t const at = head.find( "encoding", declaration );
  if ( at == std::string::npos ||
       ( declaration_end != std::string::npos && at > declaration_end ) )
    return std::string();
  std::size_t const quote_start = head.find_first_of( "\"'", at );
  if ( quote_start == std::string::npos )
    return std::string();
  std::size_t const quote_end = head.find( head[ quote_start ], quote_start + 1 );
  if ( quote_end == std::string::npos )
    return std::string();
  std::string name( head, quote_start + 1, quote_end - quote_start - 1 );
  for ( char &c : name )
    c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
  for ( char const *const known :
        { "utf-8", "utf8", "us-ascii", "ascii", "iso-8859-", "iso8859-", "latin",
          "windows-125", "cp125", "utf-16", "ucs-2", "ucs2", "shift_jis",
          "euc-", "gb", "big5" } )
    if ( name.compare( 0, std::strlen( known ), known ) == 0 )
      return std::string();
  return "the encoding " + name;
}

///////////////////////////////////////////////////////////////////////////////
// XML

char const* xml_text( xmlChar const *s ) {
  return s ? reinterpret_cast<char const*>( s ) : "";
}

// Never add NOENT, DTDLOAD, DTDATTR, DTDVALID, XINCLUDE or HUGE: without them
// the parser reads no external entity or DTD and expands no entity.
int xml_options() {
  int options = XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING;
#if LIBXML_VERSION >= 21300
  options |= XML_PARSE_NO_XXE;
#endif
  return options;
}

bool only_space( char const *s ) {
  for ( ; *s; ++s )
    if ( !std::isspace( static_cast<unsigned char>( *s ) ) )
      return false;
  return true;
}

ScanPreview xml_preview( std::string const &data, bool truncated,
                         std::atomic<bool> const *stop,
                         std::chrono::steady_clock::time_point deadline ) {
  ScanPreview result;
  if ( data.empty() ) {
    result.json = error_preview( "an empty file" );
    return result;
  }
  std::string refusal( encoding_refusal( data ) );
  if ( refusal.empty() )
    refusal = markup_refusal( data, false );
  if ( !refusal.empty() ) {
    result.json = error_preview( "not parsed: " + refusal );
    return result;
  }
  xmlTextReaderPtr const reader =
    xmlReaderForMemory( data.data(), static_cast<int>( data.size() ), nullptr,
                        nullptr, xml_options() );
  if ( !reader ) {
    result.json = error_preview( "the parser cannot start" );
    return result;
  }

  std::string root;
  std::vector<std::pair<std::string,std::string> > namespaces;
  std::vector<std::string> paths;
  std::unordered_set<std::string> seen;
  bool more_paths = false;
  std::vector<std::string> stack;       // the element path per depth
  std::string sample_path, sample_value;
  std::size_t records = 0;
  bool too_deep = false;
  bool too_slow = false;
  std::size_t list_bytes = 0;
  unsigned long nodes = 0;
  int ret;

  auto add_path = [&]( std::string const &path ) {
    if ( seen.count( path ) )
      return;
    std::size_t const size = json_quoted( path ).size();
    if ( paths.size() >= max_paths || path.size() > max_name_bytes ||
         list_bytes + size > preview_budget ) {
      more_paths = true;
      return;
    }
    list_bytes += size;
    seen.insert( path );
    paths.push_back( path );
  };

  while ( ( ret = xmlTextReaderRead( reader ) ) == 1 ) {
    if ( stop && stop->load( std::memory_order_relaxed ) )
      break;
    if ( ++nodes % 256 == 0 && std::chrono::steady_clock::now() > deadline ) {
      too_slow = true;
      break;
    }
    int const type = xmlTextReaderNodeType( reader );
    if ( type == XML_READER_TYPE_ELEMENT ) {
      int const d = xmlTextReaderDepth( reader );
      if ( d < 0 )
        continue;
      std::size_t const depth = static_cast<std::size_t>( d );
      if ( depth >= max_xml_depth ) {
        too_deep = true;
        break;
      }
      char const *const element = xml_text( xmlTextReaderConstName( reader ) );
      std::string const name(
        short_text( element, std::strlen( element ), max_name_bytes ) );
      if ( depth == 0 && root.empty() )
        root = name;
      if ( depth == 1 )
        ++records;
      stack.resize( depth );
      stack.push_back( ( depth ? stack[ depth - 1 ] : std::string() ) + "/" + name );
      add_path( stack.back() );
      if ( xmlTextReaderHasAttributes( reader ) == 1 ) {
        while ( xmlTextReaderMoveToNextAttribute( reader ) == 1 ) {
          char const *const value = xml_text( xmlTextReaderConstValue( reader ) );
          if ( xmlTextReaderIsNamespaceDecl( reader ) == 1 ) {
            char const *const prefix =
              xml_text( xmlTextReaderConstPrefix( reader ) );
            char const *const local =
              xml_text( xmlTextReaderConstLocalName( reader ) );
            std::string const declared(
              *prefix ? short_text( local, std::strlen( local ), max_name_bytes )
                      : std::string() );
            std::string const uri(
              short_text( value, std::strlen( value ), max_name_bytes ) );
            std::size_t const size = json_quoted( declared ).size() + json_quoted( uri ).size();
            // Only the first declaration of each prefix: JSON has no repeats.
            if ( namespaces.size() < max_namespaces &&
                 list_bytes + size <= preview_budget &&
                 std::none_of( namespaces.begin(), namespaces.end(),
                   [&declared]( std::pair<std::string,std::string> const &n ) {
                     return n.first == declared;
                   } ) ) {
              list_bytes += size;
              namespaces.emplace_back( declared, uri );
            }
          } else {
            char const *const attr = xml_text( xmlTextReaderConstName( reader ) );
            add_path( stack.back() + "/@" +
                      short_text( attr, std::strlen( attr ), max_name_bytes ) );
          }
        }
        xmlTextReaderMoveToElement( reader );
      }
    } else if ( ( type == XML_READER_TYPE_TEXT ||
                  type == XML_READER_TYPE_CDATA ) && sample_path.empty() &&
                !stack.empty() ) {
      char const *const value = xml_text( xmlTextReaderConstValue( reader ) );
      int const d = xmlTextReaderDepth( reader );
      if ( !only_space( value ) && d > 0 &&
           static_cast<std::size_t>( d ) <= stack.size() ) {
        sample_path = stack[ static_cast<std::size_t>( d ) - 1 ];
        sample_value = short_text( value, std::strlen( value ), max_value_bytes );
      }
    }
  }

  std::string error;
  if ( too_deep ) {
    error = "nesting deeper than " + std::to_string( max_xml_depth );
  } else if ( too_slow ) {
    error = "parsing stopped after " +
      std::to_string( max_parse_time.count() ) + " s";
  } else if ( ret < 0 ) {
    // A file cut by the cap ends in the middle of something: only an error
    // before the end of what was read says the file is malformed.
    long const consumed = xmlTextReaderByteConsumed( reader );
    if ( !truncated || consumed < 0 ||
         static_cast<std::size_t>( consumed ) + 4096 < data.size() ) {
#if LIBXML_VERSION >= 21300
      xmlError const *const e = xmlTextReaderGetLastError( reader );
#else
      xmlError const *const e = xmlGetLastError();  // per thread
#endif
      error = e && e->message ? e->message : "not well-formed";
      while ( !error.empty() && error.back() == '\n' )
        error.pop_back();
    }
  } else if ( ret == 0 && root.empty() ) {
    error = "no root element";
  }
  xmlFreeTextReader( reader );

  std::string out( "{" );
  member( out, "root", root.empty() ? "null" : json_quoted( root ) );
  std::string ns( "{" );
  for ( auto const &n : namespaces ) {
    if ( ns.size() > 1 )
      ns += ',';
    append_json_string( ns, n.first );
    ns += ':';
    append_json_string( ns, n.second );
  }
  member( out, "namespaces", ns + "}" );
  member( out, "paths", string_array( paths ) );
  if ( more_paths )
    member( out, "paths_more", "true" );
  if ( !sample_path.empty() ) {
    std::string s( "{" );
    member( s, "path", json_quoted( sample_path ) );
    member( s, "value", json_quoted( sample_value ) );
    member( out, "sample", s + "}" );
  }
  finish( out, truncated, !error.empty(), records, error );
  result.json = std::move( out );
  result.fields = fields_members( paths, more_paths );
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// HTML

#if defined( LIBXML_HTML_ENABLED ) && defined( LIBXML_PUSH_ENABLED )

// What the SAX callbacks collect. No tree is built: memory does not grow with
// the markup, and nesting takes no stack here.
struct HtmlCollector {
  xmlParserCtxtPtr ctxt = nullptr;
  std::string title;
  bool have_title = false;
  bool in_title = false;
  bool in_header = false;
  std::size_t tables = 0;
  std::vector<std::vector<std::string> > headers;
  std::vector<std::size_t> open_tables;  // index into headers + 1; 0: not kept
  std::size_t depth = 0;
  bool too_deep = false;
  bool failed = false;                  // an allocation failed
};

bool html_name_is( xmlChar const *name, char const *what ) {
  return xmlStrcasecmp( name, reinterpret_cast<xmlChar const*>( what ) ) == 0;
}

// No exception may leave a callback: libxml2 is C code. An allocation that fails
// stops the parser.
void html_start_element( HtmlCollector &c, xmlChar const *name );

void html_start( void *user, xmlChar const *name, xmlChar const** ) {
  HtmlCollector &c = *static_cast<HtmlCollector*>( user );
  try {
    html_start_element( c, name );
  }
  catch ( std::exception const& ) {
    c.failed = true;
    xmlStopParser( c.ctxt );
  }
}

void html_start_element( HtmlCollector &c, xmlChar const *name ) {
  if ( c.too_deep || c.failed )
    return;
  if ( ++c.depth > max_html_depth ) {
    c.too_deep = true;
    xmlStopParser( c.ctxt );
    return;
  }
  if ( html_name_is( name, "title" ) ) {
    if ( !c.have_title ) {
      c.have_title = true;
      c.in_title = true;
    }
  } else if ( html_name_is( name, "table" ) ) {
    ++c.tables;
    if ( c.headers.size() < max_tables ) {
      c.headers.emplace_back();
      c.open_tables.push_back( c.headers.size() );
    } else {
      c.open_tables.push_back( 0 );
    }
  } else if ( html_name_is( name, "th" ) && !c.open_tables.empty() &&
              c.open_tables.back() != 0 ) {
    std::vector<std::string> &cells = c.headers[ c.open_tables.back() - 1 ];
    if ( cells.size() < max_header_cells ) {
      cells.emplace_back();
      c.in_header = true;
    }
  }
}

void html_end( void *user, xmlChar const *name ) {
  HtmlCollector &c = *static_cast<HtmlCollector*>( user );
  if ( c.depth > 0 )
    --c.depth;
  if ( html_name_is( name, "title" ) ) {
    c.in_title = false;
  } else if ( html_name_is( name, "th" ) ) {
    c.in_header = false;
  } else if ( html_name_is( name, "table" ) && !c.open_tables.empty() ) {
    c.open_tables.pop_back();
    c.in_header = false;
  }
}

void html_characters( void *user, xmlChar const *text, int length ) {
  HtmlCollector &c = *static_cast<HtmlCollector*>( user );
  std::size_t const keep = 4 * max_value_bytes;
  std::string *target = nullptr;
  if ( c.in_title )
    target = &c.title;
  else if ( c.in_header && !c.open_tables.empty() && c.open_tables.back() != 0 )
    target = &c.headers[ c.open_tables.back() - 1 ].back();
  if ( !target || target->size() >= keep || length <= 0 )
    return;
  try {
    target->append( reinterpret_cast<char const*>( text ),
                    std::min( static_cast<std::size_t>( length ),
                              keep - target->size() ) );
  }
  catch ( std::exception const& ) {
    c.failed = true;
    xmlStopParser( c.ctxt );
  }
}

std::string trimmed_short( std::string const &text ) {
  std::size_t const first = text.find_first_not_of( " \t\r\n" );
  if ( first == std::string::npos )
    return std::string();
  std::size_t const last = text.find_last_not_of( " \t\r\n" );
  return short_text( text.data() + first, last - first + 1, max_value_bytes );
}

ScanPreview html_preview( std::string const &data, bool truncated,
                          std::atomic<bool> const *stop,
                          std::chrono::steady_clock::time_point deadline ) {
  ScanPreview result;
  if ( data.empty() ) {
    result.json = error_preview( "an empty file" );
    return result;
  }
  if ( has_nul( data ) ) {
    result.json = error_preview( "binary data: a NUL byte" );
    return result;
  }
  std::string const refusal( markup_refusal( data, true ) );
  if ( !refusal.empty() ) {
    result.json = error_preview( "not parsed: " + refusal );
    return result;
  }

  HtmlCollector collected;
  htmlSAXHandler sax;
  std::memset( &sax, 0, sizeof sax );
  sax.startElement = html_start;
  sax.endElement = html_end;
  sax.characters = html_characters;
  htmlParserCtxtPtr const ctxt = htmlCreatePushParserCtxt(
    &sax, &collected, nullptr, 0, nullptr, XML_CHAR_ENCODING_NONE );
  if ( !ctxt ) {
    result.json = error_preview( "the parser cannot start" );
    return result;
  }
  collected.ctxt = ctxt;
  // IGNORE_ENC: a declared encoding could make the parser see other markup than
  // was checked above.
  if ( htmlCtxtUseOptions( ctxt, HTML_PARSE_NONET | HTML_PARSE_NOERROR |
                                 HTML_PARSE_NOWARNING | HTML_PARSE_RECOVER |
                                 HTML_PARSE_IGNORE_ENC ) < 0 ) {
    htmlFreeParserCtxt( ctxt );
    result.json = error_preview( "the parser cannot start" );
    return result;
  }

  std::size_t const chunk = 64 * 1024;
  bool too_slow = false;
  bool stopped = false;
  for ( std::size_t at = 0;
        at < data.size() && !collected.too_deep && !collected.failed; at += chunk ) {
    if ( stop && stop->load( std::memory_order_relaxed ) ) {
      stopped = true;
      break;
    }
    if ( std::chrono::steady_clock::now() > deadline ) {
      too_slow = true;
      break;
    }
    // What the parser reports is not an error here: it recovers.
    int const parsed = htmlParseChunk(
      ctxt, data.data() + at,
      static_cast<int>( std::min( chunk, data.size() - at ) ), 0 );
    ( void )parsed;
  }
  if ( !stopped && !too_slow && !collected.too_deep && !collected.failed ) {
    int const parsed = htmlParseChunk( ctxt, nullptr, 0, 1 );
    ( void )parsed;
  }
  htmlFreeParserCtxt( ctxt );

  std::string out( "{" );
  if ( collected.have_title )
    member( out, "title", json_quoted( trimmed_short( collected.title ) ) );
  member( out, "tables", truncated ? "null" : std::to_string( collected.tables ) );
  std::string h( "[" );
  for ( std::size_t i = 0; i < collected.headers.size(); ++i ) {
    std::vector<std::string> cells;
    for ( std::string const &cell : collected.headers[i] )
      cells.push_back( trimmed_short( cell ) );
    std::string const table( string_array( cells ) );
    if ( h.size() + table.size() > preview_budget )
      break;
    if ( i )
      h += ',';
    h += table;
  }
  member( out, "headers", h + "]" );
  if ( truncated )
    member( out, "truncated", "true" );
  if ( collected.too_deep )
    member( out, "error", json_quoted( "nesting deeper than " +
                                  std::to_string( max_html_depth ) ) );
  else if ( collected.failed )
    member( out, "error", json_quoted( "out of memory" ) );
  else if ( too_slow )
    member( out, "error", json_quoted( "parsing stopped after " +
                                  std::to_string( max_parse_time.count() ) + " s" ) );
  out += '}';
  result.json = std::move( out );
  return result;
}

#else

ScanPreview html_preview( std::string const&, bool, std::atomic<bool> const*,
                          std::chrono::steady_clock::time_point ) {
  ScanPreview result;
  result.json = error_preview( "libxml2 was built without HTML support" );
  return result;
}

#endif /* LIBXML_HTML_ENABLED */

} // namespace

///////////////////////////////////////////////////////////////////////////////

ScanFormat scan_format( std::string const &name ) {
  std::size_t const slash = name.rfind( '/' );
  std::size_t const dot = name.rfind( '.' );
  if ( dot == std::string::npos ||
       ( slash != std::string::npos && dot < slash ) )
    return ScanFormat::none;
  std::string ext( name.substr( dot + 1 ) );
  for ( char &c : ext )
    if ( c >= 'A' && c <= 'Z' )
      c = static_cast<char>( c - 'A' + 'a' );
  if ( ext == "xml" || ext == "xhtml" )
    return ScanFormat::xml;
  if ( ext == "json" || ext == "jsonl" || ext == "ndjson" || ext == "geojson" )
    return ScanFormat::json;
  if ( ext == "csv" )
    return ScanFormat::csv;
  if ( ext == "html" || ext == "htm" )
    return ScanFormat::html;
  return ScanFormat::none;
}

char const* scan_format_name( ScanFormat format ) {
  switch ( format ) {
    case ScanFormat::xml:  return "xml";
    case ScanFormat::json: return "json";
    case ScanFormat::csv:  return "csv";
    case ScanFormat::html: return "html";
    case ScanFormat::none: break;
  }
  return "";
}

ScanPreview make_scan_preview( ScanFormat format, std::string const &data,
                               bool truncated, std::atomic<bool> const *stop ) {
  auto const deadline = std::chrono::steady_clock::now() + max_parse_time;
  ScanPreview result;
  switch ( format ) {
    case ScanFormat::xml:
      result = xml_preview( data, truncated, stop, deadline );
      break;
    case ScanFormat::html:
      result = html_preview( data, truncated, stop, deadline );
      break;
    case ScanFormat::csv:
      result = csv_preview( data, truncated );
      break;
    case ScanFormat::json:
      if ( has_nul( data ) )
        result.json = error_preview( "binary data: a NUL byte" );
      else
        result = JsonReader( data, truncated ).run();
      break;
    case ScanFormat::none:
      return result;
  }
  // The lists have budgets; this is for the members that do not.
  if ( result.json.size() >= max_preview_bytes ) {
    result.json = error_preview( "the preview is over " +
      std::to_string( max_preview_bytes ) + " bytes" );
    result.fields.clear();
  }
  return result;
}

std::string scan_path_text( std::string const &path ) {
  static char const hex[] = "0123456789ABCDEF";
  std::string out;
  char const *p = path.data();
  char const *const end = p + path.size();
  while ( p < end ) {
    std::size_t const len = utf8_length( p, end );
    if ( len ) {
      out.append( p, len );
      p += len;
    } else {
      unsigned char const b = static_cast<unsigned char>( *p++ );
      out += "\\x";
      out += hex[ b >> 4 ];
      out += hex[ b & 0x0F ];
    }
  }
  return out;
}

std::string scan_path_from_text( std::string const &text ) {
  auto digit = []( char c ) {
    if ( c >= '0' && c <= '9' )
      return c - '0';
    if ( c >= 'A' && c <= 'F' )
      return c - 'A' + 10;
    return -1;
  };
  std::string out;
  for ( std::size_t i = 0; i < text.size(); ++i ) {
    if ( text[i] == '\\' && i + 3 < text.size() && text[ i + 1 ] == 'x' &&
         digit( text[ i + 2 ] ) >= 0 && digit( text[ i + 3 ] ) >= 0 ) {
      out += static_cast<char>( digit( text[ i + 2 ] ) * 16 + digit( text[ i + 3 ] ) );
      i += 3;
    } else {
      out += text[i];
    }
  }
  return out;
}

std::string scan_valid_utf8( std::string const &s ) {
  return short_text( s, s.size() + 1 + 3 * s.size() );
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
