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

#ifndef ZORBA_STORE_JSON_ITEMS_H
#define ZORBA_STORE_JSON_ITEMS_H

#include <memory>
#include <vector>

#include <zorba/config.h>
#include "util/stl_util.h"
#include "util/unordered_map.h"
#include "util/hash/hash.h"

#include "diagnostics/assert.h"

#include "store/api/item_handle.h"
#include "store/api/iterator.h"

#include "atomic_items.h"
#include "content_reader.h"
#include "collection_tree_info.h"
#include "simple_collection.h"
#include "structured_item.h"

#include "runtime/json/nesting.h"

namespace zorba
{

namespace store
{
class CopyMode;
}

namespace simplestore
{

class CollectionTreeInfoGetters;

namespace json
{

/******************************************************************************

*******************************************************************************/

class JSONNull : public AtomicItem
{
protected:
  SYNC_CODE(mutable RCLock  theRCLock;)

public:
  JSONNull() : AtomicItem(store::JS_NULL) { }

  SYNC_CODE(RCLock* getRCLock() const { return &theRCLock; })

  zstring getStringValue() const { return "null"; }

  void getStringValue2(zstring& val) const { val = "null"; }

  void appendStringValue(zstring& buf) const { buf += "null"; }

  void getTypedValue(store::Item_t& val, store::Iterator_t& iter) const;

  store::SchemaTypeCode getTypeCode() const { return store::JS_NULL; }

  store::Item* getType() const; 

  bool equals(
      const store::Item* other,
      long timezone = 0,
      const XQPCollator* collation = 0) const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  long compare(const store::Item* i, long tz = 0, const XQPCollator* c = 0) const
  {
    if (i->getTypeCode() == store::JS_NULL)
      return 0;
    else
      return -1;
  }

  bool getEBV() const { return false; }
};


/******************************************************************************

*******************************************************************************/

class JSONItem : public StructuredItem
{
  // Used to access collection tree information.
  friend class zorba::simplestore::CollectionTreeInfoGetters;

protected:
  SYNC_CODE(mutable RCLock  theRCLock;)

  CollectionTreeInfoWithTreeId * theCollectionInfo;

  // The levels of arrays and objects in this value, as seen when its own
  // members last changed: 1 if none is a JSON item. An update of a member does
  // not reach this item (see runtime/json/nesting.h).
  uint32_t                       theHeight;

  // Raises the limit error if aMember would make this value too deep, and
  // takes it into account otherwise. To be called before aMember is added.
  void addHeightOf(const store::Item* aMember)
  {
    if (aMember->isJSONItem())
    {
      uint32_t const lHeight = static_cast<const JSONItem*>(aMember)->theHeight;
      if (lHeight >= json_nesting::MAX_DEPTH)
        json_nesting::raise_limit();
      if (lHeight >= theHeight)
        theHeight = lHeight + 1;
    }
  }

  // Whether removing aMember can make this value less deep.
  bool isDeepest(const store::Item* aMember) const
  {
    return aMember->isJSONItem() &&
           static_cast<const JSONItem*>(aMember)->theHeight + 1 == theHeight;
  }

  // Adds the JSON items among the members to aJSONMembers and propagates
  // the collection info to the other structured ones.
  virtual void setCollectionInfoOfMembers(
      CollectionTreeInfo* collectionInfo,
      std::vector<JSONItem*>& aJSONMembers) = 0;

public:
  SYNC_CODE(RCLock* getRCLock() const { return &theRCLock; })

  JSONItem(store::Item::ItemKind k)
    : StructuredItem(k), theCollectionInfo(NULL), theHeight(1) {}

  virtual ~JSONItem();
  
  virtual void free();

  virtual void destroy();

  // StructuredItem API

  void attachToCollection(
      Collection* aCollection,
      const TreeId& aTreeId,
      const xs_integer& aPosition);

  void detachFromCollection();
  
  CollectionTreeInfo* getCollectionTreeInfo() const { return theCollectionInfo; }

  void setCollectionTreeInfo(CollectionTreeInfo* collectionInfo);
  
  long getCollectionTreeRefCount() const;

  // store API

  const store::Collection* getCollection() const
  {
    return (theCollectionInfo ? theCollectionInfo->getCollection() : NULL);
  }

  virtual bool equals(
      const store::Item* other,
      long timezone = 0,
      const XQPCollator* aCollation = 0) const
  {
    return this == other;
  }

public:
#ifndef NDEBUG
  virtual void assertInvariant() const;

