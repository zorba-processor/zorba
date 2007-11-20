/*
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Author: David Graf (david.graf@28msec.com)
 *
 *  Info: This file contains the class declarations of all nodes defined in [http://www.w3.org/TR/xpath-datamodel/#Nodes].
 */

#ifndef XQP_NODE_ITEMS_H
#define XQP_NODE_ITEMS_H

#include <vector>
#include <string>

#include "util/Assert.h"
#include "store/api/item.h"

namespace xqp
{

class NodeNaive;
class NsBindingsContext;

template <class Object> class rchandle;

typedef rchandle<class Item> Item_t;
typedef rchandle<class NodeItem> NodeItem_t;
typedef rchandle<class NodeNaive> NodeNaive_t;

typedef rchandle<class TempSeq> TempSeq_t;

typedef rchandle<class NsBindingsContext> NsBindingsContext_t;


/*******************************************************************************
virtual void setChildren(const TempSeq_t& seq)
********************************************************************************/
class NodeNaive : public NodeItem
{
public:
  NodeNaive() {}

  NodeNaive(const Item_t& aParent) : NodeItem(aParent) {}

  virtual void setParent(const Item_t& p) { theParent = p.get_ptr(); }

  virtual void setChildren(const TempSeq_t& seq) { Assert(0); }

  virtual NsBindingsContext_t getNsBindingsContext() const { return NULL; }
};


/*******************************************************************************

********************************************************************************/
class DocumentNodeNaive : public NodeNaive
{
 private:
  xqpStringStore_t theBaseURI;
  xqpStringStore_t theDocURI;

  TempSeq_t        theChildren;

 public:
  DocumentNodeNaive(
        const xqpStringStore_t& baseURI,
        const xqpStringStore_t& documentURI);

  DocumentNodeNaive(
        const xqpStringStore_t& baseURI,
        const xqpStringStore_t& documentURI,
        const TempSeq_t& children);

  virtual ~DocumentNodeNaive();

  virtual StoreConsts::NodeKind_t getNodeKind() const;
  QNameItem_t getType() const;

  virtual xqp_string getBaseURI() const;
  virtual xqp_string getDocumentURI() const;

  virtual Iterator_t getChildren() const;
  virtual void setChildren(const TempSeq_t& seq);

  virtual Iterator_t getTypedValue() const;
  virtual Item_t getAtomizationValue() const;
  virtual xqp_string getStringProperty() const;

  virtual xqp_string show() const;

  // Used when zorba supports DTD
  // xqp_string getUnparsedEntityPublicId() const;
  // xqp_string getunparsedEntitySystemId() const;
};


/*******************************************************************************

********************************************************************************/
class ElementNodeNaive : public NodeNaive
{
 private:
  QNameItem_t          theName;
  QNameItem_t          theType;
  TempSeq_t            theChildren;
  TempSeq_t            theAttributes;
  TempSeq_t            theNsUris;
  NsBindingsContext_t  theNsBindings;

 public:
  ElementNodeNaive(
        const QNameItem_t& name,
        const QNameItem_t& type,
        TempSeq_t& seqAttributes,
        const NamespaceBindings& nsBindings);

  ElementNodeNaive(
			  const QNameItem_t& name,
        const QNameItem_t& type,
        TempSeq_t& seqChildren,
        TempSeq_t& seqAttributes,
        TempSeq_t& seqNsUris,
        const NamespaceBindings& nsBindings,
        bool copy,
        bool newTypes);
			
  virtual ~ElementNodeNaive();

  virtual StoreConsts::NodeKind_t getNodeKind() const;
  QNameItem_t getType() const;

  virtual QNameItem_t getNodeName() const;

  virtual Iterator_t getAttributes() const;
  virtual Iterator_t getChildren() const;
  virtual void setChildren(const TempSeq_t& seq);
  void setParent(const Item_t& p);

  virtual Iterator_t getTypedValue() const;
  virtual Item_t getAtomizationValue() const;
  virtual xqp_string getStringProperty() const;

  virtual bool getNilled() const;

  virtual NamespaceBindings getNamespaceBindings() const;

  NsBindingsContext_t getNsBindingsContext() const
  {
    return theNsBindings;
  }

  void setNsBindingsContext(const NsBindingsContext_t& ctx)
  {
    theNsBindings = ctx;
  }

