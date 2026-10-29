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

#include <memory.h>
#include <string>
#include <iostream>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parserInternals.h>
#include <libxml/uri.h>
#include <libxml/SAX2.h>
#include <cstring>
#include <utility>

#include <zorba/store_consts.h>

#include "store_defs.h"
#include "string_pool.h"
#include "qname_pool.h"
#include "simple_store.h"
#include "atomic_items.h"
#include "node_items.h"
#include "dataguide.h"
#include "nsbindings.h"
#include "loader.h"
#include "simple_item_factory.h"
#include "node_factory.h"
#include "zorbatypes/datetime.h"
#include "zorbatypes/URI.h"

#include "diagnostics/xquery_diagnostics.h"
#include "diagnostics/assert.h"
#include "util/stream_util.h"


namespace zorba { namespace simplestore {

#ifndef NDEBUG

#define LOADER_TRACE(level, msg)             \
{                                            \
  if (level <= loader.theTraceLevel)         \
    std::cout << msg << std::endl;           \
}

#define LOADER_TRACE1(msg) LOADER_TRACE(1, msg);
#define LOADER_TRACE2(msg) LOADER_TRACE(2, msg);
#define LOADER_TRACE3(msg) LOADER_TRACE(3, msg);

#else
#define LOADER_TRACE(msg)
#define LOADER_TRACE1(msg)
#define LOADER_TRACE2(msg)
#define LOADER_TRACE3(msg)
#endif

#define ZORBA_LOADER_CHECK_ERROR(loader) \
  do { \
    if (!loader.theXQueryDiagnostics->errors().empty()) \
      return; \
  } while (0);


/*******************************************************************************

********************************************************************************/
void XmlLoader::applyLoadOptions(const store::LoadProperties& props, xmlParserCtxtPtr ctxt)
{
  xmlCtxtUseOptions(ctxt, props.toLibXmlOptions());
}


/*******************************************************************************

  FragmentXmlLoader parses an XML external parsed entity (a sequence of
  elements, text, comments and PIs, optionally preceded by a text declaration
  and, as a Zorba extension, a DOCTYPE).

  It uses only libxml2's public push-parser API. The input is wrapped in a
  synthetic element so that libxml2 sees a well-formed document. SAX events
  are recorded into the FragmentIStream's event queue and replayed into the
  loader, so that each loadXml() call can stop at a top-level node boundary
  and the next call can resume where the previous one left off.

********************************************************************************/
namespace {

char const FRAGMENT_WRAPPER_START[] = "<zorba-fragment-wrapper>";
char const FRAGMENT_WRAPPER_END[] = "</zorba-fragment-wrapper>";

// Stop looking for a DOCTYPE after this many bytes of prolog.
std::string::size_type const MAX_PROLOG_SCAN = 1024 * 1024;

inline bool starts_with(std::string const &s, std::string::size_type pos,
                        char const *prefix) {
  return s.compare(pos, std::strlen(prefix), prefix) == 0;
}

inline bool is_xml_space(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/**
 * Scans the start of a fragment for a text declaration and a DOCTYPE.
 *
 * @param s The buffered start of the input.
 * @param eof Whether \a s holds the whole input.
 * @param complete Set to \c false if more input is needed to decide.
 * @param decl_end Set to the offset just past the text declaration, or 0.
 * @param doctype_end Set to the offset just past the DOCTYPE, or 0.
 */
void scan_prolog(std::string const &s, bool eof, bool &complete,
                 std::string::size_type &decl_end,
                 std::string::size_type &doctype_end) {
  typedef std::string::size_type size_type;
  complete = true;
  decl_end = doctype_end = 0;

  size_type i = 0;
  if (starts_with(s, 0, "\xEF\xBB\xBF"))
    i = 3;

  if (starts_with(s, i, "<?xml") && s.size() > i + 5 && is_xml_space(s[i + 5])) {
    size_type const end = s.find("?>", i);
    if (end == std::string::npos) {
      complete = eof;
      return;
    }
    decl_end = end + 2;
  }

  for (i = decl_end; ; ) {
    while (i < s.size() && is_xml_space(s[i]))
      ++i;
    if (i >= MAX_PROLOG_SCAN)
      return;
    if (s.size() - i < 9 && !eof) {
      complete = false;
      return;
    }
    char const *close = NULL;
    if (starts_with(s, i, "<!--"))
      close = "-->";
    else if (starts_with(s, i, "<?"))
      close = "?>";
    if (close) {
      size_type const end = s.find(close, i);
      if (end == std::string::npos) {
        complete = eof;
        return;
      }
      i = end + std::strlen(close);
      continue;
    }
    if (!starts_with(s, i, "<!DOCTYPE"))
      return;

    int brackets = 0;
    char quote = 0;
    for (size_type k = i + 9; k < s.size(); ++k) {
      char const c = s[k];
      if (quote) {
        if (c == quote)
          quote = 0;
      } else if (c == '"' || c == '\'') {
        quote = c;
      } else if (c == '[') {
        ++brackets;
      } else if (c == ']') {
        --brackets;
      } else if (c == '>' && brackets <= 0) {
        doctype_end = k + 1;
        return;
      }
    }
    complete = eof;
    return;
  }
}

inline xmlChar const* xml_str(std::string const &s) {
  return reinterpret_cast<xmlChar const*>(s.c_str());
}

inline std::string to_string(xmlChar const *s) {
  return s ? std::string(reinterpret_cast<char const*>(s)) : std::string();
}

} // anonymous namespace


FragmentXmlLoader::FragmentXmlLoader(
    store::ItemFactory* factory,
    XQueryDiagnostics* xqueryDiagnostics,
    const store::LoadProperties& loadProperties,
    bool dataguide)
  :
  FastXmlLoader(factory, xqueryDiagnostics, loadProperties, dataguide),
  theFragmentStream(NULL)
{
  theOrdPath.init();

  theTraceLevel = GET_STORE().getTraceLevel();

  // The parser context is the SAX user data, so libxml2's own SAX2 handlers
  // can maintain the DTD and entities; the FragmentIStream is reached via
  // ctxt->_private.
  memset(&theSaxHandler, 0, sizeof(theSaxHandler) );
  theSaxHandler.initialized = XML_SAX2_MAGIC;
  theSaxHandler.startDocument = &xmlSAX2StartDocument;
  theSaxHandler.endDocument = &xmlSAX2EndDocument;
  theSaxHandler.internalSubset = &xmlSAX2InternalSubset;
  theSaxHandler.entityDecl = &xmlSAX2EntityDecl;
  theSaxHandler.getEntity = &xmlSAX2GetEntity;
  theSaxHandler.getParameterEntity = &xmlSAX2GetParameterEntity;
  theSaxHandler.startElementNs = &FragmentXmlLoader::recordStartElement;
  theSaxHandler.endElementNs = &FragmentXmlLoader::recordEndElement;
  theSaxHandler.characters = &FragmentXmlLoader::recordCharacters;
  theSaxHandler.cdataBlock = &FragmentXmlLoader::recordCdataBlock;
  theSaxHandler.comment = &FragmentXmlLoader::recordComment;
  theSaxHandler.processingInstruction =
    &FragmentXmlLoader::recordProcessingInstruction;
  theSaxHandler.serror = &FragmentXmlLoader::recordError;
}


FragmentXmlLoader::~FragmentXmlLoader()
{
}


/*******************************************************************************
  Creates the push parser and feeds it the prolog, the wrapper start tag and
  the first part of the input.
********************************************************************************/
void FragmentXmlLoader::startParser()
{
  FragmentIStream &fs = *theFragmentStream;

  fs.ctxt = xmlCreatePushParserCtxt(&theSaxHandler, NULL, NULL, 0,
                                    get_uri( *fs.theStream ));
  if (fs.ctxt == NULL)
  {
    theXQueryDiagnostics->add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR, ERROR_PARAMS( ZED( XMLParserInitFailed ) )));
    throw 0; // the argument to throw is not used by the catch clause
  }
  fs.ctxt->_private = &fs;
  // Entity substitution is what xmlSubstituteEntitiesDefault(1) used to do.
  xmlCtxtUseOptions(fs.ctxt, theLoadProperties.toLibXmlOptions() | XML_PARSE_NOENT);
  fs.parser_started = true;

