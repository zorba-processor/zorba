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
#include <cstring>
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

std::string quoted( std::string const &s ) {
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
    member( out, "error", quoted( short_text( error, max_error_bytes ) ) );
  out += '}';
}

std::string error_preview( std::string const &error ) {
  std::string out( "{" );
  member( out, "error", quoted( short_text( error, max_error_bytes ) ) );
  out += '}';
  return out;
}

std::string fields_members( std::vector<std::string> const &names, bool more ) {
  if ( names.empty() )
    return std::string();
  std::string out( "\"fields\":[" );
  for ( std::size_t i = 0; i < names.size() && i < max_fields; ++i ) {
    if ( i )
      out += ',';
    append_json_string( out, names[i] );
  }
  out += ']';
  if ( more || names.size() > max_fields )
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
      if ( columns.size() < max_columns )
        columns.push_back( short_text( cell, max_name_bytes ) );
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
    member( out, "shape", quoted( shape ) );

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
      if ( theKeys.size() >= max_paths || path.size() > max_name_bytes ) {
        theMoreKeys = true;
        return;
      }
      index = theKeys.size();
      theKeyIndex.emplace( path, index );
      theKeys.push_back( Key{ path, {} } );
    }
    if ( sample && theKeys[ index ].samples.size() < max_samples )
      theKeys[ index ].samples.push_back( *sample );
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
        std::string const sample( quoted( short_text( text, max_value_bytes ) ) );
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
      : quoted( short_text( start, len, max_value_bytes ) ) );
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
                path.size() <= max_name_bytes ) {
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
  std::vector<std::pair<std::string,std::size_t> > theArrays;
};

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
                         std::atomic<bool> const *stop ) {
  ScanPreview result;
  if ( data.empty() ) {
    result.json = error_preview( "an empty file" );
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
  int ret;

  auto add_path = [&]( std::string const &path ) {
    if ( seen.count( path ) )
      return;
    if ( paths.size() >= max_paths || path.size() > max_name_bytes ) {
      more_paths = true;
      return;
    }
    seen.insert( path );
    paths.push_back( path );
  };

  while ( ( ret = xmlTextReaderRead( reader ) ) == 1 ) {
    if ( stop && stop->load( std::memory_order_relaxed ) )
      break;
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
            if ( namespaces.size() < max_namespaces ) {
              char const *const prefix =
                xml_text( xmlTextReaderConstPrefix( reader ) );
              namespaces.emplace_back(
                *prefix ? xml_text( xmlTextReaderConstLocalName( reader ) ) : "",
                short_text( value, std::strlen( value ), max_name_bytes ) );
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
  member( out, "root", root.empty() ? "null" : quoted( root ) );
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
    member( s, "path", quoted( sample_path ) );
    member( s, "value", quoted( sample_value ) );
    member( out, "sample", s + "}" );
  }
  finish( out, truncated, !error.empty(), records, error );
  result.json = std::move( out );
  result.fields = fields_members( paths, more_paths );
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// HTML

#ifdef LIBXML_HTML_ENABLED

bool is_element( xmlNode const *node, char const *name ) {
  return node->type == XML_ELEMENT_NODE &&
    xmlStrcasecmp( node->name, reinterpret_cast<xmlChar const*>( name ) ) == 0;
}

std::string node_text( xmlNode const *node ) {
  xmlChar *const text = xmlNodeGetContent( node );
  std::string out( xml_text( text ) );
  if ( text )
    xmlFree( text );
  std::size_t const first = out.find_first_not_of( " \t\r\n" );
  if ( first == std::string::npos )
    return std::string();
  std::size_t const last = out.find_last_not_of( " \t\r\n" );
  return short_text( out.data() + first, last - first + 1, max_value_bytes );
}

ScanPreview html_preview( std::string const &data, bool truncated,
                          std::atomic<bool> const *stop ) {
  ScanPreview result;
  if ( data.empty() ) {
    result.json = error_preview( "an empty file" );
    return result;
  }
  if ( has_nul( data ) ) {
    result.json = error_preview( "binary data: a NUL byte" );
    return result;
  }
  htmlParserCtxtPtr const ctxt = htmlNewParserCtxt();
  if ( !ctxt ) {
    result.json = error_preview( "the parser cannot start" );
    return result;
  }
  htmlDocPtr const doc = htmlCtxtReadMemory(
    ctxt, data.data(), static_cast<int>( data.size() ), nullptr, nullptr,
    HTML_PARSE_NONET | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING |
    HTML_PARSE_RECOVER );
  if ( !doc ) {
    htmlFreeParserCtxt( ctxt );
    result.json = error_preview( "not parsable as HTML" );
    return result;
  }

  std::string title;
  bool have_title = false;
  std::size_t tables = 0;
  std::vector<std::vector<std::string> > headers;
  std::vector<std::size_t> open_tables;  // 0: over max_tables
  std::size_t depth = 0;
  bool too_deep = false;

  // No recursion: a tree of any depth takes no stack.
  xmlNode const *node = xmlDocGetRootElement( doc );
  xmlNode const *const top = node;
  while ( node ) {
    if ( stop && stop->load( std::memory_order_relaxed ) )
      break;
    if ( is_element( node, "title" ) && !have_title ) {
      title = node_text( node );
      have_title = true;
    } else if ( is_element( node, "table" ) ) {
      ++tables;
      if ( headers.size() < max_tables ) {
        headers.emplace_back();
        open_tables.push_back( headers.size() );
      } else {
        open_tables.push_back( 0 );
      }
    } else if ( is_element( node, "th" ) && !open_tables.empty() &&
                open_tables.back() != 0 ) {
      std::vector<std::string> &cells = headers[ open_tables.back() - 1 ];
      if ( cells.size() < max_header_cells )
        cells.push_back( node_text( node ) );
    }
    if ( node->children && node->type == XML_ELEMENT_NODE &&
         !is_element( node, "th" ) && !is_element( node, "title" ) ) {
      if ( depth < max_html_depth ) {
        ++depth;
        node = node->children;
        continue;
      }
      too_deep = true;
    }
    // Leave the node, and the ancestors it was the last child of.
    for ( ;; ) {
      if ( is_element( node, "table" ) )
        open_tables.pop_back();
      if ( node == top ) {
        node = nullptr;
        break;
      }
      if ( node->next ) {
        node = node->next;
        break;
      }
      node = node->parent;
      --depth;
    }
  }
  xmlFreeDoc( doc );
  htmlFreeParserCtxt( ctxt );

  std::string out( "{" );
  if ( have_title )
    member( out, "title", quoted( title ) );
  member( out, "tables", truncated ? "null" : std::to_string( tables ) );
  std::string h( "[" );
  for ( std::size_t i = 0; i < headers.size(); ++i ) {
    if ( i )
      h += ',';
    h += string_array( headers[i] );
  }
  member( out, "headers", h + "]" );
  if ( truncated )
    member( out, "truncated", "true" );
  if ( too_deep )
    member( out, "error", quoted( "nesting deeper than " +
                                  std::to_string( max_html_depth ) ) );
  out += '}';
  result.json = std::move( out );
  return result;
}

#else

ScanPreview html_preview( std::string const&, bool,
                          std::atomic<bool> const* ) {
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
  switch ( format ) {
    case ScanFormat::xml:  return xml_preview( data, truncated, stop );
    case ScanFormat::html: return html_preview( data, truncated, stop );
    case ScanFormat::csv:  return csv_preview( data, truncated );
    case ScanFormat::json: {
      if ( has_nul( data ) ) {
        ScanPreview result;
        result.json = error_preview( "binary data: a NUL byte" );
        return result;
      }
      return JsonReader( data, truncated ).run();
    }
    case ScanFormat::none: break;
  }
  return ScanPreview();
}

std::string scan_valid_utf8( std::string const &s ) {
  return short_text( s, s.size() + 1 + 3 * s.size() );
}

} // namespace mcp
} // namespace zorba
/* vim:set et sw=2 ts=2: */
