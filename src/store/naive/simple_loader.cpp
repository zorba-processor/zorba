
#include <stdio.h>
#include <memory.h>
#include <string>
#include <libxml/xmlmemory.h>

#include "store/api/store_consts.h"
#include "store/naive/simple_store.h"
#include "store/naive/simple_loader.h"
#include "store/naive/simple_temp_seq.h"
#include "store/naive/node_items.h"

namespace xqp
{

#ifndef NDEBUG
#define LOADER_TRACE(msg)                                                     \
{                                                                             \
  std::cout << __FUNCTION__ << ": " << std::endl << "  " << msg << std::endl; \
}
#else
#define LOADER_TRACE(msg)
#endif


XmlLoader::XmlLoader()
{
  // This initializes the library and check potential ABI mismatches between
  // the version it was compiled for and the actual shared library used.
  LIBXML_TEST_VERSION

  // See http://xmlsoft.org/html/libxml-tree.html#xmlSAXHandler
  memset(&theSaxHandler, 0, sizeof(theSaxHandler) );
  theSaxHandler.initialized = XML_SAX2_MAGIC;
  theSaxHandler.startDocument = &XmlLoader::startDocument;
  theSaxHandler.endDocument = &XmlLoader::endDocument;
  theSaxHandler.startElementNs = &XmlLoader::startElement;
  theSaxHandler.endElementNs = &XmlLoader::endElement;
  theSaxHandler.characters = &XmlLoader::characters;
  theSaxHandler.cdataBlock = &XmlLoader::cdataBlock;
  theSaxHandler.comment = &XmlLoader::comment;
  theSaxHandler.processingInstruction = &XmlLoader::processingInstruction;
  theSaxHandler.warning = &XmlLoader::warning;
  theSaxHandler.error = &XmlLoader::error;
}


XmlLoader::~XmlLoader()
{
}


/*******************************************************************************

********************************************************************************/
long XmlLoader::readPacket(std::iostream& stream, char* buf, long size)
{
  try
  {
    stream.read(buf, size);

    if (stream.bad())
    {
      ZORBA_ERROR_ALERT(error_messages::XQP0016_LOADER_IO_ERROR,
                        error_messages::SYSTEM_ERROR,
                        NULL,
                        true,
                        "");
    }

    return stream.gcount();
  }
  catch (iostream::failure e)
  {
    ZORBA_ERROR_ALERT(error_messages::XQP0016_LOADER_IO_ERROR,
                      error_messages::SYSTEM_ERROR,
                      NULL,
                      true,
                      e.what());
  }
  return -1;
}


/*******************************************************************************

********************************************************************************/
Item_t XmlLoader::loadXml(std::iostream& stream)
{
  xmlParserCtxtPtr ctxt;
  char buf[4096];
  long numChars;

  numChars = readPacket(stream, buf, 4096);
  if (numChars < 0)
  {
    return NULL;
  }

  ctxt = xmlCreatePushParserCtxt(&theSaxHandler, this, buf, numChars, NULL);
  if (ctxt == NULL)
  {
    return NULL;
  }

  while ((numChars = readPacket(stream, buf, 4096)) > 0)
  {
    xmlParseChunk(ctxt, buf, numChars, 0);
  }

  xmlParseChunk(ctxt, buf, 0, 1);

  bool ok = ctxt->wellFormed;

  // Cleanup
  xmlFreeParserCtxt(ctxt);

  while(!thePath.empty())
    thePath.pop();

  Item_t resultNode = theRootNode;

  theRootNode = NULL;

  if (!ok)
  {
    ZORBA_ERROR_ALERT(error_messages::XQP0017_LOADER_NOT_WELL_FORMED_XML,
                      error_messages::USER_ERROR,
                      NULL,
                      true,
                      theErrors.c_str());
    theErrors.clear();
    return NULL;
  }

  std::cout << std::endl << resultNode->show() << std::endl;

  return resultNode;
}


/*******************************************************************************
  SAX2 callback when the start of document has been detected by the parser.

  ctx: the user data (XML parser context)
********************************************************************************/
void XmlLoader::startDocument(void * ctx)
{
  SimpleStore& store = *(static_cast<SimpleStore*>(&Store::getInstance()));
  BasicItemFactory& factory = *(static_cast<BasicItemFactory*>(&store.getItemFactory()));
  XmlLoader& loader = *(static_cast<XmlLoader *>(ctx));

  LOADER_TRACE("");

  xqpStringStore_t baseUri(new xqpStringStore(""));
  xqpStringStore_t docUri(new xqpStringStore("boo"));

  Item_t docNode = factory.createDocumentNode(baseUri, docUri);

  loader.theRootNode = docNode;
  loader.thePath.push(docNode);
  loader.thePath.push(NULL);
}


/*******************************************************************************
  SAX2 callback when the end of document has been detected by the parser.

  ctx: the user data (XML parser context)
********************************************************************************/
void XmlLoader::endDocument(void * ctx)
{
  XmlLoader& loader = *(static_cast<XmlLoader *>(ctx));

  LOADER_TRACE("");

  std::vector<Item_t> childNodes;
  std::vector<Item_t> revChildNodes;

  Item_t childNode = loader.thePath.top();

  while (childNode != NULL)
  {
    revChildNodes.push_back(childNode);
    loader.thePath.pop();
    childNode = loader.thePath.top();
  }
  loader.thePath.pop();

  childNodes.resize(revChildNodes.size());
  std::vector<Item_t>::const_reverse_iterator it;
  unsigned long i = 0;
  for (it = revChildNodes.rbegin(); it != (std::vector<Item_t>::const_reverse_iterator)revChildNodes.rend(); it++, i++)
    childNodes[i] = *it;

  TempSeq_t childSeq(new SimpleTempSeq(childNodes));

  NodeNaive* docNode = reinterpret_cast<NodeNaive*>(loader.thePath.top().get_ptr());

  docNode->setChildren(childSeq);
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
void XmlLoader::startElement(
    void * ctx, 
    const xmlChar * localName, 
    const xmlChar * prefix, 
    const xmlChar * uri,
    int numNamespaces, 
    const xmlChar ** namespaces, 
    int numAttributes, 
    int numDefaulted, 
    const xmlChar ** attributes)
{
  SimpleStore& store = *(static_cast<SimpleStore*>(&Store::getInstance()));
  BasicItemFactory& factory = *(static_cast<BasicItemFactory*>(&store.getItemFactory()));
  XmlLoader& loader = *(static_cast<XmlLoader *>(ctx));

  QNameItem_t qname;
  QNameItem_t tname;
  Item_t elemNode;
  std::vector<Item_t> attrNodes;
  NamespaceBindings nsBindings;

  LOADER_TRACE("Element name ["
               << (prefix != NULL ? prefix : (xmlChar*)"") << ":" << localName
               << " (" << (uri != NULL ? uri : (xmlChar*)"NULL") << ")]");

  qname = factory.createQName(reinterpret_cast<const char*>(uri),
                              reinterpret_cast<const char*>(prefix),
                              reinterpret_cast<const char*>(localName));

  tname = store.theAnyType;

  for (long i = 0; i < numNamespaces; ++i)
  {
    const char* prefix = reinterpret_cast<const char*>(namespaces[i * 2]);
    const char* uri = reinterpret_cast<const char*>(namespaces[i * 2 + 1]);

    xqpStringStore_t pooledUri;
    store.getUriPool().insert(uri, pooledUri);

    nsBindings.push_back(std::pair<xqp_string, xqp_string>(prefix, pooledUri));

    LOADER_TRACE("namespace decl: [" << (prefix != NULL ? prefix : "")
                 << ":" << uri << "]");
  }

  xqp_ulong index = 0;
  for (long i = 0; i < numAttributes; ++i, index += 5)
  {
    const char* localName = reinterpret_cast<const char*>(attributes[index]);
    const char* prefix = reinterpret_cast<const char*>(attributes[index+1]);
    const char* uri = reinterpret_cast<const char*>(attributes[index+2]);
    const char* valueBegin = reinterpret_cast<const char*>(attributes[index+3]);
    const char* valueEnd = reinterpret_cast<const char*>(attributes[index+4]);

    QNameItem_t qname = factory.createQName(uri, prefix, localName);
    QNameItem_t tname = store.theUntypedAtomicType;

    xqpStringStore_t value(new xqpStringStore(valueBegin, valueEnd));
    Item_t lexicalValue = factory.createString(value);
    Item_t typedValue = factory.createUntypedAtomic(value);

    Item_t attrNode = factory.createAttributeNode(qname, tname,
                                                  lexicalValue,
                                                  typedValue);

    attrNodes.push_back(attrNode);

    LOADER_TRACE("Attribute name [" << (prefix != NULL ? prefix : "")
                 << ":" << localName << " (" << (uri != NULL ? uri : "NULL")
                 << "]" << std::endl << "  Attribute value: " << value);
  }

  TempSeq_t attrSeq(new SimpleTempSeq(attrNodes));

  elemNode = factory.createElementNode(qname, tname, attrSeq, nsBindings);

  for (long i = 0; i < numAttributes; ++i)
  {
    reinterpret_cast<NodeNaive*>(attrNodes[i].get_ptr())->setParent(elemNode);
  }

  if (loader.thePath.empty())
    loader.theRootNode = elemNode;

  loader.thePath.push(elemNode);
  loader.thePath.push(NULL);
}

  
/*******************************************************************************
  SAX2 callback when an element end has been detected by the parser. It
  provides the expanded qname of the element.

  ctx:       the user data (XML parser context)
  localname: the local name of the element
  prefix:    the element namespace prefix if available
  URI:       the element namespace name if available
********************************************************************************/
void  XmlLoader::endElement(
    void * ctx, 
    const xmlChar * localName, 
    const xmlChar * prefix, 
    const xmlChar * uri)
{
  XmlLoader& loader = *(static_cast<XmlLoader *>(ctx));

  LOADER_TRACE("Element name ["
               << (prefix != NULL ? prefix : (xmlChar*)"") << ":" << localName
               << " (" << (uri != NULL ? uri : (xmlChar*)"NULL") << ")]");

  std::vector<Item_t> childNodes;
  std::vector<Item_t> revChildNodes;

  Item_t childNode = loader.thePath.top();

  while (childNode != NULL)
  {
    revChildNodes.push_back(childNode);
    loader.thePath.pop();
    childNode = loader.thePath.top();
  }
  loader.thePath.pop();

  childNodes.resize(revChildNodes.size());
  std::vector<Item_t>::const_reverse_iterator it;
  unsigned long i = 0;
  for (it = revChildNodes.rbegin(); it != (std::vector<Item_t>::const_reverse_iterator)revChildNodes.rend(); it++, i++)
    childNodes[i] = *it;

  TempSeq_t childSeq(new SimpleTempSeq(childNodes));

  NodeNaive* elemNode = reinterpret_cast<NodeNaive*>(loader.thePath.top().get_ptr());

  elemNode->setChildren(childSeq);
}


/*******************************************************************************
  SAX2 callback when xml text has been detected by the parser.

  ctx: the user data (XML parser context)
  ch:  a xmlChar string
  len: the number of xmlChar
********************************************************************************/
void XmlLoader::characters(void * ctx, const xmlChar * ch, int len)
{
  SimpleStore& store = *(static_cast<SimpleStore*>(&Store::getInstance()));
  BasicItemFactory& factory = *(static_cast<BasicItemFactory*>(&store.getItemFactory()));
  XmlLoader& loader = *(static_cast<XmlLoader *>( ctx ));

  xqpStringStore_t content(new xqpStringStore(reinterpret_cast<const char*>(ch), len));

  Item_t textNode = factory.createTextNode(content);

  if (loader.thePath.empty())
    loader.theRootNode = textNode;

  loader.thePath.push(textNode);
}


/*******************************************************************************
  SAX2 callback when a CDATA block has been detected by the parser.

  ctx: the user data (XML parser context)
  ch:  a xmlChar string
  len: the number of xmlChar
********************************************************************************/
void XmlLoader::cdataBlock(void * ctx, const xmlChar * ch, int len)
{
  SimpleStore& store = *(static_cast<SimpleStore*>(&Store::getInstance()));
  BasicItemFactory& factory = *(static_cast<BasicItemFactory*>(&store.getItemFactory()));
  XmlLoader& loader = *(static_cast<XmlLoader *>( ctx ));

  xqpStringStore_t content(new xqpStringStore(reinterpret_cast<const char*>(ch), len));

  Item_t textNode = factory.createTextNode(content);

  if (loader.thePath.empty())
    loader.theRootNode = textNode;

  loader.thePath.push(textNode);
}


/*******************************************************************************
  SAX2 callback when a comment has been detected by the parser.

  ctx: the user data (XML parser context)
  content:  the comment content
********************************************************************************/
void XmlLoader::comment(
    void * ctx,
    const xmlChar * content)
{
  SimpleStore& store = *(static_cast<SimpleStore*>(&Store::getInstance()));
  BasicItemFactory& factory = *(static_cast<BasicItemFactory*>(&store.getItemFactory()));
  XmlLoader& loader = *(static_cast<XmlLoader *>( ctx ));

  LOADER_TRACE(content);

  xqpStringStore_t contentp(new xqpStringStore(reinterpret_cast<const char*>(content)));

  Item_t commentNode = factory.createCommentNode(contentp);

  if (loader.thePath.empty())
    loader.theRootNode = commentNode;

  loader.thePath.push(commentNode);
}


/*******************************************************************************
  SAX2 callback when a processing instruction has been detected by the parser.

  ctx: the user data (XML parser context)

********************************************************************************/
void XmlLoader::processingInstruction(
    void * ctx, 
    const xmlChar * target, 
    const xmlChar * data)
{
  SimpleStore& store = *(static_cast<SimpleStore*>(&Store::getInstance()));
  BasicItemFactory& factory = *(static_cast<BasicItemFactory*>(&store.getItemFactory()));
  XmlLoader& loader = *(static_cast<XmlLoader *>( ctx ));

  LOADER_TRACE("target : " << target << " data: " << data);

  xqpStringStore_t datap(new xqpStringStore(reinterpret_cast<const char*>(data)));
  xqpStringStore_t targetp(new xqpStringStore(reinterpret_cast<const char*>(target)));

  Item_t piNode = factory.createPiNode(targetp, datap);

  if (loader.thePath.empty())
    loader.theRootNode = piNode;

  loader.thePath.push(piNode);
}


/*******************************************************************************
  Display and format an error messages, callback.
 
   ctx:  an XML parser context
   msg:  the message to display/transmit
   ...:  extra parameters for the message display
********************************************************************************/
void  XmlLoader::error(void * ctx, const char * msg, ... )
{
  XmlLoader& loader = *(static_cast<XmlLoader *>( ctx ));
  char buf[1024];
  va_list args;
  va_start(args, msg);
  vsprintf(buf, msg, args);
  va_end(args);
  loader.theErrors += "+ ";
  loader.theErrors += buf;
  loader.theErrors += "\n";
}


/*******************************************************************************
   Display and format a warning messages, callback.
  
   ctx:  an XML parser context
   msg:  the message to display/transmit
   ...:  extra parameters for the message display
********************************************************************************/
void  XmlLoader::warning(void * ctx, const char * msg, ... )
{
  XmlLoader& loader = *(static_cast<XmlLoader *>( ctx ));
  va_list args;
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);
}

}


