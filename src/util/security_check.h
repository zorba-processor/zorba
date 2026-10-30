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
#ifndef ZORBA_SECURITY_CHECK_H
#define ZORBA_SECURITY_CHECK_H

#include <string>
#include <vector>

#include <zorba/security.h>

namespace zorba {
namespace security {

/**
 * What a URI is accessed for. It decides which capability applies.
 */
enum class Use
{
  /** Module or schema source. Files under a trusted path need no capability;
   *  network access needs RemoteImport and Network. */
  Import,
  /** Thesaurus or stop words. Files under a trusted path need no capability;
   *  network access needs Network. */
  Data,
  /** Documents, unparsed text, external entities, anything else: FileRead
   *  (or FileWrite) and Network. */
  Document,
  /** Collections live in the store; no capability needed. */
  Collection
};

/**
 * The outcome of a check. If the access is denied, reason says why (for the
 * error message), e.g. "requires capability FileRead".
 */
struct Verdict
{
  bool        allowed = true;
  std::string reason;
};

/**
 * Checks an access to aURI under aPolicy.
 *
 * - A path or file: URI is checked against FileRead (FileWrite if aWrite)
 *   and the read (write) paths. The path compared is the canonical form of
 *   the path the file resolver opens: normalized, made absolute against the
 *   current directory, with symbolic links of its existing part resolved.
 *   For Use::Import and Use::Data, a path under one of aTrustedDirs (which
 *   must be canonical, see canonical_path()) is allowed without capability.
 * - An http, https or ftp URL needs Network (and RemoteImport for
 *   Use::Import), must match a network prefix if there are any (same scheme,
 *   host and port; path on a segment boundary, after dot-segment removal),
 *   and must not resolve to a loopback, private or link-local address unless
 *   the policy allows private addresses or the matching prefix names that
 *   address literally (or "localhost").
 * - "xqftts:" thesaurus URIs are file: URIs; "wordnet:" thesauri come from
 *   the library path and are allowed.
 * - Any other scheme is allowed only if neither file nor network access is
 *   restricted at all.
 */
Verdict check(
    const SecurityPolicy& aPolicy,
    const std::string& aURI,
    Use aUse,
    bool aWrite = false,
    const std::vector<std::string>* aTrustedDirs = nullptr);

/**
 * The canonical form of a path or file: URI: normalized, absolute against
 * the current directory, symbolic links resolved in the longest existing
 * leading part. Throws std::invalid_argument if it cannot be computed.
 */
std::string canonical_path(const std::string& aPathOrFileURI);

/**
 * True if canonical path aPath equals canonical directory aDir or lies below
 * it. Case-insensitive where the file system of aDir is (Windows; macOS
 * volumes that report so).
 */
bool path_within(const std::string& aDir, const std::string& aPath);

/**
 * The lower-cased scheme of a URI, or "" if it has none (a single letter
 * before ':' is a Windows drive, not a scheme).
 */
std::string uri_scheme(const std::string& aURI);

} // namespace security
} // namespace zorba

#endif /* ZORBA_SECURITY_CHECK_H */
/* vim:set et sw=2 ts=2: */
