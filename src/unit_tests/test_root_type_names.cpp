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

// The type-name QNames of the root type manager are shared by every query of
// every thread, so they are immortal: copying and releasing a handle leaves
// their count alone, and the store's QName pool frees them when it is
// destroyed.

#include <iostream>

#include "store/api/item.h"
#include "system/globalenv.h"
#include "types/root_typemanager.h"

using namespace std;

namespace zorba {
namespace UnitTests {

int test_root_type_names(int, char*[])
{
  RootTypeManager const& rtm = GENV_TYPESYSTEM;
  struct name {
    char const* label;
    store::Item* qname;
  };
  name const names[] = {
    { "anyType", rtm.XS_ANY_TYPE_QNAME.getp() },
    { "untyped", rtm.XS_UNTYPED_QNAME.getp() },
    { "anySimpleType", rtm.XS_ANY_SIMPLE_TYPE_QNAME.getp() },
    { "NMTOKENS", rtm.XS_NMTOKENS_QNAME.getp() },
    { "IDREFS", rtm.XS_IDREFS_QNAME.getp() },
    { "ENTITIES", rtm.XS_ENTITIES_QNAME.getp() },
    { "anyAtomicType", rtm.XS_ANY_ATOMIC_QNAME.getp() },
    { "string", rtm.XS_STRING_QNAME.getp() },
    { "js:null", rtm.JS_NULL_QNAME.getp() },
  };

  int failures = 0;
  for (name const& n : names)
  {
    long const before = n.qname->getRefCount();
    store::Item_t const copy(n.qname);
    store::Item_t const another(copy);
    bool const immortal = n.qname->isImmortal();
    bool const unchanged = n.qname->getRefCount() == before;
    if (!immortal || !unchanged)
    {
      cerr << "FAILED: " << n.label << ": immortal " << immortal
           << ", count unchanged by two copies " << unchanged << endl;
      ++failures;
    }
  }
  cout << (failures ? "FAILED" : "passed") << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
