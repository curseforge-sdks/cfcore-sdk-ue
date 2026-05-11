/*MIT License

Copyright (c) 2025 Overwolf Ltd.

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
#include "diagnostics_service_impl.h"

#include <common/constants.h>
#include <common/cfcore_error.h>
#include <common/future_utils.h>
#include <services/logger/models/logs_collection_params.h>

#include <services/logger/logger_macros.h>

#include <services/logger/logger_service.h>
#include <api/cfcore_api_authorized.h>
#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/http/http_file_service.h>
#include <api/cfcore_api_delegates_dispatcher.h>
#include <api/cfcore_api_authorized_delegates.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
DiagnosticsServiceImpl::DiagnosticsServiceImpl(
  TSharedRef<ILoggerService> logger_service,
  TSharedRef<ICFCoreApiAuthorized> api_authorized,
  TSharedRef<IDirectoriesService> directories_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<IHttpFileService> http_file_service,
  TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
  auth_delegates_dispatcher) :
  logger_service_(logger_service),
  api_authorized_(api_authorized),
  directories_service_(directories_service),
  file_system_service_(file_system_service),
  http_file_service_(http_file_service),
  delegates_(MakeShared<CFCoreApiAuthorizedDelegates>()),
  is_diagnostics_in_progress_(false) {

  RegisterCFCoreApiDispatcherDelegates(auth_delegates_dispatcher);
}

// -----------------------------------------------------------------------------
void DiagnosticsServiceImpl::RegisterCFCoreApiDispatcherDelegates(
  TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
    auth_delegates_dispatcher) {

  FGuid id = FGuid::NewGuid();

  delegates_->get_me = ICFCoreApiAuthorized::FGetMeDelegate::CreateLambda(
    [this, auth_delegates_dispatcher, id](
      const TOptional<FMe>& opt_me,
      const TOptional<FCFCoreApiResponseError>& opt_err) {

      if (opt_err.IsSet()) {
        CFCORE_LOG(Warning,
          TEXT("Skipping diagnostics - GetMe failed (%s)"),
          *opt_err->description);
        return;
      }

      if (!opt_me.IsSet()) {
        CFCORE_LOG(Warning,
          TEXT("Skipping diagnostics - GetMe returned no user"));
        return;
      }

      if (!opt_me->diagnostics) {
        return;
      }

      if (is_diagnostics_in_progress_) {
        CFCORE_LOG(Log, TEXT("Skipping diagnostics - already in progress"));
        return;
      }
      is_diagnostics_in_progress_ = true;

      auth_delegates_dispatcher->Unregister(id);
      SendUserLogs();
  });

  auth_delegates_dispatcher->Register(id, delegates_);
}

// -----------------------------------------------------------------------------
void DiagnosticsServiceImpl::SendUserLogs() {
  CFCORE_LOG(Log, TEXT("Starting diagnostics logs collection"));

  auto upload_url = MakeShared<FString>();
  auto zip_file = MakeShared<FString>(CreateZipFilename());
  auto buf = MakeShared<TArray<uint8>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, upload_url]() { return GetLogsCollectionUrl(upload_url); },
      [this, zip_file]() { return ZipLogs(*zip_file); },
      [this, buf, zip_file]() { return ReadZipFileToBuffer(zip_file, buf); },
      [this, upload_url, buf]() { return UploadBuffer(upload_url, buf); },
      [this, zip_file]() { return DeleteZip(zip_file); }
  });

  auto future = FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
  future.Next([this](FCFCoreError err) {
    if (err.isError) {
      CFCORE_LOG(Warning,
        TEXT("Failed to send user logs (code %d, description=%s)"),
        static_cast<int>(err.code),
        *err.description);
    }
    is_diagnostics_in_progress_ = false;
  });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DiagnosticsServiceImpl::ZipLogs(
  const FString& zip_file) {

  CFCORE_LOG(Log, TEXT("Compressing user logs into %s"), *zip_file);

  FLogsCollectionParamsRef params = MakeShared<FLogsCollectionParams>();
  params->zip_file = zip_file;
  params->promise = MakeUnique<TPromise<FCFCoreError>>();

  logger_service_->ZipUserLogs(params);

  return params->promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DiagnosticsServiceImpl::GetLogsCollectionUrl(
  OUT TSharedRef<FString> out_upload_url) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  api_authorized_->DiagnosticsInit(
    ICFCoreApiAuthorized::FDiagnosticsInitDelegate::CreateLambda(
      [out_upload_url, promise](
        const TOptional<FDiagnosticsInitData>& opt_diag_init_data,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

      if (opt_err.IsSet()) {
        CFCORE_LOG(Error,
          TEXT("Failed to initialize diagnostics (%s)"),
          *opt_err->description);
        FCFCoreError err(ECFCoreErrorCodes::ApiError);
        promise->SetValue(err);
        return;
      }

      if (!opt_diag_init_data.IsSet()) {
        CFCORE_LOG(Error,
          TEXT("Failed to initialize diagnostics - missing upload URL"));
        FCFCoreError err(ECFCoreErrorCodes::ApiError);
        promise->SetValue(err);
        return;
      }

      *out_upload_url = opt_diag_init_data->uploadUrl;
      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DiagnosticsServiceImpl::ReadZipFileToBuffer(
  const TSharedRef<FString> zip_file,
  OUT TSharedRef<TArray<uint8>> out_buffer) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = file_system_service_->ReadBuffer(*zip_file);

  future.Next([this, zip_file, out_buffer, promise](ReadBufferRes res) {
    FCFCoreError err;

    if (res.Key != EFileSystemError::None) {
      CFCORE_LOG(Warning,
        TEXT("Failed to read compressed user logs from disk (%s)"),
        **zip_file);
      err = FCFCoreError(ECFCoreErrorCodes::FileSystemError);
      promise->SetValue(err);
      return;
    }

    TSharedPtr<TArray<uint8>> zip_data = res.Value;
    *out_buffer = *zip_data;

    promise->SetValue(err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DiagnosticsServiceImpl::UploadBuffer(
  TSharedRef<FString> upload_url,
  TSharedRef<TArray<uint8>> buffer) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TMap<FString, FString> headers;
  headers.Add(kHttpHeaderContentType, kHttpContentTypeApplicationZip);

  http_file_service_->PutBuffer(
    *upload_url,
    headers,
    buffer,
    IHttpFileService::FProgressDelegate(),
    IHttpFileService::FCompleteDelegate::CreateLambda(
      [this, promise](const HttpResponse& response,
                      TSharedPtr<IBuffer>) {

      FCFCoreError err;
      if (response.statusCode != 200) {
        CFCORE_LOG(Warning,
          TEXT("Failed to upload user logs (%d, %s)"),
          response.statusCode,
          *response.extraInfo);
        err = FCFCoreError(ECFCoreErrorCodes::ApiError);
      }
      else {
        CFCORE_LOG(Log, TEXT("User logs uploaded successfully"));
      }

      promise->SetValue(err);
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DiagnosticsServiceImpl::DeleteZip(
  const TSharedRef<FString> zip_file) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = file_system_service_->DeleteFile(zip_file);

  future.Next([promise](bool success) {
    if (!success) {
      CFCORE_LOG(Warning, TEXT("Failed to delete temp user logs zip file"));
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
FString DiagnosticsServiceImpl::CreateZipFilename() {
  const FString& user_dir = directories_service_->GetUserDataDirectory();
  const FString zip_file = user_dir / TEXT("user_logs.zip");
  return zip_file;
}
