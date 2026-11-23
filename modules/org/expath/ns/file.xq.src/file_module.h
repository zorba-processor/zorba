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

#ifndef ZORBA_FILEMODULE_FILEMODULE_H
#define ZORBA_FILEMODULE_FILEMODULE_H

#include <map>
#include <memory>

#include <zorba/zorba.h>
#include <zorba/external_module.h>

namespace zorba {
namespace filemodule {

///////////////////////////////////////////////////////////////////////////////

class FileModule : public ExternalModule {
public:
  static void streamReleaser( std::istream *stream ) {
    delete stream;
  }

  FileModule();

  virtual void destroy();
  virtual ExternalFunction* getExternalFunction( String const &aLocalname );
  virtual String getURI() const;

  ItemFactory* getItemFactory() const {
    return theFactory;
  }

protected:
  // Filled by the constructor: compilations on several threads look the
  // functions up.
  typedef std::map<String,std::unique_ptr<ExternalFunction>> FuncMap_t;
  FuncMap_t theFunctions;

private:
  ItemFactory *const theFactory;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace filemodule
} // namespace zorba

#endif /* ZORBA_FILEMODULE_FILEMODULE_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
