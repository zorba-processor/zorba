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

#include "file.h"
#include "file_module.h"
#include "file_function.h"

namespace zorba {
namespace filemodule {

///////////////////////////////////////////////////////////////////////////////

FileModule::FileModule() :
  theFactory( Zorba::getInstance(0)->getItemFactory() )
{
  theFunctions["append-text"] = std::make_unique<AppendTextFunction>( this );
  theFunctions["append-text-lines"] = std::make_unique<AppendTextLinesFunction>( this );
  theFunctions["append-binary"] = std::make_unique<AppendBinaryFunction>( this );
  theFunctions["base-name"] = std::make_unique<BaseNameFunction>( this );
  theFunctions["copy-file-impl"] = std::make_unique<CopyFileImplFunction>( this );
  theFunctions["create-directory"] = std::make_unique<CreateDirectoryFunction>( this );
  theFunctions["delete-file-impl"] = std::make_unique<DeleteFileImplFunction>( this );
  theFunctions["dir-name"] = std::make_unique<DirNameFunction>( this );
  theFunctions["directory-separator"] = std::make_unique<DirectorySeparator>( this );
  theFunctions["exists"] = std::make_unique<ExistsFunction>( this );
  theFunctions["is-directory"] = std::make_unique<IsDirectoryFunction>( this );
  theFunctions["is-file"] = std::make_unique<IsFileFunction>( this );
  theFunctions["is-symlink"] = std::make_unique<IsSymlinkFunction>( this );
  theFunctions["last-modified"] = std::make_unique<LastModifiedFunction>( this );
  theFunctions["list"] = std::make_unique<ListFunction>( this );
  theFunctions["path-separator"] = std::make_unique<PathSeparator>( this );
  theFunctions["path-to-native"] = std::make_unique<PathToNativeFunction>( this );
  theFunctions["path-to-uri"] = std::make_unique<PathToUriFunction>( this );
  theFunctions["read-binary"] = std::make_unique<ReadBinaryFunction>( this );
  theFunctions["read-text"] = std::make_unique<ReadTextFunction>( this );
  theFunctions["read-text-lines"] = std::make_unique<ReadTextLinesFunction>( this );
  theFunctions["resolve-path"] = std::make_unique<ResolvePathFunction>( this );
  theFunctions["write-text"] = std::make_unique<WriteTextFunction>( this );
  theFunctions["write-text-lines"] = std::make_unique<WriteTextLinesFunction>( this );
  theFunctions["write-binary"] = std::make_unique<WriteBinaryFunction>( this );
  theFunctions["size"] = std::make_unique<SizeFunction>( this );
}

ExternalFunction* FileModule::getExternalFunction( String const &aLocalname ) {
  FuncMap_t::const_iterator const i = theFunctions.find( aLocalname );
  return i == theFunctions.end() ? nullptr : i->second.get();
}

void FileModule::destroy() {
  delete this;
}

String FileModule::getURI() const {
  return "http://expath.org/ns/file";
}

///////////////////////////////////////////////////////////////////////////////

} // namespace filemodule
} // namespace zorba
/* vim:set et sw=2 ts=2: */