  // Buffer enough input to find the end of the prolog.
  std::string head;
  bool eof = false;
  bool complete = false;
  std::string::size_type decl_end, doctype_end;
  std::vector<char> buf(FragmentIStream::DEFAULT_BUFFER_SIZE);
  while (true)
  {
    scan_prolog(head, eof, complete, decl_end, doctype_end);
    if (complete)
      break;
    std::streamsize const n = readPacket(*fs.theStream, &buf[0], buf.size());
    if (n < 0)
    {
      theXQueryDiagnostics->add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0020_LOADER_IO_ERROR));
      throw 0;
    }
    head.append(&buf[0], static_cast<std::string::size_type>(n));
    eof = fs.theStream->eof() || n == 0;
  }

  if (doctype_end && theLoadProperties.getErrorOnDoctype())
  {
    theXQueryDiagnostics->add_error(theDocUri.empty() ?
      NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR, ERROR_PARAMS( ZED( ParseFragmentDoctypeNotAllowed ))) :
      NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR, ERROR_PARAMS( ZED( ParseFragmentDoctypeNotAllowed ), theDocUri))
    );
    throw 0;
  }

  std::string::size_type const prolog_end = doctype_end ? doctype_end : decl_end;
  std::string prolog(head, 0, prolog_end);
  // A text declaration may omit the version; an XML declaration may not.
  if (decl_end && prolog.find("version", 0) >= decl_end)
  {
    std::string::size_type const pos = prolog.find("<?xml");
    prolog.insert(pos + 5, " version=\"1.0\"");
  }
  prolog += FRAGMENT_WRAPPER_START;
  prolog.append(head, prolog_end, std::string::npos);

  xmlParseChunk(fs.ctxt, prolog.data(), static_cast<int>(prolog.size()), 0);
  if (eof)
    finishParser();
}


/*******************************************************************************
  Closes the wrapper element and terminates the parse.
********************************************************************************/
void FragmentXmlLoader::finishParser()
{
  FragmentIStream &fs = *theFragmentStream;
  xmlParseChunk(fs.ctxt, FRAGMENT_WRAPPER_END,
                static_cast<int>(sizeof(FRAGMENT_WRAPPER_END) - 1), 1);
  fs.parser_done = true;
}


/*******************************************************************************
  Feeds the next packet of input to the parser.
********************************************************************************/
void FragmentXmlLoader::feedParser()
{
  FragmentIStream &fs = *theFragmentStream;
  std::vector<char> buf(FragmentIStream::DEFAULT_BUFFER_SIZE * 16);
  std::streamsize const n = readPacket(*fs.theStream, &buf[0], buf.size());
  if (n < 0)
  {
    theXQueryDiagnostics->add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0020_LOADER_IO_ERROR));
    throw 0;
  }
  if (n > 0)
    xmlParseChunk(fs.ctxt, &buf[0], static_cast<int>(n), 0);
  if (fs.theStream->eof() || n == 0)
    finishParser();
}


/*******************************************************************************
  Replays one recorded event into this loader, applying root_elements_to_skip.
********************************************************************************/
void FragmentXmlLoader::replay(FragmentEvent const &ev)
{
  FragmentIStream &fs = *theFragmentStream;
  int const skip = fs.root_elements_to_skip;

  switch (ev.kind)
  {
  case FragmentEvent::START_ELEMENT:
  {
    fs.state = FragmentIStream::FRAGMENT_CONTENT;
    fs.current_element_depth++;
    fs.ns_scopes.push_back(ev.namespaces);
    if (fs.current_element_depth <= skip)
      break;

    // The first element below the skipped ones gets every in-scope
    // namespace binding, so it stays self-contained.
    std::vector<std::pair<std::string,std::string> > in_scope;
    if (fs.current_element_depth == skip + 1)
    {
      for (std::size_t d = 0; d < fs.ns_scopes.size(); ++d)
      {
        for (std::size_t i = 0; i < fs.ns_scopes[d].size(); ++i)
        {
          std::size_t j = 0;
          while (j < in_scope.size() && in_scope[j].first != fs.ns_scopes[d][i].first)
            ++j;
          if (j < in_scope.size())
            in_scope[j] = fs.ns_scopes[d][i];
          else
            in_scope.push_back(fs.ns_scopes[d][i]);
        }
      }
    }
    std::vector<std::pair<std::string,std::string> > const &bindings =
      fs.current_element_depth == skip + 1 ? in_scope : ev.namespaces;

    std::vector<xmlChar const*> ns;
    for (std::size_t i = 0; i < bindings.size(); ++i)
    {
      ns.push_back(bindings[i].first.empty() ? NULL : xml_str(bindings[i].first));
      ns.push_back(xml_str(bindings[i].second));
    }
    std::vector<xmlChar const*> attrs;
    for (std::size_t i = 0; i < ev.attributes.size(); ++i)
    {
      FragmentEvent::attribute const &a = ev.attributes[i];
      attrs.push_back(xml_str(a.localname));
      attrs.push_back(a.has_prefix ? xml_str(a.prefix) : NULL);
      attrs.push_back(a.has_uri ? xml_str(a.uri) : NULL);
      attrs.push_back(xml_str(a.value));
      attrs.push_back(xml_str(a.value) + a.value.size());
    }
    FastXmlLoader::startElement(
      this, xml_str(ev.name),
      ev.has_prefix ? xml_str(ev.prefix) : NULL,
      ev.has_uri ? xml_str(ev.uri) : NULL,
      static_cast<int>(bindings.size()), ns.empty() ? NULL : &ns[0],
      static_cast<int>(ev.attributes.size()), ev.nb_defaulted,
      attrs.empty() ? NULL : &attrs[0]);
    break;
  }
  case FragmentEvent::END_ELEMENT:
    if (fs.current_element_depth > skip)
      FastXmlLoader::endElement(
        this, xml_str(ev.name),
        ev.has_prefix ? xml_str(ev.prefix) : NULL,
        ev.has_uri ? xml_str(ev.uri) : NULL);
    fs.ns_scopes.pop_back();
    fs.current_element_depth--;
    break;
  case FragmentEvent::CHARACTERS:
    if (fs.current_element_depth >= skip)
      FastXmlLoader::characters(this, xml_str(ev.text),
                                static_cast<int>(ev.text.size()));
    break;
  case FragmentEvent::CDATA:
    if (fs.current_element_depth >= skip)
      FastXmlLoader::cdataBlock(this, xml_str(ev.text),
                                static_cast<int>(ev.text.size()));
    break;
  case FragmentEvent::COMMENT:
    if (fs.current_element_depth >= skip)
      FastXmlLoader::comment(this, xml_str(ev.text));
    break;
  case FragmentEvent::PI:
    if (fs.current_element_depth >= skip)
      FastXmlLoader::processingInstruction(this, xml_str(ev.name),
                                           xml_str(ev.text));
    break;
  }
}


