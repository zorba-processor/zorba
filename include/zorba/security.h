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
#include <memory>
#include <vector>

#include <zorba/config.h>
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
  /** http, https and ftp access for documents, unparsed text and the
   *  http-client and ftp-client modules. */
  Network          = 1u << 3,
  /** Module and schema import over the network (formerly the
   *  http-uri-resolution feature). Also requires Network. */
  RemoteImport     = 1u << 4,
  /** fn:environment-variable, fn:available-environment-variables, and the
   *  ZORBA_URI_PATH / ZORBA_LIB_PATH environment variables. */
  Environment      = 1u << 5,
  /** Loading native libraries that implement external functions. */
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
 * StaticContext::setSecurityPolicy()). The policy in effect for a context is
 * the narrowest combination (see narrowedBy()) of its own policy and the
 * policies of all its ancestors up to the instance default, so a context can
 * restrict but never extend what its parent allows.
 *
 * Queries cannot change the policy; only the C++ API can.
 *
 * \note Enforcement is being introduced in stages. Setting a policy does not
 *       yet restrict query evaluation, and the instance default is currently
 *       unrestricted(). It will become secure() in Zorba 5.0.0.
 */
class ZORBA_DLL_PUBLIC SecurityPolicy
{
public:
  /**
   * \brief The secure policy: no capability, no allow-list entries, private
   *        network addresses denied, no timeout, call depth limited to
   *        1024, libxml2's default entity amplification limit.
   *
   * This will be the library default in Zorba 5.0.0.
   */
  static SecurityPolicy secure();

  /**
   * \brief The unrestricted policy: all capabilities, no allow-lists, private
   *        network addresses allowed, no timeout, call depth limited to 1024,
   *        libxml2's default entity amplification limit.
   *
   * This is the policy of the \c zorba command-line tool and the current
   * library default. The call depth limit is kept so that runaway recursion
   * still raises an error instead of overflowing the stack.
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
