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

#ifndef ZORBA_FILEMODULE_FILEFUNCTION_H
#define ZORBA_FILEMODULE_FILEFUNCTION_H

// standard
#include <fstream>
#include <istream>
#include <memory>

// Zorba
#include <zorba/error.h>
#include <zorba/function.h>
#include <zorba/item.h>
#include <zorba/iterator.h>
#include <zorba/options.h>

namespace zorba {
namespace filemodule {

class FileModule;

///////////////////////////////////////////////////////////////////////////////

class FileFunction : public ContextualExternalFunction {
public:
  virtual String getLocalName() const;
  virtual String getURI() const;

protected:
  FileFunction( FileModule const *module, char const *local_name );

  /**
   * Gets the argument on position pos as a normalised file system path.
   * pos must point to a file function argument. No checks are made.
   */
  String getPathArg( ExternalFunction::Arguments_t const&, unsigned pos ) const;

  /**
   * What a function does with a path, for the security policy.
   */
  enum class Access { Read, Write };

  /**
   * Gets the argument on position pos as a normalised file system path, like
   * getPathArg(), and checks that the security policy of aSctx allows the
   * access (Capability::FileRead or Capability::FileWrite and the read or
   * write paths). Raises zerr:ZXQP0029 if it does not.
   */
  String getPathArg( ExternalFunction::Arguments_t const&, unsigned pos,
                     StaticContext const *aSctx, Access ) const;

  /**
   * Raises zerr:ZXQP0029 unless the security policy of aSctx allows the
   * access to aPath.
   */
  void checkAccess( StaticContext const *aSctx, String const &aPath,
                    Access ) const;

  /**
   * Opens aPath for reading under the security policy of aSctx, checking the
   * file actually opened (see SecurityPolicy::openFile()). Raises
   * zerr:ZXQP0029 if the access is denied and file:FOFL9999 if the file
   * cannot be opened.
   */
  std::unique_ptr<std::istream> openForReading( StaticContext const *aSctx,
                                                String const &aPath,
                                                bool aBinary ) const;

  /**
   * Raises zerr:ZXQP0029 for aPath with the given reason.
   */
  [[noreturn]] void raiseAccessDenied( String const &aPath,
                                       String const &aReason ) const;

  String getEncodingArg( ExternalFunction::Arguments_t const&,
                         unsigned pos ) const;

  String getStringArg( ExternalFunction::Arguments_t const&,
                       unsigned pos ) const;

  String pathToUriString(const String& path) const;

  int raiseFileError( char const *qName, char const *message,
                      String const &path ) const;

  FileModule const *module_;
  char const *const local_name_;        // points to string literal
};

///////////////////////////////////////////////////////////////////////////////

class WriteBinaryFunctionImpl : public FileFunction {
public:
  ItemSequence_t evaluate( ExternalFunction::Arguments_t const&,
                           StaticContext const*,
                           DynamicContext const* ) const;

protected:
  WriteBinaryFunctionImpl( FileModule const*, char const *local_name,
                           bool append );

  bool const append_;
};

///////////////////////////////////////////////////////////////////////////////

class WriteTextFunctionImpl : public FileFunction {
public:
  ItemSequence_t evaluate( ExternalFunction::Arguments_t const&,
                           StaticContext const*,
                           DynamicContext const* ) const;

protected:
  WriteTextFunctionImpl( FileModule const*, char const *local_name,
                         bool append, bool newlines );

  bool const append_;
  bool const newlines_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace filemodule
} // namespace zorba

#endif /* ZORBA_FILEMODULE_FILEFUNCTION_H */
/* vim:set et sw=2 ts=2: */
