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

#include <istream>
#include <memory>
#include <stdexcept>
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
 * The message for an access denied by the security policy:
 * "\"aURI\": access denied by security policy (aReason)", without the URI
 * part if aURI is empty. Every denial uses this form, whatever error code
 * reports it, so that the policy and its reason can be told from the message.
 */
std::string denial_message(const std::string& aURI, const std::string& aReason);

/**
 * Thrown by a built-in URL resolver when the policy denies the resource it
 * is about to read (the file actually opened, or a redirect or connection of
 * an HTTP request). static_context::apply_url_resolvers() catches it: the
 * resource is then not available, and what() (a denial_message()) is the
 * error message the caller reports with its own error code.
 */
class AccessDenied : public std::runtime_error
{
public:
  explicit AccessDenied(const std::string& aMessage)
    : std::runtime_error(aMessage) {}
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
 * - An http, https, ftp or ftps URL needs Network (and RemoteImport for
 *   Use::Import), must match a network prefix if there are any (same scheme,
 *   host and port; path on a segment boundary, after dot-segment removal),
 *   and must not resolve to a loopback, private or link-local address unless
 *   the policy allows private addresses or the matching prefix names that
 *   address literally (or "localhost").
 * - "xqftts:" thesaurus URIs are file: URIs. "wordnet:" thesauri (and the
 *   default thesaurus, "##default") are read from the library path, like
 *   native modules, and need NativeModules.
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
 * Checks a network connection made to fetch aURL (an http, https, ftp or
 * ftps URL) to the numeric address aAddress (IPv4 or IPv6, as curl reports
 * it). Used for every connection the HTTP and FTP clients make, including
 * redirects, so that neither a redirect nor a DNS answer that differs from
 * the one seen when the URI was resolved can reach a denied host.
 *
 * The URL must be allowed by the network rules of check() (Network, network
 * prefixes); instead of resolving the host, the address actually connected
 * to must not be a loopback, private or link-local address unless the policy
 * allows private addresses or the matching prefix names a private host
 * literally (or "localhost").
 */
Verdict check_connection(
    const SecurityPolicy& aPolicy,
    const std::string& aURL,
    const std::string& aAddress);

/**
 * True if the policy lets file reads through unchecked: FileRead allowed
 * with no read paths.
 */
bool file_reads_unrestricted(const SecurityPolicy& aPolicy);

/**
 * Opens a file for reading and checks the file actually opened.
 *
 * The file is opened first; then the real path of the open file (from the
 * file descriptor: /proc/self/fd on Linux, F_GETPATH on macOS) is checked
 * with check(aPolicy, path, aUse, false, aTrustedDirs). A symbolic link
 * swapped between an earlier check and the open therefore cannot redirect
 * the read outside the allowed paths. Where the real path of a descriptor is
 * not available, the canonical path is checked and must name the same file
 * (device and inode) as the descriptor. On Windows the path is checked and
 * then opened (not race-free).
 *
 * If the policy does not restrict file reads (file_reads_unrestricted()),
 * the file is opened with std::ifstream without any check, as before.
 *
 * aBinary selects binary mode where text mode differs (Windows); the
 * descriptor-based stream used elsewhere is always binary.
 *
 * Returns the stream, or null if the file cannot be opened or the access is
 * denied; oVerdict tells which.
 */
std::unique_ptr<std::istream> open_file(
    const SecurityPolicy& aPolicy,
    const std::string& aPath,
    Use aUse,
    const std::vector<std::string>* aTrustedDirs,
    bool aBinary,
    Verdict& oVerdict);

/**
 * The policy under which the built-in URL resolvers run on this thread.
 * static_context::apply_url_resolvers() sets it around each call to a
 * built-in resolver, so that a resolver can apply the policy of the context
 * that asked (the file resolver checks the file it opens; the HTTP resolver
 * runs its request under that policy). Scopes nest.
 */
class ResolutionScope
{
public:
  ResolutionScope(const SecurityPolicy& aPolicy,
                  Use aUse,
                  const std::vector<std::string>* aTrustedDirs);
  ~ResolutionScope();

  ResolutionScope(const ResolutionScope&) = delete;
  ResolutionScope& operator=(const ResolutionScope&) = delete;

  /** The innermost scope of this thread, or null. */
  static const ResolutionScope* current();

  const SecurityPolicy& policy() const { return thePolicy; }
  Use use() const { return theUse; }
  const std::vector<std::string>* trustedDirs() const { return theTrustedDirs; }

private:
  const SecurityPolicy&            thePolicy;
  Use                              theUse;
  const std::vector<std::string>*  theTrustedDirs;
  const ResolutionScope*           thePrevious;
};

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
