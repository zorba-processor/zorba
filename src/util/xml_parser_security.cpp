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

#include <atomic>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include <libxml/parserInternals.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlversion.h>

#include <zorba/util/fs_util.h>

#include "context/static_context.h"
#include "system/globalenv.h"

#include "xml_parser_security.h"

namespace zorba {
namespace xml {

namespace {

// The ParserSecurity the entity loader consults on this thread.
thread_local ParserSecurity* theCurrent = nullptr;

#if LIBXML_VERSION >= 21400

// Per-context resource loader. Only installed on contexts configured by
// ParserSecurity, so a load outside a Scope is refused (fail closed).
xmlParserErrors resource_loader(
    void* /*aCtxt*/,
    const char* aURL,
    const char* /*aPublicId*/,
    xmlResourceType aType,
    xmlParserInputFlags aFlags,
    xmlParserInputPtr* aOut)
{
  *aOut = nullptr;
  if (aType != XML_RESOURCE_MAIN_DOCUMENT)
  {
    ParserSecurity* const lSecurity = theCurrent;
    if (lSecurity == nullptr || !lSecurity->allowResource(aURL))
      return XML_IO_EACCES;
  }
  return xmlNewInputFromUrl(aURL, aFlags, aOut);
}

#else /* LIBXML_VERSION < 21400 */

// libxml2 before 2.14 has no per-context loader. The process-wide external
// entity loader is replaced (once, lazily) by one that consults the current
// ParserSecurity and then delegates to the loader it replaced. Parses that do
// not run inside a Scope (the embedder's own libxml2 use) are not affected.
std::atomic<xmlExternalEntityLoader> theNextLoader(nullptr);

xmlParserInputPtr entity_loader(
    const char* aURL,
    const char* aID,
    xmlParserCtxtPtr aCtxt)
{
  if (ParserSecurity* const lSecurity = theCurrent)
  {
    if (!lSecurity->allowResource(aURL))
      return nullptr;
  }
  xmlExternalEntityLoader const lNext = theNextLoader.load();
  if (lNext == nullptr)
    return xmlNoNetExternalEntityLoader(aURL, aID, aCtxt);
  return lNext(aURL, aID, aCtxt);
}

void install_entity_loader()
{
  xmlExternalEntityLoader const lCurrent = xmlGetExternalEntityLoader();
  if (lCurrent != &entity_loader)
  {
    theNextLoader.store(lCurrent);
    xmlSetExternalEntityLoader(&entity_loader);
  }
}

#endif /* LIBXML_VERSION */


// The scheme of a URI, lower-cased, or "" if it has none. A single letter
// followed by ':' is a Windows drive, not a scheme.
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

bool is_separator(char c)
{
  return c == '/' || c == '\\';
}

// Absolute, lexically normalized path of a file path or file: URI, with
// symbolic links resolved where the path (or its parent directory) exists.
// Throws std::invalid_argument for malformed input.
std::string canonical_path(const std::string& aPathOrURI)
{
  std::string const lCwd = fs::curdir();
  std::string lPath = fs::normalize_path(aPathOrURI.c_str(), lCwd.c_str());
#ifndef WIN32
  char lBuf[PATH_MAX];
  if (::realpath(lPath.c_str(), lBuf) != nullptr)
    return lBuf;
  std::string::size_type const lSep = lPath.rfind('/');
  if (lSep != std::string::npos && lSep > 0)
  {
    std::string const lParent(lPath, 0, lSep);
    if (::realpath(lParent.c_str(), lBuf) != nullptr)
      return std::string(lBuf) + lPath.substr(lSep);
  }
#endif
  return lPath;
}

// True if path aInner equals directory aOuter or lies below it.
bool dir_covers(std::string aOuter, const std::string& aInner)
{
  while (aOuter.size() > 1 && is_separator(aOuter.back()))
    aOuter.erase(aOuter.size() - 1);
  if (aInner.compare(0, aOuter.size(), aOuter) != 0)
    return false;
  return aInner.size() == aOuter.size() ||
         is_separator(aOuter.back()) ||
         is_separator(aInner[aOuter.size()]);
}

bool file_allowed(const SecurityPolicy& aPolicy, const std::string& aURI)
{
  if (!aPolicy.allows(Capability::FileRead))
    return false;
  const std::vector<String>& lDirs = aPolicy.getReadPaths();
  if (lDirs.empty())
    return true;
  try
  {
    std::string const lPath = canonical_path(aURI);
    for (const String& lDir : lDirs)
    {
      if (dir_covers(canonical_path(lDir.str()), lPath))
        return true;
    }
  }
  catch (const std::exception&)
  {
  }
  return false;
}

// Plain prefix match on the URI as the parser sees it. Normalization of
// network URLs (host case, default ports) comes with the general URL
// enforcement in static_context.
bool network_allowed(const SecurityPolicy& aPolicy, const std::string& aURI)
{
  if (!aPolicy.allows(Capability::Network))
    return false;
  const std::vector<String>& lPrefixes = aPolicy.getNetworkPrefixes();
  if (lPrefixes.empty())
    return true;
  for (const String& lPrefix : lPrefixes)
  {
    if (aURI.compare(0, lPrefix.size(), lPrefix.str()) == 0)
      return true;
  }
  return false;
}

} // anonymous namespace


bool allows_external_resource(const SecurityPolicy& aPolicy, const char* aURI)
{
  if (!aPolicy.allows(Capability::ExternalEntities) ||
      aURI == nullptr || *aURI == '\0')
    return false;

  std::string const lURI(aURI);
  std::string const lScheme = uri_scheme(lURI);

  if (lScheme.empty() || lScheme == "file")
    return file_allowed(aPolicy, lURI);

  if (lScheme == "http" || lScheme == "https" || lScheme == "ftp")
    return network_allowed(aPolicy, lURI);

  // Anything else is up to libxml2's loader; allow it only if neither file
  // nor network access is restricted.
  return aPolicy.allows(Capability::FileRead) &&
         aPolicy.getReadPaths().empty() &&
         aPolicy.allows(Capability::Network) &&
         aPolicy.getNetworkPrefixes().empty();
}


ParserSecurity::ParserSecurity(const SecurityPolicy& aPolicy)
  :
  thePolicy(aPolicy),
  theDenied(false)
{
}


ParserSecurity::ParserSecurity(const SecurityPolicy* aPolicy)
  :
  thePolicy(aPolicy ? *aPolicy
                    : GENV_ROOT_STATIC_CONTEXT.get_security_policy()),
  theDenied(false)
{
}


int ParserSecurity::options(int aRequested) const
{
  int lOptions = aRequested;
  lOptions |= XML_PARSE_NONET;
  lOptions &= ~XML_PARSE_HUGE;
#if LIBXML_VERSION >= 21400
  lOptions |= XML_PARSE_NO_SYS_CATALOG;
#endif
  if (!thePolicy.allows(Capability::ExternalEntities))
    lOptions &= ~(XML_PARSE_DTDLOAD | XML_PARSE_XINCLUDE);
  return lOptions;
}


void ParserSecurity::configure(xmlParserCtxtPtr aCtxt, int aRequested) const
{
  xmlCtxtUseOptions(aCtxt, options(aRequested));

#if LIBXML_VERSION >= 21100
  if (uint32_t const lMax = thePolicy.getMaxEntityAmplification())
    xmlCtxtSetMaxAmplification(aCtxt, lMax);
#endif

#if LIBXML_VERSION >= 21400
  xmlCtxtSetResourceLoader(aCtxt, &resource_loader, nullptr);
#else
  install_entity_loader();
#endif
}


bool ParserSecurity::allowResource(const char* aURI)
{
  if (allows_external_resource(thePolicy, aURI))
    return true;
  if (!theDenied)
  {
    theDenied = true;
    theDeniedURI = aURI ? aURI : "";
  }
  return false;
}


ParserSecurity::Scope::Scope(ParserSecurity& aSecurity)
  :
  thePrevious(theCurrent)
{
  theCurrent = &aSecurity;
}


ParserSecurity::Scope::~Scope()
{
  theCurrent = thePrevious;
}

} // namespace xml
} // namespace zorba
/* vim:set et sw=2 ts=2: */
