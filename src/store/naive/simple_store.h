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
#ifndef ZORBA_SIMPLE_STORE
#define ZORBA_SIMPLE_STORE

#include "zorbautils/mutex.h"
#include "zorbautils/lock.h"
#include "zorbautils/hashmap_itemp.h"

#include "common/shared_types.h"

#include "store/api/store.h"
#include "store/api/collection.h"
#include "store/api/index.h"

#include "store/util/hashmap_stringp.h"

#include "store/naive/shared_types.h"
#include "store/naive/node_items.h"

#include "store/naive/store_config.h"

namespace zorba { 

class GlobalEnvironment;

namespace error 
{
  class ErrorManager;
}

namespace store
{
  class IteratorFactory;
  class ValueIndexSpecification;
}

namespace simplestore 
{

class SimpleStoreManager;
class StringPool;
class QNamePool;
class XmlLoader;
class FastXmlLoader;
class Index;

typedef rchandle<XmlNode> XmlNode_t;

typedef store::StringHashMap<XmlNode_t> DocumentSet;
typedef store::StringHashMap<store::Collection_t> CollectionSet;
typedef ItemPointerHashMap<store::Index_t> IndexSet;



/*******************************************************************************
  theSchemaTypeNames : Maps each enum value from SchemaTypeNames (see 
                       store_defs.h) to its associated QName item.
********************************************************************************/
class SimpleStore : public store::Store
{
  friend class zorba::GlobalEnvironment;
  friend class simplestore::SimpleStoreManager;
 
public:
  static const char* XS_URI;
  static const char* XML_URI;
  static const char* ZXSE_URI;

  static const ulong XML_URI_LEN;

protected:
  static const ulong DEFAULT_COLLECTION_MAP_SIZE;
  static const ulong NAMESPACE_POOL_SIZE;

public:
  xqpStringStore_t              theEmptyNs;
  xqpStringStore_t              theXmlSchemaNs;

  checked_vector<store::Item_t> theSchemaTypeNames;

protected:
  bool                          theIsInitialized;

  ulong                         theUriCounter;
  SYNC_CODE(Mutex               theUriCounterMutex;)

  ulong                         theTreeCounter;
  SYNC_CODE(Mutex               theTreeCounterMutex;)

  StringPool                  * theNamespacePool;
  QNamePool                   * theQNamePool;

  store::ItemFactory          * theItemFactory;
  store::IteratorFactory      * theIteratorFactory;

  DocumentSet                   theDocuments;
  CollectionSet                 theCollections;
  IndexSet                      theIndices;

  checked_vector<store::Item_t> theItemUris;

  SYNC_CODE(Lock                theGlobalLock;)

  long                          theTraceLevel;

private:
  SimpleStore();

  virtual ~SimpleStore();

  void init();
  void initTypeNames();

public:
  void shutdown();

  store::ItemFactory* getItemFactory() const { return theItemFactory; }

  store::IteratorFactory* getIteratorFactory() const { return theIteratorFactory; }

  StringPool& getNamespacePool() const    { return *theNamespacePool; }

  QNamePool& getQNamePool() const         { return *theQNamePool; }

  SYNC_CODE(Lock& getGlobalLock()         { return theGlobalLock; })

  long getTraceLevel() const              { return theTraceLevel; }

  XmlLoader* getXmlLoader(error::ErrorManager*);

  ulong getTreeId();

  store::Item_t createUri();

  store::Index_t createIndex(
        const store::Item_t& qname,
        const store::IndexSpecification& spec);

  store::Index* getIndex(const store::Item_t& qname);

  void deleteIndex(const store::Item_t& qname);

  store::Collection_t createCollection(const xqpStringStore_t& uri);

  store::Collection_t createCollection();

  store::Collection_t getCollection(const xqpStringStore_t& uri);

  void deleteCollection(const xqpStringStore_t& uri);

  store::Iterator_t listCollectionUris();

  store::Item_t loadDocument(
        const xqpStringStore_t& uri,
        std::istream& stream,
        bool storeDocument);

  store::Item_t loadDocument(
        const xqpStringStore_t& uri,
        std::istream* stream,
        bool storeDocument);

  void addNode(const xqpStringStore_t& uri, const store::Item_t& node);

  store::Item_t getDocument(const xqpStringStore_t& uri);

  void deleteDocument(const xqpStringStore_t& uri);

  void deleteAllDocuments();

  short compareNodes(store::Item* node1, store::Item* node2) const;

  store::Iterator_t sortNodes(
        store::Iterator* iterator,
        bool             ascendent,
        bool             duplicateElemination,
        bool             aAllowAtomics = false);

  store::Iterator_t distinctNodes(store::Iterator*, bool aAllowAtomics = false);

  bool getPathInfo(
    const store::Item*               docUri,
    std::vector<const store::Item*>& contextPath,
    std::vector<const store::Item*>& relativePath,
    bool                             isAttrPath,
    bool&                            found,
    bool&                            unique);

  bool getReference(store::Item_t& result, const store::Item* node);
  bool getNodeByReference(store::Item_t& result, const store::Item* uri);

  store::TempSeq_t createTempSeq();

  store::TempSeq_t createTempSeq(
        store::Iterator* iterator,
        bool copyNodes = false,
        bool lazy = true);

  store::TempSeq_t createTempSeq(const std::vector<store::Item_t>& item_v);

#ifdef ZORBA_STORE_MSDOM
  IXMLDOMNode   *exportItemAsMSDOM(store::Item_t it);
  store::Item_t  importMSDOM(IXMLDOMNode* domNode,
                            xqpStringStore_t docUri,
                            xqpStringStore_t baseUri);
#endif
};



} // namespace store
} // namespace zorba

#endif /* ZORBA_STORE_SIMPLE_STORE_H */

/*
 * Local variables:
 * mode: c++
 * End:
 */
