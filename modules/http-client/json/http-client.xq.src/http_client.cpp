/*
 * Copyright 2006-2013 The FLWOR Foundation.
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
#include <curl/curl.h>
#include <map>
#include <zorba/zorba.h>
#include <zorba/serializer.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/empty_sequence.h>
#include <zorba/user_exception.h>
#include <zorba/internal/unique_ptr.h>

#include "http_request_handler.h"
#include "request_parser.h"
#include "http_response_handler.h"
#include "http_response_parser.h"

#ifdef WIN32
# include <Windows.h>
# define MAX_BUF_SIZE 2048
#endif

namespace zorba {

  namespace http_client {
#ifdef WIN32
static void set_cacert(CURL* lCurl, std::string aPath)
{
  TCHAR path[MAX_BUF_SIZE];
  int r = GetModuleFileName(NULL, path, 2048);
  if (r == -1)
  return;
#	ifdef UNICODE
  char buf[MAX_BUF_SIZE];
  memset(buf, 0, MAX_BUF_SIZE);
  for (int i = 0; i <= r; ++i)
  {
    buf[i] = (char) path[i];
  }
  std::string lPath(buf);
#	else
  std::string lPath(path);
#	endif
  aPath = lPath.substr(0, lPath.rfind('\\'));
  aPath += "\\cacert.pem";
  if(GetFileAttributesA(aPath.c_str()) != INVALID_FILE_ATTRIBUTES)
  curl_easy_setopt(lCurl, CURLOPT_CAINFO, aPath.c_str());
  else
  curl_easy_setopt(lCurl, CURLOPT_SSL_VERIFYPEER, 0L);
}
#endif //WIN32

    class HttpSendFunction : public ContextualExternalFunction {
    protected:
      const ExternalModule*     theModule;
      ItemFactory*              theFactory;
      
    public:
      HttpSendFunction(const ExternalModule* aModule) 
      : theModule(aModule),
      theFactory(Zorba::getInstance(0)->getItemFactory()) {}
      
      virtual ~HttpSendFunction() {}
      
    public:
      virtual String
      getURI() const { return theModule->getURI(); }
      
      virtual String
      getLocalName() const { return "http-sequential-impl"; }
      
      virtual ItemSequence_t 
      evaluate(const ExternalFunction::Arguments_t& args,
               const StaticContext* aStaticContext, const DynamicContext* aDynamicContext)
      const;
    };
    
    class HttpReadFunction : public HttpSendFunction {
    public:
      HttpReadFunction(const ExternalModule* aModule) 
      : HttpSendFunction(aModule) {}
      
      virtual ~HttpReadFunction() {}
      
    public:
      virtual String
      getLocalName() const { return "http-nondeterministic-impl"; }
      
    }; 
    
    class HttpClientModule : public ExternalModule {
    protected:
      class ltstr
      {
      public:
        bool operator()(const String& s1, const String& s2) const
        {
          return s1.compare(s2) < 0;
        }
      };
      
      typedef std::map<String, ExternalFunction*, ltstr> FuncMap_t;
      
      FuncMap_t theFunctions;
      
    public:
      virtual ~HttpClientModule();
      
      HttpClientModule() : theModuleUri("http://zorba.io/modules/http-client")
      {
        for (FuncMap_t::const_iterator lIter = theFunctions.begin();
             lIter != theFunctions.end(); ++lIter) {
          delete lIter->second;
        }
        theFunctions.clear();
      }
      
      virtual String
      getURI() const { return theModuleUri; }
      
      virtual ExternalFunction*
      getExternalFunction(const String& aLocalname)
      {
        ExternalFunction*& lFunc = theFunctions[aLocalname];
        if (!lFunc) {
          if (aLocalname == "http-sequential-impl") {
            lFunc = new HttpSendFunction(this);
          } else if (aLocalname == "http-nondeterministic-impl") {
            lFunc = new HttpReadFunction(this);
          } 
        }
        return lFunc;
      }
      
      virtual void
      destroy()
      {
        if (!dynamic_cast<HttpClientModule*>(this)) {
          return;
        }
        delete this;
      }
      
    private:
      String theModuleUri;
    };

    ItemSequence_t
    general_evaluate(
        const ExternalFunction::Arguments_t& args,
        const StaticContext* aStaticContext,
        const DynamicContext* aDynamicContext,
        ItemFactory* aFactory,
        const String& aTheModuleURI)
    {
      CURL* lCURL = curl_easy_init();
      
      Item lRequest;
      Item lHref;
      Item lContent;

      Iterator_t arg0_iter = args[0]->getIterator();
      arg0_iter->open();
      bool lReqSet = arg0_iter->next(lRequest);
      arg0_iter->close();

      std::string lData;

      std::unique_ptr<HttpRequestHandler> lHandler;
      std::unique_ptr<RequestParser> lParser;
      struct curl_slist* lHeaderList = 0;

      ErrorThrower thrower(aFactory, &lHeaderList,aTheModuleURI);

      if (lReqSet) {
        lHandler.reset(new HttpRequestHandler(lCURL));
        lParser.reset(new RequestParser(lHandler.get(), thrower, aFactory));
        lParser->parseRequest(lRequest);
      }
      //curl_easy_setopt(lCURL, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      //curl_easy_setopt(lCURL, CURLOPT_PROXY, "localhost:8888");
#ifdef WIN32
      std::string caCertPath;
      set_cacert(lCURL, caCertPath);
#endif
      HttpResponseHandler lRespHandler(aFactory, lHeaderList);
      String lOverrideContentType;
      if (lHandler.get())
        lHandler->getOverrideContentType(lOverrideContentType);
      bool lStatusOnly =
          lHandler.get() == NULL ? false : (lHandler->isStatusOnly() || lHandler->isHeadRequest());
      // This gives the ownership of lCurl to the HttpResponseParser
      std::unique_ptr<HttpResponseParser> lRespParser(new HttpResponseParser(lRespHandler, lCURL, thrower,
        lOverrideContentType.c_str(), lStatusOnly));
      int lRetCode = lRespParser->parse();

      if (lRetCode == CURLE_OPERATION_TIMEDOUT)
        thrower.raiseException("TIMEOUT", "A timeout occurred waiting for the response");
      else if (lRetCode)
      {
        thrower.raiseException("HTTP", "An HTTP error occurred");
      }

      // If the Parser is "self contained", that means it didn't create any
      // objects with a lifecycle longer than itself; therefore we should free
      // it (by letting unique_ptr delete it). If the Parser is not self contained,
      // then it will have arranged for some other memory manager to free it
      // later when appropriate; therefore we should NOT let unique_ptr delete it
      // now.
      if ( ! lRespParser->selfContained()) {
        lRespParser.release();
      }
      return ItemSequence_t(lRespHandler.releaseResult());
    }

    ItemSequence_t 
    HttpSendFunction::evaluate(const ExternalFunction::Arguments_t& args,
      const StaticContext* aStaticContext, const DynamicContext* aDynamicContext) const 
    {
      return general_evaluate(args, aStaticContext, aDynamicContext, theFactory, getURI());
    }

    HttpClientModule::~HttpClientModule()
    {
      for (FuncMap_t::const_iterator lIter = theFunctions.begin();
           lIter != theFunctions.end(); ++lIter) {
        delete lIter->second;
      }
      theFunctions.clear();
    }
  } // namespace http_request
} // namespace zorba

#ifdef WIN32
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT __attribute__ ((visibility("default")))
#endif

extern "C" DLL_EXPORT zorba::ExternalModule* createModule() {
  return new zorba::http_client::HttpClientModule();
}
