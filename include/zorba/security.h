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
#ifndef ZORBA_SECURITY_API_H
#define ZORBA_SECURITY_API_H

#include <chrono>
#include <cstdint>
#include <istream>
#include <memory>
#include <vector>

#include <zorba/config.h>
#include <zorba/uri_resolvers.h>
#include <zorba/zorba_string.h>

namespace zorba {

/**
 * \brief A capability that a query may be granted.
 *
 * Capabilities are the unit of permission of a SecurityPolicy. A capability
 * that is not allowed cannot be exercised by a query, by the modules it
 * imports, or by the XML parser on its behalf.
 *
 * The values are bit flags; they are stable and may be stored.
 */
enum class Capability : uint32_t {
  /** External DTD subsets, external entities and XInclude in XML input. */
  ExternalEntities = 1u << 0,
  /** Reading from the file system: documents, unparsed text, stop words,
   *  thesauri, file module reads. Module and schema import from the trusted
   *  URI/lib paths does not need this capability. */
  FileRead         = 1u << 1,
  /** Writing to the file system: file module writes, deletes, mkdir. */
  FileWrite        = 1u << 2,
  /** http, https, ftp and ftps access for documents, unparsed text and the
   *  http-client and ftp-client modules. */
  Network          = 1u << 3,
  /** Module and schema import over the network (formerly the
   *  http-uri-resolution feature). Also requires Network. */
  RemoteImport     = 1u << 4,
  /** fn:environment-variable, fn:available-environment-variables, and the
   *  ZORBA_URI_PATH / ZORBA_LIB_PATH environment variables. */
  Environment      = 1u << 5,
  /** Loading native libraries that implement external functions, and the
   *  WordNet thesaurus data that is installed with them in the library
   *  path. */
  NativeModules    = 1u << 6
};

/**
 * \brief The set of capabilities and limits under which queries run.
 *
 * A SecurityPolicy is a value type: copies are independent. All modifiers
 * return <code>*this</code> so they can be chained:
 * \code
 * zorba->setSecurityPolicy(SecurityPolicy::secure()
 *     .allow(Capability::FileRead).addReadPath("/srv/data")
 *     .setTimeout(std::chrono::seconds(10)));
 * \endcode
 *
 * A policy is attached to the Zorba instance (the process-wide default, see
 * Zorba::setSecurityPolicy()) and optionally to a StaticContext (see
 * StaticContext::setSecurityPolicy()):
 * - A context with no policy of its own inherits the policy in effect for its
 *   parent; contexts created with Zorba::createStaticContext() inherit the
 *   instance default.
 * - A policy set through the API on a context replaces the inherited one for
 *   that context and its descendants. The embedder is trusted, so it may be
 *   wider than the parent's, e.g. a secure() instance default and one
 *   context allowed to read a given directory:
 * \code
 * zorba->setSecurityPolicy(SecurityPolicy::secure());
 * StaticContext_t trusted = zorba->createStaticContext();
 * trusted->setSecurityPolicy(SecurityPolicy::secure()
 *     .allow(Capability::FileRead).addReadPath("/srv/data"));
 * \endcode
 * - Queries can never widen the policy they run under: there is no XQuery
 *   construct that changes it, and the contexts the engine creates on a
 *   query's behalf (module imports, eval, the zorba-query module) inherit it
 *   and can only narrow it (see narrowedBy()).
 *
 * A resource that the policy denies is not available: the query gets the
 * error the specification prescribes for a resource that cannot be
 * retrieved, and the message says that the access was denied by the security
 * policy and why, e.g. <code>access denied by security policy (requires
 * capability RemoteImport)</code>:
 * - module or schema import: \c err:XQST0059;
 * - fn:doc: \c err:FODC0002 (fn:doc-available returns false);
 * - fn:unparsed-text, fn:unparsed-text-lines: \c err:FOUT1170
 *   (fn:unparsed-text-available returns false);
 * - thesaurus: \c err:FTST0018; stop words: \c err:FTST0008;
 * - JSound schema import: \c jse:SCHEMA_NOT_FOUND; the fetch module:
 *   \c uri:URI_UNRESOLVED_OR_NOSTREAM;
 * - the file module: \c file:FOFL9999; the http-client module: \c http:HTTP
 *   (\c error:HC001 through the EXPath interface); the ftp-client module:
 *   \c ftp:FTP_ERROR.
 *
 * \c zerr:ZXQP0029 is raised only for an external entity or DTD that the
 * policy refuses while a document is parsed (where no other error applies)
 * and for a URI that a URIMapper refuses (URIMapper::DENY_ACCESS).
 *
 * \note Enforcement is being introduced in stages. Currently enforced: the
 *       XML parser (external entities); URI resolution by the built-in
 *       resolvers (documents, unparsed text, module, schema, thesaurus and
 *       stop-word loading, the fetch module); module import (the file,
 *       http-client and ftp-client modules need their capabilities, native
 *       libraries need Capability::NativeModules); the file, http-client
 *       and ftp-client modules (every path, URL, redirect and connection);
 *       the environment functions.
 *
 * The instance default is secure() (since Zorba 5.0.0; earlier versions
 * behaved like unrestricted()). An embedder that needs files, the network,
 * external entities, native modules or the environment allows them
 * explicitly, or calls
 * <code>zorba->setSecurityPolicy(SecurityPolicy::unrestricted())</code> to
 * restore the behavior of earlier versions.
 */
class ZORBA_DLL_PUBLIC SecurityPolicy
{
public:
  /**
   * \brief The secure policy: no capability, no allow-list entries, private
   *        network addresses denied, no timeout, call depth limited to
   *        1024, expression nesting depth limited to 1500, libxml2's
   *        default entity amplification limit.
   *
   * This is the library default: the policy of a Zorba instance on which
   * Zorba::setSecurityPolicy() was not called.
   */
  static SecurityPolicy secure();