  virtual bool isThisTreeOfAllDescendants(
      const CollectionTreeInfo* collectionInfo) const = 0;
#endif
};


/******************************************************************************

*******************************************************************************/

class JSONObject : public JSONItem
{
public:

  JSONObject() : JSONItem(store::Item::OBJECT) {}

  // store API

  virtual store::StoreConsts::JSONItemKind getJSONItemKind() const 
  {
    return store::StoreConsts::jsonObject;
  }

  virtual store::Iterator_t getObjectKeys() const = 0;

  virtual store::Item_t getObjectValue(const store::Item_t& aKey) const = 0;

  virtual Item* getType() const;

  // updates
  
  virtual bool add(
      const store::Item_t& aName,
      const store::Item_t& aValue,
      bool accumulate) = 0;

  virtual store::Item_t remove(const store::Item_t& aName) = 0;

  virtual store::Item_t setValue(
    const store::Item_t& aName,
    const store::Item_t& aValue) = 0;
    
  virtual bool rename(
    const store::Item_t& aName,
    const store::Item_t& aNewName) = 0;
};


/******************************************************************************

*******************************************************************************/

class SimpleJSONObject : public JSONObject
{
protected:
  typedef std::vector<std::pair<store::Item*, store::Item*> > Pairs;
  typedef Pairs::size_type size_type;

  // Index from key string to position in thePairs. The map owns its keys: a
  // zstring copy shares the key item's representation when that one is
  // sharable and is a deep clone otherwise, so a key never refers to memory
  // the object does not hold.
  typedef std::unordered_map<zstring, size_type> Keys;

public:
  // Objects of at most this many pairs have no Keys index: a key is found by
  // a linear search over thePairs. The index is built when an object grows
  // past it and dropped when it shrinks back to it. 16 is the largest object
  // size measured where the linear search is not slower to build than the
  // index (bench/store/RESULTS.md).
  static constexpr size_type KEY_INDEX_THRESHOLD = 16;

protected:

  // Returns the keys the object had when open() or reset() was called (see
  // ContentReader): if the object changes while the iterator is open, the
  // iterator copies them first.
  class KeyIterator : public store::Iterator, public ContentReader
  {
    protected:
      SimpleJSONObject_t          theObject;
      size_type                   thePos;
      size_type                   theSize;
      std::vector<store::Item_t>  theCopy;  // the keys, once the object changed

      void copyContent();

    public:
      KeyIterator(const SimpleJSONObject_t& aObject)
        : theObject(aObject), thePos(0), theSize(0) {}

      virtual ~KeyIterator();

      virtual void open();

      virtual bool next(store::Item_t&);

      virtual void reset();

      virtual void close();
  };

private:
  Pairs                 thePairs;
  std::unique_ptr<Keys> theKeys;  // null while thePairs is small

  size_type findKey(store::Item const *aName, zstring const &aNameStr) const;
  void      buildKeyIndex();

public:
  SimpleJSONObject() : JSONObject() {}

  virtual ~SimpleJSONObject();

  // store API

  size_t alloc_size() const;
  size_t dynamic_size() const;

  store::Iterator_t getObjectKeys() const;

  store::Item_t getObjectValue(const store::Item_t& aKey) const;

  xs_integer getNumObjectPairs() const;

  store::Item* copy(
      store::Item* parent,
      const store::CopyMode& copymode) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  void getTypedValue(store::Item_t& val, store::Iterator_t& iter) const;

  zstring show() const;

  // updates
  
  virtual bool add(
      const store::Item_t& aName,
      const store::Item_t& aValue,
      bool accumulate);

  virtual store::Item_t remove(const store::Item_t& aName);

  virtual store::Item_t setValue(
      const store::Item_t& aName,
      const store::Item_t& aValue);
    
  virtual bool rename(
      const store::Item_t& aName,
      const store::Item_t& aNewName);

  // StructuredItem API

  bool isInSubtree(const StructuredItem* aJSONItem) const;

  virtual void swap(store::Item* anotherItem);

protected:
  void setCollectionInfoOfMembers(
      CollectionTreeInfo* collectionInfo,
      std::vector<JSONItem*>& aJSONMembers);

private:
  void recomputeHeight();

public:
  // Invariant handling
#ifndef NDEBUG
  void assertInvariant() const;
  
