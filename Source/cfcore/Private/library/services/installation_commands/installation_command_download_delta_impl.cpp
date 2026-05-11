/* MIT License

Copyright (c) 2024 Overwolf Ltd.

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
SOFTWARE. */
#include "installation_command_download_delta_impl.h"

#include <common/macros.h>
#include <common/constants.h>
#include "common/delta_diff_utils.h"

#include "installation_command_utils_service.h"

#include "models/file_delta_diff_extra_data.h"

#include <services/logger/logger_macros.h>
#include <api/cfcore_api.h>

#include <services/settings_service.h>
#include <services/http/http_service.h>
#include <library/services/chunked_file_download/chunked_file_download_service.h>
#include <services/user_context/shared_context_service.h>
#include <library/services/installation_commands/chunk_completion_persistency_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandDownloadDeltaImpl::InstallationCommandDownloadDeltaImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<IHttpService> http_service,
  TSharedRef<IChunkedFileDownloadService> chunked_file_download_service,
  TSharedRef<ISharedContextService> shared_context_service,
  TSharedRef<IChunkCompletionPersistencyService> chunk_completion_service) :
  utils_service_(utils_service),
  settings_service_(settings_service),
  cfcore_api_(cfcore_api),
  http_service_(http_service),
  chunked_file_download_service_(chunked_file_download_service),
  shared_context_service_(shared_context_service),
  chunk_completion_service_(chunk_completion_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDownloadDeltaImpl::Run(
  FInstallModRequestPtr request) {

  if (!IsInstalledModStatusValid(request)) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedStrategy;
    CFCORE_LOG(Log,
               TEXT("The mod status doesn't allow for diff (modId=%lld)"),
               request->modId);
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  TSharedPtr<FFileDeltaDiff> diff_info = request->state.file_delta_diff_info;
  if (!diff_info.IsValid()) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedStrategy;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  TSharedRef<FChunkedDownloadParams> params =
    CreateChunkedDownloadParams(request, *diff_info);

  RecordDownloadedFile(request, params);

  SHAREABLE_PROMISE(FCFCoreError, promise);

  // Prepare chunked download params
  FillChunkedDownloadParams(request, params)
    .Next([this, params, request, promise](FCFCoreError err) {

    FillAnalyticsParams(request);

    if (err.isError) {
      promise->SetValue(err);
      return;
    } else if (request->cancelRequest) {
      promise->SetValue(FCFCoreError(ECFCoreErrorCodes::InstallCancelled));
      return;
    }

    // Perform the actual download
    PerformDownload(request, params).Next([promise](FCFCoreError err) {
      promise->SetValue(err);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandDownloadDeltaImpl::Cancel(
  FInstallModRequestPtr request) {

  request->cancelRequest = true;
  return chunked_file_download_service_->CancelDownload(request->cancelId);
}

// -----------------------------------------------------------------------------
bool InstallationCommandDownloadDeltaImpl::IsInstalledModStatusValid(
  FInstallModRequestPtr request) {

  if (request->installedMod.status != EInstalledModStatus::Invalid) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
TSharedRef<FChunkedDownloadParams>
InstallationCommandDownloadDeltaImpl::CreateChunkedDownloadParams(
  FInstallModRequestPtr request,
  const FFileDeltaDiff& file_delta_diff) {

  auto params = MakeShared<FChunkedDownloadParams>();
  const FString& url = file_delta_diff.downloadUrl;

  FString delta_filename = FString::Format(TEXT("{0}_{1}.zip"), {
    file_delta_diff.oldFileId,
    file_delta_diff.newFileId
  });

  params->destination_file_path =
    utils_service_->GetUnderTempDir(delta_filename);
  params->download_url = params->head_request_url = url;
  params->tracking = request->additional_params.tracking;
  params->throttle_download_kbps = request->additional_params.throttleDownloadKbps;

  return params;
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadDeltaImpl::RecordDownloadedFile(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {

  request->state.downloaded_mod_file = params->destination_file_path;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDownloadDeltaImpl::FillChunkedDownloadParams(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {

  if (utils_service_->IsResumableDownloadSupported(request->installedMod)) {
    params->allow_resumable_download = true;
    params->existing_download_info = request->installedMod.downloadInfo;
  }

  return SetDirectDownloadDomain(request, params);
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadDeltaImpl::FillAnalyticsParams(
  FInstallModRequestPtr request) {

  request->analytics_params.isDownload = true;
}

// -----------------------------------------------------------------------------
// Because we are working with the chunked downloader, we need the direct domain
// for downloading the file so that we don't get blocked by the edge servers -
// see FChunkedDownloadParams docs.
TFuture<FCFCoreError>
InstallationCommandDownloadDeltaImpl::SetDirectDownloadDomain(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_api_->GetDirectDownloadDomainEx(
    ICFCoreApi::FGetDirectDownloadDomainExDelegate::CreateLambda(
      [this, promise, params](
        const TOptional<FDirectDownloadDomainEx>& domain_info,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          promise->SetValue(FCFCoreError(opt_err));
          return;
        }

        if (!domain_info.IsSet() || domain_info->domain.IsEmpty()) {
          CFCORE_LOG(Warning,
                     TEXT("Didn't get a direct domain! Using regular url."));
          promise->SetValue(FCFCoreError());
          return;
        }

        UpdateDirectDomain(*domain_info, params);

        promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadDeltaImpl::UpdateDirectDomain(
  const FDirectDownloadDomainEx& domain_info,
  TSharedRef<FChunkedDownloadParams> params) {

  FString& directFileUrl = params->download_url;
  FString old_domain = http_service_->GetUrlDomain(directFileUrl);
  FString new_domain = http_service_->GetUrlDomain(domain_info.domain);

  directFileUrl = directFileUrl.Replace(*old_domain, *new_domain);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDownloadDeltaImpl::PerformDownload(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {

  return chunked_file_download_service_->Download(
    *params,
    IChunkedFileDownloadService::FDownloadInitiatedDelegate::CreateLambda(
      [request](const FDownloadInitiated& download_init) {

      request->cancelId = download_init.cancel_id;
      request->analytics_params.wasResumed = download_init.is_resumed_download;
    }),
    IChunkedFileDownloadService::FFileTransferProgressDelegate::CreateLambda(
      [this, request](const FFileTransferProgress& progress) {

      HandleDownloadProgress(request, progress);
    }),
    IChunkedFileDownloadService::FChunkCompletionDelegate::CreateLambda(
      [this, request](const FChunkCompletionInfo& chunk_info) {

      chunk_completion_service_->HandleChunkCompletion(request, chunk_info);
    })
  );
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadDeltaImpl::HandleDownloadProgress(
  FInstallModRequestPtr request,
  const FFileTransferProgress& progress) {

  FLibraryProgressDataTransfer data_transfer;
  data_transfer.progress = progress.progress;
  data_transfer.transferredBytes = progress.transferredBytes;
  data_transfer.transferRateBytesPerSecond = progress.bytesPerSecond;

  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::Downloading,
                                       &data_transfer);
}
