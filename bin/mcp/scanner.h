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

#ifndef ZORBA_MCP_SCANNER_H
#define ZORBA_MCP_SCANNER_H

#include <cstddef>
#include <functional>
#include <string>

namespace zorba {
namespace mcp {

/** What the scanner may use. */
struct ScanLimits {
  /** Bytes read from one file. */
  std::size_t max_file_bytes = 1024 * 1024;
  /** Files kept in the listing. */
  std::size_t max_files = 1000000;
  /** Total bytes of previews kept in memory. */
  std::size_t max_preview_memory = 64 * 1024 * 1024;
};

/**
 * Called by the scanner's threads before they list a directory (stage "walk")
 * and before they read a file (stage "sample"), with the path relative to the
 * root. For tests, to hold a thread at a known point.
 */
typedef std::function<void( char const *stage, std::string const &path )>
  ScanHook;

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_SCANNER_H */
/* vim:set et sw=2 ts=2: */