  bool isThisTreeOfAllDescendants(const CollectionTreeInfo* collectionInfo) const;
#endif
};


/******************************************************************************

*******************************************************************************/

class JSONArray : public JSONItem
{
public:
  JSONArray() : JSONItem(store::Item::ARRAY) {}

  // store API
  
  store::StoreConsts::JSONItemKind
  getJSONItemKind() const { return store::StoreConsts::jsonArray; }

  virtual store::Item*
  getType() const;
  
  virtual xs_integer getArraySize() const = 0;

  virtual store::Item_t getArrayValue(const xs_integer& position) const = 0;

  virtual store::Iterator_t getArrayValues() const = 0;

  // updates

  virtual void
  push_back(const store::Item_t& aValue) = 0;

  virtual void
  push_back(const std::vector<store::Item_t>& members) = 0;

  virtual void
  push_front(const std::vector<store::Item_t>& members) = 0;

  virtual void
  insert_before(const xs_integer& pos, const store::Item_t& member) = 0;

  virtual void
  insert_before(const xs_integer& pos, const std::vector<store::Item_t>& members) = 0;

  virtual void
  insert_after(const xs_integer& pos, const std::vector<store::Item_t>& members) = 0;

  virtual store::Item_t
  remove(const xs_integer& pos) = 0;

  virtual store::Item_t
  replace(const xs_integer& pos, const store::Item_t& value) = 0;
};


/******************************************************************************

*******************************************************************************/

class SimpleJSONArray : public JSONArray
{
protected:
  typedef std::vector<store::Item*> Members;

  // Returns the members the array had when open() or reset() was called (see
  // ContentReader): if the array changes while the iterator is open, the
  // iterator copies them first.
  class ValuesIterator : public store::Iterator, public ContentReader
  {
  protected:
    SimpleJSONArray_t           theArray;
    Members::size_type          thePos;
    Members::size_type          theSize;
    std::vector<store::Item_t>  theCopy;  // the members, once the array changed

    void copyContent();

  public:
    ValuesIterator(const SimpleJSONArray_t& anArray)
      : theArray(anArray), thePos(0), theSize(0) {}

    virtual ~ValuesIterator();
    
    virtual void open();

    virtual bool next(store::Item_t&);

    virtual void reset();

    virtual void close();
  };

private:
  Members theContent;

public:
  SimpleJSONArray() : JSONArray()
  {
  }

  SimpleJSONArray(size_t aReservedSize)
  {
    theContent.reserve(aReservedSize);
  }

  virtual ~SimpleJSONArray();

  // store API

  size_t alloc_size() const;
  size_t dynamic_size() const;

  xs_integer getArraySize() const;

  store::Item_t getArrayValue(const xs_integer& position) const;
  
  store::Iterator_t getArrayValues() const;

  store::Item* copy(
      store::Item* parent,
      const store::CopyMode& copymode) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  void getTypedValue(store::Item_t& val, store::Iterator_t& iter) const;

  zstring show() const;

  // updates
  
  virtual void
  push_back(const store::Item_t& aValue);

  virtual void
  push_back(const std::vector<store::Item_t>& members);

  virtual void
  push_front(const std::vector<store::Item_t>& members);

  virtual void
  insert_before(const xs_integer& pos, const store::Item_t& member);

  virtual void
  insert_before(const xs_integer& pos, const std::vector<store::Item_t>& members);

  virtual void
  insert_after(const xs_integer& pos, const std::vector<store::Item_t>& members);

  virtual store::Item_t
  remove(const xs_integer& aPos);

  virtual store::Item_t
  replace(const xs_integer& aPos, const store::Item_t& value);

  // StructuredItem API
  
  virtual void swap(Item* anotherItem);

  bool isInSubtree(const StructuredItem* aJSONItem) const;

protected:
  void setCollectionInfoOfMembers(
      CollectionTreeInfo* collectionInfo,
      std::vector<JSONItem*>& aJSONMembers);

  void recomputeHeight();

  void add(uint64_t pos, const std::vector<store::Item_t>& aNewMembers);

  static uint64_t cast(const xs_integer& i);

  // Invariant handling
public:
#ifndef NDEBUG
  bool isThisTreeOfAllDescendants(const CollectionTreeInfo* colectionInfo) const;
#endif
};


#if 0
#define ASSERT_INVARIANT() assertInvariant()
#else
#define ASSERT_INVARIANT()
#endif

} // namespace json
} // namespace simplestore
} // namespace zorba
#endif /* ZORBA_STORE_JSON_ITEMS_H */

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
