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
#ifndef ZORBA_SIMPLESTORE_DOCUMENT_NAME_ITERATOR_H
#define ZORBA_SIMPLESTORE_DOCUMENT_NAME_ITERATOR_H

#include <vector>

#include "common/common.h"
#include "store/api/iterator.h"
#include "system/globalenv.h"
#include "zorbautils/mutex.h"

namespace zorba {

  namespace simplestore {

/*******************************************************************************
  The URIs of the documents in the store. open() and reset() copy them under
  the map's mutex: fn:doc adds documents while other queries run.
********************************************************************************/
template < typename T >
class DocumentNameIterator : public store::Iterator
{
private:
  const T&             theItems;
  std::vector<zstring> theUris;
  csize                thePos;

  void copyUris()
  {
    theUris.clear();
    thePos = 0;
    SYNC_CODE(AutoMutex lock(theItems.get_mutex());)
    for (typename T::iterator it = theItems.begin(); it != theItems.end(); ++it)
      theUris.push_back(it.getKey());
  }

public:
  DocumentNameIterator(const T& aItems)
    :
    theItems(aItems),
    thePos(0)
  {
  }

  virtual void open()
  {
    copyUris();
  }

  virtual bool next(store::Item_t& aResult)
  {
    if (thePos == theUris.size())
    {
      aResult = NULL;
      return false;
    }
    zstring lUri = theUris[thePos++];
    GENV_ITEMFACTORY->createString(aResult, lUri);
    return true;
  }

  virtual void reset()
  {
    copyUris();
  }

  virtual void close()
  {
    theUris.clear();
    thePos = 0;
  }
};


} // namespace store
} // namespace zorba


#endif
