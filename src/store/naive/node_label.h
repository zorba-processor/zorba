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

#ifndef ZORBA_STORE_NODE_LABEL_H
#define ZORBA_STORE_NODE_LABEL_H

#include <string>

#include "ordpath.h"


namespace zorba { namespace simplestore {

class NodeLabelBuilder;


/*******************************************************************************
  The identity of a node and its place in document order within its tree.
  Nodes depend on this class only; OrdPath is its one labeling strategy.
********************************************************************************/
class NodeLabel
{
public:
  typedef enum
  {
    PRECEDING,
    ANCESTOR,
    SELF,
    DESCENDANT,
    FOLLOWING,
    PRECEDING_SIBLING,
    FOLLOWING_SIBLING,
    PARENT,
    CHILD,
    OTHER
  }
  RelativePosition;

private:
  OrdPath theOrdPath;

public:
  bool isValid() const { return theOrdPath.isValid(); }

  void setAsRoot() { theOrdPath.setAsRoot(); }

  // The label of the child at a position among the children (attributes
  // first) of a parent that never lost a child.
  void setAsChild(const NodeLabel& parent, csize position)
  {
    theOrdPath = parent.theOrdPath;
    theOrdPath.appendComp(static_cast<int32_t>(2 * position + 1));
  }

  void setBefore(const NodeLabel& parent, const NodeLabel& sibling)
  {
    OrdPath::insertBefore(parent.theOrdPath, sibling.theOrdPath, theOrdPath);
  }

  void setAfter(const NodeLabel& parent, const NodeLabel& sibling)
  {
    OrdPath::insertAfter(parent.theOrdPath, sibling.theOrdPath, theOrdPath);
  }

  void setBetween(
      const NodeLabel& parent,
      const NodeLabel& lower,
      const NodeLabel& upper)
  {
    OrdPath::insertInto(parent.theOrdPath,
                        lower.theOrdPath,
                        upper.theOrdPath,
                        theOrdPath);
  }

  NodeLabel& operator=(const NodeLabelBuilder& builder);

  bool operator==(const NodeLabel& other) const
  {
    return theOrdPath == other.theOrdPath;
  }

  bool operator<(const NodeLabel& other) const
  {
    return theOrdPath < other.theOrdPath;
  }

  bool operator>(const NodeLabel& other) const
  {
    return theOrdPath > other.theOrdPath;
  }

  // Where "other" is: PRECEDING, ANCESTOR, SELF, DESCENDANT or FOLLOWING.
  RelativePosition getRelativePosition(const NodeLabel& other) const
  {
    return static_cast<RelativePosition>(
        theOrdPath.getRelativePosition(other.theOrdPath));
  }

  // Where "other" is: PARENT, CHILD, PRECEDING_SIBLING, FOLLOWING_SIBLING or
  // OTHER (any other place, self included).
  RelativePosition getAdjacentPosition(const NodeLabel& other) const
  {
    return static_cast<RelativePosition>(
        theOrdPath.getRelativePosition2(other.theOrdPath));
  }

  // The root is at level 1.
  ulong getLevel() const { return theOrdPath.getLevel(); }

  // The form structural URIs carry.
  std::string serialize() const { return theOrdPath.serialize(); }

  bool deserialize(const std::string& s) { return theOrdPath.deserialize(s); }

  zstring show() const { return theOrdPath.show(); }

  size_t alloc_size() const { return theOrdPath.alloc_size(); }
};


#define ZORBA_SAME_POSITION(pos) \
  static_assert(int(NodeLabel::pos) == int(OrdPath::pos), #pos)
ZORBA_SAME_POSITION(PRECEDING);
ZORBA_SAME_POSITION(ANCESTOR);
ZORBA_SAME_POSITION(SELF);
ZORBA_SAME_POSITION(DESCENDANT);
ZORBA_SAME_POSITION(FOLLOWING);
ZORBA_SAME_POSITION(PRECEDING_SIBLING);
ZORBA_SAME_POSITION(FOLLOWING_SIBLING);
ZORBA_SAME_POSITION(PARENT);
ZORBA_SAME_POSITION(CHILD);
ZORBA_SAME_POSITION(OTHER);
#undef ZORBA_SAME_POSITION


/*******************************************************************************
  The labels of the nodes a loader builds in document order: the label of the
  next node, moved down to a first child, across to a next sibling, or up to
  the sibling after the parent.
********************************************************************************/
class NodeLabelBuilder
{
  friend class NodeLabel;

private:
  OrdPathStack theStack;

public:
  void init() { theStack.init(); }

  void pushChild() { theStack.pushChild(); }

  void nextChild() { theStack.nextChild(); }

  void popChild() { theStack.popChild(); }
};


inline NodeLabel& NodeLabel::operator=(const NodeLabelBuilder& builder)
{
  theOrdPath = builder.theStack;
  return *this;
}


} // namespace simplestore
} // namespace zorba

#endif /* ZORBA_STORE_NODE_LABEL_H */
/* vim:set et sw=2 ts=2: */
