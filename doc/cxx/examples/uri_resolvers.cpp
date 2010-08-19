/*
 * Copyright 2006-2008 The FLWOR Foundation.
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

#include <zorba/zorba.h>
#include <zorba/store_manager.h>

#include <zorba/uri_resolvers.h>

using namespace zorba;

/** Document Resolver */
class MyDocumentURIResolverResult : public DocumentURIResolverResult
{
  public:
    virtual Item
    getDocument() const
    {
      return theDocument;
    }

  protected:
    friend class MyDocumentURIResolver;
    Item theDocument;
};


class MyDocumentURIResolver : public  DocumentURIResolver
{
public:
  virtual ~MyDocumentURIResolver() {}

  virtual std::auto_ptr<DocumentURIResolverResult>
  resolve(
        const Item& aURI,
        StaticContext* aStaticContext,
        XmlDataManager* aXmlDataManager,
        bool validate,
        bool tidying,
        bool replaceDoc,
        const Item& tidyUserOpt)
    {
      std::auto_ptr<MyDocumentURIResolverResult> lResult(new MyDocumentURIResolverResult());
      if (aURI.getStringValue() == "mydoc.xml") 
      {
        // we have only one document
        lResult->theDocument = aXmlDataManager->getDocument(aURI.getStringValue());
        lResult->setError(URIResolverResult::UR_NOERROR);
      }
      else
      {
        lResult->setError(URIResolverResult::UR_FODC0002);
        std::stringstream lErrorStream;
        lErrorStream << "Document could not be found " << aURI.getStringValue();
        lResult->setErrorDescription(lErrorStream.str());
      }
      return std::auto_ptr<DocumentURIResolverResult>(lResult.release()); 
    }
};


bool 
resolver_example_1(Zorba* aZorba)
{
  // load one document into the store
  {
    std::stringstream lDocStream;
    lDocStream << "<mydoc><a>1</a></mydoc>";

    XmlDataManager* lDataManager = aZorba->getXmlDataManager();

    lDataManager->loadDocument("mydoc.xml", lDocStream);
  }

  StaticContext_t lContext = aZorba->createStaticContext();

  MyDocumentURIResolver lResolver;

  lContext->setDocumentURIResolver(&lResolver);

  try 
  {
    XQuery_t lQuery = aZorba->compileQuery("fn:doc('mydoc.xml')", lContext); 
    std::cout << lQuery << std::endl;
  }
  catch (ZorbaException& e)
  {
    std::cerr << e.getDescription() << std::endl;
  }


	return true;
}

/** Collection Resolver */
class MyCollectionURIResolverResult : public CollectionURIResolverResult
{
  public:
    virtual Collection_t
    getCollection() const
    {
      return theCollection;
    }

  protected:
    friend class MyCollectionURIResolver;
    Collection_t theCollection;
};

class MyCollectionURIResolver : public  CollectionURIResolver
{
  public:
    virtual ~MyCollectionURIResolver() {}

    virtual std::auto_ptr<CollectionURIResolverResult>
    resolve(const Item& aURI, StaticContext* aStaticContext, XmlDataManager* aXmlDataManager)
    {
      std::auto_ptr<MyCollectionURIResolverResult> lResult(new MyCollectionURIResolverResult());
      if (aURI.getStringValue() == "mycollection.xml") {
        // we have only one document
        lResult->theCollection = aXmlDataManager->getCollection(aURI.getStringValue());
        lResult->setError(URIResolverResult::UR_NOERROR);
      } else {
        lResult->setError(URIResolverResult::UR_FODC0002);
        std::stringstream lErrorStream;
        lErrorStream << "Collection could not be found " << aURI.getStringValue();
        lResult->setErrorDescription(lErrorStream.str());
      }
      return std::auto_ptr<CollectionURIResolverResult>(lResult.release()); 
    }
};

