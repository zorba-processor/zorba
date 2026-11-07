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
#ifndef ZORBA_SIMPLE_STORE_NODE_ITERATORS
#define ZORBA_SIMPLE_STORE_NODE_ITERATORS

#include <algorithm>

#include <cassert>
#include <set>
#include <vector>

#include "store/api/iterator.h"
#include "shared_types.h"
#include "node_items.h"
#include "json_items.h"
#include "content_reader.h"

#include "zorbautils/hashfun.h"
#include "zorbautils/hashset_structured_itemh.h"


namespace zorba { namespace simplestore {


/*******************************************************************************
  The copy that an iterator over the children or the attributes of a node
  takes when the node is about to change (see ContentReader): the nodes, in
  the order of the node's storage, with each connector replaced by the node
  it stands for, and each node held by a reference.
********************************************************************************/
class NodeCopy
{
  std::vector<XmlNode*> theNodes;

public:
  NodeCopy() = default;

  NodeCopy(const NodeCopy&) = delete;
  NodeCopy& operator=(const NodeCopy&) = delete;

  ~NodeCopy() { clear(); }

  // If the allocation throws, the copy is unchanged. Iter is a node vector
  // iterator.
  template <class Iter>
  void assign(Iter begin, Iter end)
  {
    std::vector<XmlNode*> nodes;
    nodes.reserve(end - begin);
    for (; begin != end; ++begin)
    {
      XmlNode* node = ((*begin)->isConnectorNode() ?
                       static_cast<ConnectorNode*>(*begin)->getNode() :
                       *begin);
      node->addReference();
      nodes.push_back(node);
    }
    nodes.swap(theNodes);
    for (XmlNode* node : nodes)
      node->removeReference();
  }

  void clear()
  {
    std::vector<XmlNode*> nodes;
    nodes.swap(theNodes);
    for (XmlNode* node : nodes)
      node->removeReference();
  }

  XmlNode** begin() { return theNodes.data(); }