store::Item_t FragmentXmlLoader::loadXml(
    const zstring& baseUri,
    const zstring& docUri,
    std::istream& stream)
{
  theTree = GET_STORE().getNodeFactory().createXmlTree();

  theBaseUri = baseUri;

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

  theFragmentStream = static_cast<FragmentIStream*>(&stream);
  FragmentIStream &fs = *theFragmentStream;

  try
  {
    fs.current_loader = static_cast<XmlLoader*>(this);
    if (!fs.parser_started)
      startParser();
    if (!theXQueryDiagnostics->errors().empty())
      throw 0;

    FastXmlLoader::startDocument(this);
    if (fs.state == FragmentIStream::FRAGMENT_FIRST_START_DOC)
      fs.state = FragmentIStream::FRAGMENT_PROLOG;

    // Without only_one_doc_node, return after each top-level node or, when
    // root elements are skipped, after a batch of nodes below them.
    unsigned int const batch_size = fs.root_elements_to_skip > 0 ?
      FragmentIStream::PARSED_NODES_BATCH_SIZE : 1;
    unsigned int produced = 0;
    while (true)
    {
      if (fs.events.empty())
      {
        if (fs.parser_done)
          break;
        feedParser();
        if (!theXQueryDiagnostics->errors().empty())
          throw 0;
        continue;
      }

      FragmentEvent const ev(std::move(fs.events.front()));
      fs.events.pop_front();
      replay(ev);
      if (!theXQueryDiagnostics->errors().empty())
        throw 0;

      if (!fs.only_one_doc_node &&
          fs.current_element_depth <= fs.root_elements_to_skip &&
          ++produced >= batch_size)
        break;
    }

    if (fs.parser_done && fs.events.empty() && !fs.ctxt->wellFormed)
    {
      theXQueryDiagnostics->add_error(theDocUri.empty() ?
          NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR, ERROR_PARAMS( ZED( BadXMLDocument_2o ))) :
          NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR, ERROR_PARAMS( ZED( BadXMLDocument_2o ), theDocUri))
        );
      throw 0;
    }

    FastXmlLoader::endDocument(this);
  }
  catch (...)
  {
    fs.current_loader = NULL;
    fs.parser_done = true;
    fs.events.clear();
    abortload();
    thePathStack.clear();
    return NULL;
  }

  fs.current_loader = NULL;

  // The doc may be well formed, but it may have other kinds of errors, e.g., unresolved ns prefixes.
  if (!theXQueryDiagnostics->errors().empty())
  {
    fs.parser_done = true;
    fs.events.clear();
    abortload();
    return NULL;
  }

  thePathStack.pop();
  assert(thePathStack.empty());

  XmlNode* resultNode;
  resultNode = theRootNode;
  reset();
  return resultNode;
}


/*******************************************************************************
  SAX callbacks: record events. ctx is the libxml2 parser context.
********************************************************************************/
FragmentIStream* FragmentXmlLoader::streamOf(void *ctx)
{
  return static_cast<FragmentIStream*>(static_cast<xmlParserCtxtPtr>(ctx)->_private);
}


void FragmentXmlLoader::recordStartElement(
    void * ctx,
    const xmlChar * localname,
    const xmlChar * prefix,
    const xmlChar * URI,
    int nb_namespaces,
    const xmlChar ** namespaces,
    int nb_attributes,
    int nb_defaulted,
    const xmlChar ** attributes)
{
  FragmentIStream &fs = *streamOf(ctx);
  if (++fs.parse_depth == 1)
    return;                             // the wrapper element

  FragmentEvent ev;
  ev.kind = FragmentEvent::START_ELEMENT;
  ev.name = to_string(localname);
  ev.has_prefix = prefix != NULL;
  ev.prefix = to_string(prefix);
  ev.has_uri = URI != NULL;
  ev.uri = to_string(URI);
  for (int i = 0; i < nb_namespaces; ++i)
    ev.namespaces.push_back(std::make_pair(to_string(namespaces[2 * i]),
                                           to_string(namespaces[2 * i + 1])));
  for (int i = 0; i < nb_attributes; ++i)
  {
    xmlChar const **const a = attributes + 5 * i;
    FragmentEvent::attribute attr;
    attr.localname = to_string(a[0]);
    attr.has_prefix = a[1] != NULL;
    attr.prefix = to_string(a[1]);
    attr.has_uri = a[2] != NULL;
    attr.uri = to_string(a[2]);
    attr.value.assign(reinterpret_cast<char const*>(a[3]),
                      static_cast<std::string::size_type>(a[4] - a[3]));
    ev.attributes.push_back(attr);
  }
  ev.nb_defaulted = nb_defaulted;
  fs.events.push_back(std::move(ev));
}


void FragmentXmlLoader::recordEndElement(
    void * ctx,
    const xmlChar * localname,
    const xmlChar * prefix,
    const xmlChar * URI)
{
  FragmentIStream &fs = *streamOf(ctx);
  if (fs.parse_depth-- == 1)
    return;                             // the wrapper element

  FragmentEvent ev;
  ev.kind = FragmentEvent::END_ELEMENT;
  ev.name = to_string(localname);
  ev.has_prefix = prefix != NULL;
  ev.prefix = to_string(prefix);
  ev.has_uri = URI != NULL;
  ev.uri = to_string(URI);
  fs.events.push_back(std::move(ev));
}


void FragmentXmlLoader::recordText(
    void * ctx,
    FragmentEvent::kind_type kind,
    const xmlChar * text,
    int len)
{
  FragmentIStream &fs = *streamOf(ctx);
  if (fs.parse_depth == 0)
    return;                             // whitespace around the wrapper

  // libxml2 may split text; merge consecutive pieces of the same kind.
  if (!fs.events.empty() && fs.events.back().kind == kind)
  {
    fs.events.back().text.append(reinterpret_cast<char const*>(text), len);
    return;
  }
  FragmentEvent ev;
  ev.kind = kind;
  ev.text.assign(reinterpret_cast<char const*>(text), len);
  fs.events.push_back(std::move(ev));
}


void FragmentXmlLoader::recordCharacters(
    void * ctx,
    const xmlChar * ch,
    int len)
{
  recordText(ctx, FragmentEvent::CHARACTERS, ch, len);
}


void FragmentXmlLoader::recordCdataBlock(
    void * ctx,
    const xmlChar * value,
    int len)
{
  recordText(ctx, FragmentEvent::CDATA, value, len);
}


void FragmentXmlLoader::recordComment(
    void * ctx,
    const xmlChar * value)
{
  FragmentEvent ev;
  ev.kind = FragmentEvent::COMMENT;
  ev.text = to_string(value);
  streamOf(ctx)->events.push_back(std::move(ev));
}


