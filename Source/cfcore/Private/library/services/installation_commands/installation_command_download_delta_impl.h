/*MIT License

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
SOFTWARE.*/
#pragma once
#include <CoreMinimal.h>
#include "installation_command.h"

#include <common/macros.h>
#include <api/services/api_helper_service.h>
#include <services/http/http_file_service.h>

#include <api/models/file_delta_diff.h>
#include <api/models/file_transfer_progress.h>
#include <api/models/mod_file_download_url.h>
#include <api/models/direct_download_domain_ex.h>

#include <library/services/chunked_file_download/models/chunked_download_params.h>
#include <library/services/chunked_file_download/models/chunk_completion_info.h>

namespace cfcore {
class IInstallationCommandUtilsService;
class ISettingsService;
class ICFCoreApi;
class ICFCorePremiumMods;
class IHttpService;
class IChunkedFileDownloadService;
class ISharedContextService;
class IChunkCompletionPersistencyService;

// -----------------------------------------------------------------------------
class InstallationCommandDownloadDeltaImpl : public IInstallationCommand {
public:
  InstallationCommandDownloadDeltaImpl(
    TSharedRef<IInstallationCommandUtilsService> utils_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<ICFCoreApi> cfcore_api,
    TSharedRef<IHttpService> http_service,
    TSharedRef<IChunkedFileDownloadService> chunked_file_download_service,
    TSharedRef<ISharedContextService> shared_context_service,
    TSharedRef<IChunkCompletionPersistencyService> chunk_completion_service);
  virtual ~InstallationCommandDownloadDeltaImpl() = default;

// IInstallationCommand
public:
  virtual ECFCoreInstallationCommands GetCommandId() const {
    return ECFCoreInstallationCommands::DownloadDelta;
  }

  virtual TFuture<FCFCoreError> Run(FInstallModRequestPtr request) override;

  virtual bool Cancel(FInstallModRequestPtr request) override;

  virtual void Visit(IInstallationCommandVisitor& visitor) override {};

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(InstallationCommandDownloadDeltaImpl);

  bool IsInstalledModStatusValid(FInstallModRequestPtr request);

  TSharedRef<FChunkedDownloadParams> CreateChunkedDownloadParams(
    FInstallModRequestPtr request,
    const FFileDeltaDiff& file_delta_diff);

  void RecordDownloadedFile(FInstallModRequestPtr request,
                            TSharedRef<FChunkedDownloadParams> params);

  TFuture<FCFCoreError> FillChunkedDownloadParams(
    FInstallModRequestPtr request,
    TSharedRef<FChunkedDownloadParams> params);

  void FillAnalyticsParams(FInstallModRequestPtr request);

  TFuture<FCFCoreError> SetDirectDownloadDomain(
    FInstallModRequestPtr request,
    TSharedRef<FChunkedDownloadParams> params);

  void UpdateDirectDomain(const FDirectDownloadDomainEx& domain_info,
                          TSharedRef<FChunkedDownloadParams> params);

  TFuture<FCFCoreError> PerformDownload(
    FInstallModRequestPtr request,
    TSharedRef<FChunkedDownloadParams> params);

  void HandleDownloadProgress(FInstallModRequestPtr request,
                              const FFileTransferProgress& progress);

private:
  TSharedRef<IInstallationCommandUtilsService> utils_service_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<IHttpService> http_service_;
  TSharedRef<IChunkedFileDownloadService> chunked_file_download_service_;
  TSharedRef<ISharedContextService> shared_context_service_;
  TSharedRef<IChunkCompletionPersistencyService> chunk_completion_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallationCommandDownloadDeltaImpl)