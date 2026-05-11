/* MIT License

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
SOFTWARE. */
#include "installation_command_download_impl.h"

#include <common/macros.h>
#include <common/future_utils.h>

#include "installation_command_utils_service.h"

#include <services/logger/logger_macros.h>
#include <api/cfcore_api.h>

#include <services/settings_service.h>
#include <services/http/http_service.h>
#include <library/services/chunked_file_download/chunked_file_download_service.h>
#include <library/services/installation_commands/chunk_completion_persistency_service.h>
#include <premium/cfcore_premium_mods.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandDownloadImpl::InstallationCommandDownloadImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<ICFCorePremiumMods> cfcore_premium_mods,
  TSharedRef<IHttpService> http_service,
  TSharedRef<IChunkedFileDownloadService> chunked_file_download_service,
  TSharedRef<IChunkCompletionPersistencyService> chunk_completion_service) :
  utils_service_(utils_service),
  settings_service_(settings_service),
  cfcore_api_(cfcore_api),
  cfcore_premium_mods_(cfcore_premium_mods),
  http_service_(http_service),
  chunked_file_download_service_(chunked_file_download_service),
  chunk_completion_service_(chunk_completion_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDownloadImpl::Run(
  FInstallModRequestPtr req) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TSharedRef<FChunkedDownloadParams> params = CreateChunkedDownloadParams(req);

  RecordDownloadedFile(req, params);

  // Prepare chunked download params
  FillChunkedDownloadParams(req, params)
    .Next([this, params, req, promise](FCFCoreError err) {

    FillAnalyticsParams(req);

    if (err.isError) {
      promise->SetValue(err);
      return;
    } else if (req->cancelRequest) {
      promise->SetValue(FCFCoreError(ECFCoreErrorCodes::InstallCancelled));
      return;
    }

    // Perform the actual download
    PerformDownload(req, params, promise).Next([promise](FCFCoreError err) {
      promise->SetValue(err);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandDownloadImpl::Cancel(FInstallModRequestPtr request) {
  request->cancelRequest = true;
  return chunked_file_download_service_->CancelDownload(request->cancelId);
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadImpl::Visit(
  IInstallationCommandVisitor& visitor) {

  if (!visitor.ShouldInitialize()) {
    return;
  }
}

// -----------------------------------------------------------------------------
TSharedRef<FChunkedDownloadParams>
InstallationCommandDownloadImpl::CreateChunkedDownloadParams(
  FInstallModRequestPtr request) {

  auto info = MakeShared<FChunkedDownloadParams>();
  const FFile& file = request->installedMod.latestUpdatedFile;
  const FString& url = file.downloadUrl;

  info->destination_file_path = utils_service_->GetUnderTempDir(file.fileName);
  info->download_url = info->head_request_url = url;

  info->tracking = request->additional_params.tracking;
  info->throttle_download_kbps = request->additional_params.throttleDownloadKbps;

  return info;
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadImpl::RecordDownloadedFile(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {

  const FFile& file = request->installedMod.latestUpdatedFile;
  request->state.downloaded_mod_file = params->destination_file_path;
  request->state.file_info = MakeShared<FFile>(file);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDownloadImpl::FillChunkedDownloadParams(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {


  if (utils_service_->IsResumableDownloadSupported(request->installedMod)) {
    params->allow_resumable_download = true;
    params->existing_download_info = request->installedMod.downloadInfo;
  }

  using TSeqFuncArray = TArray<TSequenceFunction<FCFCoreError>>;
  TSharedRef<bool> temp_file_exists = MakeShared<bool>(false);

  auto functions = MakeShared<TSeqFuncArray>(TSeqFuncArray {
    [this, request, params]() {
      return SetPremiumModUrls(request, params);
    },
    [this, request, params]() {
      return SetDirectDownloadDomain(request, params);
    }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadImpl::FillAnalyticsParams(
  FInstallModRequestPtr request) {

  request->analytics_params.isDownload = true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDownloadImpl::SetPremiumModUrls(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params) {

  bool is_server = settings_service_->GetSettings().isServer;

  // NOTE(twolf): Server mods are never protected and thus should be treated
  // as non-premium mods.
  // TODO(twolf): Consider allowing GetModFileDownloadUrl to support non-premium
  // mods, so that we don't need this extra edge case in the SDK.
  if (is_server || !request->installedMod.details.premiumDetails.isPremium) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  const FFile& file = request->installedMod.latestUpdatedFile;

  cfcore_premium_mods_->GetModFileDownloadUrl(
    request->installedMod.details.id,
    file.id,
    ICFCorePremiumMods::FGetModFileDownloadUrlDelegate::CreateLambda(
      [this, params, promise](
        const TOptional<FModFileDownloadUrl>& download_url,
        const TOptional<FCFCoreError>& err) {

      if (err.IsSet()) {
        promise->SetValue(err);
        return;
      }

      params->download_url = CalcDownloadUrl(*download_url);
      params->head_request_url = params->download_url;

      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
FString InstallationCommandDownloadImpl::CalcDownloadUrl(
  const FModFileDownloadUrl& download_url) {

  if (download_url.securedParams.IsEmpty()) {
    return download_url.url;
  }

  FString separator = "?";
  if (download_url.url.Contains("?")) {
    separator = "&";
  }

  return FString::Format(TEXT("{0}{1}{2}"), {
    download_url.url,
    separator,
    download_url.securedParams
  });
}

// -----------------------------------------------------------------------------
// Because we are working with the chunked downloader, we need the direct domain
// for downloading the file so that we don't get blocked by the edge servers -
// see FChunkedDownloadParams docs.
TFuture<FCFCoreError> InstallationCommandDownloadImpl::SetDirectDownloadDomain(
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
          promise->SetValue({});
          return;
        }

        UpdateDirectDomain(*domain_info, params);

        promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void InstallationCommandDownloadImpl::UpdateDirectDomain(
  const FDirectDownloadDomainEx& domain_info,
  TSharedRef<FChunkedDownloadParams> params) {

  FString& directFileUrl = params->download_url;
  FString old_domain = http_service_->GetUrlDomain(directFileUrl);
  FString new_domain = http_service_->GetUrlDomain(domain_info.domain);

  directFileUrl = directFileUrl.Replace(*old_domain, *new_domain);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDownloadImpl::PerformDownload(
  FInstallModRequestPtr request,
  TSharedRef<FChunkedDownloadParams> params,
  TSharedRef<TPromise<FCFCoreError>> promise) {

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
void InstallationCommandDownloadImpl::HandleDownloadProgress(
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
