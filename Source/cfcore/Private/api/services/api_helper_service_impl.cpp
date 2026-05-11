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
#include "api_helper_service_impl.h"

#include <JsonUtilities.h>

#include <services/http/models/http_response.h>
#include <common/constants.h>
#include <common/macros.h>
#include <services/logger/logger_macros.h>
#include <api/models/model_responses/game_response.h>
#include <api/common/api_response_error_internal.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
ApiHelperServiceImpl::ApiHelperServiceImpl(
  TSharedPtr<ISettingsService> settings_service,
  TSharedPtr<IHttpService> http_service,
  TSharedPtr<IHttpFileService> http_file_service,
  TSharedPtr<IUserContextService> user_context_service) :
  settings_service_(settings_service),
  http_service_(http_service),
  http_file_service_(http_file_service),
  user_context_service_(user_context_service) {
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::PerformGet(const FString& url,
                                      TSharedPtr<IApiResponseHandler> handler) {
  TMap<FString, FString> headers;
  FillHttpHeaders(headers);

  http_service_->Get(
    url,
    headers,
    IHttpService::FCompleteDelegate::CreateLambda(
      [this, handler](const HttpResponse& http_response) {
        HandleApiResponse(http_response, handler);
      }
    )
  );
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::PerformPost(
  const FString& url,
  const FString& json_data,
  TSharedPtr<IApiResponseHandler> handler) {

  TMap<FString, FString> headers;
  FillHttpHeaders(headers);

  headers.Add(kHttpHeaderContentType, kHttpContentTypeApplicationJson);

  http_service_->Post(
    url,
    headers,
    json_data,
    IHttpService::FCompleteDelegate::CreateLambda(
      [this, handler](const HttpResponse& http_response) {
        HandleApiResponse(http_response, handler);
      }
    )
  );
}

// -----------------------------------------------------------------------------
int64 ApiHelperServiceImpl::PerformPost(
  const FString& url,
  const WWWForm& www_form,
  TSharedPtr<IApiResponseHandler> handler) {

  TMap<FString, FString> headers;
  FillHttpHeaders(headers);

  return http_file_service_->PostWebForm(
    url,
    headers,
    www_form,
    IHttpFileService::FProgressDelegate::CreateLambda(
      [handler](const HttpProgress& progress) {
        handler->HandleProgress(progress);
      }),
    IHttpFileService::FCompleteDelegate::CreateLambda(
      [this, handler](const HttpResponse& http_response, TSharedPtr<IBuffer>) {
        HandleApiResponse(http_response, handler);
      })
  );
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::PerformPut(
  const FString& url,
  const FString& json_data,
  TSharedPtr<IApiResponseHandler> handler) {

  TMap<FString, FString> headers;
  FillHttpHeaders(headers);

  headers.Add(kHttpHeaderContentType, kHttpContentTypeApplicationJson);

  http_service_->Put(
    url,
    headers,
    json_data,
    IHttpService::FCompleteDelegate::CreateLambda(
      [this, handler](const HttpResponse& http_response) {
        HandleApiResponse(http_response, handler);
      }
    )
  );
}

// -----------------------------------------------------------------------------
int64 ApiHelperServiceImpl::PerformPut(
  const FString& url,
  const WWWForm& www_form,
  TSharedPtr<IApiResponseHandler> handler) {

  TMap<FString, FString> headers;
  FillHttpHeaders(headers);

  return http_file_service_->PutWebForm(
    url,
    headers,
    www_form,
    IHttpFileService::FProgressDelegate::CreateLambda(
      [handler](const HttpProgress& progress) {
        handler->HandleProgress(progress);
      }),
    IHttpFileService::FCompleteDelegate::CreateLambda(
      [this, handler](const HttpResponse& http_response, TSharedPtr<IBuffer>) {
        HandleApiResponse(http_response, handler);
      })
  );
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::PerformDelete(
  const FString& url,
  TSharedPtr<IApiResponseHandler> handler) {

  TMap<FString, FString> headers;
  FillHttpHeaders(headers);

  http_service_->Delete(
    url,
    headers,
    IHttpService::FCompleteDelegate::CreateLambda(
      [this, handler](const HttpResponse& http_response) {
        HandleApiResponse(http_response, handler);
      }
    )
  );
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::CancelFileOperation(int64 operation_id) {
  http_file_service_->Abort(operation_id);
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::GetApiRequestHeaders(
  TMap<FString, FString>& headers) {
  FillHttpHeaders(headers);
}

// -----------------------------------------------------------------------------
TOptional<FCFCoreApiResponseError> ApiHelperServiceImpl::GetApiResponseError(
  const HttpResponse& http_response) {

  if (!IsError(http_response)) {
    return {};
  }

  FCFCoreApiResponseError error;

  if (http_response.cancelled) {
    error.cancelled = true;
    error.description = kApiResponseErrorRequestCancelled;
    return error;
  }

  if (http_response.nonHttpError) {
    error.serverUnreachable = true;
    error.description = FString::Format(TEXT("{0} (serverUnreachable)"), {
      http_response.extraInfo
    });

    return error;
  }

  error.description = FString::Format(
    kApiResponseErrorFailedToSendRequest, {
      http_response.statusCode
    });

  switch (http_response.statusCode) {
  case 400:
    error.badRequest = true;
    break;
  case 401:
    error.tokenExpired = true;
    error.description = kApiResponseErrorUserTokenExpired;
    break;
  case 403:
    error.missingPriviliges = true;
    error.description = kApiResponseErrorMissingPriviliges;
    break;
  case 404:
    error.entityNotFound = true;
    break;
  case 410: // GONE
    error.resourceExpired = true;
    error.description = kApiResponseErrorResourceExpired;
    break;
  case 500:
  case 502:
  case 503:
  case 504:
    error.serverUnreachable = true;
    break;
  }

  FillApiError(http_response, error);

  return error;
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::FillApiError(const HttpResponse& http_response,
                                        FCFCoreApiResponseError& error) {
  if (!http_response.contentType.StartsWith(kHttpContentTypeApplicationJson)) {
    return;
  }

  FApiResponseErrorInternal api_response_error;
  bool success = FJsonObjectConverter::JsonObjectStringToUStruct(
    http_response.body, &api_response_error, 0, 0);

  if (!success) {
    return;
  }

  if (!api_response_error.message.IsEmpty()) {
    error.description = api_response_error.message;
  }

  error.errorCode = api_response_error.errorCode;
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::FillHttpHeaders(TMap<FString, FString>& headers) {
  FString user_agent = settings_service_->GetInternalSettings().userAgent;
  FString platform = FormatPlatformHeaderValue();

  headers.Add(kHttpHeaderUserAgent, user_agent);
  headers.Add(kHttpHeaderPlatform, platform);
  SetProviderHeader(headers);

  FString api_key = settings_service_->GetSettings().apiKey;
  if (!api_key.IsEmpty()) {
    headers.Add(kHttpHeaderApiKey, api_key);
  }

  if (settings_service_->GetSettings().isServer) {
    headers.Add(kHttpHeaderModServerRequest, "true");
  }

  SetAuthTokenHeader(headers);
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::SetProviderHeader(TMap<FString, FString>& headers) {
  ECFCoreExternalAuthProvider provider =
    settings_service_->GetSettings().provider;

  if (provider == ECFCoreExternalAuthProvider::None) {
    return;
  }

  ENUM_TO_STR(str_provider, provider);
  headers.Add(kHttpHeaderProvider, str_provider);
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::SetAuthTokenHeader(TMap<FString, FString>& headers) {
  FString auth_token = user_context_service_->UserContextInfo().authToken;
  if (auth_token.IsEmpty()) {
    return;
  }

  FString auth_header = FString::Format(TEXT("Bearer {0}"), {*auth_token});
  headers.Add(kHttpHeaderAuthorization, auth_header);
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::HandleApiResponse(
  const HttpResponse& http_response,
  TSharedPtr<IApiResponseHandler> handler) {

  auto error = GetApiResponseError(http_response);
  if (!error.IsSet()) {
    handler->HandleResponse(http_response);
    return;
  }

  HandleErrors(error.GetValue());
  handler->HandleApiResponseError(error.GetValue());
}

// -----------------------------------------------------------------------------
bool ApiHelperServiceImpl::IsError(const HttpResponse& http_response) {
  if (http_response.nonHttpError) {
    return true;
  }

  return http_response.statusCode < 200 || http_response.statusCode >= 400;
}

// -----------------------------------------------------------------------------
void ApiHelperServiceImpl::HandleErrors(const FCFCoreApiResponseError& error) {
  if (error.tokenExpired) {
    CFCORE_LOG(Log, TEXT("Got TokenExpired - erasing local token"));
    user_context_service_->SetAuthToken("");
  }
}

// -----------------------------------------------------------------------------
FString ApiHelperServiceImpl::FormatPlatformHeaderValue() {
  FString platform = settings_service_->GetInternalSettings().platform;

  if (settings_service_->GetSettings().isServer) {
    if (platform.EndsWith(kServerSuffix)) {
      return platform;
    }

    platform = FString::Format(TEXT("{0}_{1}"), { platform, kServerSuffix });
  }

  return platform;
}