  XmlNode** end() { return theNodes.data() + theNodes.size(); }
};


/*******************************************************************************
  This iterator is used to iterate over the children of a document or element
  node. It implements the interface of a generic iterator, but also offers the
  following additional methods:

  - An init method that takes as input a document or element node and
    initializes the iterator so that it will start returning the children of
    this node. The method also takes an optional child parameter; if a child
    is given, the iterator will position itself on that child, so that the next
    next() call will return the right sibling of that child.
  - A next method that returns pointers to the children instead of rchandles.
    These pointers should not be used beyond the lifetime of the ChildrenIterator
    object.

  The iterator returns the children the node had when init() was called, and
  reset() or open() start them again (see ContentReader): if the node is
  updated while the iterator is open, the iterator copies them first. The
  copy holds references to the children, deleted ones included, until the
  next init(), close() or the destruction of the iterator, also after the
  last child was returned: reset() must be able to return them again.

  theParentNode : The element or doc node whose children are being retrieved.
  theStart      : The first child to return.
  theEnd        : The end of the children.
  theIte        : The next child to return.
  theCopy       : The children, once the node changed after init().
********************************************************************************/
class ChildrenIteratorImpl : public store::ChildrenIterator,
                             public ContentReader
{
protected:
  store::ItemHandle<InternalNode>  theParentNode;

  InternalNode::iterator           theEnd;
  InternalNode::iterator           theStart;
  InternalNode::iterator           theIte;

  NodeCopy                         theCopy;

  void setRange(const store::Item* child)
  {
    // PROBE: the iterator does not register (the defect fixed by #189)

    theEnd = theParentNode->childrenEnd();
    theStart = theParentNode->childrenBegin();

    if (child != NULL && theStart != theEnd)
    {
      XmlNode* myChild = ((*theStart)->isConnectorNode() ?
                          static_cast<ConnectorNode*>(*theStart)->getNode() :
                          (*theStart));

      while (myChild != child)
      {
        ++theStart;

        myChild = ((*theStart)->isConnectorNode() ?
                   static_cast<ConnectorNode*>(*theStart)->getNode() :
                   (*theStart));
      }

      ++theStart;
    }

    theIte = theStart;
  }

  // Stops reading the node before the iterator releases it.
  void stop()
  {
    stopReading();
    theCopy.clear();
    theStart = theEnd = theIte = InternalNode::iterator();
  }

  void copyContent()
  {
    csize const pos = theIte - theStart;
    theCopy.assign(theStart, theEnd);
    theStart = theCopy.begin();
    theEnd = theCopy.end();
    theIte = theStart + pos;
  }

public:
  ChildrenIteratorImpl() { }

  ~ChildrenIteratorImpl() { stopReading(); }

  void init(store::Item_t& parent, const store::Item* child = NULL)
  {
    stop();
    theParentNode.transfer(parent);
    setRange(child);
  }

  void init(const store::Item* parent, const store::Item* child = NULL)
  {
    stop();
    theParentNode = parent;
    setRange(child);
  }

  store::Item* next()
  {
    if (theIte == theEnd)
      return NULL;

    if ((*theIte)->isConnectorNode())
    {
      store::Item* res = static_cast<ConnectorNode*>(*theIte)->getNode();
      ++theIte;
      return res;
    }
    else
    {
      return *theIte++;
    }
  }

  void open() { theIte = theStart; }

  void reset() { theIte = theStart; }

  void close() { stop(); theParentNode = NULL; }

  bool next(store::Item_t& result);
};


/*******************************************************************************
  This iterator is used to iterate over the children of a document or element
  node in reverse document order. It implements the interface of a generic
  iterator, but also offers the following additional methods:

  - An init method that takes as input a document or element node and
    initializes the iterator so that it will start returning the children of
    this node. The method also takes an optional child parameter; if a child
    is given, the iterator will position itself on that child, so that the next
    next() call will return the left sibling of that child.
  - A next method that returns pointers to the children instead of rchandles.
    These pointers should not be used beyond the lifetime of the ChildrenIterator
    object.

  It returns the children the node had when init() was called, like
  ChildrenIteratorImpl.

  theParentNode : The element or doc node whose children are being retrieved.
  theStart      : The first child to return.
  theEnd        : The end of the children.
  theIte        : The next child to return.
  theCopy       : The children in document order, once the node changed after
                  init().
********************************************************************************/
class ChildrenReverseIteratorImpl : public store::ChildrenReverseIterator,
                                    public ContentReader
{
protected:
  store::ItemHandle<InternalNode>  theParentNode;

  InternalNode::reverse_iterator   theEnd;
  InternalNode::reverse_iterator   theStart;
  InternalNode::reverse_iterator   theIte;

  NodeCopy                         theCopy;

  void setRange(const store::Item* child)
  {
    // PROBE: the iterator does not register (the defect fixed by #189)

    theEnd = theParentNode->childrenREnd();
    theStart = theParentNode->childrenRBegin();

    if (child != NULL && theStart != theEnd)
    {
      XmlNode* myChild = ((*theStart)->isConnectorNode() ?
                          static_cast<ConnectorNode*>(*theStart)->getNode() :
                          (*theStart));

      while (myChild != child)
      {
        ++theStart;

        myChild = ((*theStart)->isConnectorNode() ?
                   static_cast<ConnectorNode*>(*theStart)->getNode() :
                   (*theStart));
      }

      ++theStart;
    }

    theIte = theStart;
  }

  void stop()
  {
    stopReading();
    theCopy.clear();
    theStart = theEnd = theIte = InternalNode::reverse_iterator();
  }

  void copyContent()
  {
    csize const pos = theIte - theStart;
    theCopy.assign(theEnd.base(), theStart.base());
    theStart = InternalNode::reverse_iterator(theCopy.end());
    theEnd = InternalNode::reverse_iterator(theCopy.begin());
    theIte = theStart + pos;
  }

public:
  ChildrenReverseIteratorImpl() { }

  ~ChildrenReverseIteratorImpl() { stopReading(); }

  void init(store::Item_t& parent, const store::Item* child = NULL)
  {
    stop();
    theParentNode.transfer(parent);
    setRange(child);
  }

  void init(const store::Item* parent, const store::Item* child = NULL)
  {
    stop();
    theParentNode = parent;
    setRange(child);
  }

  store::Item* next()
  {
    if (theIte == theEnd)
      return NULL;

    if ((*theIte)->isConnectorNode())
    {
      store::Item* res = static_cast<ConnectorNode*>(*theIte)->getNode();
      ++theIte;
      return res;
    }
    else
    {
      return *theIte++;
    }
  }

  void open() { theIte = theStart; }

  void reset() { theIte = theStart; }

  void close() { stop(); theParentNode = NULL; }

  bool next(store::Item_t& result);
};


/*******************************************************************************
  This iterator is used during the getAttributes() method to set the parent
  pointer and the nodeid in each attribute node of an element node.

  It returns the attributes the node had when init() was called, like
  ChildrenIteratorImpl.

  theParentNode : The element node whose attributes are being retrieved.
  theStart      : The first attribute.
  theEnd        : The end of the attributes.
  theIte        : The next attribute to return.
  theCopy       : The attributes, once the node changed after init().
********************************************************************************/
class AttributesIteratorImpl : public store::AttributesIterator,
                               public ContentReader
{
protected:
  store::ItemHandle<ElementNode>  theParentNode;

  InternalNode::iterator          theEnd;
  InternalNode::iterator          theStart;
  InternalNode::iterator          theIte;

  NodeCopy                        theCopy;

  void setRange()
  {
    // PROBE: the iterator does not register (the defect fixed by #189)

    theEnd = theParentNode->attrsEnd();
    theStart = theParentNode->attrsBegin();
    theIte = theStart;
  }

  void stop()
  {
    stopReading();
    theCopy.clear();
    theStart = theEnd = theIte = InternalNode::iterator();
  }

  void copyContent()
  {
    csize const pos = theIte - theStart;
    theCopy.assign(theStart, theEnd);
    theStart = theCopy.begin();
    theEnd = theCopy.end();
    theIte = theStart + pos;
  }

public:
  AttributesIteratorImpl() { }

  ~AttributesIteratorImpl() { stopReading(); }

  void init(store::Item_t& parent)
  {
    stop();
    theParentNode.transfer(parent);
    setRange();
  }

  void init(const store::Item* parent)
  {
    stop();
    theParentNode = parent;
    setRange();
  }

  store::Item* next()
  {
    if (theIte == theEnd)
      return NULL;

    store::Item* node = ((*theIte)->isConnectorNode() ?
                         static_cast<ConnectorNode*>(*theIte)->getNode() :
                         (*theIte));

    assert(node->getNodeKind() == store::StoreConsts::attributeNode);

    AttributeNode* attr = static_cast<AttributeNode*>(node);

    while (attr->isHidden())
    {
      ++theIte;

      if (theIte == theEnd)
        return NULL;

      node = ((*theIte)->isConnectorNode() ?
              static_cast<ConnectorNode*>(*theIte)->getNode() :
              (*theIte));

      assert(node->getNodeKind() == store::StoreConsts::attributeNode);

      attr = static_cast<AttributeNode*>(node);
    }

    ++theIte;

    return attr;
  }


  void open()
  {
    theIte = theStart;
  }

  void reset()
  {
    if (theParentNode)
      theIte = theStart;
  }

  void close()
  {
    stop();
    theParentNode = NULL;
  }

  bool next(store::Item_t& result);
};


/*******************************************************************************
  This iterator is used to eliminated duplicate nodes in the multiset of nodes
  produced by another iterator.

  theInput   : input iterator
  theNodeSet : A hash-based set used to do the duplicate elimination. The
               elimination is done based on the values of the pointers that
               point to the input nodes.
********************************************************************************/
class StoreNodeDistinctIterator : public store::Iterator
{
protected:
  store::Iterator_t   theInput;
  StructuredItemHandleHashSet   theNodeSet;
  bool                theCheckOnly;

public:
  StoreNodeDistinctIterator(store::Iterator* input, bool checkOnly)
    :
    theInput(input),
    theNodeSet(1024, false),
    theCheckOnly(checkOnly)
  {
  }