  /**
   * \brief The unrestricted policy: all capabilities, no allow-lists, private
   *        network addresses allowed, no timeout, call depth limited to 1024,
   *        expression nesting depth limited to 1500, libxml2's default
   *        entity amplification limit.
   *
   * This is the policy of the \c zorba command-line tool (unless
   * <code>--secure</code> is given) and the behavior of Zorba versions before
   * 5.0.0. Embedders that relied on that behavior set it explicitly with
   * Zorba::setSecurityPolicy(). The call depth and expression nesting limits are kept
   * so that runaway recursion and pathologically nested queries still raise
   * an error instead of overflowing the stack.
   *
   * In builds without file access (ZORBA_WITH_FILE_ACCESS off),
   * Capability::FileRead and Capability::FileWrite are not included.
   */
  static SecurityPolicy unrestricted();

  /** \brief Constructs a policy equal to secure(). */
  SecurityPolicy();

  SecurityPolicy(const SecurityPolicy& aOther);

  SecurityPolicy& operator=(const SecurityPolicy& aOther);

  ~SecurityPolicy();

  /**
   * \brief Allows a capability.
   *
   * In builds without file access (ZORBA_WITH_FILE_ACCESS off), allowing
   * Capability::FileRead or Capability::FileWrite has no effect.
   */
  SecurityPolicy& allow(Capability aCap);

  /** \brief Denies a capability. */
  SecurityPolicy& deny(Capability aCap);

  /** \brief Returns true if the capability is allowed. */
  bool allows(Capability aCap) const;

  /**
   * \brief Adds a directory under which Capability::FileRead applies.
   *
   * If the list is empty, Capability::FileRead (when allowed) applies to any
   * path. A path is under a directory if it equals the directory or continues
   * it with a path separator; paths are compared after canonicalization.
   *
   * @param aDir an absolute directory path.
   */
  SecurityPolicy& addReadPath(const String& aDir);

  /**
   * \brief Adds a directory under which Capability::FileWrite applies.
   *
   * Same rules as addReadPath().
   */
  SecurityPolicy& addWritePath(const String& aDir);

  /**
   * \brief Adds a URL prefix to which Capability::Network applies.
   *
   * If the list is empty, Capability::Network (when allowed) applies to any
   * http, https or ftp URL. Example prefix: "https://api.example.com/v1/".
   */
  SecurityPolicy& addNetworkPrefix(const String& aUrlPrefix);

  /** \brief Returns the directories added with addReadPath(). */
  const std::vector<String>& getReadPaths() const;

  /** \brief Returns the directories added with addWritePath(). */
  const std::vector<String>& getWritePaths() const;

