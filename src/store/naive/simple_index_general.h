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
#ifndef ZORBA_SIMPLE_STORE_INDEX_HASH_GENERAL
#define ZORBA_SIMPLE_STORE_INDEX_HASH_GENERAL

#include <cassert>
#include <map>

#include "simple_index.h"
#include "store_defs.h"

namespace zorba 
{ 

namespace simplestore 
{

class AtomicItem;


/******************************************************************************
  Acts as a comparison function for the hash or tree map that implements a
  general index.
********************************************************************************/
class GeneralIndexCompareFunction
{
private:
  long               theTimezone;
  const XQPCollator* theCollator;

public:
  GeneralIndexCompareFunction(long timezone, const std::string& collation);

  ~GeneralIndexCompareFunction();

  const XQPCollator* getCollator() const { return theCollator; }

  uint32_t hash(const store::Item* key) const;

  bool equal(const store::Item* key1, const store::Item* key2) const;

  long compare(const store::Item* key1, const store::Item* key2) const;

  bool operator()(const store::Item* key1, const store::Item* key2) const
  {
    return compare(key1, key2) < 0;
  }
};


/**************************************************************************//**
  Stores all the domain nodes associated with a key K. For each such node N, a
  boolean flag is also stored. The flag is true if  K was produced by
  casting an untypedAtomic key that was found in the result of the key expr
  for node N. A second field says whether K is not a key of N but the xs:double
  (DOUBLE_IMAGE) or xs:float (FLOAT_IMAGE) value of a key of N that is stored
  exactly in the xs:long or xs:decimal map (hash indexes only, see
  GeneralIndex).
*******************************************************************************/
class GeneralIndexValue
{
public:
  enum ImageKind { NOT_IMAGE, DOUBLE_IMAGE, FLOAT_IMAGE };

  struct NodeInfo
  {
    store::Item_t theNode;
    bool          theUntyped;
    ImageKind     theImage;
  };

  typedef std::vector<NodeInfo>::const_iterator const_iterator;

  typedef std::vector<NodeInfo>::iterator iterator;

protected:
  std::vector<NodeInfo> theNodes;

public:
  GeneralIndexValue(csize size = 0) : theNodes(size) {}

  bool empty() const { return theNodes.empty(); }

  void clear() { theNodes.clear(); }

  const_iterator begin() const { return theNodes.begin(); }

  const_iterator end() const { return theNodes.end(); }

  void addNode(store::Item_t& node, bool untyped, ImageKind image);

  bool removeNode(const store::Item_t& node, bool untyped, ImageKind image);

  bool imagesOnly() const;
};


/******************************************************************************

  theKeyTypeCode:
  ---------------
  The type code for the builtin atomic key type (set to XS_LAST if the index
  is not typed).

  theCompFunction:
  ----------------
  An instance of GeneralIndexCompareFunction that acts as a comparison function
  for the hash or tree map that implements the index.

  theEmptyKeyNodes:
  -----------------
  A vector storing all domain nodes for which the key expr returns the empty
  sequence.

  theNumUntypedEntries:
  ---------------------
  The number of [key, node] index entries where the key is the result of casting
  an originally untyped key to a non-string atomic item. If > 0, then any value
  probes with a key (or keys) whose type cannot be promoted to xs:string will
  raise an error.

  theNumMultiKeyNodes:
  --------------------
  The number of domain nodes for which the key expression returns more than one
  items. If > 0, then any value probes on the general index raise an error.

  Numeric keys (if the index is not typed):
  -----------------------------------------
  Each numeric map holds keys of one type: the xs:long map the xs:decimal keys
  within the xs:long range, the xs:decimal map the other xs:decimal keys, the
  xs:double map the xs:double keys and the untyped keys cast to xs:double, and
  the xs:float map the xs:float keys. A key and a search key of another type
  compare as in a general comparison: the one that is not an xs:double is
  promoted to xs:double, else the one that is not an xs:float to xs:float.
  Promotion keeps the order, so a tree map is probed with the search key as is.
  A hash map is probed with the promotion of the search key to the type of its
  keys. Several xs:decimal keys can be equal to one xs:double or xs:float value,
  so in a hash index the xs:double and xs:float maps also hold, as images, the
  promotions of the xs:decimal keys that a probe of the xs:long map with the
  exact value of an xs:double or xs:float search key does not find.
*******************************************************************************/
class GeneralIndex : public IndexImpl
{
  friend class IndexImpl;
  friend class GeneralHashIndex;
  friend class GeneralTreeIndex;
  friend class ProbeGeneralIndexIterator;
  friend class ProbeGeneralTreeIndexIterator;

