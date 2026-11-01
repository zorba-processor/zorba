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
#include <climits>
#include <sstream>
#include <string>
#include <vector>

#include <libxml/HTMLparser.h>
#include <libxml/SAX2.h>

#include "store_defs.h"
#include "simple_store.h"
#include "node_items.h"
#include "loader.h"
#include "node_factory.h"

#include "diagnostics/xquery_diagnostics.h"
#include "diagnostics/assert.h"
#include "util/stream_util.h"

namespace zorba
{

namespace simplestore
{

namespace
{

inline bool is_namespace_declaration(const xmlChar* aName)
{
  return xmlStrEqual(aName, BAD_CAST "xmlns") ||
         xmlStrncmp(aName, BAD_CAST "xmlns:", 6) == 0;
}

} // anonymous namespace


/*******************************************************************************

********************************************************************************/
HtmlXmlLoader::HtmlXmlLoader(
    store::ItemFactory* factory,
    XQueryDiagnostics* xqueryDiagnostics,
    const store::LoadProperties& loadProperties,
    bool dataguide)
  :
  FastXmlLoader(factory, xqueryDiagnostics, loadProperties, dataguide),
  theDepth(0)
{
  // The HTML parser reports elements through the SAX1 callbacks only. It has
  // its own entity table and no DTD, so the entity callbacks are not needed.
  theSaxHandler.startElementNs = NULL;
  theSaxHandler.endElementNs = NULL;
  theSaxHandler.startElement = &HtmlXmlLoader::htmlStartElement;
  theSaxHandler.endElement = &HtmlXmlLoader::htmlEndElement;
  theSaxHandler.characters = &HtmlXmlLoader::htmlCharacters;
  theSaxHandler.ignorableWhitespace = &HtmlXmlLoader::htmlCharacters;
  // Raw text (script, style) is reported as CDATA: a text node.
  theSaxHandler.cdataBlock = &HtmlXmlLoader::htmlCharacters;
  theSaxHandler.getEntity = NULL;
  theSaxHandler.getParameterEntity = NULL;
  theSaxHandler.entityDecl = NULL;
  theSaxHandler.serror = &HtmlXmlLoader::htmlError;
}


/*******************************************************************************
  The whole input is read first and given to the push parser at once: HTML
  documents are small next to the tree built from them, and the encoding
  detection (byte order mark, meta charset) then sees all of it.
********************************************************************************/
store::Item_t HtmlXmlLoader::loadXml(
    const zstring& baseUri,
    const zstring& docUri,
    std::istream& stream)
{
  theTree = GET_STORE().getNodeFactory().createXmlTree();
  theBaseUri = baseUri;
  theDepth = 0;

  char const *doc_uri;
  if ( char const *const stream_uri = get_uri( stream ) )
    doc_uri = stream_uri;
  else
    doc_uri = docUri.c_str();

  if (docUri.empty())
  {
    std::ostringstream uristream;
    uristream << "zorba://internalDocumentURI-" << theTree->getTreeId();
    theDocUri = uristream.str();
  }
  else
  {
    theDocUri = docUri;
  }

  thePathStack.push(PathStepInfo(NULL, baseUri));

  xml::ParserSecurity::Scope lSecurityScope(theParserSecurity);

  try
  {
    std::string lInput;
    std::streamsize numChars;
    while ((numChars = readPacket(stream, theBuffer, INPUT_CHUNK_SIZE)) > 0)
      lInput.append(theBuffer, static_cast<std::string::size_type>(numChars));

    if (numChars < 0 || !theXQueryDiagnostics->errors().empty())
    {
      if (theXQueryDiagnostics->errors().empty())
        theXQueryDiagnostics->
        add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0020_LOADER_IO_ERROR));
      abortload();
      return NULL;
    }

    if (!lInput.empty())
    {
      if (lInput.size() > static_cast<std::string::size_type>(INT_MAX))
      {
        theXQueryDiagnostics->
        add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR,
                  ERROR_PARAMS(theDocUri, 0, 0, "input too large")));
        abortload();
        return NULL;
      }

      bool const lIgnoreEncoding =
        theLoadProperties.getIgnoreDeclaredEncoding();

