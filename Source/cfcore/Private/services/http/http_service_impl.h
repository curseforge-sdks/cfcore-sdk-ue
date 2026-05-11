/*MIT License

Copyright (c) 2022 Overwolf Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#pragma once
#include <CoreMinimal.h>
#include <Http.h>
#include <HttpRetrySystem.h>

#include "http_service.h"
#include <common/ue_compatibility_ticker.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class HttpServiceImpl : public IHttpService, public TICKER_OBJECT_BASE {
public:
  HttpServiceImpl();
  virtual ~HttpServiceImpl() = default;

// FTickerObjectBase
public:
  virtual bool Tick(float DeltaTime) override;

// IHttpService
public:
  virtual void Head(const FString& url,
                    const TMap<FString, FString>& headers,
                    FCompleteDelegate delegate) override;

  virtual void Get(const FString& url,
                   const TMap<FString, FString>& headers,
                   FCompleteDelegate delegate) override;

  virtual void Post(const FString& url,
                    const TMap<FString, FString>& headers,
                    const FString& data,
                    FCompleteDelegate delegate) override;

  virtual void Put(const FString& url,
                   const TMap<FString, FString>& headers,
                   const FString& data,
                   FCompleteDelegate delegate) override;

  virtual void Delete(const FString& url,
                      const TMap<FString, FString>& headers,
                      FCompleteDelegate delegate) override;

  virtual FString GetUrlDomain(const FStringView url) override;

private:
  void operator=(const HttpServiceImpl&) = delete;
  HttpServiceImpl(HttpServiceImpl&&) = delete;
  HttpServiceImpl& operator=(HttpServiceImpl&&) = delete;

private:
  FHttpRequestRef CreateBaseHttpRequest(const FString& url,
                                        const TMap<FString, FString>& headers,
                                        FCompleteDelegate delegate);

  void FillHttpResponse(FHttpRequestPtr request,
                        FHttpResponsePtr response,
                        OUT struct HttpResponse& http_response);

  void FillAllHeaders(FHttpResponsePtr response,
                      OUT struct HttpResponse& http_response);

private:
  // NOTE(twolf): In the UE5.4 release - they seem to be checking this is a
  // shared ptr (can't be unique ptr - unless I didn't understand something).
  TSharedPtr<class FHttpRetrySystem::FManager> http_retry_system_;
};

} // namespace cfcore