  virtual ~StoreNodeDistinctIterator() { close(); }

  void open();
  bool next(store::Item_t& result);
  void reset();
  void close();
};


/*******************************************************************************
  This is an extension to StoreNodeDistinctIterator which allows atomic items
  in the received sequences. The received sequences must contain nodes or atomic
  items, but not a mixture. In case of atomic items, the output sequences
  is equivalent to the input sequence, else, the same operations as in
  StoreNodeDistinctIterator are applied.
********************************************************************************/
class StoreNodeDistinctOrAtomicIterator : public StoreNodeDistinctIterator
{
protected:
  bool theAtomicMode;
  bool theNodeMode;

public:
  StoreNodeDistinctOrAtomicIterator(store::Iterator* aInput)
    :
    StoreNodeDistinctIterator(aInput, false),
    theAtomicMode(false),
    theNodeMode(false)
  {
  }

  bool next(store::Item_t& result);

  void reset()
  {
    theNodeMode = theAtomicMode = false;
    StoreNodeDistinctIterator::reset();
  }

  void close() { StoreNodeDistinctIterator::close(); }
};


/*******************************************************************************
  This iterator is used to sort by document order the nodes produced by another
  iterator.

  theInput  : input iterator
  theNodes  : A vector that stores rchandles to the nodes.
********************************************************************************/
class StoreNodeSortIterator : public store::Iterator
{
protected:
  class ComparisonFunction
  {
  protected:
    bool theAscending;

