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

#ifndef ZORBA_MCP_SCAN_PREVIEW_H
#define ZORBA_MCP_SCAN_PREVIEW_H

#include <atomic>
#include <string>

namespace zorba {
namespace mcp {

/** The formats of the directory module (see dir:format). */
enum class ScanFormat { none, xml, json, csv, html };

/** The format of a file name, by extension, as dir:format decides it. */
ScanFormat scan_format( std::string const &name );

/** The name of a format, for answers; never called for ScanFormat::none. */
char const* scan_format_name( ScanFormat format );

struct ScanPreview {
  /** The preview, as a JSON object. */
  std::string json;
  /**
   * The members \c "fields":[...] (and \c "fields_more":true) to put in a
   * listing, as JSON text; empty if the format has none.
   */
  std::string fields;
};

/**
 * Makes the preview of a file from the bytes read from it. Touches nothing
 * but its arguments and libxml2 contexts of its own, so any thread can call
 * it; libxml2 must have been initialized (xmlInitParser). Never throws for
 * bad input: what cannot be parsed gives an \c error member.
 *
 * @param format the format; not ScanFormat::none.
 * @param data the bytes.
 * @param truncated whether the file goes on after \a data.
 * @param stop if it becomes true, the result may be incomplete and is to be
 * thrown away.
 * @return the preview: under 8 KB, with an \c error if parsing took over 5 s, or
 * if markup has one tag over 16 KB (without quoted values) or 256 attributes.
 */
ScanPreview make_scan_preview( ScanFormat format, std::string const &data,
                               bool truncated, std::atomic<bool> const *stop );

/**
 * A path as text: bytes that are not UTF-8 become \\xNN (two upper-case hex
 * digits), so that different names stay different.
 */
std::string scan_path_text( std::string const &path );

/** The bytes of a path from its text (see scan_path_text). */
std::string scan_path_from_text( std::string const &text );

/** \a s with every invalid UTF-8 sequence replaced by U+FFFD. */
std::string scan_valid_utf8( std::string const &s );

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_SCAN_PREVIEW_H */
/* vim:set et sw=2 ts=2: */
