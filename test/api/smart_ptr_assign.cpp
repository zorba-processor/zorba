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

// Converting assignment between SmartPtr types is limited to upcasts. The
// checks are static_asserts: the build fails if the restriction regresses.

#include <type_traits>

#include <zorba/util/smart_ptr.h>

using namespace zorba;

namespace {

struct Base : SmartObject {};
struct Derived : Base {};
struct Unrelated : SmartObject {};

using BasePtr = SmartPtr<Base>;
using DerivedPtr = SmartPtr<Derived>;
using UnrelatedPtr = SmartPtr<Unrelated>;

static_assert(std::is_assignable_v<BasePtr&, DerivedPtr>);
static_assert(std::is_assignable_v<BasePtr&, DerivedPtr const&>);
static_assert(std::is_assignable_v<BasePtr&, BasePtr const&>);
static_assert(!std::is_assignable_v<DerivedPtr&, BasePtr>);
static_assert(!std::is_assignable_v<DerivedPtr&, BasePtr const&>);
static_assert(!std::is_assignable_v<BasePtr&, UnrelatedPtr>);
static_assert(!std::is_assignable_v<UnrelatedPtr&, BasePtr>);

static_assert(std::is_assignable_v<BasePtr&, Derived*>);
static_assert(std::is_constructible_v<BasePtr, Derived*>);
static_assert(!std::is_constructible_v<DerivedPtr, BasePtr>);

} // anonymous namespace

int
smart_ptr_assign(int, char*[])
{
  DerivedPtr lDerived(new Derived);
  BasePtr lBase;
  lBase = lDerived;
  return lBase.get() == lDerived.get() && lBase->getRefCount() == 2 ? 0 : 1;
}
/* vim:set et sw=2 ts=2: */