void FragmentXmlLoader::recordProcessingInstruction(
    void * ctx,
    const xmlChar * target,
    const xmlChar * data)
{
  FragmentEvent ev;
  ev.kind = FragmentEvent::PI;
  ev.name = to_string(target);
  ev.text = to_string(data);
  streamOf(ctx)->events.push_back(std::move(ev));
}


void FragmentXmlLoader::recordError(void* ctx, ZORBA_XML_ERROR_PTR error)
{
  FragmentIStream &fs = *streamOf(ctx);
  if (fs.current_loader)
    XmlLoader::error(fs.current_loader, error);
}


/*******************************************************************************

********************************************************************************/
DtdXmlLoader::DtdXmlLoader(
    store::ItemFactory* factory,
    XQueryDiagnostics* xqueryDiagnostics,
    const store::LoadProperties& loadProperties,
    bool dataguide)
  :
  XmlLoader(factory, xqueryDiagnostics, loadProperties, dataguide),
  theTree(NULL),
  theRootNode(NULL),
  theNodeStack(2048)
{
  theOrdPath.init();

  theTraceLevel = GET_STORE().getTraceLevel();

  memset(&theSaxHandler, 0, sizeof(theSaxHandler) );
  theSaxHandler.initialized = XML_SAX2_MAGIC;
  //theSaxHandler.startDocument = &DtdXmlLoader::startDocument;
  //theSaxHandler.endDocument = &DtdXmlLoader::endDocument;
  //theSaxHandler.startElementNs = &DtdXmlLoader::startElement;
  //theSaxHandler.endElementNs = &DtdXmlLoader::endElement;
  //theSaxHandler.characters = &DtdXmlLoader::characters;
  //theSaxHandler.cdataBlock = &DtdXmlLoader::cdataBlock;
  //theSaxHandler.comment = &DtdXmlLoader::comment;
  //theSaxHandler.processingInstruction = &DtdXmlLoader::processingInstruction;
  theSaxHandler.serror = &XmlLoader::error;
  theSaxHandler.getEntity = &DtdXmlLoader::getEntity;
  theSaxHandler.getParameterEntity = &DtdXmlLoader::getParameterEntity;
  theSaxHandler.entityDecl = &DtdXmlLoader::entityDecl;
}


/*******************************************************************************

********************************************************************************/
DtdXmlLoader::~DtdXmlLoader()
{
}


/*******************************************************************************

********************************************************************************/
void DtdXmlLoader::setRoot(XmlNode* root)
{
  theRootNode = root;
  theTree->setRoot(root);
}


/*******************************************************************************
  Method called to do cleanup in case of errors.
********************************************************************************/
void DtdXmlLoader::abortload()
{
  theBaseUri.clear();
  theDocUri.clear();

  if (theTree != NULL)
  {
    delete theTree;
    theTree = NULL;
  }

  theOrdPath.init();
  theRootNode = NULL;

  while(!theNodeStack.empty())
  {
    XmlNode* node = theNodeStack.top();
    theNodeStack.pop();
    if (node != NULL)
      node->destroy(true);
  }

  thePathStack.clear();

#ifdef DATAGUIDE
  if(!theGuideStack.empty())
  {
    GuideNode* node = theGuideStack[0];
    if (node != NULL)
      node->deleteTree();
  }

  theGuideStack.clear();
#endif

  while(!theBindingsStack.empty())
  {
    //NsBindingsContext* ctx = theBindingsStack.top();
    theBindingsStack.pop();
    //delete ctx;
  }

  if(ctxt)
  {
    xmlCtxtReset(ctxt);
    xmlFreeParserCtxt(ctxt);
  }

  ctxt = NULL;
}


/*******************************************************************************
  Method used to reset the loader so it can be used to load another document.
********************************************************************************/
void DtdXmlLoader::reset()
{
  theBaseUri.clear();
  theDocUri.clear();

  theTree = NULL;
  theOrdPath.init();
  theRootNode = NULL;

  if ( !theNodeStack.empty() )
    theNodeStack.pop();

  ZORBA_ASSERT(theNodeStack.empty());
#ifdef DATAGUIDE
  ZORBA_ASSERT(theGuideStack.empty());
#endif
  ZORBA_ASSERT(theBindingsStack.empty());
}


/*******************************************************************************
  Read up to "size" bytes from the given input stream into the given buffer.
  Return the number of bytes actually read, throw an exception if any I/O
  error occured.
********************************************************************************/
std::streamsize DtdXmlLoader::readPacket(std::istream& stream, char* buf, std::streamoff size)
{
  try
  {
    stream.read(buf, size);

    if (stream.bad())
    {
      theXQueryDiagnostics->add_error(
      	NEW_ZORBA_EXCEPTION(
          zerr::ZSTR0020_LOADER_IO_ERROR, ERROR_PARAMS( ZED( BadStreamState ) )
        )
      );
    }

    return stream.gcount();
  }
  catch (std::iostream::failure const &e)
  {
    theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION(
        zerr::ZSTR0020_LOADER_IO_ERROR, ERROR_PARAMS( e.what() )
      )
    );
  }
  catch (...)
  {
    theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZSTR0020_LOADER_IO_ERROR )
    );
  }

  return -1;
}

