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

#include "simple_collection_set.h"

#include "store/api/iterator.h"
#include "name_iterator.h"

namespace zorba { namespace simplestore {

/*******************************************************************************

********************************************************************************/
const ulong SimpleCollectionSet::DEFAULT_COLLECTION_MAP_SIZE = 32;


SimpleCollectionSet::SimpleCollectionSet()
  :
  theCollections(HashMapItemPointerCmp(0, NULL), DEFAULT_COLLECTION_MAP_SIZE, true)
{
}


SimpleCollectionSet::~SimpleCollectionSet()
{
}


void SimpleCollectionSet::clear() 
{
  theCollections.clear();
}


bool SimpleCollectionSet::insert(
    const store::Item* name,
    store::Collection_t& collection)
{
  store::Item* qname = const_cast<store::Item*>(name);

  return theCollections.insert(qname, collection);
}


bool SimpleCollectionSet::get(
    const store::Item* name,
    store::Collection_t& collection,
    bool isDynamic) 
{
  if (theCollections.get(const_cast<store::Item*>(name), collection)) 
  {
    return (collection->isDynamic() == isDynamic);
  }
  else 
  {
    return false;
  }
}


bool SimpleCollectionSet::remove(
    const store::Item* name,
    bool isDynamic) 
{
  store::Collection_t lColl;
  if (!get(name, lColl, isDynamic))
  {
    return false;
  }
  else
  {
    return theCollections.erase(const_cast<store::Item*>(name));
  }
}


// The collection set lists either its dynamic or its static collections.
template<> bool
NameIterator<SimpleCollectionSet::Set>::isListed(
    const SimpleCollectionSet::Set::iterator& aEntry) const;


store::Iterator_t SimpleCollectionSet::names(bool dynamic)
{
  return new NameIterator<Set>(theCollections, dynamic);
}


template<> bool
NameIterator<SimpleCollectionSet::Set>::isListed(
    const SimpleCollectionSet::Set::iterator& aEntry) const
{
  return aEntry.getValue()->isDynamic() == theDynamicCollections;
}


} /* namespace simplestore */
} /* namespace zorba */
/* vim:set et sw=2 ts=2: */
