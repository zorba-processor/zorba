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

#include <deque>
#include <stack>
#include <utility>
#include <vector>
#include <zorba/internal/unique_ptr.h>

#include "system/globalenv.h"
#include "diagnostics/xquery_diagnostics.h"
#include "diagnostics/assert.h"
#include "zorbatypes/datetime.h"
#include "zorbatypes/duration.h"

#include "store/api/copymode.h"
#include "store/api/collection.h"
#include "store/api/update_consts.h"

#include "store_defs.h"
#include "simple_store.h"
#include "nsbindings.h"
#include "node_items.h"
#include "pul_primitives.h"
#include "simple_pul.h"
#include "atomic_items.h"
#include "simple_item_factory.h"
#include "node_factory.h"

#include "store/api/validator.h"
#include "qname_pool.h"


namespace zorba { namespace simplestore {


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  NodeTypeInfo                                                               //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


NodeTypeInfo::~NodeTypeInfo() 
{
  if (theChildFlags & XmlNode::IsTyped)
    theTextContent.setValue(NULL);
  else
    theTextContent.destroyText(); 
}


NodeTypeInfo::NodeTypeInfo(const NodeTypeInfo& other)
{
  theNode = other.theNode;
  theTypeName = other.theTypeName;
  theFlags = other.theFlags;
  theChildFlags = other.theChildFlags;
  
  if (theChildFlags & XmlNode::IsTyped)
    theTextContent.copyValue(other.theTextContent.getValue());
  else
    theTextContent.copyText(other.theTextContent.getText());
}


NodeTypeInfo& NodeTypeInfo::operator=(const NodeTypeInfo& other)
{
  theNode = other.theNode;
  theTypeName = other.theTypeName;
  theFlags = other.theFlags;

  if (theChildFlags & XmlNode::IsTyped)
    theTextContent.setValue(NULL);
  else
    theTextContent.destroyText();

  theChildFlags = other.theChildFlags;
  
  if (theChildFlags & XmlNode::IsTyped)
    theTextContent.copyValue(other.theTextContent.getValue());
  else
    theTextContent.copyText(other.theTextContent.getText());

  return *this;
}


void NodeTypeInfo::transfer(NodeTypeInfo& other)
{
  theNode = other.theNode;
  theTypeName.transfer(other.theTypeName);
  theFlags = other.theFlags;

  if (theChildFlags & XmlNode::IsTyped)
    theTextContent.setValue(NULL);
  else
    theTextContent.destroyText();
  
  theChildFlags = other.theChildFlags;

  if (theChildFlags & XmlNode::IsTyped)
    theTextContent.setValue(other.theTextContent.releaseValue());
  else
    theTextContent.setText(other.theTextContent);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Node Update Utils                                                          //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
  Make the tree rooted at "this" node a subtree of the given parent node at the
  given position among the children of the parent node.
********************************************************************************/
void XmlNode::attach(InternalNode* parent, csize pos, AttachRecord& record)
{
  ZORBA_ASSERT(theParent == NULL);
  ZORBA_ASSERT(parent != NULL);

  store::StoreConsts::NodeKind nodeKind = getNodeKind();

  XmlTree* newTree = parent->getTree();
  XmlTree* oldTree = getTree();

  // All-or-nothing: everything that can fail (a node id over its size limit,
  // an allocation) happens before the subtree changes, and if something
  // throws, the subtree is left as it was. Nothing after the ids are swapped
  // in can fail.
  std::deque<NodeLabel> newLabels;
  std::vector<std::pair<LabeledNode*, NodeLabel*> > newLabelOf;

  // The nodes that move to the new tree: "this", then its attributes and
  // descendants.
  std::vector<XmlNode*> moved;

  // The hidden base-uri attributes to re-resolve (with their new value) or to
  // remove (with no value).
  std::vector<std::pair<AttributeNode*, store::Item_t> > baseUris;

  // The moved nodes whose types newTree already has.
  csize numTyped = 0;

  AttributeNode* removedBaseUri = NULL;

#ifndef TEXT_ORDPATH
  bool hasLabel = (nodeKind != store::StoreConsts::textNode);
#else
  bool hasLabel = true;
#endif

  std::unique_ptr<NodeLabel> oldLabel;

  try
  {
    if (hasLabel)
    {
      LabeledNode* self = static_cast<LabeledNode*>(this);
      oldLabel = std::make_unique<NodeLabel>(self->theLabel);

      self->setLabel(parent, false, pos, nodeKind);

      if (nodeKind == store::StoreConsts::elementNode)
        computeSubtreeLabels(newLabels, newLabelOf);
    }

    collectMoved(parent, moved, baseUris);

    // Room for one more node, growing geometrically: a statement inserting many
    // nodes into one parent reserves once per insert.
    if (parent->theNodes.size() == parent->theNodes.capacity())
      parent->theNodes.reserve(2 * parent->theNodes.size() + 1);

    // The hidden base-uri attribute that is taken off is freed without
    // removing its type (see ~AttachRecord()), so newTree must not have it.
    for (csize i = 0; i < baseUris.size(); ++i)
    {
      if (baseUris[i].second == NULL)
        removedBaseUri = baseUris[i].first;
    }

    for (; numTyped < moved.size(); ++numTyped)
    {
      if (moved[numTyped] == removedBaseUri)
        continue;
      store::Item_t type = moved[numTyped]->typeToMove();
      if (type != NULL)
        newTree->addType(moved[numTyped], type);
    }
  }
  catch (...)
  {
    for (csize i = 0; i < numTyped; ++i)
    {
      if (moved[i] != removedBaseUri && moved[i]->typeToMove() != NULL)
        newTree->removeType(moved[i]);
    }

    if (oldLabel)
      oldLabel->swap(static_cast<LabeledNode*>(this)->theLabel);
    throw;
  }

  for (csize i = 0; i < newLabelOf.size(); ++i)
    newLabelOf[i].first->theLabel.swap(*newLabelOf[i].second);

  oldTree->setRoot(NULL);

  // The nodes and their references move together, under the locks of both
  // trees (see XmlNode::lockTree()). The old tree keeps its type entries, for
  // undoAttach().
  SYNC_CODE(oldTree->getRCLock()->acquire());
  SYNC_CODE(newTree->getRCLock()->acquire());
  for (csize i = 0; i < moved.size(); ++i)
    moved[i]->setTreeInternal(newTree);
  newTree->getRefCount() += oldTree->getRefCount();
  oldTree->getRefCount() = 0;
  SYNC_CODE(newTree->getRCLock()->release());
  SYNC_CODE(oldTree->getRCLock()->release());

  theParent = parent;

  if (nodeKind == store::StoreConsts::attributeNode)
    reinterpret_cast<ElementNode*>(parent)->insertAttr(this, pos);
  else
    parent->insertChild(this, pos);

  for (csize i = 0; i < moved.size(); ++i)
  {
    if (moved[i]->getNodeKind() != store::StoreConsts::elementNode)
      continue;

    ElementNode* elem = reinterpret_cast<ElementNode*>(moved[i]);

    // Maintain recursiveness info: check if the current node is a descendant
    // of another node with the same name.
    XmlNode* ancestor = parent;
    while (ancestor != NULL &&
           ancestor->getNodeKind() == store::StoreConsts::elementNode)
    {
      ElementNode* elemAncestor = reinterpret_cast<ElementNode*>(ancestor);
      if (elemAncestor->theName->equals(elem->theName))
      {
        elemAncestor->setRecursive();
        break;
      }

      ancestor = ancestor->theParent;
    }
  }

  // baseUris keeps the old values, for undoAttach().
  for (csize i = 0; i < baseUris.size(); ++i)
  {
    AttributeNode* attr = baseUris[i].first;

    if (baseUris[i].second != NULL)
    {
      store::Item_t value;
      value.transfer(baseUris[i].second);
      baseUris[i].second = attr->theTypedValue;
      attr->setTypedValue(value);
    }
    else
    {
      ElementNode* elem = reinterpret_cast<ElementNode*>(attr->theParent);
      attr->disconnect(record.theRemovedBaseUriPos);
      record.theRemovedBaseUri = attr;
      elem->resetHaveBaseUri();
    }
  }

  record.theOldTree = oldTree;
  record.theMoved.swap(moved);
  record.theBaseUris.swap(baseUris);
}


/*******************************************************************************
  Undo attach(), from what it recorded and without allocating: undoing an
  update cannot fail. The node goes back to its old tree, with its old types,
  base URIs and namespace context. Node ids stay as attach() made them.
********************************************************************************/
void XmlNode::undoAttach(AttachRecord& record)
{
  ZORBA_ASSERT(theParent != NULL && record.theOldTree != NULL);

  XmlTree* newTree = getTree();
  XmlTree* oldTree = record.theOldTree;

  SYNC_CODE(oldTree->getRCLock()->acquire());
  SYNC_CODE(newTree->getRCLock()->acquire());

  long refcount = 0;

  for (csize i = 0; i < record.theMoved.size(); ++i)
  {
    XmlNode* node = record.theMoved[i];
    refcount += node->theRefCount.load();
    if (node != record.theRemovedBaseUri && node->typeToMove() != NULL)
      newTree->removeType(node);
    node->setTreeInternal(oldTree);
  }

  for (csize i = 0; i < record.theBaseUris.size(); ++i)
  {
    if (record.theBaseUris[i].second != NULL)
      record.theBaseUris[i].first->setTypedValue(record.theBaseUris[i].second);
  }

  if (record.theRemovedBaseUri != NULL)
  {
    record.theRemovedBaseUri->connect(static_cast<InternalNode*>(this),
                                      record.theRemovedBaseUriPos);
    static_cast<ElementNode*>(this)->setHaveBaseUri();
    record.theRemovedBaseUri = NULL;
  }

  if (record.theOldNsContext != NULL)
    static_cast<ElementNode*>(this)->theNsContext.transfer(
      record.theOldNsContext);

  if (getNodeKind() == store::StoreConsts::attributeNode)
    theParent->removeAttr(this);
  else
    theParent->removeChild(this);
  theParent = NULL;

  oldTree->setRoot(this);
  record.theOldTree = NULL;

  newTree->getRefCount() -= refcount;
  oldTree->getRefCount() += refcount;
  SYNC_CODE(newTree->getRCLock()->release());
  SYNC_CODE(oldTree->getRCLock()->release());
}


/*******************************************************************************
  For attach(): the nodes of the subtree rooted at "this" ("this" first) and
  the changes to the hidden base-uri attributes of its elements once it is
  under parent: one with an explicit xml:base is re-resolved against parent's
  base URI; one without (only the old root has such) is removed.
********************************************************************************/
void XmlNode::collectMoved(
    const InternalNode* parent,
    std::vector<XmlNode*>& moved,
    std::vector<std::pair<AttributeNode*, store::Item_t> >& baseUris)
{
  moved.push_back(this);

  for (csize next = 0; next < moved.size(); ++next)
  {
    if (moved[next]->getNodeKind() != store::StoreConsts::elementNode)
      continue;

    ElementNode* elem = reinterpret_cast<ElementNode*>(moved[next]);

    AttributeNode* baseUriAttr = NULL;
    AttributeNode* hiddenBaseUriAttr = NULL;

    InternalNode::iterator ite = elem->attrsBegin();
    InternalNode::iterator end = elem->attrsEnd();

    for (; ite != end; ++ite)
    {
      AttributeNode* attr = static_cast<AttributeNode*>(*ite);

      moved.push_back(attr);

      if (attr->isBaseUri())
      {
        if (attr->isHidden())
          hiddenBaseUriAttr = attr;
        else
          baseUriAttr = attr;
      }
    }

    if (hiddenBaseUriAttr)
    {
      store::Item_t value;

      if (baseUriAttr)
      {
        zstring absuri;
        zstring reluri;
        parent->getBaseURI(absuri);
        baseUriAttr->getBaseURI(reluri);
        ElementNode::resolveBaseUri(absuri, reluri, value);
      }
      else
      {
        ZORBA_ASSERT(elem == this);
      }

      baseUris.push_back(std::make_pair(hiddenBaseUriAttr, value));
    }

    ite = elem->childrenBegin();
    end = elem->childrenEnd();

    for (; ite != end; ++ite)
      moved.push_back(*ite);
  }
}


/*******************************************************************************
  For attach(): the type that must move with "this" to another tree, or NULL.
********************************************************************************/
store::Item_t XmlNode::typeToMove() const
{
  store::StoreConsts::NodeKind kind = getNodeKind();

  if ((kind == store::StoreConsts::elementNode &&
       static_cast<const ElementNode*>(this)->haveType()) ||
      (kind == store::StoreConsts::attributeNode &&
       static_cast<const AttributeNode*>(this)->haveType()))
    return getType();

  return NULL;
}


/*******************************************************************************
  For the subtree rooted at "this" element, whose own new label is already
  set, compute the new label of each descendant and attribute (as attach()
  sets them) into labels, and pair each node with its label in labelOf.
********************************************************************************/
void XmlNode::computeSubtreeLabels(
    std::deque<NodeLabel>& labels,
    std::vector<std::pair<LabeledNode*, NodeLabel*> >& labelOf) const
{
  std::stack<std::pair<const ElementNode*, const NodeLabel*> > elems;
  elems.push(std::make_pair(reinterpret_cast<const ElementNode*>(this),
                            &static_cast<const LabeledNode*>(this)->theLabel));

  while (!elems.empty())
  {
    const ElementNode* elem = elems.top().first;
    const NodeLabel* elemLabel = elems.top().second;
    elems.pop();

    csize numAttrs = elem->numAttrs();

    InternalNode::const_iterator ite = elem->attrsBegin();
    InternalNode::const_iterator end = elem->attrsEnd();

    for (csize i = 0; ite != end; ++i, ++ite)
    {
      labels.push_back(NodeLabel());
      labels.back().setAsChild(*elemLabel, i);
      labelOf.push_back(std::make_pair(static_cast<LabeledNode*>(*ite),
                                       &labels.back()));
    }

    ite = elem->childrenBegin();
    end = elem->childrenEnd();

    for (csize i = 0; ite != end; ++i, ++ite)
    {
      XmlNode* child = (*ite);

#ifndef TEXT_ORDPATH
      if (child->getNodeKind() == store::StoreConsts::textNode)
        continue;
#endif

      labels.push_back(NodeLabel());
      labels.back().setAsChild(*elemLabel, numAttrs + i);
      labelOf.push_back(std::make_pair(static_cast<LabeledNode*>(child),
                                       &labels.back()));

      if (child->getNodeKind() == store::StoreConsts::elementNode)
        elems.push(std::make_pair(reinterpret_cast<const ElementNode*>(child),
                                  &labels.back()));
    }
  }
}


/*******************************************************************************
  Undo the first numApplied attaches of upd, last first.
********************************************************************************/
void InternalNode::undoAttaches(UpdatePrimitive& upd, csize numApplied)
{
  for (csize i = numApplied; i-- > 0; )
  {
    AttachRecord& record = upd.theAttachRecords[i];
    record.theMoved[0]->undoAttach(record);
  }
}


/*******************************************************************************
  Detach a text node into tree, an empty tree made beforehand: no allocation.
********************************************************************************/
void XmlNode::detachText(XmlTree* tree)
{
  ZORBA_ASSERT(getNodeKind() == store::StoreConsts::textNode &&
               !static_cast<TextNode*>(this)->isTyped());

  XmlTree* oldTree = getTree();

  SYNC_CODE(oldTree->getRCLock()->acquire());
  SYNC_CODE(tree->getRCLock()->acquire());

  setTreeInternal(tree);
  tree->setRoot(this);

  theParent->removeChild(this);
  theParent = NULL;

  long const refcount = theRefCount.load();
  oldTree->getRefCount() -= refcount;
  tree->getRefCount() += refcount;
  bool const unused = (tree->getRefCount() == 0);
  SYNC_CODE(tree->getRCLock()->release());
  SYNC_CODE(oldTree->getRCLock()->release());

  if (unused)
    tree->free();
}


/*******************************************************************************
  Make what finishDetach() needs, while "this" has its former parent: the new
  tree with the moved nodes' types, the namespace contexts that keep the
  in-scope bindings, the hidden base-uri attribute that keeps an inherited
  base URI, and a typed text node's text. This is where detaching can fail.
********************************************************************************/
void XmlNode::prepareDetach(DetachRecord& record)
{
  record.theNewTree = GET_STORE().getNodeFactory().createXmlTree();

  store::StoreConsts::NodeKind nodeKind = getNodeKind();

  if (nodeKind == store::StoreConsts::textNode)
  {
    record.theMoved.push_back(this);
    TextNode* text = static_cast<TextNode*>(this);
    if (text->isTyped())
    {
      text->getStringValue2(record.theText);
      record.theRevertText = true;
    }
  }
  else if (nodeKind != store::StoreConsts::elementNode)
  {
    record.theMoved.push_back(this);
  }
  else
  {
    ElementNode* root = static_cast<ElementNode*>(this);

    bool localBaseUri;
    zstring baseUri;
    getBaseURIInternal(baseUri, localBaseUri);
    if (!localBaseUri)
      theParent->getBaseURI(baseUri);
    if (!localBaseUri && !baseUri.empty())
    {
      const Store& store = GET_STORE();
      store::Item_t qname;
      store.getQNamePool().insert(qname, store.XML_URI, "xml", "base");
      store::Item_t typeName = store.theSchemaTypeNames[store::XS_ANY_URI];
      store::Item_t typedValue;
      GET_FACTORY().createAnyURI(typedValue, baseUri);

      // The root of the new tree until finishDetach() uses it.
      record.theBaseUriAttr = GET_NODE_FACTORY().createAttributeNode(
        record.theNewTree, NULL, false, 0, qname, typeName, typedValue,
        false, true);

      if (root->theNodes.size() == root->theNodes.capacity())
        root->theNodes.reserve(2 * root->theNodes.size() + 1);
    }

    // Which elements get a context of their own, which take their parent's.
    std::vector<std::pair<XmlNode*, NsBindingsContext*> > pending;
    pending.push_back(std::make_pair(this, theParent->getNsContext()));

    while (!pending.empty())
    {
      XmlNode* node = pending.back().first;
      NsBindingsContext* parentNsContext = pending.back().second;
      pending.pop_back();

      record.theMoved.push_back(node);

      if (node->getNodeKind() != store::StoreConsts::elementNode)
        continue;

      ElementNode* elem = static_cast<ElementNode*>(node);
      NsBindingsContext* nsContext = elem->getNsContext();

      if (elem == root ||
          nsContext == NULL ||
          (elem->haveLocalBindings() && nsContext->getParent() != parentNsContext) ||
          nsContext != parentNsContext)
      {
        if (nsContext != NULL)
        {
          rchandle<NsBindingsContext> ctx = new NsBindingsContext();
          elem->getNamespaceBindings(ctx->getBindings());

          if (!ctx->empty())
            record.theOwnContexts.push_back(std::make_pair(elem, ctx));
        }
      }
      else
      {
        record.theInheriting.push_back(elem);
      }

      InternalNode::iterator ite = elem->attrsBegin();
      InternalNode::iterator end = elem->attrsEnd();
      for (; ite != end; ++ite)
        record.theMoved.push_back(*ite);

      ite = elem->childrenBegin();
      end = elem->childrenEnd();
      for (; ite != end; ++ite)
        pending.push_back(std::make_pair(*ite, nsContext));
    }
  }

  for (csize i = 0; i < record.theMoved.size(); ++i)
  {
    store::Item_t type = record.theMoved[i]->typeToMove();
    if (type != NULL)
      record.theNewTree->addType(record.theMoved[i], type);
  }
}


/*******************************************************************************
  Detach "this" with what prepareDetach() made, without allocating: it runs after
  the updates are committed, and cannot fail. "this" has its former parent
  set again (see CollectionPul::finalizeUpdates()).
********************************************************************************/
void XmlNode::finishDetach(DetachRecord& record)
{
  ZORBA_ASSERT(theParent != NULL && record.theNewTree != NULL);

  XmlTree* oldTree = getTree();
  XmlTree* newTree = record.theNewTree;
  record.theNewTree = NULL;

  SYNC_CODE(oldTree->getRCLock()->acquire());
  SYNC_CODE(newTree->getRCLock()->acquire());

  newTree->setRoot(this);
  if (oldTree->getRoot() == this)
    oldTree->setRoot(NULL);

  long refcount = 0;

  // A node that a later update removed from the subtree stays where it is.
  // theMoved has parents before their children and attributes.
  for (csize i = 0; i < record.theMoved.size(); ++i)
  {
    XmlNode* node = record.theMoved[i];
    bool const stays = node != this &&
      (node->theParent == NULL || node->theParent->getTree() != newTree);

    if (stays)
    {
      if (node->typeToMove() != NULL)
        newTree->removeType(node);
      continue;
    }

    refcount += node->theRefCount.load();
    if (node->typeToMove() != NULL)
      node->getTree()->removeType(node);
    node->setTreeInternal(newTree);
  }

  if (record.theRevertText)
  {
    TextNode* text = static_cast<TextNode*>(this);
    text->setValue(NULL);
    text->setText(record.theText);
    text->theFlags &= ~IsTyped;
  }

  if (record.theBaseUriAttr != NULL)
  {
    record.theBaseUriAttr->connect(static_cast<InternalNode*>(this), 0);
    static_cast<ElementNode*>(this)->setHaveBaseUri();
    record.theBaseUriAttr = NULL;
  }

  for (csize i = 0; i < record.theOwnContexts.size(); ++i)
  {
    ElementNode* elem = record.theOwnContexts[i].first;
    if (elem->getTree() != newTree)
      continue;
    elem->theNsContext.transfer(record.theOwnContexts[i].second);
    elem->theFlags |= HaveLocalBindings;
  }

  for (csize i = 0; i < record.theInheriting.size(); ++i)
  {
    ElementNode* elem = record.theInheriting[i];
    if (elem->getTree() != newTree)
      continue;
    elem->setNsContext(elem->theParent->getNsContext());
  }

  if (getNodeKind() == store::StoreConsts::attributeNode)
    theParent->removeAttr(this);
  else
    theParent->removeChild(this);
  theParent = NULL;

  newTree->getRefCount() += refcount;
  oldTree->getRefCount() -= refcount;
  bool const newUnused = (newTree->getRefCount() == 0);
  bool const oldUnused = (oldTree->getRefCount() == 0);
  SYNC_CODE(newTree->getRCLock()->release());
  SYNC_CODE(oldTree->getRCLock()->release());

  // Freed with no tree lock held: a tree's destruction takes the store's
  // references mutex, which is taken before a tree lock (getNodeByReference).
  if (newUnused)
    newTree->free();
  if (oldUnused)
    oldTree->free();
}


/*******************************************************************************

********************************************************************************/
DetachRecord::DetachRecord(DetachRecord&& other) noexcept
  :
  theNewTree(other.theNewTree),
  theBaseUriAttr(other.theBaseUriAttr),
  theRevertText(other.theRevertText)
{
  theMoved.swap(other.theMoved);
  theOwnContexts.swap(other.theOwnContexts);
  theInheriting.swap(other.theInheriting);
  theText.swap(other.theText);
  other.theNewTree = NULL;
  other.theBaseUriAttr = NULL;
}


DetachRecord::~DetachRecord()
{
  // The base-uri attribute, if any, is the root of the unused tree.
  if (theNewTree != NULL)
    theNewTree->free();
}


/*******************************************************************************

********************************************************************************/
AttachRecord::AttachRecord(AttachRecord&& other) noexcept
  :
  theOldTree(other.theOldTree),
  theRemovedBaseUri(other.theRemovedBaseUri),
  theRemovedBaseUriPos(other.theRemovedBaseUriPos)
{
  theMoved.swap(other.theMoved);
  theBaseUris.swap(other.theBaseUris);
  theOldNsContext.transfer(other.theOldNsContext);
  other.theOldTree = NULL;
  other.theRemovedBaseUri = NULL;
}


AttachRecord::~AttachRecord()
{
  if (theOldTree != NULL)
    theOldTree->free();

  if (theRemovedBaseUri != NULL)
    theRemovedBaseUri->destroy(false);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Node Update Primitives                                                     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************

********************************************************************************/
void XmlNode::removeType(UpdatePrimitive& upd)
{
  TypeUndoList& undoList = upd.theTypeUndoList;
  csize undoSize = undoList.size();

  zorba::store::Item* revalidationNode = NULL;
  XmlNode* currNode = this;

  while (currNode != NULL)
  {
    NodeTypeInfo tinfo;
    tinfo.theNode = currNode;

    store::StoreConsts::NodeKind nodeKind = currNode->getNodeKind();

    if (nodeKind == store::StoreConsts::elementNode)
    {
      ElementNode* n = reinterpret_cast<ElementNode*>(currNode);

      if (n->getType()->equals(GET_STORE().XS_UNTYPED_QNAME))
      {
        revalidationNode = currNode;
        break;
      }

      if (n->getType()->equals(GET_STORE().XS_ANY_QNAME))
      {
        // The type stays, but the nilled property is removed with it
        if (n->getNilled())
        {
          tinfo.theTypeName = n->getType();
          tinfo.theFlags = n->theFlags;
          undoList.resize(++undoSize);
          undoList[undoSize - 1].transfer(tinfo);
          n->resetNilled();
        }

        revalidationNode = currNode;
        break;
      }

#ifdef EMBEDED_TYPE
      tinfo.theTypeName.transfer(n->theTypeName);
      n->theTypeName = GET_STORE().XS_ANY_QNAME;
#else
      assert(n->haveType());
      tinfo.theTypeName = n->getType();
      n->setType(GET_STORE().XS_ANY_QNAME);
#endif
      tinfo.theFlags = n->theFlags;

      TextNode* textChild;

      if (n->haveTypedTypedValue(textChild))
      {
        zstring textValue;
        textChild->getStringValue2(textValue);

        tinfo.theChildFlags = textChild->theFlags;
        tinfo.theTextContent.setValue(textChild->theContent.releaseValue());

        textChild->theFlags &= ~IsTyped;
        textChild->setText(textValue);
      }

      undoList.resize(++undoSize);
      undoList[undoSize - 1].transfer(tinfo);

      n->setHaveTypedValue();
      n->resetHaveEmptyTypedValue();
      n->resetInSubstGroup();
      n->resetNilled();

      revalidationNode = currNode;
    }
    else if (nodeKind == store::StoreConsts::attributeNode)
    {
      AttributeNode* n = reinterpret_cast<AttributeNode*>(currNode);

      if (n->getType()->equals(GET_STORE().theSchemaTypeNames[store::XS_UNTYPED_ATOMIC]))
      {
        undoList.resize(++undoSize);
        undoList[undoSize - 1].transfer(tinfo);
      }
      else
      {
#ifdef EMBEDED_TYPE
        tinfo.theTypeName.transfer(n->theTypeName);
        n->theTypeName = GET_STORE().theSchemaTypeNames[store::XS_UNTYPED_ATOMIC];
#else
        assert(n->haveType());
        tinfo.theTypeName = n->getType();
        // restoreType() puts back the type map entry that this takes out.
        n->getTree()->reserveTypes();
        n->setType(GET_STORE().theSchemaTypeNames[store::XS_UNTYPED_ATOMIC]);
#endif

        tinfo.theFlags = n->theFlags;

        undoList.resize(++undoSize);
        undoList[undoSize - 1].transfer(tinfo);

        n->resetHaveListValue();
      }
    }
    else if (nodeKind == store::StoreConsts::documentNode)
    {
      revalidationNode = currNode;
      break;
    }
    else
    {
      ZORBA_FATAL(0, "");
    }

    currNode = currNode->theParent;
  }

  if (revalidationNode != NULL)
  {
    while (revalidationNode->getParent() != NULL)
      revalidationNode = revalidationNode->getParent();

    upd.addNodeForValidation(revalidationNode);
  }
}


/*******************************************************************************

********************************************************************************/
void XmlNode::restoreType(TypeUndoList& undoList)
{
  if (undoList.empty())
    return;

  ulong pos = 0;
  ulong numNodes = (ulong)undoList.size();
  XmlNode* currNode = this;

  while(currNode != NULL && pos < numNodes)
  {
    ZORBA_FATAL(currNode == undoList[pos].theNode, "");
 
    store::StoreConsts::NodeKind nodeKind = currNode->getNodeKind();

    NodeTypeInfo& tinfo = undoList[pos];

    if (nodeKind == store::StoreConsts::elementNode)
    {
      ElementNode* n = reinterpret_cast<ElementNode*>(currNode);

      n->setType(tinfo.theTypeName);
      n->theFlags = tinfo.theFlags;

      if (tinfo.theChildFlags & XmlNode::IsTyped)
      {
        TextNode* textChild = n->getUniqueTextChild();

        if (!textChild->isTyped())
        {
          textChild->theFlags = tinfo.theChildFlags;
          textChild->destroyText();
          textChild->setValue(tinfo.theTextContent.releaseValue());
        }
        else
        {
          ZORBA_ASSERT(textChild->getValue()->equals(
                       tinfo.theTextContent.getValue()));
        }
      }
    }
    else if (nodeKind == store::StoreConsts::attributeNode)
    {
      AttributeNode* n = reinterpret_cast<AttributeNode*>(currNode);

      if (tinfo.theTypeName != NULL)
      {
        n->setType(tinfo.theTypeName);
        n->theFlags = tinfo.theFlags;
      }
    }
    else if (nodeKind == store::StoreConsts::documentNode)
    {
      break;
    }
    else
    {
      ZORBA_FATAL(0, "");
    }

    currNode = currNode->theParent;
    ++pos;
  }
}


/*******************************************************************************

********************************************************************************/
void InternalNode::deleteChild(UpdDelete& upd)
{
  XmlNode* child = BASE_NODE(upd.theTarget);

  // For now, just diconnect but do not detach the child. This way, undo becomes
  // easy as we just have to connect the child back to it parent. The child will
  // be detached later, after all updates have been applied with no errors (see.
  // applyUpdates() method). 
  csize pos;
  if (! child->disconnect(pos))
    return;

  upd.thePos = pos;
  upd.theIsApplied = true;

  if (upd.theRemoveType)
    removeType(upd);

  // Check if two text nodes might become adjacent due to this delete
  store::StoreConsts::NodeKind childKind = child->getNodeKind();
  store::StoreConsts::NodeKind parentKind = getNodeKind();

  if (childKind == store::StoreConsts::attributeNode ||
      (parentKind != store::StoreConsts::elementNode &&
       parentKind != store::StoreConsts::documentNode))
    return;

  // Note: numChildren() returns the number of children AFTER the child deletion
  if (pos == 0 || pos == numChildren())
    return;

  XmlNode* rsib = getChild(pos);
  XmlNode* lsib = getChild(pos - 1);

  if (lsib->getNodeKind() != store::StoreConsts::textNode ||
      rsib->getNodeKind() != store::StoreConsts::textNode)
    return;

  upd.theCollectionPul->addToCheckForMerge(this);
}


void InternalNode::restoreChild(UpdDelete& upd)
{
  if (!upd.theIsApplied)
    return;

  XmlNode* child = BASE_NODE(upd.theTarget);
  csize pos = upd.thePos;

  child->connect(this, pos);

  if (!upd.theTypeUndoList.empty())
    restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  Insert a set of new nodes as children of "this" node. The new children must be
  inserted in the order they appear in the given vector (newChildren). Let S and
  E be the first and last nodes in this sequence. The new nodes will be injected
  between the current children of "this" at positions pos - 1 and pos.
  Let L and R be the children at these positions (L is NULL if startPos == 0, and
  R is NULL if startPos == this->numChildren()). If L and S are both text nodes,
  S must be merged later into L. Similarly, if E and R are both text nodes, E
  must be merged later into R. It is assumed that the newChildren sequence does 
  not contain any 2 adjacent text nodes.

  Note: L and R cannot both be text nodes, so at most one merging of text nodes
  will be performed.
********************************************************************************/
void InternalNode::insertChildren(UpdInsertChildren& upd, csize pos)
{
  assert(pos <= numChildren());

  csize numNewChildren = upd.theNewChildren.size();
  XmlNode* rsib = (pos < numChildren() ? getChild(pos) : NULL);
  XmlNode* lsib = (pos > 0 ? getChild(pos-1) : NULL);

  // Do this before inserting the new children, so that if "this" has a
  // single typed text child, its typed value will be saved and the child
  // will be converted to an untyped text child.
  if (upd.theRemoveType)
    removeType(upd);

  upd.theAttachRecords.resize(numNewChildren);

  // Insert the new children without merging text nodes
  for (ulong i = 0; i < numNewChildren; ++i)
  {
    XmlNode* child = BASE_NODE(upd.theNewChildren[i]);

    child->attach(this, pos + i, upd.theAttachRecords[i]);
    
    if (getNodeKind() == store::StoreConsts::elementNode &&
        child->getNodeKind() == store::StoreConsts::elementNode &&
        upd.thePul->inheritNSBindings())
    {
      ElementNode* elem = static_cast<ElementNode*>(child);
      upd.theAttachRecords[i].theOldNsContext = elem->theNsContext;
      elem->setNsContext(static_cast<ElementNode*>(this)->getNsContext());
    }

    upd.theNumApplied++;
  }

  // Check if there are any adjacent text nodes on the left or right boundary
  XmlNode* firstNew = BASE_NODE(upd.theNewChildren[0]);
  XmlNode* lastNew = BASE_NODE(upd.theNewChildren[numNewChildren-1]);

  if ((lsib != NULL &&
       lsib->getNodeKind() == store::StoreConsts::textNode &&
       firstNew->getNodeKind() == store::StoreConsts::textNode)
      ||
      (rsib != NULL &&
       rsib->getNodeKind() == store::StoreConsts::textNode &&
       lastNew->getNodeKind() == store::StoreConsts::textNode))
  {
	  upd.theCollectionPul->addToCheckForMerge(this);
  }
}


/*******************************************************************************
  ELEMENT/TEXT/PI/COMMENT - INSERT SIBLINGS BEFORE
********************************************************************************/
void XmlNode::insertSiblingsBefore(UpdInsertChildren& upd)
{
  ZORBA_FATAL(theParent, "");

  InternalNode* parent = static_cast<InternalNode*>(upd.theSibling->getParent());

  csize pos = parent->findChild(this);

  parent->insertChildren(upd, pos);
}


/*******************************************************************************
  ELEMENT/TEXT/PI/COMMENT - INSERT SIBLINGS AFTER
********************************************************************************/
void XmlNode::insertSiblingsAfter(UpdInsertChildren& upd)
{
  ZORBA_FATAL(theParent, "");

  InternalNode* parent = static_cast<InternalNode*>(upd.theSibling->getParent());

  csize pos = parent->findChild(this);
  pos++;

  parent->insertChildren(upd, pos);
}


/*******************************************************************************

********************************************************************************/
void InternalNode::undoInsertChildren(UpdInsertChildren& upd)
{
  if (upd.theNumApplied == 0)
    return;

  csize pos = findChild(BASE_NODE(upd.theNewChildren[0]));

  ZORBA_FATAL(pos < numChildren(), "");

  undoAttaches(upd, upd.theNumApplied);

  restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ELEMENT - INSERT ATTRIBUTES
********************************************************************************/
void ElementNode::insertAttributes(UpdInsertAttributes& upd)
{
  csize numAttrs = this->numAttrs();
  csize numNewAttrs = upd.theNewAttrs.size();

  removeType(upd);

  upd.theAttachRecords.resize(numNewAttrs);

  for (csize i = 0; i < numNewAttrs; i++)
  {
    AttributeNode* attr = reinterpret_cast<AttributeNode*>(
                          upd.theNewAttrs[i].getp());
    try
    {
      checkUniqueAttr(attr->getNodeName());
    }
    catch (ZorbaException const& e)
    {
      if (e.diagnostic() == err::XQDY0025)
        upd.theCollectionPul->thePrimitivesToRecheck.push_back(&upd);
      else
        throw;
    }

    if (addBindingForQName(attr->theName, true, false))
    {
      upd.theNewBindings.push_back(attr->theName);
    }

    attr->attach(this, numAttrs + i, upd.theAttachRecords[i]);

    if (!upd.thePul->inheritNSBindings())
    {
      csize numChildren = this->numChildren();

      for (csize i = 0; i < numChildren; ++i)
      {
        if (getChild(i)->getNodeKind() == store::StoreConsts::elementNode)
        {
          static_cast<ElementNode*>(getChild(i))->
            uninheritBinding(theNsContext, attr->theName->getPrefix());
        }
      }  
    }

    upd.theNumApplied++;
  }
}


void ElementNode::undoInsertAttributes(UpdInsertAttributes& upd)
{
  if (upd.theNumApplied == 0)
    return;

  csize pos = findAttr(BASE_NODE(upd.theNewAttrs[0]));

  ZORBA_FATAL(pos < numAttrs(), "");

  undoAttaches(upd, upd.theNumApplied);

  csize numBindings = upd.theNewBindings.size();
  for (csize i = 0; i < numBindings; ++i)
  {
    removeLocalBinding(upd.theNewBindings[i]->getPrefix(),
                       upd.theNewBindings[i]->getNamespace());
  }

  restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ELEMENT - REPLACE ATTRIBUTE
********************************************************************************/
void ElementNode::replaceAttribute(UpdReplaceAttribute& upd)
{
  removeType(upd);

  AttributeNode* oldAttr = ATTR_NODE(upd.theAttr);

  csize pos;
  oldAttr->disconnect(pos);

  upd.thePos = pos;

  csize numNewAttrs = upd.theNewAttrs.size();

  upd.theAttachRecords.resize(numNewAttrs);

  for (csize i = 0; i < numNewAttrs; ++i)
  {
    AttributeNode* attr = static_cast<AttributeNode*>(upd.theNewAttrs[i].getp());
    try
    {
      checkUniqueAttr(attr->getNodeName());
    }
    catch (ZorbaException const& e)
    {
      if (e.diagnostic() == err::XQDY0025)
        upd.theCollectionPul->thePrimitivesToRecheck.push_back(&upd);
      else
        throw;
    }

    bool newBinding = addBindingForQName(attr->theName, true, false);

    if (newBinding)
      upd.theNewBindings.push_back(attr->theName);
  
    attr->attach(this, pos + i, upd.theAttachRecords[i]);

    if (newBinding && !upd.thePul->inheritNSBindings())
    {
      iterator ite = childrenBegin();
      iterator end = childrenEnd();

      for (; ite != end; ++ite)
      {
        if ((*ite)->getNodeKind() == store::StoreConsts::elementNode)
        {
          static_cast<ElementNode*>((*ite))->
          uninheritBinding(theNsContext, attr->theName->getPrefix());
        }
      }  
    }

    upd.theNumApplied++;
  }
}


void ElementNode::restoreAttribute(UpdReplaceAttribute& upd)
{
  if (upd.theNumApplied == 0)
  {
    if (upd.theIsApplied)
    {
      XmlNode* attr = BASE_NODE(upd.theAttr);
      attr->connect(this, upd.thePos);
      restoreType(upd.theTypeUndoList);
    }

    return;
  }

  XmlNode* attr = BASE_NODE(upd.theAttr);

  csize pos = findAttr(BASE_NODE(upd.theNewAttrs[0]));

  ZORBA_FATAL(pos < numAttrs(), "");

  undoAttaches(upd, upd.theNumApplied);

  csize numBindings = upd.theNewBindings.size();
  for (csize i = 0; i < numBindings; ++i)
    removeLocalBinding(upd.theNewBindings[i]->getPrefix(),
                       upd.theNewBindings[i]->getNamespace());

  attr->connect(this, pos);

  restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ELEMENT/DOCUMENT - REPLACE CHILD
********************************************************************************/
void InternalNode::replaceChild(UpdReplaceChild& upd)
{
  XmlNode* child = BASE_NODE(upd.theChild);

  csize numNewChildren = upd.theNewChildren.size();

  if (numNewChildren == 0)
  {
    csize pos;
    bool found = child->disconnect(pos);
    ZORBA_ASSERT(found);

    upd.thePos = pos;
    upd.theIsApplied = true;

    if (upd.theRemoveType)
      removeType(upd);
  
    // Note: numChildren() returns the number of children AFTER the child deletion
    if (pos == 0 || pos == numChildren())
      return;

    XmlNode* rsib = getChild(pos);
    XmlNode* lsib = getChild(pos - 1);

    if (lsib->getNodeKind() != store::StoreConsts::textNode ||
        rsib->getNodeKind() != store::StoreConsts::textNode)
      return;

    upd.theCollectionPul->addToCheckForMerge(this);
  }
  else
  {
    csize pos = findChild(child);

    if (pos >= numChildren())
      return;

    upd.thePos = pos;
    upd.theIsApplied = true;

    if (upd.theRemoveType)
      removeType(upd);

    XmlNode* rsib = (pos < numChildren() - 1 ? getChild(pos+1) : NULL);
    XmlNode* lsib = (pos > 0 ? getChild(pos-1) : NULL);

    upd.theAttachRecords.resize(numNewChildren);

    // Insert the new children without merging text nodes
    for (csize i = 0; i < numNewChildren; ++i)
    {
      XmlNode* child = BASE_NODE(upd.theNewChildren[i]);

      child->attach(this, pos + i, upd.theAttachRecords[i]);

      if (getNodeKind() == store::StoreConsts::elementNode &&
          child->getNodeKind() == store::StoreConsts::elementNode &&
          upd.thePul->inheritNSBindings())
      {
        ElementNode* elem = static_cast<ElementNode*>(child);
        upd.theAttachRecords[i].theOldNsContext = elem->theNsContext;
        elem->setNsContext(static_cast<ElementNode*>(this)->getNsContext());
      }
      
      upd.theNumApplied++;
    }
    
    // Remove the child that is being replaced
    removeChild(pos + numNewChildren);

    // Check if there are any adjacent text nodes on the left or right boundary
    XmlNode* firstNew = BASE_NODE(upd.theNewChildren[0]);
    XmlNode* lastNew = BASE_NODE(upd.theNewChildren[numNewChildren-1]);

    if  ((lsib != NULL &&
          lsib->getNodeKind() == store::StoreConsts::textNode &&
          firstNew->getNodeKind() == store::StoreConsts::textNode)
         ||
         (rsib != NULL &&
          rsib->getNodeKind() == store::StoreConsts::textNode &&
          lastNew->getNodeKind() == store::StoreConsts::textNode))
    {
      upd.theCollectionPul->addToCheckForMerge(this);
    }
  }
}


void InternalNode::restoreChild(UpdReplaceChild& upd)
{
  XmlNode* child = BASE_NODE(upd.theChild);

  csize numNewChildren = upd.theNewChildren.size();

  if (numNewChildren == 0)
  {
    if (!upd.theIsApplied)
      return;

    csize pos = upd.thePos;
    child->connect(this, pos);
  }
  else
  {
    if (upd.theNumApplied == 0)
      return;

    csize pos = upd.thePos;

    ZORBA_ASSERT(getChild(pos) == upd.theNewChildren[0].getp());

    undoAttaches(upd, upd.theNumApplied);

    child->connect(this, pos);
  }

  if (upd.theRemoveType)
    restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ELEMENT - REPLACE CONTENT
********************************************************************************/
void ElementNode::replaceContent(UpdReplaceElemContent& upd)
{
  // The list of the old children, and room for the new child, are made before
  // the old children are taken out. theNodes keeps its capacity, so a failure
  // after that puts the old children back without allocating.
  upd.theOldChildren.insert(upd.theOldChildren.begin(),
                            childrenBegin(),
                            childrenEnd());

  theNodes.reserve(theNumAttrs + 1);
  theNodes.resize(theNumAttrs);

  try
  {
    replaceChildrenWithText(upd);
  }
  catch (...)
  {
    restoreContent(upd);
    throw;
  }
}


void ElementNode::replaceChildrenWithText(UpdReplaceElemContent& upd)
{
  if (upd.theNewChild == NULL || upd.theNewChild->getStringValue().empty())
    return;

#ifndef ZORBA_NO_XMLSCHEMA
  if ( upd.thePul->getValidator()->isPossibleSimpleContentRevalidation(getType()) )
  {
    std::vector<store::Item_t> result;
    upd.thePul->getValidator()->validateSimpleContent(getType(), 
                                                      upd.theNewChild->getStringValue(),
                                                      result);
    
    GENV_ITEMFACTORY->createTextNode(upd.theNewChild, this, result);
  }
  else
#endif
  {
    TextNode* newChild = TEXT_NODE(upd.theNewChild);
    
    XmlTree* oldTree = newChild->getTree();
    XmlTree* newTree = getTree();
    
    delete oldTree;
    
    SYNC_CODE(newTree->getRCLock()->acquire());
    newTree->getRefCount() += 1;
    SYNC_CODE(newTree->getRCLock()->release());
    
    newChild->setTree(newTree);
#ifdef TEXT_ORDPATH
    newChild->setLabel(this, true, 0, store::StoreConsts::textNode);
#endif
    newChild->connect(this, 0);
    
    upd.theNewChild = NULL;
    
    removeType(upd);
  }
}


void ElementNode::restoreContent(UpdReplaceElemContent& upd)
{
  if (numChildren() > 0)
  {
    ZORBA_FATAL(numChildren() == 1, "");

    XmlNode* child = getChild(0);
 
    child->destroy(true);
  }

  ElementNode* target1 = reinterpret_cast<ElementNode*>(this);
  target1->theNodes.insert(childrenBegin(),
                           upd.theOldChildren.begin(),
                           upd.theOldChildren.end()); 

  restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ELEMENT - RENAME
********************************************************************************/
void ElementNode::replaceName(UpdRenameElem& upd)
{
  if (upd.theNewName->equals(theName))
    return;

  upd.theNewBinding = addBindingForQName(upd.theNewName, false, false);

  upd.theOldName.transfer(theName);
  theName.transfer(upd.theNewName);

  if (upd.theNewBinding && 
      (!upd.thePul->inheritNSBindings() ||  theName->getPrefix().empty()))
  {
    iterator ite = childrenBegin();
    iterator end = childrenEnd();

    for (; ite != end; ++ite)
    {
      if ((*ite)->getNodeKind() == store::StoreConsts::elementNode)
      {
        static_cast<ElementNode*>((*ite))->
        uninheritBinding(theNsContext, theName->getPrefix());
      }
    }  
  }

  store::Item* typeName = getType();

  if (theParent &&
      (typeName->equals(GET_STORE().XS_UNTYPED_QNAME) ||
       typeName->equals(GET_STORE().XS_ANY_QNAME)))
  {
    // Even if "this" is untyped, we must call removeType() on the parent
    // because renaming of an elelemt may require revalidation of the ancestors.
    upd.theRestoreParentType = true;
    theParent->removeType(upd);
  }
  else
  {
    removeType(upd);
  }

  if (theParent)
  {
    XmlNode* ancestor = theParent;

    while (ancestor != NULL &&
           ancestor->getNodeKind() == store::StoreConsts::elementNode)
    {
      ElementNode* elemAncestor = reinterpret_cast<ElementNode*>(ancestor);
      if (elemAncestor->theName->equals(theName))
      {
        elemAncestor->setRecursive();
        break;
      }
      
      ancestor = ancestor->theParent;
    }
  }
}


void ElementNode::restoreName(UpdRenameElem& upd)
{
  if (upd.theNewBinding)
  {
    const zstring& prefix = theName->getPrefix();
    const zstring& ns = theName->getNamespace();

    removeLocalBinding(prefix, ns);
  }

  theName.transfer(upd.theOldName);

  if (upd.theRestoreParentType)
    theParent->restoreType(upd.theTypeUndoList);
  else
    restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ATTRIBUTE - REPLACE VALUE
********************************************************************************/
void AttributeNode::replaceValue(UpdReplaceAttrValue& upd)
{
  upd.theOldValue.transfer(theTypedValue);

#ifndef ZORBA_NO_XMLSCHEMA
  if ( upd.thePul->getValidator()->isPossibleSimpleContentRevalidation(getType()) )
  {
    std::vector<store::Item_t> result;
    upd.thePul->getValidator()->validateSimpleContent(getType(), 
                                                      upd.theNewValue,
                                                      result);
    
    if ( result.size()==1 )
      theTypedValue.transfer(result[0]);
    else
    {
      theTypedValue = new ItemVector(result) ;
    }
  }
  else
#endif
  {
    store::Item_t newValue;
    GET_STORE().getItemFactory()->createUntypedAtomic(newValue, upd.theNewValue);
    theTypedValue.transfer(newValue);
    removeType(upd);
  }
}


void AttributeNode::restoreValue(UpdReplaceAttrValue& upd)
{
  theTypedValue.transfer(upd.theOldValue);

  restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  ATTRIBUTE - RENAME
********************************************************************************/
void AttributeNode::replaceName(UpdRenameAttr& upd)
{
  if (upd.theNewName->equals(theName))
    return;

  ElementNode* parent = reinterpret_cast<ElementNode*>(theParent);

  if (parent)
  {
    try
    {
      parent->checkUniqueAttr(upd.theNewName);
    }
    catch (ZorbaException const& e)
    {
      if (e.diagnostic() == err::XQDY0025)
        upd.theCollectionPul->thePrimitivesToRecheck.push_back(&upd);
      else
        throw;
    }

    upd.theNewBinding = parent->addBindingForQName(upd.theNewName, true, false);

    if (upd.theNewBinding && !upd.thePul->inheritNSBindings())
    {
      InternalNode::iterator ite = parent->childrenBegin();
      InternalNode::iterator end = parent->childrenEnd();

      for (; ite != end; ++ite)
      {
        if ((*ite)->getNodeKind() == store::StoreConsts::elementNode)
        {
          static_cast<ElementNode*>((*ite))->
          uninheritBinding(parent->theNsContext, upd.theNewName->getPrefix());
        }
      }
    }
  }

  upd.theOldName.transfer(theName);
  theName.transfer(upd.theNewName);

  if (getType()->equals(GET_STORE().theSchemaTypeNames[store::XS_UNTYPED_ATOMIC]))
  {
    // We must call removeType() even if "this" is untyped, because renaming
    // of an attribute may require revalidation of the ancestors.
    removeType(upd);
  }
  else
  {
    // We must convert the current typed value to an untyped one.
    zstring strvalue;
    theTypedValue->getStringValue2(strvalue);
    store::Item_t newValue;
    GET_STORE().getItemFactory()->createUntypedAtomic(newValue, strvalue);

    upd.theOldValue.transfer(theTypedValue);
    theTypedValue.transfer(newValue);

    removeType(upd);
  }
}


void AttributeNode::restoreName(UpdRenameAttr& upd)
{
  if (theParent && upd.theNewBinding)
  {
    ElementNode* parent = reinterpret_cast<ElementNode*>(theParent);

    parent->removeLocalBinding(theName->getPrefix(), theName->getNamespace());
  }

  theName.transfer(upd.theOldName);

  if (upd.theOldValue != NULL)
    theTypedValue.transfer(upd.theOldValue);

  restoreType(upd.theTypeUndoList);
}


/*******************************************************************************
  TEXT - REPLACE VALUE
********************************************************************************/
void TextNode::replaceValue(UpdReplaceTextValue& upd)
{
  InternalNode* parent = theParent;

#ifndef ZORBA_NO_XMLSCHEMA
  if ( upd.thePul->getValidator()->isPossibleSimpleContentRevalidation(getType()) )
  {
    if (isTyped())
    {
      assert(parent);
      upd.theIsTyped = true;
    }
    else
    {
      upd.theIsTyped = false;
      upd.theOldContent.setText(theContent);
    }

    std::vector<store::Item_t> result;
    upd.thePul->getValidator()->validateSimpleContent(getType(), 
                                                      upd.theNewContent,
                                                      result);
    
    if ( result.size()==1 )
      setValue(result[0]);
    else
      setValue( new ItemVector(result) );
  }
  else
#endif
  {
    if (parent)
      parent->removeType(upd);
    
    if (isTyped())
    {
      assert(parent);
      upd.theIsTyped = true;
    }
    else
    {
      upd.theIsTyped = false;
      upd.theOldContent.setText(theContent);
    }

    setText(upd.theNewContent);
  }
}


void TextNode::restoreValue(UpdReplaceTextValue& upd)
{
  ElementNode* parent = reinterpret_cast<ElementNode*>(theParent);

  if (upd.theIsTyped)
  {
    parent->restoreType(upd.theTypeUndoList);
  }
  else
  {
    theContent.setText(upd.theOldContent);

    if (parent)
      parent->restoreType(upd.theTypeUndoList);
  }
}


/*******************************************************************************
  PI - REPLACE VALUE
********************************************************************************/
void PiNode::replaceValue(UpdReplacePiValue& upd)
{
  upd.theOldValue.take(theContent);
  theContent.take(upd.theNewValue);
}


void PiNode::restoreValue(UpdReplacePiValue& upd)
{
  theContent.take(upd.theOldValue);
}


/*******************************************************************************
  PI - RENAME
********************************************************************************/
void PiNode::replaceName(UpdRenamePi& upd)
{
  upd.theOldName.take(theTarget);
  theTarget.take(upd.theNewName);
}


void PiNode::restoreName(UpdRenamePi& upd)
{
  theTarget.take(upd.theOldName);
}


/*******************************************************************************
 COMMENT - REPLACE VALUE
********************************************************************************/
void CommentNode::replaceValue(UpdReplaceCommentValue& upd)
{
  upd.theOldValue.take(theContent);
  theContent.take(upd.theNewValue);
}


void CommentNode::restoreValue(UpdReplaceCommentValue& upd)
{
  theContent.take(upd.theOldValue);
}


}
}
/* vim:set et sw=2 ts=2: */