  typedef std::pair<const store::Item*, GeneralIndexValue*> IndexMapPair;

protected:
  store::SchemaTypeCode       theKeyTypeCode;

  GeneralIndexCompareFunction theCompFunction;

  std::vector<store::Item_t>  theEmptyKeyNodes;

  csize                       theNumUntypedEntries;

  csize                       theNumMultiKeyNodes;

protected:
  GeneralIndex(const store::Item_t& name, const store::IndexSpecification& spec);

  virtual ~GeneralIndex();

  bool isTyped() const { return (theKeyTypeCode != store::XS_LAST); }

  bool insertInMap(
      store::Item_t& key,
      store::Item_t& node,
      store::SchemaTypeCode targetMap,
      bool untyped,
      GeneralIndexValue::ImageKind image = GeneralIndexValue::NOT_IMAGE);

  bool removeFromMap(
      const store::Item_t& key,
      const store::Item_t& node,
      store::SchemaTypeCode targetMap,
      bool untyped,
      GeneralIndexValue::ImageKind image = GeneralIndexValue::NOT_IMAGE);

  void getImages(
      const AtomicItem* key,
      store::SchemaTypeCode targetMap,
      store::Item_t& doubleImage,
      store::Item_t& floatImage) const;

  bool insertExact(
      store::Item_t& key,
      store::Item_t& node,
      store::SchemaTypeCode targetMap);

  bool removeExact(
      const store::Item_t& key,
      const store::Item_t& node,
      store::SchemaTypeCode targetMap);

public:
  const XQPCollator* getCollator(csize i) const;

  void addMultiKey() { ++theNumMultiKeyNodes; }

  void removeMultiKey() { assert(theNumMultiKeyNodes > 0); --theNumMultiKeyNodes; }

  csize size() const;

  bool insert(store::Item_t& key, store::Item_t& node);

  bool insert(store::IndexKey*& key, store::Item_t& node);

  bool remove(const store::IndexKey* key, const store::Item_t& node, bool all);

  bool remove(const store::Item_t& key, const store::Item_t& node);
};


/******************************************************************************
  theMaps:
  --------
  One hash map for each builtin atomic type T, except from xs:untypedAtomic 
  and xs:anyAtomicType. TheMaps is indexed by the type code of T.

  theSingleMap:
  -------------
  If the keyspec of the index specifies an atomic type T and T is neither
  xs:untypedAtomic nor xs:anyAtomicType, then theSingleMap points to theMaps
  entry that corresponds to the builtin base type of T. Otherwise, theSingleMap
  is NULL.
*******************************************************************************/
class GeneralHashIndex : public GeneralIndex
{
  friend class GeneralIndex;
  friend class ProbeGeneralIndexIterator;
  friend class ProbeGeneralHashIndexIterator;

  typedef HashMap<const store::Item*,
                  GeneralIndexValue*,
                  GeneralIndexCompareFunction> IndexMap;

  typedef IndexMap::iterator EntryIterator;

  /*
   * This iterator iterates over all index keys from all types
   */
  class KeyIterator : public Index::KeyIterator
  {
  protected:
    IndexMap::iterator     theIterator;
    IndexMap* const*       theMaps;
    ulong                  theCurType;
  public:
    KeyIterator(IndexMap* const* aMaps);
    ~KeyIterator();

    void open();
    bool next(store::IndexKey&);
    void close();

