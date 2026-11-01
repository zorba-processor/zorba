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
#include <vector>
#include <iostream>
#include <sstream>

#include <zorba/util/error_util.h>

#include "api/unmarshaller.h"
#include "zorbamisc/ns_consts.h"
#include "context/static_context.h"

#include "http_util.h"


namespace zorba {
  
  HttpStream::HttpStream(const zstring& aUri, const SecurityPolicy* aPolicy)
    : theUri(aUri),
      thePolicy(aPolicy ? new SecurityPolicy(*aPolicy) : nullptr)
  {
  }
  
  HttpStream::~HttpStream()
  {
    if (theIterator)
    {
      theIterator->close();
    }
  }
  
  void HttpStream::init()
  {
    bool succeed = false;

#ifdef ZORBA_HAVE_CURL
    Zorba* lInstance = Zorba::getInstance(0);
    theStaticContext = lInstance->createStaticContext();
    if (thePolicy)
    {
      // The inner query runs fixed engine code, so loading the native
      // http-client library is allowed even where the caller's policy does
      // not allow NativeModules; everything else is the caller's policy, so
      // the http-client module checks the URL, every redirect and every
      // connection against it.
      theStaticContext->setSecurityPolicy(
          SecurityPolicy(*thePolicy).allow(Capability::NativeModules));
    }
    ItemFactory* lFactory = lInstance->getItemFactory();
    Item lNodeName = lFactory->createQName("http://expath.org/ns/http-client", "http", "request");
    Item lEmptyItem;

    NsBindings nsPairs;
    nsPairs.push_back(std::make_pair(String(XML_SCHEMA_PREFIX),
                                     String(static_context::W3C_XML_SCHEMA_NS)));

    Item lRequestElement =
    lFactory->createElementNode(lEmptyItem, lNodeName,
                                lFactory->createQName(static_context::W3C_XML_SCHEMA_NS,
                                                      "", "untyped"),
                                true, false, nsPairs);

    lFactory->createAttributeNode(lRequestElement,
                                  lFactory->createQName("", "method"),
                                  Item(),
                                  lFactory->createString("GET"));

    lFactory->createAttributeNode(lRequestElement, lFactory->createQName("", "href"), Item(), lFactory->createString(theUri.c_str()));

    lFactory->createAttributeNode(lRequestElement, lFactory->createQName("", "override-media-type"), Item(), lFactory->createString("text/plain"));

    lFactory->createAttributeNode(lRequestElement, lFactory->createQName("", "follow-redirect"), Item(), lFactory->createString("true"));

    // A query, not StaticContext::invoke(): an invoke query does not
    // initialize the global variables of the modules it calls into, and the
    // http-client wrapper reads them (e.g. $error:HC001) when it reports an
    // error, such as a redirect denied by the security policy.
    Zorba_CompilerHints_t lHints;
    lHints.opt_level = ZORBA_OPT_LEVEL_O1;
    theQuery = lInstance->createQuery();
    theQuery->compile(
        "import module namespace httpc = "
        "\"http://www.zorba-xquery.com/modules/http-client\";\n"
        "declare variable $request external;\n"
        "httpc:send-request($request, (), ())",
        theStaticContext, lHints);
    theQuery->getDynamicContext()->setVariable("request", lRequestElement);
    theIterator = theQuery->iterator();
    theIterator->open();
    Item lItem;
    theIterator->next(lItem);
    Iterator_t lIter = lItem.getAttributes();
    lIter->open();
    Item lAttr;
    succeed = true;
    while (lIter->next(lAttr)) {
      Item lName;
      lAttr.getNodeName(lName);
      String lNameString = lName.getStringValue();
      if (lNameString == "status") {
        std::stringstream lStream(lAttr.getStringValue().c_str());
        int status;
        lStream >> status;
        succeed = status < 400;
        break;
      }
    }
    lIter->close();
#endif /* ZORBA_HAVE_CURL */

    if (!succeed)
      throw os_error::exception("", theUri.c_str(), "Could not create stream resource");
    theIterator->next(theStreamableString);
  }
  
  std::istream& HttpStream::getStream()
  {
    return theStreamableString.getStream();
  }

  StreamReleaser HttpStream::getStreamReleaser()
  {
    store::Item* lItem = Unmarshaller::getInternalItem(theStreamableString);
    return lItem->getStreamReleaser();
  }

  void HttpStream::setStreamReleaser(StreamReleaser aReleaser)
  {
    store::Item* lItem = Unmarshaller::getInternalItem(theStreamableString);
    lItem->setStreamReleaser(aReleaser);
  }
}
