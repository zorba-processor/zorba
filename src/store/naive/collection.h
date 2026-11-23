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
#ifndef ZORBA_SIMPLESTORE_COLLECTION
#define ZORBA_SIMPLESTORE_COLLECTION

#include <memory>

#include "store/api/collection.h"
#include "zorbatypes/integer.h"

#include "shared_types.h"
#include "tree_id.h"

namespace zorba { namespace simplestore {

class CollectionTreeInfo;

/*******************************************************************************
  Interface for Collection used in the SimpleStore

  theName: 
  --------
  The user-provided qname of the collection.
********************************************************************************/
class Collection : public store::Collection
{
protected:
  store::Item_t  theName;

public:
  /***************************** Constructors *********************************/

  Collection(const store::Item_t& name) : theName(name) { }

  Collection() { }

  virtual ~Collection() {}

  /***************** zorba::store::Collection interface ***********************/

  const store::Item* getName() const { return theName.getp(); }

  zorba::xs_integer size() const  = 0;

  virtual bool isDynamic() const = 0;

  virtual void getAnnotations(std::vector<store::Annotation_t>&) const = 0;

  store::Iterator_t getIterator(const xs_integer& skip, const zstring& start) = 0;

  virtual bool findNode(const store::Item* node, xs_integer& position) const = 0;

  virtual zorba::store::Item_t nodeAt(xs_integer position) = 0;

  /***************************** ID Management ********************************/

  virtual ulong getId() const = 0;

  virtual TreeId createTreeId() = 0;

  /************************* Updates on collection ****************************/

  virtual void addNode(
      store::Item* node,
      xs_integer position = xs_integer(-1)) = 0;

  virtual csize addNodes(
      std::vector<store::Item_t>& nodes,
      const store::Item* targetNode,
      bool before) = 0;

  // Removes the tree of the given root, if in this collection, and returns its
  // collection info, for restoreNode(), with its position; otherwise NULL.
  virtual std::unique_ptr<CollectionTreeInfo> removeNode(
      store::Item* node,
      csize& position) = 0;

  // Puts back at its position a tree that removeNode() removed, with no
  // allocation: for an undo.
  virtual void restoreNode(
      store::Item* node,
      csize position,
      std::unique_ptr<CollectionTreeInfo> info) = 0;

  // Removes the num trees from the given position on, which must be nodes[0]
  // to nodes[num - 1], with no allocation (an xs_integer allocates in bigint
  // builds): for an undo.
  virtual void removeNodes(
      const std::vector<store::Item_t>& nodes,
      csize position,
      csize num) = 0;

  // Removes the last num trees, which must be nodes[0] to nodes[num - 1], with
  // no allocation: for an undo.
  virtual void removeLastNodes(
      const std::vector<store::Item_t>& nodes,
      csize num) = 0;

  virtual void removeAll() = 0;
  
  virtual void adjustTreePositions() = 0;

  /*********************** Indices ********************************************/

  static void getIndexes(
      const store::Item* name,
      std::vector<store::Index*>& indexes);

  void getIndexes(std::vector<store::Index*>& indexes);

  /*********************** Integrity Constraints ******************************/

  static void getActiveICs(
      const store::Item* name,
      std::vector<store::IC*>& ics);

  void getActiveICs(std::vector<store::IC*>& ics);
};


} /* namespace simplestore */ } /* namespace zorba */

#endif