  virtual xqp_string show() const;
};


/*******************************************************************************

********************************************************************************/
class AttributeNodeNaive : public NodeNaive
{
 private:
  QNameItem_t  theName;
  QNameItem_t  theType;
  Item_t       theLexicalValue;
  Item_t       theTypedValue;

  bool         theIsId;
  bool         theIsIdrefs;
  
 public:
  AttributeNodeNaive (
			  const QNameItem_t& name,
        const QNameItem_t& type,
        const Item_t& lexicalValue,
        const Item_t& typedValue,
        bool isId,
        bool isIdrefs);
			
  AttributeNodeNaive(
			  const Item_t& parent,
        const QNameItem_t& name,
        const QNameItem_t& type,
        const Item_t& lexicalValue,
        const Item_t& typedValue,
        bool isId,
        bool isIdrefs);
			
  virtual ~AttributeNodeNaive();

  virtual StoreConsts::NodeKind_t getNodeKind() const;
  QNameItem_t getType() const;

  virtual QNameItem_t getNodeName() const;

  virtual Iterator_t getTypedValue() const;
  virtual Item_t getAtomizationValue() const;
  virtual xqp_string getStringProperty() const;
  virtual xqp_string getStringValue() const;

  virtual bool isId() const;
  virtual bool isIdrefs() const;

  virtual xqp_string show() const;
};


/*******************************************************************************

********************************************************************************/
class TextNodeNaive : public NodeNaive
{
 private:
  xqpStringStore_t theContent;

 public:
  TextNodeNaive(const xqpStringStore_t& content);
  virtual ~TextNodeNaive();
  
  virtual StoreConsts::NodeKind_t getNodeKind() const;
  virtual QNameItem_t getType() const;

  virtual Iterator_t getTypedValue() const;
  virtual Item_t getAtomizationValue() const;
  virtual xqp_string getStringProperty() const;
  virtual xqp_string getStringValue() const;
			
  virtual xqp_string show() const;
};


/*******************************************************************************

********************************************************************************/
class PiNodeNaive : public NodeNaive
{
 private:
  xqpStringStore_t theTarget;
  xqpStringStore_t theData;

public:
  PiNodeNaive(const xqpStringStore_t& target, const xqpStringStore_t& data);

  virtual ~PiNodeNaive();

  virtual StoreConsts::NodeKind_t getNodeKind() const;
  virtual QNameItem_t getType() const;

  virtual Iterator_t getTypedValue() const;
  virtual Item_t getAtomizationValue() const;
  virtual xqp_string getStringProperty() const;
  virtual xqp_string getStringValue() const;

  virtual xqp_string getTarget() const;

  virtual xqp_string show() const;
};


/*******************************************************************************

********************************************************************************/
class CommentNodeNaive : public NodeNaive
{
private:
  xqpStringStore_t theContent;

public:
  CommentNodeNaive(const xqpStringStore_t& content);
  virtual ~CommentNodeNaive();

  virtual StoreConsts::NodeKind_t getNodeKind() const;
  virtual QNameItem_t getType() const;

  virtual Iterator_t getTypedValue() const;
  virtual Item_t getAtomizationValue() const;
  virtual xqp_string getStringProperty() const;
  virtual xqp_string getStringValue() const;

  virtual xqp_string show() const;
};


/*******************************************************************************

  This iterator is used during the getChildren() or getAttributes() methods
  to set the parent pointer to each child or attribute node of "this" node.

********************************************************************************/
class ChildrenIterator : public Iterator
{
protected:
  Iterator_t  theInput;
  NodeNaive_t theParentNode;

public:
  ChildrenIterator(const Iterator_t& input, NodeNaive* parent)
    :
    theInput(input),
    theParentNode(parent)
  {
  }

  Item_t next();
  void reset();
};


/*******************************************************************************

********************************************************************************/
class NsBindingsContext : public rcobject
{
 private:
  NamespaceBindings    theBindings;
  NsBindingsContext_t  theParentContext;

public:
  NsBindingsContext() {}

  NsBindingsContext(const NamespaceBindings& bindings)
    :
    theBindings(bindings)
  {
  }

  const NamespaceBindings& getBindings() const        { return theBindings; }

  void setParentContext(const NsBindingsContext_t& p) { theParentContext = p; }
  NsBindingsContext_t getParentContext() const        { return theParentContext; } 
};

} /* namespace xqp */

#endif /* XQP_NODE_ITEMS_H */