  private:
    void setNextIter();

    void skipImageKeys();
  };

  typedef rchandle<KeyIterator> KeyIterator_t;

private:
  IndexMap  * theMaps[store::XS_LAST];
  IndexMap  * theSingleMap;

protected:
  bool insertInMap(
      store::Item_t& key,
      store::Item_t& node,
      IndexMap*& targetMap,
      bool untyped,
      GeneralIndexValue::ImageKind image);

  bool removeFromMap(
      const store::Item_t& key,
      const store::Item_t& node,
      IndexMap* targetMap,
      bool untyped,
      GeneralIndexValue::ImageKind image);

public:
  GeneralHashIndex(
      const store::Item_t& name,
      const store::IndexSpecification& spec);

  ~GeneralHashIndex();

  Index::KeyIterator_t keys() const;

  void clear();
};


/******************************************************************************

*******************************************************************************/
class GeneralTreeIndex : public GeneralIndex
{
  friend class GeneralIndex;
  friend class ProbeGeneralIndexIterator;
  friend class ProbeGeneralTreeIndexIterator;

  typedef std::map<const store::Item*,
                   GeneralIndexValue*,
                   GeneralIndexCompareFunction> IndexMap;

  typedef IndexMap::const_iterator EntryIterator;


  class KeyIterator : public Index::KeyIterator
  {
  public:
    ~KeyIterator();

    void open();
    bool next(store::IndexKey&);
    void close();
  };

  typedef rchandle<KeyIterator> KeyIterator_t;

private:
  IndexMap       * theMaps[store::XS_LAST];
  IndexMap       * theSingleMap;
  SYNC_CODE(Mutex  theMapMutex;)

protected:
  bool insertInMap(
      store::Item_t& key,
      store::Item_t& node,
      IndexMap*& targetMap,
      bool untyped,
      GeneralIndexValue::ImageKind image);

  bool removeFromMap(
      const store::Item_t& key,
      const store::Item_t& item,
      IndexMap* targetMap,
      bool untyped,
      GeneralIndexValue::ImageKind image);

public:
  GeneralTreeIndex(
      const store::Item_t& qname,
      const store::IndexSpecification& spec);

  ~GeneralTreeIndex();

  Index::KeyIterator_t keys() const;

  void clear();
};


/******************************************************************************
  Iterator to probe a general index.

  theIndex:
  ---------
  The index to probe

  theProbeKind: 
  --------------
  The kind of condition to satisfy. May be POINT_VALUE, POINT_GENERAL, BOX_VALUE,
  or BOX_GENERAL.

  theCondition:
  -------------
  The condition to satisfy. 

  theIsUntypedProbe:
  ------------------

  theIsFullProbe:
  ---------------
  Set to true if the condition is a range condition with no bounds. In this case,
  domain nodes that appear in the index with their associated "untyped" flag set
  to true must be skipped from the result because they will be just duplicates
  (they appear for sure in the STRING map). This is used for range probes only.
  Image entries are skipped too, because their exact keys are in the LONG or
  DECIMAL map.

  theSelectsEmptyKeys:
  --------------------
  Set to true if the condition is a range condition with no bounds, which
  selects every domain node: after the maps, the nodes whose key is the empty
  sequence are returned too.

  theEmptyKeyPos:
  ---------------
  The position in theEmptyKeyNodes of the index of the next such node.

  theNumberKind:
  --------------
  Whether the search key is an xs:decimal (or a subtype), an xs:double (or an
  untyped value cast to xs:double), or an xs:float. A probe uses only the
  images of its own type (see GeneralIndex).

  theResultSets:
  --------------
  The node sets associated with the keys that match the condition. This is used
  for point probes only.

  theResultSetsIte:
  -----------------
  Iterator over theResultSets; points to the "current" result set. This is used
  for point probes only.

  theResultSetsEnd:
  -----------------
  The "end" iterator of theResultSets. This is used for point probes only. 

  theMapBegins:
  -------------
  A vector that stores an iterator pointing to the 1st qualifying entry in 
  each tree that has at least one qualifying entry. This is used for range 
  probes only.

  theMapEnds:
  -----------
  A vector that stores an iterator pointing to the last qualifying entry in 
  each tree that has at least one qualifying entry. This is used for range 
  probes only.

  theMapIte:
  ----------

  theIte:
  -------
  For iterating over the domain nodes associated with an index key that satisfies
  the search condition. These domain nodes are returned as results of "this"
  probe iterator.

  theEnd:
  -------
  For marking the end of an iteration over the domain nodes associated with an
  index key that satisfies the search condition..
********************************************************************************/
class ProbeGeneralIndexIterator : public store::IndexProbeIterator
{
  friend class ProbeGeneralHashIndexIterator;
  friend class ProbeGeneralTreeIndexIterator;