/*******************************************************************************

********************************************************************************/
store::Item_t DtdXmlLoader::loadXml(
    const zstring& baseUri,
    const zstring& docUri,
    std::istream& stream)
{
  theTree = GET_STORE().getNodeFactory().createXmlTree();

  xmlSubstituteEntitiesDefault(1);

  theBaseUri = baseUri;

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

  try
  {
    stream.seekg(0, std::ios::end);
    std::streamoff fileSize = stream.tellg();
    stream.seekg(0, std::ios::beg);

    unsigned int fSize = static_cast<unsigned int>(fileSize);
    theBuffer.resize(fSize+1);
    theBuffer[fSize] = 0;

    std::streamsize numChars = readPacket(stream,
                                          static_cast<char*>(&theBuffer[0]),
                                          fileSize);

    if (numChars < 0)
    {
      theXQueryDiagnostics->
      add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0020_LOADER_IO_ERROR));

      abortload();
      return NULL;
    }
    else if (numChars == 0)
    {
      theXQueryDiagnostics->
      add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0020_LOADER_IO_ERROR,
                                    ERROR_PARAMS(ZED(NoInputData))));
      abortload();
      return NULL;
    }

    ctxt = xmlCreatePushParserCtxt(NULL, //&theSaxHandler,
                                   this,
                                   static_cast<char*>(&theBuffer[0]),
                                   static_cast<int>(numChars),
                                   doc_uri);

    if (ctxt == NULL)
    {
      theXQueryDiagnostics->add_error(
        NEW_ZORBA_EXCEPTION(
          zerr::ZSTR0021_LOADER_PARSING_ERROR,
          ERROR_PARAMS( ZED( XMLParserInitFailed ) )
        )
      );
      abortload();
      return NULL;
    }

    // Set the LibXml DTD validation options
    /*
    int options = XML_PARSE_COMPACT;
    options |= XML_PARSE_DTDVALID;
    options |= XML_PARSE_DTDLOAD;
    */
    //options |= XML_PARSE_SAX1;
    //xmlSAXDefaultVersion(1);

    // xmlCtxtUseOptions(ctxt, options);

    xmlLoadExtDtdDefaultValue |= XML_DETECT_IDS;
    xmlLoadExtDtdDefaultValue |= XML_COMPLETE_ATTRS;

    // Apply loader options
    applyLoadOptions(theLoadProperties, ctxt);

    if ( xmlParseDocument(ctxt)==-1 )
    {
      // std::cout << "  xmlParseDocument: Error: Unable to create tree: " << ctxt->lastError.message << std::endl;
      theXQueryDiagnostics->add_error(NEW_ZORBA_EXCEPTION(zerr::ZSTR0021_LOADER_PARSING_ERROR,ERROR_PARAMS( ZED( XMLParserNoCreateTree ) )));
      abortload();
      return NULL;
    }
  }
  catch(...)
  {
    abortload();
    thePathStack.clear();
    return NULL;
  }

  bool ok = ctxt->wellFormed != 0;

  // The doc may be well formed, but it may have other kinds of errors,
  // e.g., unresolved ns prefixes.
  if (!theXQueryDiagnostics->errors().empty())
  {
    abortload();
    return NULL;
  }
  else if (!ok )
  {
    if (!theDocUri.empty())
    {
      theXQueryDiagnostics->add_error(
        NEW_ZORBA_EXCEPTION(
          zerr::ZSTR0021_LOADER_PARSING_ERROR,
          ERROR_PARAMS( ZED( BadXMLDocument_2o ), theDocUri )
        )
      );
    }
    else
    {
      theXQueryDiagnostics->add_error(
        NEW_ZORBA_EXCEPTION(
          zerr::ZSTR0021_LOADER_PARSING_ERROR,
          ERROR_PARAMS( ZED( BadXMLDocument_2o ) )
        )
      );
    }

    abortload();
    return NULL;
  }
  else if ( ctxt->lastError.code != XML_ERR_OK )
  {
    /*
    std::cout << "-- LastErrorCode: " << ctxt->lastError.code << " " <<
        ctxt->lastError.message << " wellformed:" << ok << std::endl; std::cout.flush();
    */

    if ( ctxt->lastError.code == XML_NS_ERR_UNDEFINED_NAMESPACE ||
        ctxt->lastError.code != XML_ERR_NO_DTD )
    {
      theXQueryDiagnostics->add_error(
        NEW_ZORBA_EXCEPTION(
          zerr::ZSTR0021_LOADER_PARSING_ERROR,
          ERROR_PARAMS( ZED( BadXMLDocument_2o ) )
        )
      );
      abortload();
      return NULL;

    }
  }


  xmlDoc *doc = ctxt->myDoc;
  if (doc == NULL)
  {
    theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION(
        zerr::ZSTR0021_LOADER_PARSING_ERROR,
        ERROR_PARAMS( ZED( XMLParserNoCreateTree ) )
      )
    );
    abortload();
    return NULL;
  }

  loadDocument(doc);
  /* no need to free the document since is in ctxt */
  //xmlFreeDoc(doc);

  xmlCtxtReset(ctxt);
  xmlFreeParserCtxt(ctxt);
  ctxt = NULL;


  thePathStack.pop();
  assert(thePathStack.empty());

  XmlNode* resultNode;
  resultNode = theRootNode;
  reset();
  return resultNode;
}


void DtdXmlLoader::loadDocument(xmlDoc *doc)
{
  startDocument(this);

  /*Get the root element node */
  //xmlNode *root_element = xmlDocGetRootElement(doc);
  //print_element_names(root_element);
  loadNode(doc->children);

  endDocument(this);
}


void DtdXmlLoader::loadNode(xmlNode *a_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next)
  {
    switch( cur_node->type )
    {
      case XML_ELEMENT_NODE:
      {
        startElement(this, cur_node);
        loadNode(cur_node->children);
        endElement(this, cur_node);

        break;
      }
      case XML_TEXT_NODE:
      {
        xmlChar * content = xmlNodeGetContent(cur_node);
        characters(this, content, xmlStrlen(content));
        xmlFree(content);
        break;
      }
      case XML_CDATA_SECTION_NODE:
      {
        xmlChar * content = xmlNodeGetContent(cur_node);
        cdataBlock(this, content, xmlStrlen(content));
        xmlFree(content);
        break;
      }
      case XML_PI_NODE:
      {
        xmlChar * content = xmlNodeGetContent(cur_node);
        processingInstruction(this, cur_node->name, content);
        xmlFree(content);
        break;
      }
      case XML_COMMENT_NODE:
      {
        xmlChar * content = xmlNodeGetContent(cur_node);
        comment(this, content);
        xmlFree(content);
        break;
      }

      case XML_ATTRIBUTE_NODE:
      case XML_ENTITY_REF_NODE:
      case XML_ENTITY_NODE:
      case XML_DOCUMENT_NODE:
      case XML_DOCUMENT_TYPE_NODE:
      case XML_DOCUMENT_FRAG_NODE:
      case XML_NOTATION_NODE:
      case XML_HTML_DOCUMENT_NODE:
      case XML_DTD_NODE:
      case XML_ELEMENT_DECL:
      case XML_ATTRIBUTE_DECL:
      case XML_ENTITY_DECL:
      case XML_NAMESPACE_DECL:
      case XML_XINCLUDE_START:
      case XML_XINCLUDE_END:

#ifdef LIBXML_DOCB_ENABLED
      case XML_DOCB_DOCUMENT_NODE:
#endif //LIBXML_DOCB_ENABLED
        break;
      default:
        std::cout << "AssertError: Unknown libxml2 node type: " << (int)(cur_node->type)
          << std::endl; std::cout.flush();
        ZORBA_ASSERT(false);
    }
  }
}


