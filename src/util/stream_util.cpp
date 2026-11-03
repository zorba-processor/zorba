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

// standard
#include <cstring>                      /* for memcmp(3) */

// local
#include "ascii_util.h"
#include "stream_util.h"
#include "utf8_util.h"

using namespace std;

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

int get_stream_uri_index() {
  //
  // This function is out-of-line because it has a static constant within it.
  // It has a static constant within it to guarantee (1) initialization before
  // use and (2) initialization happens exactly once.
  //
  // See: "Standard C++ IOStreams and Locales: Advanced Programmer's Guide and
  // Reference," Angelika Langer and Klaus Kreft, Addison-Wesley, 2000, section
  // 3.3.1.1: "Initializing and Maintaining the iword/pword Index."
  //
  // See: "The C++ Programming Language," Bjarne Stroustrup, Addison-Wesley,
  // 2000, section 10.4.8: "Local Static Store."
  //
  static int const index = ios_base::xalloc();
  return index;
}

void delete_stream_uri_callback( ios_base::event e, ios_base &ios,
                                 int index ) {
  //
  // See: "Standard C++ IOStreams and Locales: Advanced Programmer's Guide and
  // Reference," Angelika Langer and Klaus Kreft, Addison-Wesley, 2000, section
  // 3.3.1.4: "Using Stream Callbacks for Memory Management."
  //
  if ( e == ios_base::erase_event )
    delete[] static_cast<char const*>( ios.pword( index ) );
}

///////////////////////////////////////////////////////////////////////////////

streamsize read_without_whitespace( istream &is, char *buf, streamsize n ) {
  char const *const buf_orig = buf;
  char const *const buf_end = buf + n;

  while ( buf < buf_end ) {
    is.read( buf, n );
    if ( streamsize read = is.gcount() ) {
      read = ascii::remove_space(
        buf, static_cast<ascii::size_type>( read )
      );
      buf += read, n -= read;
    } else
      break;
  }
  return buf - buf_orig;
}

ostream& roman( ostream &o, unsigned n ) {
  //
  // Based on http://rosettacode.org/wiki/Roman_numerals/Encode#C.2B.2B
  //
  struct roman_data {
    unsigned value;
    char const *numeral[2];
  };
  static roman_data const data[] = {
    { 1000, {  "m",  "M" } },
    {  900, { "cm", "CM" } },
    {  500, {  "d",  "D" } },
    {  400, { "cd", "CD" } },
    {  100, {  "c",  "C" } },
    {   90, { "xc", "XC" } },
    {   50, {  "l",  "L" } },
    {   40, { "xl", "XL" } },
    {   10, {  "x",  "X" } },
    {    9, { "ix", "IX" } },
    {    5, {  "v",  "V" } },
    {    4, { "iv", "IV" } },
    {    1, {  "i",  "I" } },
    {    0, {    0,    0 } }
  };
  bool const uc = !!(o.flags() & ios::uppercase);
  for ( roman_data const *r = data; r->value; ++r )
    for ( ; n >= r->value; n -= r->value )
      o << r->numeral[ uc ];
  return o;
}

static int utf8_bom_skipped_index() {
  static int const index = ios_base::xalloc();
  return index;
}

static bool skip_bom_bytes( istream &is ) {
  typedef streambuf::traits_type traits;
  streambuf *const buf = is.rdbuf();
  //
  // Through the streambuf: a streambuf's exception must get through, and the
  // state of the stream must not change.  The stream may be unable to seek, so
  // look only at bytes that are in the get area, where they can be put back.
  //
  if ( buf->sgetc() != traits::to_int_type( utf8::BOM[0] ) ||
       buf->in_avail() < static_cast<streamsize>( sizeof utf8::BOM ) )
    return false;
  buf->sbumpc();
  if ( buf->sgetc() == traits::to_int_type( utf8::BOM[1] ) ) {
    buf->sbumpc();
    if ( buf->sgetc() == traits::to_int_type( utf8::BOM[2] ) ) {
      buf->sbumpc();
      return true;
    }
    buf->sungetc();
  }
  buf->sungetc();
  return false;
}

bool skip_utf8_bom( istream &is, bool remember ) {
  bool const skipped = skip_bom_bytes( is );
  if ( skipped && remember )
    is.iword( utf8_bom_skipped_index() ) = 1;
  return skipped;
}

void skip_utf8_bom_again( istream &is ) {
  long &skipped = is.iword( utf8_bom_skipped_index() );
  if ( skipped && !skip_bom_bytes( is ) )
    skipped = 0;                        // the file changed: nothing to skip
}

bool get_text_line( istream &is, string *line ) {
  typedef streambuf::traits_type traits;
  streambuf *const buf = is.rdbuf();
  line->clear();
  traits::int_type c = buf->sbumpc();
  if ( traits::eq_int_type( c, traits::eof() ) )
    return false;
  for ( ; !traits::eq_int_type( c, traits::eof() ); c = buf->sbumpc() ) {
    if ( c == '\n' )
      break;
    if ( c == '\r' ) {
      if ( buf->sgetc() == '\n' )
        buf->sbumpc();
      break;
    }
    *line += traits::to_char_type( c );
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba
/* vim:set et sw=2 ts=2: */
