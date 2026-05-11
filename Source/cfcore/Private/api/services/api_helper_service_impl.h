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
#include "CoreMinimal.h"
#include "api_helper_service.h"
#include <common/macros.h>
#include <services/settings_service.h>
#include <services/http/http_service.h>
#include <services/http/http_file_service.h>
#include <services/user_context/user_context_service.h>
#include <api/common/cfcore_api_response_error.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ApiHelperServiceImpl : public IApiHelperService {
public:
  ApiHelperServiceImpl(TSharedPtr<ISettingsService> settings_service,
                       TSharedPtr<IHttpService> http_service,
                       TSharedPtr<IHttpFileService> http_file_service,
                       TSharedPtr<IUserContextService> user_context_service);

// IApiHelperService
public:
  virtual void PerformGet(const FString& url,
                          TSharedPtr<IApiResponseHandler> handler) override;

  virtual void PerformPost(const FString& url,
                           const FString& json_data,
                           TSharedPtr<IApiResponseHandler> handler) override;

	virtual int64 PerformPost(const FString& url,
                            const WWWForm& www_form,
                            TSharedPtr<IApiResponseHandler> handler) override;

	virtual void PerformPut(const FString& url,
                          const FString& json_data,
                          TSharedPtr<IApiResponseHandler> handler) override;

	virtual int64 PerformPut(const FString& url,
                           const WWWForm& www_form,
                           TSharedPtr<IApiResponseHandler> handler) override;

	virtual void PerformDelete(const FString& url,
														 TSharedPtr<IApiResponseHandler> handler) override;

  virtual void CancelFileOperation(int64 operation_id) override;

  virtual void GetApiRequestHeaders(TMap<FString, FString>& headers) override;

  virtual TOptional<FCFCoreApiResponseError> GetApiResponseError(
    const HttpResponse& http_response) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ApiHelperServiceImpl);

  void FillApiError(const HttpResponse& http_response,
                    FCFCoreApiResponseError& error);
  void FillHttpHeaders(TMap<FString, FString>& headers);
  void SetProviderHeader(TMap<FString, FString>& headers);
  void SetAuthTokenHeader(TMap<FString, FString>& headers);

  void HandleApiResponse(const HttpResponse& http_response,
                         TSharedPtr<IApiResponseHandler> handler);
  bool IsError(const HttpResponse& http_response);
  void HandleErrors(const FCFCoreApiResponseError& error);
  FString FormatPlatformHeaderValue();

private:
  TSharedPtr<ISettingsService> settings_service_;
  TSharedPtr<IHttpService> http_service_;
  TSharedPtr<IHttpFileService> http_file_service_;
  TSharedPtr<IUserContextService> user_context_service_;
};

} // namespace cfcore