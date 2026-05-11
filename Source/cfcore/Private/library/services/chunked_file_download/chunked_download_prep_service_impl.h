/*MIT License

Copyright (c) 2026 Overwolf Ltd.

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
#include <common/cfcore_error.h>

#include "chunked_download_prep_service.h"

#include "services/http/models/http_response.h"

namespace cfcore {
class IApiHelperService;
class IHttpService;
class IFileSystemService;

class ChunkedDownloadPrepServiceImpl : public IChunkedDownloadPrepService {
public:
  ChunkedDownloadPrepServiceImpl(
    TSharedRef<IApiHelperService> api_helper_service,
    TSharedRef<IHttpService> http_service,
    TSharedRef<IFileSystemService> file_system_service);

public:
  virtual TFuture<FCFCoreError> PrepareDownloadParams(
    const FChunkedDownloadParams& params,
    TSharedRef<FDownloadInProgress> download) override;

private:
  TFuture<FCFCoreError> AssureDestinationFileDeleted(
      TSharedRef<FString> file_path);

  TFuture<FCFCoreError> SetIsResumedDownload(
    const FChunkedDownloadParams& params,
    TSharedRef<FDownloadInProgress> download);

  bool IsValidResumableDownload(const FInstalledModDownloadInfo& existing);

  TFuture<FCFCoreError> CheckIfFileExists(
    TSharedRef<FString> file_path,
    TSharedRef<bool> out_exists);

  TFuture<FCFCoreError> GetRemoteFileSize(
    const FChunkedDownloadParams& params,
    TSharedRef<FDownloadInProgress> download);

  FString GetHeaderRequestUrlWithTracking(
    const FChunkedDownloadParams& params,
    TSharedRef<FDownloadInProgress> download);

  FString GetTrackingParamsString(const TMap<FString, FString>& tracking);

  TOptional<FCFCoreApiResponseError> FillDownloadWithFileSize(
    const HttpResponse& response,
    TSharedRef<FDownloadInProgress> download);

  TFuture<FCFCoreError> GenerateChunkDownloadsPlan(
    const FChunkedDownloadParams& params,
    TSharedRef<FDownloadInProgress> download);

  void GenerateChunkDownloadsPlanForGap(
    int64 gap_size,
    int64 current_file_position,
    TSharedRef<FDownloadInProgress> download);

  TFuture<FCFCoreError> CleanupStaleResumableFile(
    const FChunkedDownloadParams& params,
    const TSharedRef<FDownloadInProgress> download);

private:
  TSharedRef<IApiHelperService> api_helper_service_;
  TSharedRef<IHttpService> http_service_;
  TSharedRef<IFileSystemService> file_system_service_;
};

} // namespace cfcore