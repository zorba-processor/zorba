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

#include "diagnostics/assert.h"
#include "diagnostics/xquery_diagnostics.h"

#include "simple_collection.h"
#include "simple_index.h"
#include "store/api/ic.h"
#include "store/api/annotation.h"
#include "loader.h"
#include "simple_store.h"
#include "store_defs.h"
#include "node_items.h"
#include "json_items.h"

#include "zorbatypes/numconversions.h"

namespace zorba { namespace simplestore {

/*******************************************************************************

********************************************************************************/
SimpleCollection::SimpleCollection(
    const store::Item_t& name,
    const std::vector<store::Annotation_t>& annotations,
    bool isDynamic)
  : 
  Collection(name),
  theIsDynamic(isDynamic),  
  theAnnotations(annotations),
  theVersion(0)
{
  theId = GET_STORE().createCollectionId();
  theTreeIdGenerator = GET_STORE().getTreeIdGeneratorFactory().createTreeGenerator(0);
}


/*******************************************************************************
  Default constructor added in order to allow subclasses to instantiate a
  collection without name.
********************************************************************************/
SimpleCollection::SimpleCollection()
  : 
  theIsDynamic(false),
  theVersion(0)
{
  theTreeIdGenerator = GET_STORE().getTreeIdGeneratorFactory().createTreeGenerator(0);
}


/*******************************************************************************

********************************************************************************/
SimpleCollection::~SimpleCollection()
{
  delete theTreeIdGenerator;
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::getAnnotations(
    std::vector<store::Annotation_t>& annotations) const
{
  annotations = theAnnotations;
}


/*******************************************************************************
  Return an iterator over the nodes of this collection.

  Note: it is allowed to have several concurrent iterators on the same collection
  but each iterator should be used by a single thread only.
********************************************************************************/
store::Iterator_t SimpleCollection::getIterator(
    const xs_integer& skip,
    const zstring& startRef)
{
  store::Item_t startNode;
  xs_integer startPos;

  if (startRef.size() != 0 &&
      (!GET_STORE().getNodeByReference(startNode, startRef) ||
       !findNode(startNode.getp(), startPos)))
  {
    throw ZORBA_EXCEPTION(zerr::ZSTR0066_REFERENCED_NODE_NOT_IN_COLLECTION,
    ERROR_PARAMS(startRef, theName->getStringValue()));
  }

  try
  {
    return new CollectionIter(this, skip + startPos);
  }
  catch (const std::range_error&)
  {
    throw ZORBA_EXCEPTION(
        zerr::ZXQD0004_INVALID_PARAMETER,
        ERROR_PARAMS(ZED(ZXQD0004_NOT_WITHIN_RANGE), skip)
      );
  }
}


/*******************************************************************************
  Check if the tree rooted at the given node belongs to this collection. If yes,
  return true and the position of the tree within the collection. Otherwise, 
  return false.
********************************************************************************/
bool SimpleCollection::findNode(const store::Item* item, xs_integer& position) const
{
  csize pos;
  if (!findTree(item, pos))
    return false;

  position = xs_integer(pos);
  return true;
}


/*******************************************************************************
  findNode() with the position as a csize, which allocates nothing.
********************************************************************************/
bool SimpleCollection::findTree(const store::Item* item, csize& position) const
{
  if (!(item->isStructuredItem()))
  {
    throw ZORBA_EXCEPTION(zerr::ZSTR0013_COLLECTION_ITEM_MUST_BE_STRUCTURED,
    ERROR_PARAMS(getName()->getStringValue()));
  }

  const StructuredItem* structuredItem = static_cast<const StructuredItem*>(item);
  
  if (structuredItem->isNode())
  {
    const XmlNode* node = static_cast<const XmlNode*>(item);
    if (node->getTree()->getRoot() != node)
    {
      throw ZORBA_EXCEPTION(zerr::ZSTR0011_COLLECTION_NON_ROOT_NODE,
      ERROR_PARAMS(getName()->getStringValue()));
    }
  }

  if (theTrees.empty())
    return false;

  if (item->getCollection() != this)
    return false;

  position = structuredItem->getPosition();

  if (position < theTrees.size())
  {
    StructuredItem* collectionItem =
    static_cast<StructuredItem*>(theTrees[position].getp());

    if (collectionItem->getTreeId() == structuredItem->getTreeId())
    {
      return true;
    }
  }

  csize numTrees = theTrees.size();

  for (csize i = 0; i < numTrees; ++i)
  {
    // check if the nodes are the same
    if (item->equals(theTrees[i]))
    {
      ZORBA_ASSERT(theTrees[i]->getCollection() == this);
      position = i;
      return true;
    }
  }

  return false;
}


/*******************************************************************************
  Return the node at the given position within the collection, or NULL if the
  given position is >= than the number of nodes in the collection.
********************************************************************************/
store::Item_t SimpleCollection::nodeAt(xs_integer position)
{
  try
  {
    csize pos = to_xs_unsignedInt(position);
    if (pos >= theTrees.size())
    {
      return NULL;
    }

    return theTrees[pos];
  }
  catch (const std::range_error&)
  {
    throw ZORBA_EXCEPTION(
        zerr::ZXQD0004_INVALID_PARAMETER,
        ERROR_PARAMS(ZED(ZXQD0004_NOT_WITHIN_RANGE), position)
      );
  }
}


/*******************************************************************************

********************************************************************************/
TreeId SimpleCollection::createTreeId()
{
  return theTreeIdGenerator->create();
}


/*******************************************************************************
  Insert the given node to the collection. If the node is in any collection
  already or if the node is an xml node with a parent, this method raises an
  error. Otherwise, the node is inserted into the given position.
********************************************************************************/
void SimpleCollection::addNode(store::Item* item, xs_integer position)
{
  if (!(item->isStructuredItem()))
  {
    throw ZORBA_EXCEPTION(zerr::ZSTR0013_COLLECTION_ITEM_MUST_BE_STRUCTURED,
    ERROR_PARAMS(getName()->getStringValue()));
  }

  if (item->getCollection() != NULL)
  {
    throw ZORBA_EXCEPTION(zerr::ZSTR0010_COLLECTION_NODE_ALREADY_IN_COLLECTION,
    ERROR_PARAMS(getName()->getStringValue(),
                 item->getCollection()->getName()->getStringValue()));
  }

  StructuredItem* structuredItem = static_cast<StructuredItem*>(item);
  
  if (structuredItem->isNode())
  {
    XmlNode* node = static_cast<XmlNode*>(item);
    if (node->getRoot() != node)
    {
      throw ZORBA_EXCEPTION(zerr::ZSTR0011_COLLECTION_NON_ROOT_NODE,
      ERROR_PARAMS(getName()->getStringValue()));
    }
  }
  
  try
  {
    xs_long pos = to_xs_long(position);
    ZORBA_ASSERT_STORE_WRITE();

    csize treePos = theTrees.size();
    if (pos >= 0 && to_xs_unsignedLong(position) < treePos)
      treePos = static_cast<csize>(pos);

    // Attached first: a failed insertion detaches it, and the collection is
    // as it was.
    structuredItem->attachToCollection(this, createTreeId(), treePos);
    try
    {
      theTrees.insert(theTrees.begin() + treePos, item);
    }
    catch (...)
    {
      structuredItem->detachFromCollection();
      throw;
    }
  }
  catch (const std::range_error&)
  {
    throw ZORBA_EXCEPTION(
        zerr::ZXQD0004_INVALID_PARAMETER,
        ERROR_PARAMS(ZED(ZXQD0004_NOT_WITHIN_RANGE), position)
      );
  }
  
  ++theVersion;
}


/*******************************************************************************
  Insert the given nodes to the collection before or after the given target node.
  If any of the nodes is a non root xml node or is in any collection already,
  this method raises an error. The moethod returns the position occupied by the
  first new node after the insertion is done.
********************************************************************************/
csize SimpleCollection::addNodes(
    std::vector<store::Item_t>& items,
    const store::Item* targetNode,
    bool before)
{
  ZORBA_ASSERT_STORE_WRITE();

  csize targetPos;
  bool found = findTree(targetNode, targetPos);

  if (!found)
  {
    throw ZORBA_EXCEPTION(zerr::ZDDY0011_COLLECTION_NODE_NOT_FOUND,
    ERROR_PARAMS(theName->getStringValue()));
  }

  if (!before)
  {
    ++targetPos;
  }

  csize numNodes = theTrees.size();
  csize numNewNodes = items.size();
  
  for (csize i = 0; i < numNewNodes; ++i)
  {
    store::Item* item = items[i].getp();

    if (!(item->isStructuredItem()))
    {
      throw ZORBA_EXCEPTION(zerr::ZSTR0013_COLLECTION_ITEM_MUST_BE_STRUCTURED,
      ERROR_PARAMS(getName()->getStringValue()));
    }

    if (item->getCollection() != NULL)
    {
      throw ZORBA_EXCEPTION(zerr::ZSTR0010_COLLECTION_NODE_ALREADY_IN_COLLECTION,
      ERROR_PARAMS(getName()->getStringValue(),
                   item->getCollection()->getName()->getStringValue()));
    }

    StructuredItem* structuredItem = static_cast<StructuredItem*>(item);
  
    if (structuredItem->isNode())
    {
      XmlNode* node = static_cast<XmlNode*>(item);

      if (node->getRoot() != node)
      {
        throw ZORBA_EXCEPTION(zerr::ZSTR0011_COLLECTION_NON_ROOT_NODE,
        ERROR_PARAMS(getName()->getStringValue()));
      }
    }

    structuredItem->attachToCollection(this, createTreeId(), targetPos + i);
  } // for each new node

  theTrees.resize(numNodes + numNewNodes);

  if (targetPos < numNodes)
  {
    // Bitwise relocation of the handles; the vacated slots are reset with
    // setNull() (not released) below.
    memmove(static_cast<void*>(&theTrees[targetPos + numNewNodes]),
            static_cast<void const*>(&theTrees[targetPos]),
            (numNodes-targetPos) * sizeof(store::Item_t));
  }

  for (csize i = targetPos; i < targetPos + numNewNodes; ++i)
  {
    theTrees[i].setNull();
  }

  // Copies, not transfers: the undo checks the trees it removes against them.
  for (csize i = 0; i < numNewNodes; ++i)
  {
    theTrees[targetPos + i] = items[i];
  }

  ++theVersion;

  return targetPos;
}


/*******************************************************************************
  Remove the tree rooted at the given node, if the tree actually belongs to the
  collection. If the tree was found, return its collection info, for
  restoreNode(), and its position; otherwise, return NULL.
********************************************************************************/
std::unique_ptr<CollectionTreeInfo> SimpleCollection::removeNode(
    store::Item* item,
    csize& position)
{
  ZORBA_ASSERT_STORE_WRITE();

  if (!findTree(item, position))
    return NULL;

  ZORBA_ASSERT(item->getCollection() == this);

  std::unique_ptr<CollectionTreeInfo> info =
    static_cast<StructuredItem*>(item)->detachKeepingInfo();

  theTrees.erase(theTrees.begin() + position);
  ++theVersion;
  return info;
}


/*******************************************************************************
  Put back at its position a tree that removeNode() removed, with no
  allocation: the vector of trees has not shrunk since.
********************************************************************************/
void SimpleCollection::restoreNode(
    store::Item* item,
    csize position,
    std::unique_ptr<CollectionTreeInfo> info)
{
  ZORBA_ASSERT_STORE_WRITE();
  ZORBA_ASSERT(position <= theTrees.size() &&
               theTrees.size() < theTrees.capacity());

  theTrees.insert(theTrees.begin() + position, item);
  static_cast<StructuredItem*>(item)->reattachToCollection(std::move(info));
  ++theVersion;
}


/*******************************************************************************
  Remove the num trees from the given position on, which must be nodes[0] to
  nodes[num - 1], with no allocation: the undo of an insert.
********************************************************************************/
void SimpleCollection::removeNodes(
    const std::vector<store::Item_t>& nodes,
    csize position,
    csize num)
{
  ZORBA_ASSERT_STORE_WRITE();
  ZORBA_ASSERT(position <= theTrees.size() &&
               num <= theTrees.size() - position &&
               num <= nodes.size());

  for (csize i = 0; i < num; ++i)
  {
    store::Item* item = theTrees[position + i].getp();

    ZORBA_ASSERT(item == nodes[i].getp());
    ZORBA_ASSERT(item->getCollection() == this);

    static_cast<StructuredItem*>(item)->detachFromCollection();
  }

  theTrees.erase(theTrees.begin() + position,
                 theTrees.begin() + position + num);

  if (num > 0)
    ++theVersion;
}


/*******************************************************************************
  Remove the last num trees, which must be nodes[0] to nodes[num - 1], with no
  allocation: the undo of an insert at the end of the collection.
********************************************************************************/
void SimpleCollection::removeLastNodes(
    const std::vector<store::Item_t>& nodes,
    csize num)
{
  ZORBA_ASSERT(num <= theTrees.size());

  removeNodes(nodes, theTrees.size() - num, num);
}


/*******************************************************************************
  Remove all the nodes from the collection
********************************************************************************/
void SimpleCollection::removeAll()
{
  ZORBA_ASSERT_STORE_WRITE();

  csize numTrees = theTrees.size();

  for (csize i = 0; i < numTrees; ++i)
  {
    store::Item* item = theTrees[i].getp();

    ZORBA_ASSERT(item->getCollection() == this);
    ZORBA_ASSERT(item->isStructuredItem());

    StructuredItem* structuredItem = static_cast<StructuredItem*>(item);
    structuredItem->detachFromCollection();
  }

  theTrees.clear();
}


/*******************************************************************************
  For each tree in the collection, set its current position within the collection.
********************************************************************************/
void SimpleCollection::adjustTreePositions()
{
  csize numTrees = theTrees.size();

  for (csize i = 0; i < numTrees; ++i)
  {
    static_cast<StructuredItem*>(theTrees[i].getp())->setPosition(i);
  }
}


/*******************************************************************************

********************************************************************************/
SimpleCollection::CollectionIter::CollectionIter(
    SimpleCollection* collection,
    const xs_integer& skip)
  :
  theCollection(collection),
  theIsOpen(false),
  theSkip(static_cast<zorba::csize>(to_xs_unsignedLong(skip)))
{
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::skip()
{
  // skip by position
  if (theSkip >= theCollection->size())
  {
    // we need to skip more then possible -> jump to the end
    theIterator = theEnd;
  }
  else
  {
    theIterator += theSkip;
  }
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::open()
{
  theIsOpen = true;

  theIterator = theCollection->theTrees.begin();
  theEnd = theCollection->theTrees.end();
  
  theVersion = theCollection->theVersion;

  skip();
}


/*******************************************************************************

********************************************************************************/
bool SimpleCollection::CollectionIter::next(store::Item_t& result)
{
  if (theVersion != theCollection->theVersion)
  {
    throw ZORBA_EXCEPTION(zerr::ZDDY0041_CONCURRENT_MODIFICATION,
    ERROR_PARAMS(theCollection->getName()->getStringValue()));
  }

  if (!theIsOpen)
  {
    throw ZORBA_EXCEPTION(zerr::ZDDY0019_COLLECTION_ITERATOR_NOT_OPEN,
    ERROR_PARAMS(theCollection->getName()->getStringValue()));
  }

  if (theIterator == theEnd) 
  {
    result = NULL;
    return false;
  }

  result = *theIterator;
  ++theIterator;

  return true;
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::reset()
{
  theIterator = theCollection->theTrees.begin();
  theEnd = theCollection->theTrees.end();

  theVersion = theCollection->theVersion;
  skip();
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::close() 
{
  assert(theIsOpen);
  theIsOpen = false;
}

} // namespace simplestore
} // namespace zorba
/* vim:set et sw=2 ts=2: */