  /** \brief Returns the prefixes added with addNetworkPrefix(). */
  const std::vector<String>& getNetworkPrefixes() const;

  /**
   * \brief Returns true if the policy allows a URI to be accessed for the
   *        given kind of entity.
   *
   * This is the check Zorba applies before its built-in URL resolvers load a
   * URI. External modules that access URIs themselves can use it too.
   *
   * - Paths and \c file: URIs need Capability::FileRead (or
   *   Capability::FileWrite if \p aWrite) and, if there are read (write)
   *   paths, must lie under one of them. Paths are compared in canonical
   *   form: absolute, with <code>.</code> and <code>..</code> resolved and
   *   symbolic links of the existing part of the path followed, so neither
   *   <code>..</code> nor a symbolic link can escape a directory. The
   *   comparison is case-insensitive where the file system is (Windows,
   *   case-insensitive macOS volumes).
   * - \c http, \c https, \c ftp and \c ftps URLs need Capability::Network,
   *   and for
   *   EntityData::MODULE and EntityData::SCHEMA also
   *   Capability::RemoteImport. If there are network prefixes, the URL must
   *   lie under one of them: same scheme, host (case-insensitive) and port
   *   (default ports are implied), and a path that equals the prefix path or
   *   continues it after a '/' (after removal of dot segments). User info,
   *   query and fragment are not part of the match.
   * - Unless allowsPrivateAddresses(), the host must not resolve to a
   *   loopback, private, shared, link-local, multicast or reserved address
   *   (all addresses it resolves to are checked; a host that does not
   *   resolve is denied). A network prefix whose host is such an address
   *   literal, or "localhost", allows that host explicitly. The check is made
   *   when the URI is resolved; it does not protect against a DNS server
   *   that answers differently for the actual connection.
   * - EntityData::COLLECTION URIs name collections in the store and are
   *   always allowed.
   *
   * Module and schema imports from Zorba's trusted URI and library paths
   * are allowed without Capability::FileRead when Zorba resolves them; this
   * function does not know those paths and applies the FileRead rule.
   */
  bool checkURI(const String& aURI,
                EntityData::Kind aKind,
                bool aWrite = false) const;

  /**
   * \brief Like checkURI(const String&, EntityData::Kind, bool), and if the
   *        access is denied, stores the reason (e.g. "requires capability
   *        FileRead") in \p *aReason (if not null).
   */
  bool checkURI(const String& aURI,
                EntityData::Kind aKind,
                bool aWrite,
                String* aReason) const;

  /**
   * \brief Returns true if the policy allows a network connection to the
   *        numeric address \p aAddress, made to fetch \p aURL.
   *
   * This is the check the HTTP and FTP clients apply to every connection,
   * including connections for redirects: the URL must be allowed by the
   * network rules of checkURI() (Capability::Network, network prefixes), and
   * unless allowsPrivateAddresses() or a matching prefix names a private host
   * explicitly, \p aAddress must not be a loopback, private, link-local,
   * multicast or reserved address. Unlike checkURI(), it does not resolve
   * the host of the URL: \p aAddress is the address actually connected to.
   *
   * @param aURL an http, https, ftp or ftps URL.
   * @param aAddress a numeric IPv4 or IPv6 address.
   * @param aReason if not null and the connection is denied, receives the
   *        reason.
   */
  bool checkConnection(const String& aURL,
                       const String& aAddress,
                       String* aReason = nullptr) const;

  /**
   * \brief Opens a file for reading if the policy allows it.
   *
   * Applies the rule of checkURI() for EntityData::DOCUMENT to the file that
   * is actually opened: the file is opened first, then the real path of the
   * open file is checked. A symbolic link that is swapped between an earlier
   * check and the open therefore cannot redirect the read outside the read
   * paths. (On Windows, the path is checked and then opened.)
   *
   * If the policy allows Capability::FileRead without read paths, the file
   * is opened without any check.
   *
   * @param aPath a path or \c file: URI. A relative path is relative to the
   *        current directory.
   * @param aBinary true to open the file in binary mode, false for text mode
   *        (the modes differ only on Windows).
   * @param aReason if not null and the access is denied, receives the
   *        reason; it is left empty if the file merely cannot be opened.
   * @return the open stream, or null if the file cannot be opened or the
   *         access is denied.
   */
  std::unique_ptr<std::istream> openFile(const String& aPath,
                                         bool aBinary,
                                         String* aReason = nullptr) const;

