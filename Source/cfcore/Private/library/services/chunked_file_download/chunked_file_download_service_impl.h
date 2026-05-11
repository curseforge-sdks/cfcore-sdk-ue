/*MIT License

Copyright (c) 2023 Overwolf Ltd.

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

#include "chunked_file_download_service.h"
#include "models/chunked_download_in_progress.h"
#include "models/chunked_file_download_bytes_range.h"
#include <services/http/models/http_response.h>
#include <services/filesystem/file_system_error.h>

namespace cfcore {
class IApiHelperService;
class IHttpService;
class IHttpFileService;
class IFileSystemService;
class IBuffer;
class ITimerService;
class ILifetimeService;
class ISettingsService;
class IChunkedDownloadPrepService;

// -----------------------------------------------------------------------------
class ChunkedFileDownloadServiceImpl : public IChunkedFileDownloadService {
public:
  ChunkedFileDownloadServiceImpl(
    TSharedRef<IApiHelperService> api_helper_service,
    TSharedRef<IHttpFileService> http_file_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<ITimerService> timer_service,
    TSharedRef<ILifetimeService> lifetime_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IChunkedDownloadPrepService> chunked_download_prep_service);

// IChunkedFileDownloadService
public:
  virtual TFuture<FCFCoreError> Download(
    const FChunkedDownloadParams& params,
    FDownloadInitiatedDelegate download_init_delegate,
    FFileTransferProgressDelegate progress_delegate,
    TOptional<FChunkCompletionDelegate> opt_chunk_completion_delegate) override;

  /**
   * Use the id from: FRequestIdDelegate
   */
  virtual bool CancelDownload(int64 request_id) override;

public:
#if WITH_DEV_AUTOMATION_TESTS
  static const FString ConvertToTempFile(const FString& file_path);
#endif

private:
  TSharedRef<FDownloadInProgress> NewDownload(
    const FChunkedDownloadParams& params,
    FFileTransferProgressDelegate& progress_delegate,
    TOptional<FChunkCompletionDelegate>& opt_chunk_completion_delegate);

  TFuture<FCFCoreError> AssureDestinationFileDeleted(
    TSharedRef<FString> file_path);

  void FinalizeDownload(TSharedRef<FDownloadInProgress> download,
                        TOptional<FCFCoreError> opt_err);

  void StartNextChunkDownload(TSharedRef<FDownloadInProgress> download);

  void CancelActiveChunkDownloads(TSharedRef<FDownloadInProgress> download);

  void FinalizeDownloadWithError(const TSharedRef<FDownloadInProgress> download,
                                 FCFCoreError err);

  bool ShouldPreserveTempLocalFile(const TSharedRef<FDownloadInProgress> dwnld,
                                   FCFCoreError err);

  bool WasDownloadCancelledByUser(FCFCoreError& err);

  void LogDownloadDetails(const TSharedRef<FDownloadInProgress> download,
                          const FChunkedDownloadParams& params);

  void DownloadNextChunks(TSharedRef<FDownloadInProgress> download);

  void SetRangeHeader(const FChunkedFileDownloadBytesRange& range,
                      TMap<FString, FString>& headers);

  int64 DownloadChunk(TSharedRef<FDownloadInProgress> downld,
                      const int64 chunk_id,
                      const FChunkedFileDownloadBytesRange& range,
                      const TMap<FString, FString>& headers);

  void HandleChunkProgress(const TSharedRef<FDownloadInProgress> download,
                           const int64 chunk_id,
                           const FChunkedFileDownloadBytesRange& range,
                           const HttpProgress& http_progress);
  void HandleChunkCompletion(TSharedRef<FDownloadInProgress> download,
                             const int64 chunk_id,
                             const HttpResponse& response,
                             TSharedPtr<IBuffer> buffer);
  void HandleChunkCompletionError(TSharedRef<FDownloadInProgress> downld,
                                  const FChunkDownloadInfo& chunk_info,
                                  const FCFCoreApiResponseError err);

  void HandleFileSystemError(const TSharedRef<FDownloadInProgress> download,
                             EFileSystemError filesys_error);

  void HandleChunkWriteCompletion(TSharedRef<FDownloadInProgress> download,
                                  const FChunkDownloadInfo& chunk_info);

  int64 CalcDownloadedBytesPerSecond(
    int64 total_bytes_downloaded,
    const TSharedRef<FDownloadInProgress> download);
  int32 GetRoundedProgress(int64 downloaded_bytes, int64 file_size_in_bytes);
  bool IsDownloadFinished(const TSharedRef<FDownloadInProgress> download);
  
  bool HasChunksToDownload(const TSharedRef<FDownloadInProgress> download) const;
  bool CanStartMoreChunks(const TSharedRef<FDownloadInProgress> download) const;
  
  void LogChunkFailure(const TSharedRef<FDownloadInProgress> download,
                       const FChunkDownloadInfo& chunk_info,
                       const FCFCoreApiResponseError& api_err);
  void HandleDownloadRetriesExhausted(TSharedRef<FDownloadInProgress> download,
                                     const FCFCoreApiResponseError& api_err);
  void HandleChunkRetriesExhausted(TSharedRef<FDownloadInProgress> download,
                                  const FChunkDownloadInfo& chunk_info,
                                  const FCFCoreApiResponseError& api_err);
  void RetryChunkDownload(TSharedRef<FDownloadInProgress> download,
                          const FChunkDownloadInfo& chunk_info);

private:
  TSharedRef<IApiHelperService> api_helper_service_;
  TSharedRef<IHttpFileService> http_file_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<ITimerService> timer_service_;
  TSharedRef<ILifetimeService> lifetime_service_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IChunkedDownloadPrepService> chunked_download_prep_service_;

  TMap<int64, TSharedRef<FDownloadInProgress>> downloads_in_progress_;
};

} // namespace cfcore