/*******************************************************************************
  SAX2 callback when the start of document has been detected by the parser.

  ctx: the user data (XML parser context)
********************************************************************************/
void DtdXmlLoader::startDocument(void * ctx)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>(ctx));
  ZORBA_LOADER_CHECK_ERROR(loader);

  try
  {
    DocumentNode* docNode = GET_STORE().getNodeFactory().createDocumentNode();

    loader.setRoot(docNode);
    loader.theNodeStack.push(docNode);
    loader.theNodeStack.push(NULL);

#ifdef DATAGUIDE
    if (loader.theBuildDataGuide)
    {
      assert(loader.theGuideStack.empty());
      store::Item_t nodeName;
      ElementGuideNode* gnode = new ElementGuideNode(NULL, nodeName);
      loader.theGuideStack.push(gnode);
    }
#endif

    docNode->setBaseUri(loader.theBaseUri);
    docNode->setDocUri(loader.theDocUri);
    docNode->setId(loader.theTree, &loader.theOrdPath);
    loader.theOrdPath.pushChild();

    LOADER_TRACE1("Start Doc Node = " << docNode);
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************
  SAX2 callback when the end of document has been detected by the parser.

  ctx: the user data (XML parser context)
********************************************************************************/
void DtdXmlLoader::endDocument(void * ctx)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>(ctx));
  ZORBA_LOADER_CHECK_ERROR(loader);

  zorba::Stack<XmlNode*>& nodeStack = loader.theNodeStack;
  ulong stackSize = nodeStack.size();
  ulong firstChildPos;
  ulong numChildren;
  ulong numActualChildren;
  ulong i;
  DocumentNode* docNode;
  XmlNode* currChild;

  try
  {
    // This check is required because it is possible (in case of mal-formed doc)
    // that libXml calls endDocument() without having called startDocument().
    if (stackSize == 0)
      return;

    // Find the position of the 1st child of this doc node in the node stack
    firstChildPos = stackSize - 1;
    while (nodeStack[firstChildPos] != NULL)
      --firstChildPos;

    // Find the doc node in the stack
    docNode = dynamic_cast<DocumentNode*>(nodeStack[firstChildPos-1]);
    ZORBA_ASSERT(docNode != NULL);

    // For each child, make this doc node its parent.
    InternalNode::NodeVector& children = docNode->nodes();
    numChildren = nodeStack.size() - firstChildPos - 1;
    children.resize(numChildren);

    numActualChildren = 0;
    for (i = firstChildPos + 1; i < stackSize; ++i)
    {
      currChild = nodeStack[i];
      children[numActualChildren] = currChild;
      currChild->setParent(docNode);
      ++numActualChildren;
    }

    nodeStack.pop(numChildren+1);

#ifdef DATAGUIDE
    if (loader.theBuildDataGuide)
    {
      // Remove from the dataguide stack
      ElementGuideNode* rootGNode = loader.theGuideStack.top();
      loader.theGuideStack.pop();
      assert(loader.theGuideStack.empty());

      loader.theTree->setDataGuide(rootGNode);

#ifndef NDEBUG
      std::cout << rootGNode->show(0) << std::endl;
#endif
    }
#endif

    LOADER_TRACE2("End Doc Node = " << docNode);
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************
  SAX2 callback when an element start has been detected by the parser. It
  provides the expanded qname of the element, as well as the new namespace
  declarations on the element (if any).

  ctx:           the user data (XML parser context)
  localname:     the local name of the element
  prefix:        the element namespace prefix if available
  URI:           the element namespace name if available
  nb_namespaces: number of namespace definitions on that node
  namespaces:    pointer to the array of prefix/URI pairs namespace definitions
  nb_attributes: the number of attributes on that node
  nb_defaulted:  the number of defaulted attributes. The defaulted ones are at
                 the end of the array
 attributes:     pointer to the array of (localname/prefix/URI/value/end)
                 attribute values.
********************************************************************************/
void DtdXmlLoader::startElement(
    void * ctx,
    xmlNode *node)
{
  //std::cout << "StartElement, name: " << node->name << std::endl; std::cout.flush();

  const xmlChar * lname =  node->name;
  const xmlChar * prefix = node->ns != NULL ? node->ns->prefix : NULL;
  const xmlChar * uri =    node->ns != NULL ? node->ns->href   : NULL;

  int numAttrs = 0;
  for (xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next )
    numAttrs++;

  int numNamespaces = 0;
  for (xmlNs  *ns = node->nsDef; ns != NULL; ns = ns->next)
    numNamespaces++;

  Store& store = GET_STORE();
  NodeFactory& nfactory = store.getNodeFactory();
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>(ctx));
  ZORBA_LOADER_CHECK_ERROR(loader);
  QNamePool& qnpool = store.getQNamePool();
  zorba::Stack<XmlNode*>& nodeStack = loader.theNodeStack;
  zorba::Stack<PathStepInfo>& pathStack = loader.thePathStack;
  zstring baseUri;

  try
  {
    csize numAttributes = static_cast<csize>(numAttrs);
    csize numBindings = static_cast<csize>(numNamespaces);

    // Construct node name
    store::Item_t nodeName;
    qnpool.insert(nodeName,
                  reinterpret_cast<const char*>(uri),
                  reinterpret_cast<const char*>(prefix),
                  reinterpret_cast<const char*>(lname));

    // Create the element node and push it to the node stack
    ElementNode* elemNode = nfactory.createElementNode(nodeName,
                                                       numBindings,
                                                       numAttributes);
    if (nodeStack.empty())
      loader.setRoot(elemNode);

#ifdef DATAGUIDE
    nodeName = elemNode->getNodeName();

    // Push new node to dataguide, if not there already
    if (loader.theBuildDataGuide && loader.theNodeStack.empty())
    {
      assert(loader.theGuideStack.empty());
      ElementGuideNode* gnode = new ElementGuideNode(NULL, nodeName);
      loader.theGuideStack.push(gnode);
    }
    else if (loader.theBuildDataGuide)
    {
      bool duplicate = false;

      assert(!loader.theGuideStack.empty());

      // See if we have seen an element with the same tag name among the left
      // siblings of this element node.
      for (long i = loader.theNodeStack.size() - 1;
           loader.theNodeStack[i] != NULL;
           --i)
      {
        XmlNode* sib = loader.theNodeStack[i];
        if (sib->getNodeKind() == store::StoreConsts::elementNode &&
            sib->getNodeName()->equals(nodeName))
        {
          duplicate = true;
          break;
        }
      }

      ElementGuideNode* parentGNode = loader.theGuideStack.top();
      ElementGuideNode* childGNode = parentGNode->findChild(nodeName);

      if (duplicate)
      {
        assert(childGNode != NULL);
        childGNode->setUnique(false);
      }
      else
      {
        if (childGNode == NULL)
          childGNode = new ElementGuideNode(parentGNode, nodeName);
      }

      loader.theGuideStack.push(childGNode);
    }
#endif

    // Check for recursiveness, i.e., whether this node is a descendant of
    // another noide with the same name
    for (long j = pathStack.size() - 1; j >= 1; --j)
    {
      if (pathStack[j].theNode->theName->equals(elemNode->theName))
      {
        pathStack[j].theNode->setRecursive();
        break;
      }
    }

    // Assign the current node id to this node, and compute the next node id.
    elemNode->setId(loader.theTree, &loader.theOrdPath);
    loader.theOrdPath.pushChild();

    LOADER_TRACE1("Start Element: node = " << elemNode << " name = ["
                  << (prefix != NULL ? prefix : (xmlChar*)"") << ":" << lname
                  << " (" << (uri != NULL ? uri : (xmlChar*)"NULL") << ")]"
                  << std::endl << " ordpath = " << elemNode->getOrdPath().show()
                  << std::endl);

    // Process namespace bindings
    store::NsBindings& bindings = elemNode->getNsContext()->getBindings();
    int i = 0;
    for (xmlNs *ns = node->nsDef; ns != NULL; ns = ns->next)
    {
      const char* prefix = reinterpret_cast<const char*>(ns->prefix);
      const char* nsuri = reinterpret_cast<const char*>(ns->href);

      if (prefix == NULL)
        prefix = "";

      zstring pooledNs;
      store.getNamespacePool().insertc(nsuri, pooledNs);

      bindings[i].first = prefix;
      bindings[i].second = pooledNs;

      LOADER_TRACE1("namespace decl: [" << prefix  << ":" << nsuri << "]");
      i++;
    }

    if ( i > 0 )
    {
      loader.theBindingsStack.push(elemNode->getNsContext());
    }
    else if (pathStack.size() == 1)
    {
      elemNode->theNsContext = new NsBindingsContext;
      elemNode->theFlags |= XmlNode::HaveLocalBindings;

      loader.theBindingsStack.push(elemNode->theNsContext);
    }

    // Process attributes
    i = 0;
    InternalNode::NodeVector& attrNodes = elemNode->nodes();

    for (xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next )
    {
      //std::cout << "  att: " << attr->name << std::endl; std::cout.flush();

      const char* lname = reinterpret_cast<const char*>(attr->name);
      const char* prefix = reinterpret_cast<const char*>(attr->ns != NULL ? attr->ns->prefix : NULL);
      const char* uri = reinterpret_cast<const char*>(attr->ns != NULL ? attr->ns->href : NULL);
      store::Item_t qname;
      qnpool.insert(qname, uri, prefix, lname);

      AttributeNode* attrNode = nfactory.createAttributeNode(qname);

      xmlChar* val = xmlGetProp(node, attr->name);
      const char* valu = reinterpret_cast<const char*>(val);
      zstring value(valu);

      store::Item_t typeName;

      store::Item_t typedValue;
      switch (attr->atype)
      {
        case 0:                       // libxml2 bug
        case XML_ATTRIBUTE_CDATA:
          GET_STORE().getItemFactory()->createUntypedAtomic(typedValue, value);
          typeName = GET_STORE().theSchemaTypeNames[store::XS_UNTYPED_ATOMIC];
          break;
        case XML_ATTRIBUTE_ID:
          GET_STORE().getItemFactory()->createID(typedValue, value);
          typeName = GET_STORE().theSchemaTypeNames[store::XS_ID];
          break;
        case XML_ATTRIBUTE_IDREF:
          GET_STORE().getItemFactory()->createIDREF(typedValue, value);
          typeName = GET_STORE().theSchemaTypeNames[store::XS_IDREF];
          break;
        case XML_ATTRIBUTE_IDREFS:
          GET_STORE().getItemFactory()->createIDREFS(typedValue, value);
          GET_STORE().getItemFactory()->createQName(typeName, Store::XS_URI, "xs", "IDREFS");
          attrNode->setHaveListValue();
          break;
        case XML_ATTRIBUTE_ENTITY:
          GET_STORE().getItemFactory()->createENTITY(typedValue, value);
          typeName = GET_STORE().theSchemaTypeNames[store::XS_ENTITY];
          break;
        case XML_ATTRIBUTE_ENTITIES:
          GET_STORE().getItemFactory()->createENTITIES(typedValue, value);
          GET_STORE().getItemFactory()->createQName(typeName, Store::XS_URI, "xs", "ENTITIES");
          attrNode->setHaveListValue();
          break;
        case XML_ATTRIBUTE_NMTOKEN:
          GET_STORE().getItemFactory()->createNMTOKEN(typedValue, value);
          typeName = GET_STORE().theSchemaTypeNames[store::XS_NMTOKEN];
          break;
        case XML_ATTRIBUTE_NMTOKENS:
          GET_STORE().getItemFactory()->createNMTOKENS(typedValue, value);
          GET_STORE().getItemFactory()->createQName(typeName, Store::XS_URI, "xs", "NMTOKENS");
          attrNode->setHaveListValue();
          break;
        case XML_ATTRIBUTE_NOTATION:
          GET_STORE().getItemFactory()->createNOTATION(typedValue, value);
          GET_STORE().getItemFactory()->createQName(typeName, Store::XS_URI, "xs", "NOTATION");
          break;
        case XML_ATTRIBUTE_ENUMERATION:
          GET_STORE().getItemFactory()->createUntypedAtomic(typedValue, value);
          GET_STORE().getItemFactory()->createQName(typeName, Store::XS_URI, "xs", "anySimpleType");
          break;
        default:
          std::cout << "AssertError: unknown libxml2 attribute type: " <<
            (int)attr->atype << std::endl; std::cout.flush();
          ZORBA_ASSERT(false);
      }

      attrNode->theParent = elemNode;
      attrNode->setId(loader.theTree, &loader.theOrdPath);
      attrNode->theTypedValue.transfer(typedValue);
      attrNode->setType(typeName);

      attrNodes[i] = attrNode;

      if (attrNode->isBaseUri())
      {
        baseUri = pathStack.top().theBaseUri;
        zstring localUri;
        attrNode->theTypedValue->getStringValue2(localUri);
        if (baseUri == NULL)
        {
          elemNode->addBaseUriProperty(localUri, baseUri);
        }
        else
        {
          elemNode->addBaseUriProperty(baseUri, localUri);
        }

        elemNode->getBaseURI(baseUri);

        ++i;
      }

      loader.theOrdPath.nextChild();

      LOADER_TRACE2("Attribute: node = " << attrNode << " name ["
                    << (prefix != NULL ? prefix : "") << ":" << lname << " ("
                    << (uri != NULL ? uri : "NULL") << ")]" << " value = "
                    << attrNode->theTypedValue->getStringValue()
                    << std::endl << " ordpath = "
                    << attrNode->getOrdPath().show() << std::endl);


      xmlFree(val);
      i++;
    }

    nodeStack.push((XmlNode*)elemNode);
    nodeStack.push(NULL);
    pathStack.push(PathStepInfo(elemNode, baseUri));
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->
    add_error(NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR));
  }
}


