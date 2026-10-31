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
#include "stdafx.h"

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <system_error>

#ifdef WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <arpa/inet.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/param.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
#endif

#include <zorba/util/fs_util.h>

#include "security_check.h"

namespace zorba {
namespace security {

namespace {

const char* capability_name(Capability aCap)
{
  switch (aCap)
  {
    case Capability::ExternalEntities: return "ExternalEntities";
    case Capability::FileRead:         return "FileRead";
    case Capability::FileWrite:        return "FileWrite";
    case Capability::Network:          return "Network";
    case Capability::RemoteImport:     return "RemoteImport";
    case Capability::Environment:      return "Environment";
    case Capability::NativeModules:    return "NativeModules";
  }
  return "?";
}

Verdict deny(const std::string& aReason)
{
  Verdict v;
  v.allowed = false;
  v.reason = aReason;
  return v;
}

Verdict needs_capability(Capability aCap)
{
  return deny(std::string("requires capability ") + capability_name(aCap));
}

bool is_separator(char c)
{
#ifdef WIN32
  return c == '/' || c == '\\';
#else
  return c == '/';
#endif
}

std::string to_lower(std::string s)
{
  for (char& c : s)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

bool case_insensitive_fs(const std::string& aDir)
{
#if defined(WIN32)
  (void)aDir;
  return true;
#elif defined(__APPLE__) && defined(_PC_CASE_SENSITIVE)
  // 0: case-insensitive; 1: case-sensitive; -1: unknown, assume the more
  // restrictive case-sensitive comparison.
  return ::pathconf(aDir.c_str(), _PC_CASE_SENSITIVE) == 0;
#else
  (void)aDir;
  return false;
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Network URLs

struct NetURL
{
  std::string scheme;
  std::string host;         // lower case, no brackets, no trailing dot
  unsigned    port = 0;
  std::string path;         // normalized, starts with '/'
  bool        literal = false;  // host is an IPv4 dotted quad or IPv6 literal
};

int hex_value(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// RFC 3986, 5.2.4.
std::string remove_dot_segments(const std::string& aPath)
{
  std::vector<std::string> lSegments;
  std::string::size_type i = 0;
  bool lTrailingSlash = false;
  while (i < aPath.size())
  {
    std::string::size_type j = aPath.find('/', i + 1);
    if (j == std::string::npos)
      j = aPath.size();
    std::string const lSeg = aPath.substr(i + 1, j - i - 1);
    lTrailingSlash = (j == aPath.size() && (lSeg == "." || lSeg == ".."));
    if (lSeg == "..")
    {
      if (!lSegments.empty())
        lSegments.pop_back();
    }
    else if (lSeg != ".")
      lSegments.push_back(lSeg);
    i = j;
  }
  std::string lOut;
  for (const std::string& s : lSegments)
    lOut += "/" + s;
  if (lOut.empty() || lTrailingSlash)
    lOut += "/";
  return lOut;
}

// Normalizes a URL path for matching: percent escapes in upper case; "%2E"
// and "%2F" decoded (servers may decode them before resolving dot segments);
// dot segments removed. Fails on an encoded backslash or NUL.
bool normalize_url_path(const std::string& aPath, std::string& aOut)
{
  std::string lDecoded;
  for (std::string::size_type i = 0; i < aPath.size(); ++i)
  {
    if (aPath[i] != '%')
    {
      lDecoded += aPath[i];
      continue;
    }
    if (i + 2 >= aPath.size())
      return false;
    int const hi = hex_value(aPath[i + 1]);
    int const lo = hex_value(aPath[i + 2]);
    if (hi < 0 || lo < 0)
      return false;
    int const c = hi * 16 + lo;
    if (c == '.' || c == '/')
      lDecoded += static_cast<char>(c);
    else if (c == '\\' || c == 0)
      return false;
    else
    {
      static const char HEX[] = "0123456789ABCDEF";
      lDecoded += '%';
      lDecoded += HEX[hi];
      lDecoded += HEX[lo];
    }
    i += 2;
  }
  aOut = remove_dot_segments(lDecoded);
  return true;
}

bool is_ipv4_literal(const std::string& aHost)
{
  in_addr lAddr;
  return ::inet_pton(AF_INET, aHost.c_str(), &lAddr) == 1;
}

bool is_ipv6_literal(const std::string& aHost)
{
  in6_addr lAddr;
  return ::inet_pton(AF_INET6, aHost.c_str(), &lAddr) == 1;
}

// Parses an http, https, ftp or ftps URL. Anything ambiguous fails: control
// characters, backslashes, several '@', percent signs in the host.
bool parse_net_url(const std::string& aURL, NetURL& u)
{
  for (char c : aURL)
  {
    unsigned char const uc = static_cast<unsigned char>(c);
    if (uc <= 0x20 || uc == 0x7f || c == '\\')
      return false;
  }

  std::string::size_type const lColon = aURL.find(':');
  if (lColon == std::string::npos)
    return false;
  u.scheme = to_lower(aURL.substr(0, lColon));
  if (u.scheme == "http")
    u.port = 80;
  else if (u.scheme == "https")
    u.port = 443;
  else if (u.scheme == "ftp")
    u.port = 21;
  else if (u.scheme == "ftps")
    u.port = 990;
  else
    return false;

  if (aURL.compare(lColon + 1, 2, "//") != 0)
    return false;
  std::string::size_type const lAuthStart = lColon + 3;
  std::string::size_type lAuthEnd = aURL.find_first_of("/?#", lAuthStart);
  if (lAuthEnd == std::string::npos)
    lAuthEnd = aURL.size();
  std::string lAuthority = aURL.substr(lAuthStart, lAuthEnd - lAuthStart);

  std::string::size_type const lAt = lAuthority.find('@');
  if (lAt != std::string::npos)
  {
    if (lAuthority.find('@', lAt + 1) != std::string::npos)
      return false;
    lAuthority.erase(0, lAt + 1);
  }

  std::string lPort;
  if (!lAuthority.empty() && lAuthority[0] == '[')
  {
    std::string::size_type const lClose = lAuthority.find(']');
    if (lClose == std::string::npos)
      return false;
    u.host = to_lower(lAuthority.substr(1, lClose - 1));
    if (!is_ipv6_literal(u.host))
      return false;
    u.literal = true;
    std::string const lRest = lAuthority.substr(lClose + 1);
    if (!lRest.empty())
    {
      if (lRest[0] != ':')
        return false;
      lPort = lRest.substr(1);
    }
  }
  else
  {
    std::string::size_type const lPortColon = lAuthority.find(':');
    u.host = to_lower(lAuthority.substr(0, lPortColon));
    if (lPortColon != std::string::npos)
      lPort = lAuthority.substr(lPortColon + 1);
    for (char c : u.host)
    {
      if (!std::isalnum(static_cast<unsigned char>(c)) &&
          c != '-' && c != '.' && c != '_')
        return false;
    }
    if (!u.host.empty() && u.host.back() == '.')
      u.host.erase(u.host.size() - 1);
    u.literal = is_ipv4_literal(u.host);
  }
  if (u.host.empty())
    return false;

  if (!lPort.empty())
  {
    if (lPort.size() > 5)
      return false;
    unsigned lValue = 0;
    for (char c : lPort)
    {
      if (!std::isdigit(static_cast<unsigned char>(c)))
        return false;
      lValue = lValue * 10 + static_cast<unsigned>(c - '0');
    }
    if (lValue == 0 || lValue > 65535)
      return false;
    u.port = lValue;
  }

  std::string lPath;
  if (lAuthEnd < aURL.size() && aURL[lAuthEnd] == '/')
  {
    std::string::size_type const lPathEnd = aURL.find_first_of("?#", lAuthEnd);
    lPath = aURL.substr(lAuthEnd, lPathEnd == std::string::npos
                                    ? std::string::npos
                                    : lPathEnd - lAuthEnd);
  }
  else
    lPath = "/";
  return normalize_url_path(lPath, u.path);
}

// True if URL u is under prefix p.
bool url_under(const NetURL& p, const NetURL& u)
{
  if (p.scheme != u.scheme || p.host != u.host || p.port != u.port)
    return false;
  if (p.path.back() == '/')
    return u.path.compare(0, p.path.size(), p.path) == 0;
  return u.path == p.path ||
         (u.path.size() > p.path.size() &&
          u.path.compare(0, p.path.size(), p.path) == 0 &&
          u.path[p.path.size()] == '/');
}


////////////////////////////////////////////////////////////////////////////////
// Addresses

bool is_private_ipv4(uint32_t a)  // host byte order
{
  return (a >> 24) == 0 ||                       // 0.0.0.0/8 "this network"
         (a >> 24) == 10 ||                      // 10.0.0.0/8
         (a & 0xffc00000u) == 0x64400000u ||     // 100.64.0.0/10 shared
         (a >> 24) == 127 ||                     // 127.0.0.0/8 loopback
         (a & 0xffff0000u) == 0xa9fe0000u ||     // 169.254.0.0/16 link-local
         (a & 0xfff00000u) == 0xac100000u ||     // 172.16.0.0/12
         (a & 0xffffff00u) == 0xc0000000u ||     // 192.0.0.0/24
         (a & 0xffff0000u) == 0xc0a80000u ||     // 192.168.0.0/16
         (a & 0xfffe0000u) == 0xc6120000u ||     // 198.18.0.0/15
         (a >> 28) >= 0xe;                       // multicast, reserved
}

uint32_t ipv4_at(const unsigned char* b)
{
  return (uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) |
         (uint32_t(b[2]) << 8) | uint32_t(b[3]);
}

bool is_private_ipv6(const unsigned char* b)
{
  static const unsigned char ZERO[12] = {0};
  static const unsigned char MAPPED[12] =
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
  static const unsigned char NAT64[12] =
    {0, 0x64, 0xff, 0x9b, 0, 0, 0, 0, 0, 0, 0, 0};

  // ::, ::1 and IPv4-compatible ::a.b.c.d; IPv4-mapped; NAT64 well-known
  if (std::memcmp(b, ZERO, 12) == 0 || std::memcmp(b, MAPPED, 12) == 0 ||
      std::memcmp(b, NAT64, 12) == 0)
    return is_private_ipv4(ipv4_at(b + 12));
  if (b[0] == 0x20 && b[1] == 0x02)              // 6to4 2002::/16
    return is_private_ipv4(ipv4_at(b + 2));
  return (b[0] & 0xfe) == 0xfc ||                // fc00::/7 unique local
         (b[0] == 0xfe && (b[1] & 0xc0) == 0x80) ||  // fe80::/10 link-local
         (b[0] == 0xfe && (b[1] & 0xc0) == 0xc0) ||  // fec0::/10 site-local
         b[0] == 0xff;                           // multicast
}

std::string address_string(const sockaddr* aAddr)
{
  char lBuf[INET6_ADDRSTRLEN] = "";
  if (aAddr->sa_family == AF_INET)
    ::inet_ntop(AF_INET,
                &reinterpret_cast<const sockaddr_in*>(aAddr)->sin_addr,
                lBuf, sizeof lBuf);
  else if (aAddr->sa_family == AF_INET6)
    ::inet_ntop(AF_INET6,
                &reinterpret_cast<const sockaddr_in6*>(aAddr)->sin6_addr,
                lBuf, sizeof lBuf);
  return lBuf;
}

// True if aAddress (numeric IPv4 or IPv6, optionally in brackets or with an
// IPv6 zone) is a loopback, private or link-local address. Anything that is
// not a numeric address counts as private (fail closed).
bool is_private_address(std::string aAddress)
{
  if (aAddress.size() > 2 && aAddress.front() == '[' && aAddress.back() == ']')
    aAddress = aAddress.substr(1, aAddress.size() - 2);
  std::string::size_type const lZone = aAddress.find('%');
  if (lZone != std::string::npos)
    aAddress.erase(lZone);
  in_addr lV4;
  if (::inet_pton(AF_INET, aAddress.c_str(), &lV4) == 1)
    return is_private_ipv4(ntohl(lV4.s_addr));
  in6_addr lV6;
  if (::inet_pton(AF_INET6, aAddress.c_str(), &lV6) == 1)
    return is_private_ipv6(reinterpret_cast<const unsigned char*>(&lV6));
  return true;
}

// Resolves aHost and checks every address. Fails closed: a host that does
// not resolve is denied.
Verdict check_host_addresses(const std::string& aHost)
{
  addrinfo lHints;
  std::memset(&lHints, 0, sizeof lHints);
  lHints.ai_family = AF_UNSPEC;
  lHints.ai_socktype = SOCK_STREAM;
  addrinfo* lResult = nullptr;
  if (::getaddrinfo(aHost.c_str(), nullptr, &lHints, &lResult) != 0 ||
      lResult == nullptr)
    return deny("host \"" + aHost + "\" cannot be resolved");

  Verdict v;
  for (addrinfo* ai = lResult; ai != nullptr && v.allowed; ai = ai->ai_next)
  {
    bool lPrivate = false;
    if (ai->ai_family == AF_INET)
    {
      const sockaddr_in* sin = reinterpret_cast<const sockaddr_in*>(ai->ai_addr);
      lPrivate = is_private_ipv4(ntohl(sin->sin_addr.s_addr));
    }
    else if (ai->ai_family == AF_INET6)
    {
      const sockaddr_in6* sin6 =
        reinterpret_cast<const sockaddr_in6*>(ai->ai_addr);
      lPrivate = is_private_ipv6(
          reinterpret_cast<const unsigned char*>(&sin6->sin6_addr));
    }
    else
      continue;
    if (lPrivate)
      v = deny("host \"" + aHost + "\" has the loopback, private or "
               "link-local address " + address_string(ai->ai_addr) +
               ", which the policy does not allow");
  }
  ::freeaddrinfo(lResult);
  return v;
}

// A prefix names a private host explicitly if its host is "localhost" or a
// private address literal.
bool names_private_host(const NetURL& p)
{
  if (p.host == "localhost")
    return true;
  if (!p.literal)
    return false;
  return !check_host_addresses(p.host).allowed;
}


////////////////////////////////////////////////////////////////////////////////

Verdict check_file(
    const SecurityPolicy& aPolicy,
    const std::string& aPathOrURI,
    Use aUse,
    bool aWrite,
    const std::vector<std::string>* aTrustedDirs)
{
  Capability const lCap = aWrite ? Capability::FileWrite : Capability::FileRead;
  const std::vector<String>& lDirs =
    aWrite ? aPolicy.getWritePaths() : aPolicy.getReadPaths();
  bool const lTrustable = !aWrite && aTrustedDirs && !aTrustedDirs->empty() &&
                          (aUse == Use::Import || aUse == Use::Data);

  if (aPolicy.allows(lCap) && lDirs.empty())
    return Verdict();

  std::string lPath;
  try
  {
    lPath = canonical_path(aPathOrURI);
  }
  catch (const std::exception&)
  {
    return deny("malformed path");
  }

  if (lTrustable)
  {
    for (const std::string& lDir : *aTrustedDirs)
      if (path_within(lDir, lPath))
        return Verdict();
  }

  if (!aPolicy.allows(lCap))
    return needs_capability(lCap);
  if (lDirs.empty())
    return Verdict();

  for (const String& lDir : lDirs)
  {
    try
    {
      if (path_within(canonical_path(lDir.str()), lPath))
        return Verdict();
    }
    catch (const std::exception&)
    {
    }
  }
  return deny(std::string("\"") + lPath + "\" is outside the allowed " +
              (aWrite ? "write" : "read") + " paths");
}

// Matches URL u against the network prefixes of aPolicy (if any). On a
// match, oExplicitPrivate tells whether a matching prefix names a private
// host explicitly, which allows that host without setAllowPrivateAddresses().
Verdict match_prefixes(
    const SecurityPolicy& aPolicy,
    const NetURL& u,
    bool& oExplicitPrivate)
{
  oExplicitPrivate = false;
  const std::vector<String>& lPrefixes = aPolicy.getNetworkPrefixes();
  if (lPrefixes.empty())
    return Verdict();
  bool lMatched = false;
  for (const String& lPrefix : lPrefixes)
  {
    NetURL p;
    if (parse_net_url(lPrefix.str(), p) && url_under(p, u))
    {
      lMatched = true;
      if (!aPolicy.allowsPrivateAddresses() && names_private_host(p))
        oExplicitPrivate = true;
    }
  }
  if (!lMatched)
    return deny("not under an allowed network prefix");
  return Verdict();
}

Verdict check_network(
    const SecurityPolicy& aPolicy,
    const std::string& aURL,
    Use aUse)
{
  if (aUse == Use::Import && !aPolicy.allows(Capability::RemoteImport))
    return needs_capability(Capability::RemoteImport);
  if (!aPolicy.allows(Capability::Network))
    return needs_capability(Capability::Network);

  if (aPolicy.getNetworkPrefixes().empty() && aPolicy.allowsPrivateAddresses())
    return Verdict();

  NetURL u;
  if (!parse_net_url(aURL, u))
    return deny("malformed URL");

  bool lExplicitPrivate = false;
  Verdict const lMatch = match_prefixes(aPolicy, u, lExplicitPrivate);
  if (!lMatch.allowed)
    return lMatch;

  if (aPolicy.allowsPrivateAddresses() || lExplicitPrivate)
    return Verdict();
  return check_host_addresses(u.host);
}

#ifndef WIN32

// The real path of an open file, if the system tells it.
bool fd_real_path(int aFd, std::string& oPath)
{
#if defined(__linux__)
  char lLink[64];
  std::snprintf(lLink, sizeof lLink, "/proc/self/fd/%d", aFd);
  char lBuf[PATH_MAX];
  ssize_t const n = ::readlink(lLink, lBuf, sizeof lBuf - 1);
  if (n <= 0 || n >= static_cast<ssize_t>(sizeof lBuf - 1) || lBuf[0] != '/')
    return false;
  oPath.assign(lBuf, static_cast<std::string::size_type>(n));
  return true;
#elif defined(__APPLE__)
  char lBuf[MAXPATHLEN];
  if (::fcntl(aFd, F_GETPATH, lBuf) == -1 || lBuf[0] != '/')
    return false;
  oPath = lBuf;
  return true;
#else
  (void)aFd;
  (void)oPath;
  return false;
#endif
}

// A read-only, seekable streambuf over a file descriptor it owns.
class fd_streambuf : public std::streambuf
{
public:
  explicit fd_streambuf(int aFd) : theFd(aFd)
  {
    setg(theBuf, theBuf, theBuf);
  }

  ~fd_streambuf() override
  {
    ::close(theFd);
  }

  fd_streambuf(const fd_streambuf&) = delete;
  fd_streambuf& operator=(const fd_streambuf&) = delete;

protected:
  int_type underflow() override
  {
    if (gptr() < egptr())
      return traits_type::to_int_type(*gptr());
    ssize_t n;
    do
      n = ::read(theFd, theBuf, sizeof theBuf);
    while (n < 0 && errno == EINTR);
    if (n <= 0)
      return traits_type::eof();
    setg(theBuf, theBuf, theBuf + n);
    return traits_type::to_int_type(*gptr());
  }

  pos_type seekoff(off_type aOff,
                   std::ios_base::seekdir aDir,
                   std::ios_base::openmode aWhich) override
  {
    if (!(aWhich & std::ios_base::in))
      return pos_type(off_type(-1));
    int lWhence = SEEK_SET;
    if (aDir == std::ios_base::cur)
    {
      // The descriptor is ahead of the get pointer by the buffered bytes.
      aOff -= static_cast<off_type>(egptr() - gptr());
      lWhence = SEEK_CUR;
    }
    else if (aDir == std::ios_base::end)
      lWhence = SEEK_END;
    off_t const r = ::lseek(theFd, static_cast<off_t>(aOff), lWhence);
    if (r < 0)
      return pos_type(off_type(-1));
    setg(theBuf, theBuf, theBuf);
    return pos_type(static_cast<off_type>(r));
  }

  pos_type seekpos(pos_type aPos, std::ios_base::openmode aWhich) override
  {
    return seekoff(off_type(aPos), std::ios_base::beg, aWhich);
  }

private:
  int  theFd;
  char theBuf[8192];
};

class fd_istream : public std::istream
{
public:
  explicit fd_istream(int aFd) : std::istream(nullptr), theBuf(aFd)
  {
    rdbuf(&theBuf);
  }

private:
  fd_streambuf theBuf;
};

#endif /* WIN32 */

thread_local const ResolutionScope* theCurrentScope = nullptr;

} // anonymous namespace


std::string denial_message(const std::string& aURI, const std::string& aReason)
{
  std::string lMessage;
  if (!aURI.empty())
    lMessage = "\"" + aURI + "\": ";
  lMessage += "access denied by security policy (" + aReason + ")";
  return lMessage;
}


std::string uri_scheme(const std::string& aURI)
{
  std::string::size_type const lColon = aURI.find(':');
  if (lColon == std::string::npos || lColon < 2)
    return std::string();
  if (!std::isalpha(static_cast<unsigned char>(aURI[0])))
    return std::string();
  std::string lScheme;
  for (std::string::size_type i = 0; i < lColon; ++i)
  {
    unsigned char const c = static_cast<unsigned char>(aURI[i]);
    if (!std::isalnum(c) && c != '+' && c != '-' && c != '.')
      return std::string();
    lScheme += static_cast<char>(std::tolower(c));
  }
  return lScheme;
}


std::string canonical_path(const std::string& aPathOrFileURI)
{
  // The same normalization as the file URL resolver, so that the path
  // checked is the path opened.
  std::string const lCwd = fs::curdir();
  std::string const lPath =
    fs::normalize_path(aPathOrFileURI.c_str(), lCwd.c_str());
  std::error_code ec;
  std::filesystem::path const lCanonical =
    std::filesystem::weakly_canonical(std::filesystem::path(lPath), ec);
  if (ec)
    throw std::invalid_argument(ec.message());
  std::string lResult = lCanonical.string();
  if (lResult.empty())
    throw std::invalid_argument("empty path");
  return lResult;
}


bool path_within(const std::string& aDir, const std::string& aPath)
{
  std::string lDir = aDir;
  while (lDir.size() > 1 && is_separator(lDir.back()))
    lDir.erase(lDir.size() - 1);
  std::string lPath = aPath;
  if (case_insensitive_fs(lDir))
  {
    lDir = to_lower(lDir);
    lPath = to_lower(lPath);
  }
  if (lPath.compare(0, lDir.size(), lDir) != 0)
    return false;
  return lPath.size() == lDir.size() ||
         is_separator(lDir.back()) ||
         is_separator(lPath[lDir.size()]);
}


Verdict check(
    const SecurityPolicy& aPolicy,
    const std::string& aURI,
    Use aUse,
    bool aWrite,
    const std::vector<std::string>* aTrustedDirs)
{
  if (aUse == Use::Collection)
    return Verdict();

  // The default thesaurus is WordNet: data from the library path, which
  // needs NativeModules like the libraries there.
  if (aUse == Use::Data && aURI == "##default")
    return aPolicy.allows(Capability::NativeModules)
      ? Verdict() : needs_capability(Capability::NativeModules);

  std::string const lScheme = uri_scheme(aURI);

  if (lScheme.empty() || lScheme == "file")
    return check_file(aPolicy, aURI, aUse, aWrite, aTrustedDirs);

  if (lScheme == "http" || lScheme == "https" || lScheme == "ftp" ||
      lScheme == "ftps")
    return check_network(aPolicy, aURI, aUse);

  if (lScheme == "xqftts")
    return check_file(aPolicy, "file" + aURI.substr(6), aUse, aWrite,
                      aTrustedDirs);

  if (lScheme == "wordnet")
    return aPolicy.allows(Capability::NativeModules)
      ? Verdict() : needs_capability(Capability::NativeModules);

  if (aPolicy.allows(Capability::FileRead) &&
      aPolicy.getReadPaths().empty() &&
      aPolicy.allows(Capability::Network) &&
      aPolicy.getNetworkPrefixes().empty() &&
      aPolicy.allowsPrivateAddresses())
    return Verdict();
  return deny("scheme \"" + lScheme + "\" is not allowed by the policy");
}


Verdict check_connection(
    const SecurityPolicy& aPolicy,
    const std::string& aURL,
    const std::string& aAddress)
{
  if (!aPolicy.allows(Capability::Network))
    return needs_capability(Capability::Network);
  if (aPolicy.getNetworkPrefixes().empty() && aPolicy.allowsPrivateAddresses())
    return Verdict();

  NetURL u;
  if (!parse_net_url(aURL, u))
    return deny("malformed URL");

  bool lExplicitPrivate = false;
  Verdict const lMatch = match_prefixes(aPolicy, u, lExplicitPrivate);
  if (!lMatch.allowed)
    return lMatch;

  if (aPolicy.allowsPrivateAddresses() || lExplicitPrivate)
    return Verdict();
  if (is_private_address(aAddress))
    return deny("connection to the loopback, private or link-local address " +
                aAddress + " for host \"" + u.host +
                "\" is not allowed by the policy");
  return Verdict();
}


bool file_reads_unrestricted(const SecurityPolicy& aPolicy)
{
  return aPolicy.allows(Capability::FileRead) && aPolicy.getReadPaths().empty();
}


std::unique_ptr<std::istream> open_file(
    const SecurityPolicy& aPolicy,
    const std::string& aPath,
    Use aUse,
    const std::vector<std::string>* aTrustedDirs,
    bool aBinary,
    Verdict& oVerdict)
{
  oVerdict = Verdict();
  std::ios_base::openmode const lMode =
    aBinary ? std::ios_base::in | std::ios_base::binary : std::ios_base::in;

  std::string lPath;
  try
  {
    std::string const lCwd = fs::curdir();
    lPath = fs::normalize_path(aPath.c_str(), lCwd.c_str());
  }
  catch (const std::exception&)
  {
    oVerdict = deny("malformed path");
    return nullptr;
  }

  if (file_reads_unrestricted(aPolicy))
  {
    std::unique_ptr<std::ifstream> lStream(new std::ifstream(lPath.c_str(), lMode));
    if (!lStream->is_open())
      return nullptr;
    return lStream;
  }

#ifdef WIN32
  oVerdict = check(aPolicy, lPath, aUse, false, aTrustedDirs);
  if (!oVerdict.allowed)
    return nullptr;
  std::unique_ptr<std::ifstream> lStream(new std::ifstream(lPath.c_str(), lMode));
  if (!lStream->is_open())
    return nullptr;
  return lStream;
#else
  int lFd;
  do
    lFd = ::open(lPath.c_str(), O_RDONLY | O_CLOEXEC | O_NOCTTY);
  while (lFd < 0 && errno == EINTR);
  if (lFd < 0)
    return nullptr;

  std::string lReal;
  if (fd_real_path(lFd, lReal))
    oVerdict = check(aPolicy, lReal, aUse, false, aTrustedDirs);
  else
  {
    // No real path: check the canonical path, then make sure that it names
    // the file that was opened.
    try
    {
      lReal = canonical_path(lPath);
      oVerdict = check(aPolicy, lReal, aUse, false, aTrustedDirs);
    }
    catch (const std::exception&)
    {
      oVerdict = deny("malformed path");
    }
    struct stat lOpened;
    struct stat lNamed;
    if (oVerdict.allowed &&
        (::fstat(lFd, &lOpened) != 0 || ::stat(lReal.c_str(), &lNamed) != 0 ||
         lOpened.st_dev != lNamed.st_dev || lOpened.st_ino != lNamed.st_ino))
      oVerdict = deny("the file changed while it was opened");
  }
  if (!oVerdict.allowed)
  {
    ::close(lFd);
    return nullptr;
  }
  return std::unique_ptr<std::istream>(new fd_istream(lFd));
#endif
}


ResolutionScope::ResolutionScope(
    const SecurityPolicy& aPolicy,
    Use aUse,
    const std::vector<std::string>* aTrustedDirs)
  : thePolicy(aPolicy),
    theUse(aUse),
    theTrustedDirs(aTrustedDirs),
    thePrevious(theCurrentScope)
{
  theCurrentScope = this;
}


ResolutionScope::~ResolutionScope()
{
  theCurrentScope = thePrevious;
}


const ResolutionScope* ResolutionScope::current()
{
  return theCurrentScope;
}

} // namespace security
} // namespace zorba
/* vim:set et sw=2 ts=2: */
