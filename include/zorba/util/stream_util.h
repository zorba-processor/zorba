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

#pragma once
#ifndef ZORBA_API_STREAM_UTIL_H
#define ZORBA_API_STREAM_UTIL_H

#include <iostream>
#include <string>

#include <zorba/config.h>

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

/**
 * Skips the UTF-8 byte order mark (BOM), if any, at the current position.  This
 * also works on a stream that cannot seek, except that a BOM split across two
 * buffers of the stream stays in it.
 *
 * @param is The istream to read.
 * @param remember If \c true, the stream remembers that it had a BOM: see
 * skip_utf8_bom_again().
 * @return Returns \c true only if a BOM was present and skipped.
 */
ZORBA_DLL_PUBLIC
bool skip_utf8_bom( std::istream &is, bool remember = true );

/**
 * Skips the BOM that skip_utf8_bom() remembered, if it is still there, for code
 * that has just sought to the start of the same istream.
 *
 * @param is The istream, positioned at its start.
 */
ZORBA_DLL_PUBLIC
void skip_utf8_bom_again( std::istream &is );

/**
 * Gets the next line of a text, as F&O defines lines for fn:unparsed-text-lines:
 * they end with x0A, x0D x0A or x0D, and what follows the last end of line is a
 * line only if it is not empty.
 *
 * @param is The istream to read.
 * @param line The string to put the line into, without its end.
 * @return Returns \c true only if there was a line.
 */
ZORBA_DLL_PUBLIC
bool get_text_line( std::istream &is, std::string *line );

/**
 * Reads from the given istream until \a n non-whitespace characters are read
 * or until EOF is encountered.
 *
 * @param is The istream to read from.
 * @param buf A pointer to the start of a buffer to read into.
 * @param n The number of non-whitespace characters to read.
 * @return Returns the number of non-whitespace characters read.
 */
ZORBA_DLL_PUBLIC
std::streamsize read_without_whitespace( std::istream &is, char *buf,
                                         std::streamsize n );

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba

#endif /* ZORBA_API_STREAM_UTIL_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
