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

#include <iostream>
#include <zorba/diagnostic_list.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

// The call must raise ZAPI0014, not dereference the empty item.
static bool
rejectsEmptyArgument(ItemFactory* factory, Item base, Item typeName)
{
  try
  {
    factory->createUserTypedAtomicItem(base, typeName);
  }
  catch (ZorbaException const& e)
  {
    return e.diagnostic() == zerr::ZAPI0014_INVALID_ARGUMENT;
  }
  return false;
}

// Items must be released before the store shuts down.
static int
run(ItemFactory* factory)
{
  Item base = factory->createInteger(7);
  Item typeName = factory->createQName("http://www.example.com", "myInt");

  std::cout << "empty base item" << std::endl;
  if (!rejectsEmptyArgument(factory, Item(), typeName))
    return 1;

  std::cout << "empty type name" << std::endl;
  if (!rejectsEmptyArgument(factory, base, Item()))
    return 2;

  std::cout << "valid arguments" << std::endl;
  Item result = factory->createUserTypedAtomicItem(base, typeName);
  if (result.isNull() || result.getStringValue() != "7")
    return 3;

  return 0;
}

int user_typed_item(int, char*[])
{
  void* store = StoreManager::getStore();
  Zorba* zorba = Zorba::getInstance(store);

  int result = run(zorba->getItemFactory());

  zorba->shutdown();
  StoreManager::shutdownStore(store);
  return result;
}
/* vim:set et sw=2 ts=2: */