  public:
    ComparisonFunction(bool asc = true) : theAscending(asc) { }

    bool operator()(const XmlNode* n1, const XmlNode* n2) const
    {
      if (theAscending)
      {
        return (n1->compare2(n2) < 0);
      }
      else
      {
        return (n2->compare2(n1) < 0);
      }
    }
  };

protected:
  store::Iterator_t       theInput;
  bool                    theAscending;
  bool                    theDistinct;

  std::vector<XmlNode*>   theNodes;
  csize                   theCurrentNode;
  
  std::set<json::JSONItem*>  theJSONItems;

public:
  StoreNodeSortIterator(
      const store::Iterator_t& input,
      bool                     ascendant,
      bool                     distinct)
    :
    theInput(input),
    theAscending(ascendant),
    theDistinct(distinct),
    theCurrentNode(-1)
  {
  }

  virtual ~StoreNodeSortIterator() { close(); }

  void open();
  bool next(store::Item_t& result);
  void reset();
  void close();
};


/*******************************************************************************
  This is an extension to StoreNodeSortIterator which allows atomic items
  in the received sequences. The received sequences must contain nodes or atomic
  items, but not a mixture. In case of atomic items, the output sequences
  is equivalent to the input sequence, else, the same operations as in
  StoreNodeSortIterator are applied.
********************************************************************************/
class StoreNodeSortOrAtomicIterator : public StoreNodeSortIterator
{
protected:
  bool theAtomicMode;
  bool theNodeMode;

public:
  StoreNodeSortOrAtomicIterator(const store::Iterator_t& input, bool asc, bool distinct)
    :
    StoreNodeSortIterator(input, asc, distinct),
    theAtomicMode(false),
    theNodeMode(false)
  {
  }

  bool next(store::Item_t& result);

  void reset()
  {
    theNodeMode = theAtomicMode = false;
    StoreNodeSortIterator::reset();
  }

  void close() { StoreNodeSortIterator::close(); }
};


} // namespace simplestore
} // namespace zorba

#endif

/*
 * Local variables:
 * mode: c++
 * End:
 */

/* vim:set et sw=2 ts=2: */