      ctxt = htmlCreatePushParserCtxt(&theSaxHandler,
                                      this,
                                      lInput.data(),
                                      static_cast<int>(lInput.size()),
                                      doc_uri,
                                      lIgnoreEncoding ?
                                        XML_CHAR_ENCODING_UTF8 :
                                        XML_CHAR_ENCODING_NONE);
      if (ctxt == NULL)
      {
        theXQueryDiagnostics->
        add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR,
                                      ERROR_PARAMS(ZED(XMLParserInitFailed))));
        abortload();
        return NULL;
      }

      int lOptions = HTML_PARSE_RECOVER | HTML_PARSE_NONET;
      if (lIgnoreEncoding)
        lOptions |= HTML_PARSE_IGNORE_ENC;
      theParserSecurity.configureHtml(ctxt, lOptions);

      htmlParseChunk(ctxt, NULL, 0, 1);

      htmlFreeParserCtxt(ctxt);
      ctxt = NULL;
    }
  }
  // An exception escaping libxml2 must still be reported: the callers take a
  // null document with no error as success.
  catch (ZorbaException const& e)
  {
    theXQueryDiagnostics->add_error(e);
    abortload();
    return NULL;
  }
  catch (...)
  {
    theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION(zerr::ZXQP0003_INTERNAL_ERROR));
    abortload();
    return NULL;
  }

  if (!theXQueryDiagnostics->errors().empty())
  {
    abortload();
    return NULL;
  }

  // Empty input (or input the parser reported nothing for): a document node
  // without children.
  if (theRootNode == NULL)
  {
    startDocument(this);
    endDocument(this);
  }

  // The parser closes every open element and the document at the end of the
  // input; anything else would leave nodes on the stacks.
  if (!theXQueryDiagnostics->errors().empty() ||
      theNodeStack.size() != 1 || thePathStack.size() != 1 || theDepth != 0)
  {
    if (theXQueryDiagnostics->errors().empty())
      theXQueryDiagnostics->add_error(
        NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR,
                            ERROR_PARAMS(ZED(BadXMLDocument_2o), theDocUri)));
    abortload();
    return NULL;
  }

  thePathStack.pop();

  XmlNode* resultNode = theRootNode;
  reset();
  return resultNode;
}


/*******************************************************************************
  SAX1 start tag: attributes are (name, value) pairs; a value is NULL for an
  attribute written without one (HTML boolean attributes), which is the empty
  string.
********************************************************************************/
void HtmlXmlLoader::htmlStartElement(
    void * ctx,
    const xmlChar * name,
    const xmlChar ** atts)
{
  HtmlXmlLoader& loader = *(static_cast<HtmlXmlLoader *>(ctx));
  if (!loader.theXQueryDiagnostics->errors().empty() || name == NULL)
    return;

  if (loader.theDepth >= MAX_DEPTH)
  {
    std::ostringstream lMessage;
    lMessage << "element nesting deeper than " << MAX_DEPTH << " levels";
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR,
                          ERROR_PARAMS(loader.theDocUri,
                                       xmlSAX2GetLineNumber(loader.ctxt),
                                       xmlSAX2GetColumnNumber(loader.ctxt),
                                       lMessage.str())));
    xmlStopParser(loader.ctxt);
    return;
  }

  try
  {
    // (localname, prefix, URI, value, end) per attribute, as SAX2 has them.
    std::vector<const xmlChar*> lAttributes;
    int lNumAttributes = 0;

    if (atts != NULL)
    {
      for (csize i = 0; atts[i] != NULL; i += 2)
      {
        const xmlChar* lName = atts[i];
        const xmlChar* lValue = atts[i + 1] ? atts[i + 1] : BAD_CAST "";

        if (is_namespace_declaration(lName))
          continue;

        bool lDuplicate = false;
        for (int j = 0; j < lNumAttributes && !lDuplicate; ++j)
          lDuplicate = xmlStrEqual(lAttributes[j * 5], lName);
        if (lDuplicate)
          continue;

        lAttributes.push_back(lName);
        lAttributes.push_back(NULL);
        lAttributes.push_back(NULL);
        lAttributes.push_back(lValue);
        lAttributes.push_back(lValue + xmlStrlen(lValue));
        ++lNumAttributes;
      }
    }

    ++loader.theDepth;

    FastXmlLoader::startElement(ctx,
                                name,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                lNumAttributes,
                                0,
                                lNumAttributes > 0 ?
                                  &lAttributes[0] : NULL);
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error(e);
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION(zerr::ZXQP0003_INTERNAL_ERROR));
  }
}


/*******************************************************************************

********************************************************************************/
void HtmlXmlLoader::htmlEndElement(void * ctx, const xmlChar * name)
{
  HtmlXmlLoader& loader = *(static_cast<HtmlXmlLoader *>(ctx));
  if (!loader.theXQueryDiagnostics->errors().empty() || loader.theDepth == 0)
    return;

  --loader.theDepth;
  FastXmlLoader::endElement(ctx, name, NULL, NULL);
}


/*******************************************************************************
  Text outside the root element can only be whitespace (the parser opens html
  and body for anything else); it is dropped, as in XML documents.
********************************************************************************/
void HtmlXmlLoader::htmlCharacters(void * ctx, const xmlChar * ch, int len)
{
  HtmlXmlLoader& loader = *(static_cast<HtmlXmlLoader *>(ctx));
  if (!loader.theXQueryDiagnostics->errors().empty() || len <= 0)
    return;

  if (loader.theDepth == 0)
    return;

  FastXmlLoader::characters(ctx, ch, len);
}


/*******************************************************************************
  The parser recovers from malformed markup, so its errors are not reported;
  running out of memory or into one of its resource limits is.
********************************************************************************/
void HtmlXmlLoader::htmlError(void * ctx, ZORBA_XML_ERROR_PTR error)
{
  if (error == NULL)
    return;

  bool lReport = (error->code == XML_ERR_NO_MEMORY);
#if LIBXML_VERSION >= 21300
  lReport = lReport || (error->code == XML_ERR_RESOURCE_LIMIT);
#endif

  if (lReport)
    XmlLoader::error(ctx, error);
}


} // namespace simplestore
} // namespace zorba
/* vim:set et sw=2 ts=2: */
