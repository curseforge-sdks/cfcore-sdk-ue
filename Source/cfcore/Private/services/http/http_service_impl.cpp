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
#include "http_service_impl.h"

using namespace cfcore;

const float kRequestTimeoutInSecs = 10;
const uint32 kMaxRetries = 4;
const double kRetryDelayInSeconds = 40;

// -----------------------------------------------------------------------------
HttpServiceImpl::HttpServiceImpl() :
  TICKER_OBJECT_BASE(0.0f, BACKGROUNDABLE_TICKER::GetCoreTicker()) {

  http_retry_system_ = MakeShared<FHttpRetrySystem::FManager>(
    FHttpRetrySystem::FRetryLimitCountSetting(kMaxRetries),
    FHttpRetrySystem::FRetryTimeoutRelativeSecondsSetting(kRetryDelayInSeconds)
  );
}

// -----------------------------------------------------------------------------
bool HttpServiceImpl::Tick(float DeltaTime) {
  if (http_retry_system_.IsValid()) {
#if ENGINE_MAJOR_VERSION == 4 || (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION < 6)
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    http_retry_system_->Update();
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif
  }

  return true;
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::Head(const FString& url,
                           const TMap<FString, FString>& headers,
                           FCompleteDelegate delegate) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers, delegate);
  request->SetVerb("HEAD");
  request->ProcessRequest();
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::Get(const FString& url,
                          const TMap<FString, FString>& headers,
                          FCompleteDelegate delegate) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers, delegate);
  request->SetVerb("GET");
  request->ProcessRequest();
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::Post(const FString& url,
									         const TMap<FString, FString>& headers,
										       const FString& data,
									         FCompleteDelegate delegate) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers, delegate);
  request->SetVerb("POST");
  request->SetContentAsString(data);
  request->ProcessRequest();
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::Put(const FString& url,
                          const TMap<FString, FString>& headers,
                          const FString& data,
                          FCompleteDelegate delegate) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers, delegate);
  request->SetVerb("PUT");
  request->SetContentAsString(data);
  request->ProcessRequest();
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::Delete(const FString& url,
                             const TMap<FString, FString>& headers,
                             FCompleteDelegate delegate) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers, delegate);
  request->SetVerb("DELETE");
  request->ProcessRequest();
}

// -----------------------------------------------------------------------------
FString HttpServiceImpl::GetUrlDomain(const FStringView url) {
  return FGenericPlatformHttp::GetUrlDomain(url);
}

// -----------------------------------------------------------------------------
FHttpRequestRef HttpServiceImpl::CreateBaseHttpRequest(
  const FString& url,
  const TMap<FString, FString>& headers,
  FCompleteDelegate delegate) {

  FHttpRequestRef request = http_retry_system_->CreateRequest(
    FHttpRetrySystem::FRetryLimitCountSetting(kMaxRetries),
    FHttpRetrySystem::FRetryTimeoutRelativeSecondsSetting(kRetryDelayInSeconds),
    FHttpRetrySystem::FRetryResponseCodes({
      500, 501, 502, 504
    }),
    FHttpRetrySystem::FRetryVerbs({
      FName(TEXT("GET")),
      FName(TEXT("POST")),
      FName(TEXT("HEAD"))
    }),
    FHttpRetrySystem::FRetryDomainsPtr()
  );

  // FHttpRequestRef request = FHttpModule::Get().CreateRequest();
  // TODO(twolf): Refactor - we got failures when downloading files with spaces
  // but this might only be a problem with fiddler.
  FString encoded_url = url.Replace(TEXT(" "), TEXT("%20"));
  request->SetURL(encoded_url);

  for (const TPair<FString, FString>& pair : headers) {
    request->SetHeader(pair.Key, pair.Value);
  }

  request->OnProcessRequestComplete().BindLambda(
    [this, delegate](FHttpRequestPtr request,
                     FHttpResponsePtr response,
                     bool connected_successfully) {
      struct HttpResponse http_response = { 0 };
      FillHttpResponse(request, response, http_response);
      delegate.ExecuteIfBound(http_response);
    }
  );

#if ENGINE_MAJOR_VERSION >= 5 || ENGINE_MINOR_VERSION >= 26
  request->SetTimeout(kRequestTimeoutInSecs);
#endif

  return request;
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::FillHttpResponse(
  FHttpRequestPtr request,
  FHttpResponsePtr response,
  OUT struct HttpResponse& http_response) {

  http_response = {};
  EHttpRequestStatus::Type status = request->GetStatus();

  if (response.IsValid()) {
    FillAllHeaders(response, http_response);
    http_response.statusCode = response->GetResponseCode();
    http_response.contentType = response->GetContentType();
    http_response.body = response->GetContentAsString();
  }

  if (status != EHttpRequestStatus::Type::Succeeded) {
    http_response.nonHttpError = true;
    http_response.extraInfo = FString::Format(TEXT("ApiError: {0}"), {
      EHttpRequestStatus::ToString(status)
    });
  }
}

// -----------------------------------------------------------------------------
void HttpServiceImpl::FillAllHeaders(FHttpResponsePtr response,
  OUT struct HttpResponse& http_response) {
  TArray<FString> all_headers = response->GetAllHeaders();

  for (const FString& header : all_headers) {
    TArray<FString> split;
    if (header.ParseIntoArray(split, TEXT(":")) == 2) {
      http_response.allHeaders.Add(split[0], split[1]);
    }
  }
}
