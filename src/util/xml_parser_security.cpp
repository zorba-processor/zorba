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

#include <atomic>

#include <libxml/HTMLparser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlversion.h>

#include "context/static_context.h"
#include "system/globalenv.h"
#include "util/security_check.h"

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
  xmlParserErrors const lResult = xmlNewInputFromUrl(aURL, aFlags, aOut);
  if (lResult != XML_ERR_OK && aType == XML_RESOURCE_GENERAL_ENTITY &&
      theCurrent != nullptr)
    theCurrent->entityUnreadable();
  return lResult;
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
  ParserSecurity* const lSecurity = theCurrent;
  if (lSecurity != nullptr && !lSecurity->allowResource(aURL))
    return nullptr;
  xmlExternalEntityLoader const lNext = theNextLoader.load();
  xmlParserInputPtr const lInput =
    lNext == nullptr ? xmlNoNetExternalEntityLoader(aURL, aID, aCtxt)
                     : lNext(aURL, aID, aCtxt);
  // This loader is not told what it loads: outside the DTD (inSubset is 0),
  // it is an external general entity referenced from the content.
  if (lInput == nullptr && lSecurity != nullptr &&
      aCtxt != nullptr && aCtxt->inSubset == 0)
    lSecurity->entityUnreadable();
  return lInput;
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


} // anonymous namespace


bool allows_external_resource(const SecurityPolicy& aPolicy, const char* aURI)
{
  if (!aPolicy.allows(Capability::ExternalEntities) ||
      aURI == nullptr || *aURI == '\0')
    return false;
  return security::check(aPolicy, aURI, security::Use::Document).allowed;
}


ParserSecurity::ParserSecurity(const SecurityPolicy& aPolicy)
  :
  thePolicy(aPolicy),
  theDenied(false),
  theUnreadableEntity(false)
{
}


ParserSecurity::ParserSecurity(const SecurityPolicy* aPolicy)
  :
  thePolicy(aPolicy ? *aPolicy
                    : GENV_ROOT_STATIC_CONTEXT.get_security_policy()),
  theDenied(false),
  theUnreadableEntity(false)
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


void ParserSecurity::configureHtml(xmlParserCtxtPtr aCtxt, int aRequested) const
{
  int lOptions = aRequested;
  lOptions |= HTML_PARSE_NONET;
  // HTML_PARSE_HUGE (not declared before libxml2 2.10) is XML_PARSE_HUGE.
  lOptions &= ~XML_PARSE_HUGE;
  htmlCtxtUseOptions(aCtxt, lOptions);

  // HTML has no entity declarations, so no amplification limit; the
  // resource loader refuses whatever the parser might still try to load.
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
