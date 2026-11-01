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
#include "http_client.h"

  #include <time.h>

#include <curl/curl.h>
#include <map>
#include <zorba/serializer.h>
#include <zorba/external_module.h>
#include <zorba/empty_sequence.h>
#include <zorba/user_exception.h>
#include <zorba/internal/unique_ptr.h>
#include <zorba/vector_item_sequence.h>

#include "http_request_handler.h"
#include "request_parser.h"
#include "http_response_handler.h"
#include "http_response_parser.h"
#include "util/curl_streambuf.h"

namespace zorba {

namespace http_client {


ExternalFunction*
HttpClientModule::getExternalFunction(const String& aLocalname)
{
  ExternalFunction*& lFunc = theFunctions[aLocalname];
  if (!lFunc) {
    if (aLocalname == "http-sequential-impl") {
      lFunc = new HttpSendFunction(this);
    } else if (aLocalname == "http-nondeterministic-impl") {
      lFunc = new HttpNondeterministicFunction(this);
    } else if (aLocalname == "http-deterministic-impl") {
      lFunc = new HttpDeterministicFunction(this);
    }
  }
  return lFunc;
}

int HttpSendFunction::doRequest(Request& aRequest, ItemSequence_t& aResult,
                                const SecurityPolicy& aPolicy) const
{
  ProfileWrapper lProfileWrapper(aRequest, theJSONProfileData, theXMLProfileData);
  CURL* lCURL = curl::create();

  std::unique_ptr<HttpRequestHandler> lHandler;
  struct curl_slist* lHeaderList = 0;
  zorba::String lUri = getURI();
  ErrorThrower lThrower(theFactory, &lHeaderList, lUri);

  lHandler.reset(new HttpRequestHandler(lCURL));

  lHandler->sendRequest(aRequest);

  // Every connection (redirects included) is checked against the policy.
  curl::restrict_to_policy(lCURL, aPolicy);


  HttpResponseHandler lRespHandler(theFactory, lHeaderList);
  bool lStatusOnly =
      lHandler.get() == NULL ? false : (aRequest.theOptions.theStatusOnly || aRequest.theMethod == "HEAD");

  // This gives the ownership of lCurl to the HttpResponseParser
  std::unique_ptr<HttpResponseParser> lRespParser(new HttpResponseParser(lRespHandler, lCURL, lThrower,
      aRequest.theOptions.theOverrideContentType.c_str(), lStatusOnly));
  CURLcode lRetCode = lRespParser->parse();

  // A redirect or connection refused by the security policy is an HTTP
  // error; the message names the policy and the reason.
  std::string lDenied;
  if (lRetCode && curl::policy_denied(lCURL, &lDenied))
    lThrower.raiseException("HTTP", lDenied);

  if (lRetCode == CURLE_OPERATION_TIMEDOUT)
    lThrower.raiseException("TIMEOUT", "A timeout occurred waiting for the response");
  else if (lRetCode)
  {
    std::string lErrorStr(curl_easy_strerror(lRetCode));
    lThrower.raiseException("HTTP", "An HTTP error occurred: " + lErrorStr);
  }

  int lStatus = lRespParser->getStatus();

  // If the Parser is "self contained", that means it didn't create any
  // objects with a lifecycle longer than itself; therefore we should free
  // it (by letting unique_ptr delete it). If the Parser is not self contained,
  // then it will have arranged for some other memory manager to free it
  // later when appropriate; therefore we should NOT let unique_ptr delete it
  // now.
  if ( ! lRespParser->selfContained())
    // The parser is owned by the stream it was attached to.
    (void)lRespParser.release();

  aResult = lRespHandler.releaseResult();
  return lStatus;
}

ItemSequence_t
HttpSendFunction::evaluate(const ExternalFunction::Arguments_t& args,
    const StaticContext* aStaticContext, const DynamicContext* aDynamicContext) const
{
  Item lRequestItem;
  Iterator_t arg0_iter = args[0]->getIterator();
  arg0_iter->open();
  arg0_iter->next(lRequestItem);
  arg0_iter->close();

  zorba::String lUri = getURI();
  ErrorThrower lThrower(theFactory, NULL, lUri);
  RequestParser lParser = RequestParser(lThrower, theFactory);
  Request lRequest;

  lParser.parseRequest(lRequestItem, lRequest);

  // The URL must be allowed by the policy of the calling context; redirects
  // and the addresses connected to are checked during the transfer.
  SecurityPolicy const lPolicy = aStaticContext
    ? aStaticContext->getSecurityPolicy()
    : Zorba::getInstance(0)->getSecurityPolicy();
  if (!lRequest.theHref.empty())
  {
    String lReason;
    if (!lPolicy.checkURI(lRequest.theHref, EntityData::DOCUMENT, false,
                          &lReason))
      lThrower.raiseException("HTTP",
                              std::string("\"") + lRequest.theHref.str() +
                              "\": access denied by security policy (" +
                              lReason.str() + ")");
  }

  RetrySpecification& lRetrySpec = lRequest.theOptions.theRetrySpec;

  if (lRetrySpec.theRetry)
  {
    std::vector<int>& lDelays = lRetrySpec.theRetryDelays;
    ItemSequence_t lResult;
    for (unsigned int i = 0; i<lDelays.size(); ++i)
    {
      try
      {
        int lStatus = doRequest(lRequest, lResult, lPolicy);

        if (std::find(lRetrySpec.theRetryStatuses.begin(), lRetrySpec.theRetryStatuses.end(), lStatus) == lRetrySpec.theRetryStatuses.end())
          return lResult;
      }
      catch (ZorbaException& ex)
      {
        if (!lRetrySpec.theRetryOnConnectionError)
          throw ex;
      }

      int lMillis = lDelays[i];
      const int32_t lFactor = 1000000;
      struct timespec lTs;
      lTs.tv_sec = lMillis / 1000;
      lTs.tv_nsec = (lMillis % 1000) * lFactor;
      nanosleep(&lTs, 0);
    }
    doRequest(lRequest, lResult, lPolicy);
    return lResult;
  }
  else
  {
    ItemSequence_t lResult;
    doRequest(lRequest, lResult, lPolicy);
    return lResult;
  }
}


} // namespace http_request
} // namespace zorba
/* vim:set et sw=2 ts=2: */