  /**
   * \brief Sets whether network access may reach loopback, private
   *        (RFC 1918, RFC 4193) and link-local addresses.
   *
   * Has an effect only when Capability::Network is allowed. Default: false
   * in secure(), true in unrestricted().
   */
  SecurityPolicy& setAllowPrivateAddresses(bool aAllow);

  /** \brief Returns true if private network addresses may be reached. */
  bool allowsPrivateAddresses() const;

  /**
   * \brief Sets the maximum wall-clock time of a query evaluation.
   *
   * Zero (the default) means no limit. Negative values are treated as zero.
   */
  SecurityPolicy& setTimeout(std::chrono::milliseconds aTimeout);

  /** \brief Returns the timeout; zero means no limit. */
  std::chrono::milliseconds getTimeout() const;

  /**
   * \brief Sets the maximum depth of user-defined function calls.
   *
   * Zero means no limit. Default: 1024.
   */
  SecurityPolicy& setMaxCallDepth(uint32_t aDepth);

  /** \brief Returns the maximum call depth; zero means no limit. */
  uint32_t getMaxCallDepth() const;

  /**
   * \brief Sets the maximum nesting depth of expressions in a query.
   *
   * The depth is that of the query's syntax tree: each operand of a binary
   * operator in a chain (<code>1 + 1 + 1</code>), each parenthesized,
   * function call, constructor or FLWOR nesting level adds one or more
   * levels, and so does each clause of a FLWOR expression, each predicate
   * of a filter, and each case of a typeswitch or switch and each catch
   * clause (the compiler chains them). A query (or imported module, or eval'ed query) that nests
   * deeper fails to compile with zerr:ZXQP0070.
   *
   * Compilation (and evaluation) recurse over the query, so the depth
   * determines the stack they need. Whatever this limit, the depth is also
   * capped by the stack left on the compiling thread: Zorba assumes a few
   * KiB per level and keeps a third of the stack free, so a query on a
   * thread with a small stack (or eval called from deep recursion) fails
   * with zerr:ZXQP0070 instead of overflowing the stack. The default suits
   * threads with the usual 8 MiB stack. Zero means no limit other than the
   * stack. Default: 1500.
   */
  SecurityPolicy& setMaxExprDepth(uint32_t aDepth);

  /** \brief Returns the maximum expression nesting depth; zero means no
   *         limit. */
  uint32_t getMaxExprDepth() const;

  /**
   * \brief Sets the maximum entity amplification factor of the XML parser.
   *
   * Zero (the default) means libxml2's built-in limit. Zorba never disables
   * the parser's amplification checks, whatever this value.
   */
  SecurityPolicy& setMaxEntityAmplification(uint32_t aFactor);

  /** \brief Returns the amplification factor; zero means libxml2's default. */
  uint32_t getMaxEntityAmplification() const;

  /**
   * \brief Returns the narrowest combination of this policy and another.
   *
   * The result allows what both policies allow and nothing else:
   * - a capability is allowed only if both allow it;
   * - private addresses are allowed only if both allow them;
   * - for each limit, the smaller non-zero value wins (zero means no limit,
   *   or for the amplification factor, the parser's default);
   * - an allow-list that is empty in one policy is taken from the other; if
   *   both are non-empty, the result keeps each entry of one list that lies
   *   within an entry of the other. If that leaves no entry, the matching
   *   capability (FileRead, FileWrite or Network) is denied.
   *
   * The operation is commutative and the result is never wider than either
   * operand.
   */
  SecurityPolicy narrowedBy(const SecurityPolicy& aOther) const;

  /**
   * \brief Returns true if this policy allows nothing that \p aOther denies,
   *        i.e. if <code>narrowedBy(aOther) == *this</code>.
   */
  bool isWithin(const SecurityPolicy& aOther) const;

  bool operator==(const SecurityPolicy& aOther) const;

  bool operator!=(const SecurityPolicy& aOther) const
  {
    return !(*this == aOther);
  }

private:
  class Impl;
  std::unique_ptr<Impl> theImpl;
};

} // namespace zorba
#endif /* ZORBA_SECURITY_API_H */
/* vim:set et sw=2 ts=2: */