  typedef std::vector<GeneralIndexValue*> ResultSets;

  typedef GeneralTreeIndex::EntryIterator EntryIterator;

  typedef std::vector<EntryIterator> EntryIterators;

  enum NumberKind { NOT_NUMBER, EXACT_NUMBER, DOUBLE_NUMBER, FLOAT_NUMBER };

protected:
  rchandle<GeneralIndex>                theIndex;

  store::IndexCondition::Kind           theProbeKind;

  rchandle<GeneralIndexCondition>       theCondition;

  bool                                  theIsUntypedProbe;

  bool                                  theIsFullProbe;

  bool                                  theSelectsEmptyKeys;

  csize                                 theEmptyKeyPos;

  NumberKind                            theNumberKind;

  ResultSets                            theResultSets;

  ResultSets::const_iterator            theResultSetsIte;

  ResultSets::const_iterator            theResultSetsEnd;

  EntryIterators                        theMapBegins;

  EntryIterators                        theMapEnds;

  csize                                 theCurrentMap;

  EntryIterator                         theMapIte;

  GeneralIndexValue::const_iterator     theIte;
  GeneralIndexValue::const_iterator     theEnd;

protected:
  ProbeGeneralIndexIterator(const store::Index_t& index);

  void initPoint();

  void initBox();

  bool haveMap(store::SchemaTypeCode targetMap) const;

  bool skipsImage(GeneralIndexValue::ImageKind image) const;

  void probeMap(store::SchemaTypeCode targetMap, const store::Item* key);

  void probeMap(
      const store::SchemaTypeCode targetMap,
      const store::Item* lowerKey,
      const store::Item* upperKey);

  void probeNumber(const AtomicItem* key);

  void probeNumberRange(const store::Item* lowerKey, const store::Item* upperKey);

  void checkStringKeyType(const AtomicItem* keyItem) const;

public:
  void init(const store::IndexCondition_t& cond, const xs_integer& aSkip);

  void open();

  void reset();

  void close();

  bool next(store::Item_t& result);

  void count(store::Item_t& result);
};


/******************************************************************************
  Iterator to probe a hash-based, general index. The probe itself may be a
  value probe or a general probe.
********************************************************************************/
class ProbeGeneralHashIndexIterator : public ProbeGeneralIndexIterator
{
  friend class ProbeGeneralIndexIterator;

protected:
  void probeMap(
    const GeneralHashIndex::IndexMap* targetMap,
    const store::Item* key);

public:
  ProbeGeneralHashIndexIterator(const store::Index_t& index);
};


/******************************************************************************
  Iterator to probe a tree-based, general index. The probe itself may be a
  value probe or a general probe.
********************************************************************************/
class ProbeGeneralTreeIndexIterator : public ProbeGeneralIndexIterator
{
  friend class ProbeGeneralIndexIterator;

protected:
  void probeMap(
      const GeneralTreeIndex::IndexMap* targetMap,
      const store::Item* key);

  void probeMap(
      const GeneralTreeIndex::IndexMap* targetMap,
      const store::Item* lowerKey,
      const store::Item* upperKey);

public:
  ProbeGeneralTreeIndexIterator(const store::Index_t& index);
};


}
}

#endif

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
