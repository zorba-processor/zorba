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

#include <zorba/internal/cxx_util.h>
#include <zorba/internal/unique_ptr.h>

#include "context/default_url_resolvers.h"
#include "util/uri_util.h"
#include "util/http_util.h"
#include "util/security_check.h"
#include "util/fs_util.h"
#include "store/api/store.h"
#include "store/api/item_factory.h"
#include "store/api/collection.h"
#include "store/api/item.h"
#include "system/globalenv.h"
#include <context/static_context.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

namespace zorba {

namespace internal {

/**
 * Utility function that identifies URL schemes that will be handled
 * by the HttpStream class.
 */
bool
HTTPURLResolver::isHTTPScheme(zstring const& aUrl)
{
  uri::scheme lScheme = uri::get_scheme(aUrl);
  switch (lScheme) {
    case uri::http:
    case uri::https:
    case uri::ftp:
      return true;
    default:
      return false;
  }
}

/******
 * http: (and https: and ftp:) URL resolver.
 ******/

Resource*
HTTPURLResolver::resolveURL
(zstring const& aUrl, EntityData const* aEntityData)
{
  // HTTP resolution doesn't make sense for collections or thesauri. Whether
  // schemas and modules may be imported over the network is decided by the
  // security policy (Capability::RemoteImport) before this resolver runs.
  switch ( aEntityData->getKind() ) {
    case EntityData::COLLECTION:
#ifndef ZORBA_NO_FULL_TEXT
    case EntityData::THESAURUS:
#endif /* ZORBA_NO_FULL_TEXT */
      return nullptr;

    default:
      break;
  }

  if (!isHTTPScheme(aUrl)) {
    return nullptr;
  }
  // The request runs under the policy of the context that asked, so that
  // redirects and the addresses connected to are checked against it.
  security::ResolutionScope const* const lScope =
    security::ResolutionScope::current();
  try {
    std::unique_ptr<HttpStream> lStream = std::make_unique<HttpStream>(aUrl, lScope ? &lScope->policy() : nullptr);
    lStream->init();
    // Take ownership of the istream and pass it to the StreamResource
    StreamResource* lResource =
        new StreamResource(&(lStream->getStream()), lStream->getStreamReleaser());
    lStream->setStreamReleaser(nullptr);
    return lResource;
  } catch (ZorbaException const& e) {
    // A denial is not a failure to load: report it as such. The http-client
    // module reports a denied redirect or connection with its own error
    // code (http:HTTP, HC001 through the EXPath interface), but always with
    // a security::denial_message(), which names the policy.
    if (e.diagnostic() == zerr::ZXQP0029_URI_ACCESS_DENIED ||
        std::strstr(e.what(), "access denied by security policy"))
      throw security::AccessDenied(e.what());
    throw os_error::exception("", aUrl.c_str(), "Could not create stream resource");
  } catch (...) {
    throw os_error::exception("", aUrl.c_str(), "Could not create stream resource");
  }
}

/******
 * file: URL resolver.
 ******/

static void fileStreamReleaser(std::istream* aStream)
{
  delete aStream;
}

Resource*
FileURLResolver::resolveURL
(zstring const& aUrl, EntityData const* aEntityData)
{
  switch ( aEntityData->getKind() ) {
    case EntityData::COLLECTION:
#ifndef ZORBA_NO_FULL_TEXT
    case EntityData::THESAURUS:
#endif /* ZORBA_NO_FULL_TEXT */
      return nullptr;
    default:
      break;
  }

  uri::scheme lScheme = uri::get_scheme(aUrl);
  if (lScheme != uri::file) {
    return NULL;
  }
  try {
    std::string lPath( fs::normalize_path(aUrl) );
    if (fs::get_type(lPath) == fs::file) {
      // Where the policy restricts file reads, the access was checked before
      // this resolver ran; check again the file actually opened, so that a
      // symbolic link swapped in the meantime cannot lead elsewhere.
      security::ResolutionScope const* const lScope =
        security::ResolutionScope::current();
      if (lScope && !security::file_reads_unrestricted(lScope->policy())) {
        security::Verdict lVerdict;
        std::unique_ptr<std::istream> lChecked(security::open_file(
            lScope->policy(), lPath, lScope->use(), lScope->trustedDirs(),
            false, lVerdict));
        if (!lVerdict.allowed)
          throw security::AccessDenied(
              security::denial_message(aUrl.str(), lVerdict.reason));
        if (!lChecked)
          return NULL;
        return new StreamResource(
            lChecked.release(), &fileStreamReleaser, "", true /* seekable */);
      }
      std::ifstream* lStream = new std::ifstream(lPath.c_str());
      return new StreamResource(
          lStream, &fileStreamReleaser, "", true /* seekable */);
    }
    return NULL;
  }
  catch ( std::invalid_argument const &e ) {
    throw XQUERY_EXCEPTION( err::XPTY0004, ERROR_PARAMS( e.what() ) );
  }
}

/******
 * collection: URL resolver.
 ******/

ZorbaCollectionURLResolver::~ZorbaCollectionURLResolver()
{
}

Resource*
ZorbaCollectionURLResolver::resolveURL
(zstring const& aUrl, EntityData const* aEntityData)
{
  if (aEntityData->getKind() != EntityData::COLLECTION)
    return NULL;
  store::Item_t lName;
  GENV_STORE.getItemFactory()->createQName(lName,
                                           aUrl.c_str(),
                                           "",
                                           "zorba-internal-name-for-w3c-collections");

  store::Collection_t lColl = GENV_STORE.getCollection(lName.getp(), true);
  if ( lColl == NULL ) {
    return NULL;
  } else {
    return new CollectionResource(lColl);
  }
}

} /* namespace zorba::internal */

} /* namespace zorba */

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