bool 
resolver_example_2(Zorba* aZorba)
{
  // load one document into the store
  {
    XmlDataManager* lDataManager = aZorba->getXmlDataManager();

    Collection_t lCol = lDataManager->createCollection("mycollection.xml");

    std::stringstream lDoc;
    lDoc << "<mydoc><b>1</b></mydoc>";
    lCol->addDocument(lDoc);
  }

  StaticContext_t lContext = aZorba->createStaticContext();

  MyCollectionURIResolver lResolver;

  lContext->setCollectionURIResolver(&lResolver);

  try {
    XQuery_t lQuery = aZorba->compileQuery("fn:collection('mycollection.xml')", lContext); 
    std::cout << lQuery << std::endl;
  } catch (ZorbaException& e) {
    std::cerr << e.getDescription() << std::endl;
  }

	return true;
}


class MyModuleURIResolverResult : public ModuleURIResolverResult
{
  friend class MyModuleURIResolver;

protected:
  std::istream             * theModule;
  std::vector<std::string>   theComponentURIs;

public:
  virtual std::istream* getModuleStream() const 
  {
    return theModule;
  }

  void getModuleURL(std::string& url) const
  {
  }

  void getComponentURIs(std::vector<std::string>& uris) const 
  {
    uris = theComponentURIs;
  }
};


class MyModuleURIResolver : public ModuleURIResolver
{
public:
  virtual ~MyModuleURIResolver() {}

  std::auto_ptr<ModuleURIResolverResult> resolveTargetNamespace(
        const String& aTargetNamespaceURI,
        const StaticContext& aStaticContext)
  {
    std::auto_ptr<MyModuleURIResolverResult>
    lResult(new MyModuleURIResolverResult());

    std::string compURI = aTargetNamespaceURI.c_str();
    lResult->theComponentURIs.push_back(compURI);

    return std::auto_ptr<ModuleURIResolverResult>(lResult.release());
  }

  std::auto_ptr<ModuleURIResolverResult> resolve(
        const String& aURI,
        const StaticContext& aStaticContext)
  {
    std::auto_ptr<MyModuleURIResolverResult> lResult(new MyModuleURIResolverResult());

    if (aURI == "http://www.zorba-xquery.com/mymodule") 
    {
      // we have only one module
      lResult->theModule = new std::istringstream("module namespace lm = 'http://www.zorba-xquery.com/mymodule'; declare function lm:foo() { 'foo' };");
      lResult->setError(URIResolverResult::UR_NOERROR);
    } 
    else
    {
      lResult->setError(URIResolverResult::UR_XQST0046);
      std::stringstream lErrorStream;
      lErrorStream << "Module not found " << aURI;
      lResult->setErrorDescription(lErrorStream.str());
    }

    return std::auto_ptr<ModuleURIResolverResult>(lResult.release());
  }
};


bool 
resolver_example_3(Zorba* aZorba)
{
  StaticContext_t lContext = aZorba->createStaticContext();

  MyModuleURIResolver lResolver;

  lContext->addModuleURIResolver(&lResolver);

  try {
    XQuery_t lQuery = aZorba->compileQuery("import module namespace lm='http://www.zorba-xquery.com/mymodule'; lm:foo()", lContext); 
    std::cout << lQuery << std::endl;
  } catch (ZorbaException& e) {
    std::cerr << e.getDescription() << std::endl;
    return false;
  }

	return true;
}

int 
uri_resolvers(int argc, char* argv[])
{
  void* lStore = zorba::StoreManager::getStore();
  Zorba* lZorba = Zorba::getInstance(lStore);
  bool res = false;

  std::cout << std::endl  << "executing uri resolver example test 1" << std::endl;
  res = resolver_example_1(lZorba);
  if (!res) return 1; 
  std::cout << std::endl;

  std::cout << std::endl  << "executing uri resolver example test 2" << std::endl;
  res = resolver_example_2(lZorba);
  if (!res) return 2; 
  std::cout << std::endl;

  std::cout << std::endl  << "executing uri resolver example test 3" << std::endl;
  res = resolver_example_3(lZorba);
  if (!res) return 3; 
  std::cout << std::endl;

  lZorba->shutdown();
  zorba::StoreManager::shutdownStore(lStore);
  return 0;
}
