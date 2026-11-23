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
#ifndef ZORBA_SIMPLE_STORE_IMMORTAL_ITEM
#define ZORBA_SIMPLE_STORE_IMMORTAL_ITEM

#include "store/api/item.h"
#include "store/api/shared_types.h"

namespace zorba { namespace simplestore {

/*******************************************************************************
  Holds an item that every query of the process may share (the booleans, the
  small integers, ...). The item is immortal: its count never changes, so the
  threads that use it do not all write to one cache line, and no release
  frees it. The holder frees it when it is destroyed; nothing may use the
  item after that.
********************************************************************************/
class ImmortalItem
{
  store::Item_t theItem;

public:
  ImmortalItem() = default;

  explicit ImmortalItem(const store::Item_t& item) { set(item); }

  ImmortalItem(const ImmortalItem&) = delete;
  ImmortalItem& operator=(const ImmortalItem&) = delete;

  ~ImmortalItem()
  {
    if (theItem != NULL)
      theItem.release()->free();
  }

  void set(const store::Item_t& item)
  {
    assert(theItem == NULL);
    theItem = item;
    theItem->makeImmortal();
  }

  store::Item* get() const { return theItem.getp(); }
};

} // namespace simplestore
} // namespace zorba

#endif
/* vim:set et sw=2 ts=2: */
