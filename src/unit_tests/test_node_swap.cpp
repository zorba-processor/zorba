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

// store::Item::swap on two nodes of one tree (and on a node with itself)
// returns, leaves both nodes in the tree, and leaves the tree's count as it
// was. In a multithreaded build it takes the tree's lock once; locking it a
// second time never returned.

#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <thread>

#include <zorba/iterator.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>

#include "api/unmarshaller.h"
#include "store/api/item.h"
#include "store/api/iterator.h"
#include "store/naive/node_items.h"

using namespace std;

namespace zorba {
namespace UnitTests {

namespace {

int failures;

#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      cerr << __FILE__ << ':' << __LINE__ << ": check failed: "         \
           << #cond << endl;                                            \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

// Calls the store::Item version of swap (the one that moves the nodes and
// their counts between trees) on another thread, and gives up on a swap that
// does not return.
void swap_nodes(store::Item* a, store::Item* b)
{
  simplestore::XmlTree* const tree =
    static_cast<simplestore::XmlNode*>(a)->getTree();
  long const treeCount = tree->getRefCount();
  long const countA = a->getRefCount();
  long const countB = b->getRefCount();

  auto done = async(launch::async, [a, b]() { a->store::Item::swap(b); });
  if (done.wait_for(chrono::seconds(30)) != future_status::ready)
  {
    cerr << "FAILED: swap of two nodes of one tree did not return" << endl;
    _Exit(1);
  }
  done.get();

  CHECK(static_cast<simplestore::XmlNode*>(a)->getTree() == tree);
  CHECK(static_cast<simplestore::XmlNode*>(b)->getTree() == tree);
  CHECK(tree->getRefCount() == treeCount);
  CHECK(a->getRefCount() == countA);
  CHECK(b->getRefCount() == countB);
}

} // namespace

int test_node_swap(int, char*[])
{
  failures = 0;

  Zorba* const z = Zorba::getInstance(StoreManager::getStore());
  XQuery_t q = z->compileQuery("<a><b/><c/></a>");
  Iterator_t it = q->iterator();
  it->open();
  Item root;
  CHECK(it->next(root));
  it->close();

  store::Item* const a = Unmarshaller::getInternalItem(root);
  store::Iterator_t children = a->getChildren();
  children->open();
  store::Item_t b, c;
  CHECK(children->next(b));
  CHECK(children->next(c));
  children->close();

  if (b != NULL && c != NULL)
  {
    swap_nodes(b.getp(), c.getp());
    swap_nodes(b.getp(), b.getp());
  }

  cout << (failures ? "FAILED" : "passed") << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
