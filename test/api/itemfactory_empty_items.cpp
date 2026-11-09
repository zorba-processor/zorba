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

// Empty items passed to the public ItemFactory raise ZAPI0014. Each case runs
// as its own process, so that a case that fails by crashing reports itself.

#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

#include <zorba/diagnostic_list.h>
#include <zorba/item.h>
#include <zorba/item_factory.h>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

namespace {

// The call must raise ZAPI0014 and nothing else.
template <class Call>
int rejects(Call aCall)
{
  try
  {
    aCall();
  }
  catch (ZorbaException const& e)
  {
    return e.diagnostic() == zerr::ZAPI0014_INVALID_ARGUMENT ? 0 : 1;
  }
  return 2;
}

struct Fixture
{
  ItemFactory* f;
  Item name;
  Item type;
  Item value;
  Item element;

  explicit Fixture(ItemFactory* aFactory) : f(aFactory)
  {
    name = f->createQName("", "a");
    type = f->createQName("http://www.w3.org/2001/XMLSchema", "xs", "string");
    value = f->createString("v");
    Item noParent;
    element = f->createElementNode(noParent, name, type, true, false,
                                   NsBindings());
  }
};

int elementName(Fixture& x)
{
  Item parent, empty;
  return rejects([&] {
    x.f->createElementNode(parent, empty, x.type, false, false, NsBindings());
  });
}

int attributeName(Fixture& x)
{
  Item parent, empty;
  return rejects([&] { x.f->createAttributeNode(parent, empty, x.type, x.value); });
}

int attributeValue(Fixture& x)
{
  Item parent, empty;
  return rejects([&] { x.f->createAttributeNode(parent, x.name, x.type, empty); });
}

int attributeListName(Fixture& x)
{
  Item parent, empty;
  std::vector<Item> values(1, x.value);
  return rejects([&] { x.f->createAttributeNode(parent, empty, x.type, values); });
}

int attributeListValue(Fixture& x)
{
  Item parent;
  std::vector<Item> values(2, x.value);
  values[1] = Item();
  return rejects([&] { x.f->createAttributeNode(parent, x.name, x.type, values); });
}

int assignElement(Fixture& x)
{
  Item empty;
  return rejects([&] { x.f->assignElementTypedValue(empty, x.value); });
}

int assignValue(Fixture& x)
{
  Item empty;
  return rejects([&] { x.f->assignElementTypedValue(x.element, empty); });
}

int assignListElement(Fixture& x)
{
  Item empty;
  std::vector<Item> values(1, x.value);
  return rejects([&] { x.f->assignElementTypedValue(empty, values); });
}

int assignListValue(Fixture& x)
{
  std::vector<Item> values(2, x.value);
  values[1] = Item();
  return rejects([&] { x.f->assignElementTypedValue(x.element, values); });
}

int jsonObjectName(Fixture& x)
{
  std::vector<std::pair<Item, Item> > pairs;
  pairs.push_back(std::make_pair(Item(), x.value));
  return rejects([&] { x.f->createJSONObject(pairs); });
}

int jsonObjectValue(Fixture& x)
{
  std::vector<std::pair<Item, Item> > pairs;
  pairs.push_back(std::make_pair(x.value, Item()));
  return rejects([&] { x.f->createJSONObject(pairs); });
}

int jsonArray(Fixture& x)
{
  std::vector<Item> items(2, x.value);
  items[1] = Item();
  return rejects([&] { x.f->createJSONArray(items); });
}

// The empty parent of the node constructors, and an empty list, stay legal.
int legal(Fixture& x)
{
  Item parent;
  Item element = x.f->createElementNode(parent, x.name, x.type, false, false,
                                        NsBindings());
  Item attribute = x.f->createAttributeNode(parent, x.name, Item(), x.value);
  String content("c");
  Item text = x.f->createTextNode(parent, content);
  Item comment = x.f->createCommentNode(parent, content);
  std::vector<Item> none;
  std::vector<std::pair<Item, Item> > noPairs;
  Item array = x.f->createJSONArray(none);
  Item object = x.f->createJSONObject(noPairs);
  return element.isNull() || attribute.isNull() || text.isNull() ||
         comment.isNull() || array.isNull() || object.isNull();
}

struct Case
{
  char const* name;
  int (*run)(Fixture&);
};

Case const theCases[] = {
  {"element_name", elementName},
  {"attribute_name", attributeName},
  {"attribute_value", attributeValue},
  {"attribute_list_name", attributeListName},
  {"attribute_list_value", attributeListValue},
  {"assign_element", assignElement},
  {"assign_value", assignValue},
  {"assign_list_element", assignListElement},
  {"assign_list_value", assignListValue},
  {"json_object_name", jsonObjectName},
  {"json_object_value", jsonObjectValue},
  {"json_array", jsonArray},
  {"legal_arguments", legal},
};

} // namespace

int itemfactory_empty_items(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "usage: itemfactory_empty_items <case>" << std::endl;
    return 2;
  }
  Case const* lCase = nullptr;
  for (Case const& c : theCases)
    if (std::strcmp(c.name, argv[1]) == 0)
      lCase = &c;
  if (!lCase)
  {
    std::cerr << "unknown case " << argv[1] << std::endl;
    return 2;
  }

  void* store = StoreManager::getStore();
  Zorba* zorba = Zorba::getInstance(store);
  int result;
  {
    // The items go before the store does.
    Fixture fixture(zorba->getItemFactory());
    result = lCase->run(fixture);
  }
  zorba->shutdown();
  StoreManager::shutdownStore(store);
  return result;
}
/* vim:set et sw=2 ts=2: */