/*******************************************************************************
  SAX2 callback when an element end has been detected by the parser. It
  provides the expanded qname of the element.

  ctx:       the user data (XML parser context)
  localname: the local name of the element
  prefix:    the element namespace prefix if available
  URI:       the element namespace name if available
********************************************************************************/
void  DtdXmlLoader::endElement(
    void * ctx,
    xmlNode *node)
{
#ifndef NDEBUG
  const xmlChar * lname =  node->name;
  const xmlChar * prefix = node->ns!=NULL ? node->ns->prefix : NULL;
  const xmlChar * uri =    node->ns!=NULL ? node->ns->href   : NULL;
#endif

  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>(ctx));
  ZORBA_LOADER_CHECK_ERROR(loader);

  zorba::Stack<PathStepInfo>& pathStack = loader.thePathStack;
  zorba::Stack<XmlNode*>& nodeStack = loader.theNodeStack;
  csize stackSize = nodeStack.size();
  csize firstChildPos;
  csize numChildren;
  csize numActualNodes;
  csize i;
  ElementNode* elemNode;
  XmlNode* prevChild = NULL;
  XmlNode* currChild;

  try
  {
    // Find the position of the 1st child of this element node in the node stack
    firstChildPos = stackSize - 1;
    while (nodeStack[firstChildPos] != NULL)
      --firstChildPos;

    // Find the element node in the stack
#ifndef NDEBUG
    elemNode = dynamic_cast<ElementNode*>(nodeStack[firstChildPos-1]);
    ZORBA_ASSERT(elemNode != NULL);
#else
    elemNode = static_cast<ElementNode*>(nodeStack[firstChildPos-1]);
#endif

    LOADER_TRACE2("End Element: node = " << elemNode << " name ["
                  << (prefix != NULL ? prefix : (xmlChar*)"") << ":" << lname
                  << " (" << (uri != NULL ? uri : (xmlChar*)"NULL") << ")]");

    // For each child, make this element node its parent and fix its namespace
    // bindings context.
    InternalNode::NodeVector& nodes = elemNode->nodes();

    numChildren = nodeStack.size() - firstChildPos - 1;

    numActualNodes = nodes.size();

    nodes.resize(numActualNodes + numChildren);

    for (i = firstChildPos + 1; i < stackSize; ++i)
    {
      currChild = nodeStack[i];

      if (currChild->getNodeKind() == store::StoreConsts::textNode &&
          prevChild != NULL &&
          prevChild->getNodeKind() == store::StoreConsts::textNode)
      {
        TextNode* textSibling = reinterpret_cast<TextNode*>(prevChild);
        TextNode* textChild = reinterpret_cast<TextNode*>(currChild);

        zstring content2;
        content2.reserve(textSibling->getText().size() + textChild->getText().size());
        content2 = textSibling->getText();
        content2 += textChild->getText();

        textSibling->setText(content2);
        delete currChild;
      }
      else
      {
        nodes[numActualNodes] = currChild;
        currChild->setParent(elemNode);

        if (currChild->getNodeKind() == store::StoreConsts::elementNode &&
            !loader.theBindingsStack.empty())
        {
          reinterpret_cast<ElementNode*>(currChild)->
          setNsContext(loader.theBindingsStack.top());
        }

        prevChild = currChild;
        ++numActualNodes;
      }
    }

    nodes.resize(numActualNodes);

    nodeStack.pop(numChildren+1);
    pathStack.pop();

    if (elemNode->getNsContext() != NULL)
    {
      loader.theBindingsStack.pop();
    }

    // Adjust the dewey id
    loader.theOrdPath.popChild();

#ifdef DATAGUIDE
    if (loader.theBuildDataGuide)
    {
      // Remove from the dataguide stack
      assert(loader.theGuideStack.top()->getName()->equals(elemNode->getNodeName()));
      loader.theGuideStack.pop();
    }
#endif
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************
  SAX2 callback when xml text has been detected by the parser.

  ctx: the user data (XML parser context)
  ch:  a xmlChar string
  len: the number of xmlChar
********************************************************************************/
void DtdXmlLoader::characters(void * ctx, const xmlChar * ch, int len)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
  ZORBA_LOADER_CHECK_ERROR(loader);

  try
  {
    const char* charp = reinterpret_cast<const char*>(ch);
    zstring content(charp, len);

    TextNode* textNode = GET_STORE().getNodeFactory().createTextNode(content);

    if (loader.theNodeStack.empty())
      loader.setRoot(textNode);

    loader.theNodeStack.push(textNode);

    textNode->setId(loader.theTree, &loader.theOrdPath);
    loader.theOrdPath.nextChild();

#ifndef NDEBUG
    std::stringstream lSs;
    lSs << "Text Node = " << textNode << " content = "
        << std::string(charp, len) << std::endl
#ifdef TEXT_ORDPATH
        << " ordpath = " << textNode->getOrdPath().show()
#endif
        << std::endl;
    LOADER_TRACE2(lSs.str());
#endif
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************
  SAX2 callback when a CDATA block has been detected by the parser.

  ctx: the user data (XML parser context)
  ch:  a xmlChar string
  len: the number of xmlChar
********************************************************************************/
void DtdXmlLoader::cdataBlock(void * ctx, const xmlChar * ch, int len)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
  ZORBA_LOADER_CHECK_ERROR(loader);

  try
  {
    // If a doc contains an element like <cdata><![CDATA[ <> ]]></cdata>,
    // libxml returns the string " <> ".
    const char* charp = reinterpret_cast<const char*>(ch);
    zstring content(charp, len);

    TextNode* cdataNode = GET_STORE().getNodeFactory().createTextNode(content);

    if (loader.theNodeStack.empty())
      loader.setRoot(cdataNode);

    loader.theNodeStack.push(cdataNode);

    cdataNode->setId(loader.theTree, &loader.theOrdPath);
    loader.theOrdPath.nextChild();

#ifndef NDEBUG
    std::stringstream lSs;
    lSs << "CDATA Node = " << cdataNode << " content = "
        << std::string(charp, len) << std::endl
#ifdef TEXT_ORDPATH
        << " ordpath = " << cdataNode->getOrdPath().show()
#endif
        << std::endl;
  LOADER_TRACE2(lSs.str());
#endif
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************
  SAX2 callback when a processing instruction has been detected by the parser.

  ctx: the user data (XML parser context)

********************************************************************************/
void DtdXmlLoader::processingInstruction(
    void * ctx,
    const xmlChar * targetp,
    const xmlChar * data)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
  ZORBA_LOADER_CHECK_ERROR(loader);

  try
  {
    // bugfix: handling PIs with no data (i.e. data being NULL)
    zstring content;
    if (data)
      content = reinterpret_cast<const char*>(data);

    zstring target = reinterpret_cast<const char*>(targetp);

    PiNode* piNode = GET_STORE().getNodeFactory().createPiNode(target, content);

    if (loader.theNodeStack.empty())
      loader.setRoot(piNode);

    loader.theNodeStack.push(piNode);

    piNode->setId(loader.theTree, &loader.theOrdPath);
    loader.theOrdPath.nextChild();

    LOADER_TRACE2("Pi Node = " << piNode << " target = "
                  << targetp << std::endl << " ordpath = "
                  << piNode->getOrdPath().show() << std::endl);
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************
  SAX2 callback when a comment has been detected by the parser.

  ctx: the user data (XML parser context)
  content:  the comment content
********************************************************************************/
void DtdXmlLoader::comment(void * ctx, const xmlChar * ch)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
  ZORBA_LOADER_CHECK_ERROR(loader);

  try
  {
    const char* charp = reinterpret_cast<const char*>(ch);
    zstring content(charp);

    CommentNode* commentNode = GET_STORE().getNodeFactory().createCommentNode(content);

    if (loader.theNodeStack.empty())
      loader.setRoot(commentNode);

    loader.theNodeStack.push(commentNode);

    commentNode->setId(loader.theTree, &loader.theOrdPath);
    loader.theOrdPath.nextChild();

    LOADER_TRACE2("Comment Node = " << commentNode << " content = "
                  << charp << std::endl << " ordpath = "
                  << commentNode->getOrdPath().show() << std::endl);
  }
  catch (ZorbaException const& e)
  {
    loader.theXQueryDiagnostics->add_error( e );
  }
  catch (...)
  {
    loader.theXQueryDiagnostics->add_error(
      NEW_ZORBA_EXCEPTION( zerr::ZXQP0003_INTERNAL_ERROR )
    );
  }
}


/*******************************************************************************

********************************************************************************/
xmlEntityPtr  DtdXmlLoader::getEntity(
    void * ctx,
    const xmlChar * name)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
//  ZORBA_LOADER_CHECK_ERROR(loader);
/*
  xmlEntityPtr  parsed_entity = NULL;
  loader.entity_hash.get((const char*)name, parsed_entity);
  return parsed_entity;
*/
  return xmlGetDocEntity(loader.ctxt->myDoc, name);
}


xmlEntityPtr DtdXmlLoader::getParameterEntity(
    void *ctx,
    const xmlChar *name)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
  return xmlGetParameterEntity(loader.ctxt->myDoc, name);
}


void DtdXmlLoader::entityDecl(
    void *ctx,
    const xmlChar *name,
    int   type,
    const xmlChar *publicId,
    const xmlChar *systemId,
    xmlChar *content)
{
  DtdXmlLoader& loader = *(static_cast<DtdXmlLoader *>( ctx ));
  ZORBA_LOADER_CHECK_ERROR(loader);
}

} // namespace simplestore
} // namespace zorba
/* vim:set et sw=2 ts=2: */
