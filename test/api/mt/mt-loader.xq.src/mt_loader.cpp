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

// The external module of mt-loader.xq, which mt_dynamic_loader loads from
// several threads at once. Its table is built by a static constructor, which
// runs when the library is opened.

#include <map>
#include <memory>
#include <string>

#include <zorba/empty_sequence.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_factory.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/zorba.h>

namespace {

const char NS[] = "http://zorba-tests.example.org/modules/mt-loader";

const std::map<std::string, int> DIGITS = {
  {"zero", 0}, {"one", 1}, {"two", 2}, {"three", 3}, {"four", 4},
  {"five", 5}, {"six", 6}, {"seven", 7}, {"eight", 8}, {"nine", 9}
};

class DigitFunction : public zorba::NonContextualExternalFunction
{
public:
  zorba::String getURI() const override { return NS; }

  zorba::String getLocalName() const override { return "digit"; }

  zorba::ItemSequence_t
  evaluate(const zorba::ExternalFunction::Arguments_t& aArgs) const override
  {
    zorba::Iterator_t lIter = aArgs[0]->getIterator();
    zorba::Item lName;
    lIter->open();
    lIter->next(lName);
    lIter->close();

    auto const lDigit = DIGITS.find(lName.getStringValue().str());
    if (lDigit == DIGITS.end())
      return zorba::ItemSequence_t(new zorba::EmptySequence());
    return zorba::ItemSequence_t(new zorba::SingletonItemSequence(
        zorba::Zorba::getInstance(0)->getItemFactory()->createInteger(
            lDigit->second)));
  }
};

class LoaderModule : public zorba::ExternalModule
{
  DigitFunction theDigit;

public:
  zorba::String getURI() const override { return NS; }

  zorba::ExternalFunction*
  getExternalFunction(const zorba::String& aLocalName) override
  {
    return aLocalName == "digit" ? &theDigit : nullptr;
  }

  void destroy() override { delete this; }  // NOLINT(zorba-owning-memory): created by createModule, destroyed by the engine
};

} // namespace

extern "C" __attribute__((visibility("default")))
zorba::ExternalModule* createModule()
{
  return std::make_unique<LoaderModule>().release();
}
/* vim:set et sw=2 ts=2: */
