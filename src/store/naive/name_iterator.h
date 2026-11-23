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
#ifndef ZORBA_SIMPLESTORE_NAME_ITERATOR_H
#define ZORBA_SIMPLESTORE_NAME_ITERATOR_H

#include <vector>

#include "common/common.h"
#include "store/api/iterator.h"
#include "zorbautils/mutex.h"

namespace zorba {

namespace simplestore {

/*******************************************************************************
  Iterator to return Index and Collection Names.

  open() and reset() copy the names under the map's mutex, so that the
  iterator never walks the map while another thread changes it.
********************************************************************************/
template < typename T >
class NameIterator : public store::Iterator
{
private:
  T&                         theItems;
  bool                       theDynamicCollections;
  std::vector<store::Item_t> theNames;
  csize                      thePos;

  // Specialized for the collection set, which lists either its dynamic or its
  // static collections.
  bool isListed(const typename T::iterator&) const { return true; }

  void copyNames()
  {
    theNames.clear();
    thePos = 0;
    SYNC_CODE(AutoMutex lock(theItems.get_mutex());)
    for (typename T::iterator it = theItems.begin(); it != theItems.end(); ++it)
    {
      if (isListed(it))
        theNames.push_back(it.getKey());
    }
  }

public:
  NameIterator(T& aItems, bool dynamic)
    :
    theItems(aItems),
    theDynamicCollections(dynamic),
    thePos(0)
  {
  }

  virtual void open()
  {
    copyNames();
  }

  virtual bool next(store::Item_t& aResult)
  {
    if (thePos == theNames.size())
    {
      aResult = NULL;
      return false;
    }
    aResult = theNames[thePos++];
    return true;
  }

  virtual void reset()
  {
    copyNames();
  }

  virtual void close()
  {
    theNames.clear();
    thePos = 0;
  }
};

} // namespace store
} // namespace zorba


#endif
