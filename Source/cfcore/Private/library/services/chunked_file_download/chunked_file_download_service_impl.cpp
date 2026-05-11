/* MIT License

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
SOFTWARE. */
#include "chunked_file_download_service_impl.h"

#include "chunked_download_prep_service.h"

#include <common/macros.h>
#include <common/constants.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include <api/services/api_helper_service.h>
#include <services/http/http_file_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/timer/timer_service.h>
#include <services/lifetime/lifetime_service.h>
#include <services/settings_service.h>

namespace cfcore {
using OptError = TOptional<FCFCoreError>;
// NOTE: Changing the following constant will break the build scripts
constexpr int64 kMaxDownloadChunkSizeInBytes = 100 * 1024 * 1024;
const int32 kMaxPerDownloadRetries = 10;
const int32 kMinParallelChunks = 1;
const int32 kMaxParallelChunks = 32;

const TCHAR kDownloadTmpExtension[] = TEXT(".tmp");
};

using namespace cfcore;

// -----------------------------------------------------------------------------
ChunkedFileDownloadServiceImpl::ChunkedFileDownloadServiceImpl(
  TSharedRef<IApiHelperService> api_helper_service,
  TSharedRef<IHttpFileService> http_file_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<ITimerService> timer_service,
  TSharedRef<ILifetimeService> lifetime_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IChunkedDownloadPrepService> chunked_download_prep_service) :
  api_helper_service_(api_helper_service),
  http_file_service_(http_file_service),
  file_system_service_(file_system_service),
  timer_service_(timer_service),
  lifetime_service_(lifetime_service),
  settings_service_(settings_service),
  chunked_download_prep_service_(chunked_download_prep_service){
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ChunkedFileDownloadServiceImpl::Download(
  const FChunkedDownloadParams& params,
  FDownloadInitiatedDelegate download_init_delegate,
  FFileTransferProgressDelegate prog_delegate,
  TOptional<FChunkCompletionDelegate> opt_chunk_completion_delegate) {

  TSharedRef<FDownloadInProgress> download = NewDownload(
    params,
    prog_delegate,
    opt_chunk_completion_delegate);

  downloads_in_progress_.Add(download->id, download);

  chunked_download_prep_service_->PrepareDownloadParams(params, download)
    .Next([this, download_init_delegate, download, params](FCFCoreError err) {

    if (err.isError) {
      FinalizeDownload(download, FCFCoreError(err));
      return;
    }

    FDownloadInitiated download_init;
    download_init.cancel_id = download->id;
    download_init.is_resumed_download = download->isResumedDownload;
    download_init_delegate.ExecuteIfBound(download_init);

    LogDownloadDetails(download, params);

    DownloadNextChunks(download);
  });

  return download->promise.GetFuture();
}

// -----------------------------------------------------------------------------
bool ChunkedFileDownloadServiceImpl::CancelDownload(int64 request_id) {
  if (!downloads_in_progress_.Contains(request_id)) {
    CFCORE_LOG(
      Warning,
      TEXT("Request to cancel download that is not in progress (id=%lld"),
      request_id);
    return false;
  }

  auto download = downloads_in_progress_[request_id];
  download->cancelRequest = true;
  CancelActiveChunkDownloads(download);

  return true;
}

#if WITH_DEV_AUTOMATION_TESTS
// -----------------------------------------------------------------------------
// static
const FString ChunkedFileDownloadServiceImpl::ConvertToTempFile(
  const FString& file_path) {

  return file_path + kDownloadTmpExtension;
}
#endif

// -----------------------------------------------------------------------------
TSharedRef<FDownloadInProgress> ChunkedFileDownloadServiceImpl::NewDownload(
  const FChunkedDownloadParams& params,
  FFileTransferProgressDelegate& progress_delegate,
  TOptional<FChunkCompletionDelegate>& opt_chunk_completion_delegate) {

  TSharedRef<FDownloadInProgress> download = MakeShared<FDownloadInProgress>();

  static int64 unique_id = 1;
  download->id = unique_id++;

  download->directFileUrl = params.download_url;
  download->localFilename = MakeShared<FString>(*params.destination_file_path);

  FString tmp = *download->localFilename + kDownloadTmpExtension;
  download->tempLocalFilename = MakeShared<FString>(tmp);

  const FCFCoreSettings& settings = settings_service_->GetSettings();
  download->maxParallelChunks = FMath::Clamp(
    settings.downloads.maxParallelChunks,
    kMinParallelChunks,
    kMaxParallelChunks);

  download->maxChunkSizeInBytes = kMaxDownloadChunkSizeInBytes;
  if (params.throttle_download_kbps > 0) {
    int32 kbps = params.throttle_download_kbps;
    download->maxChunkSizeInBytes = kbps * 1024;

    download->throttleChunkPerSecond = true;
    // NOTE(apliscov): If throttling is enabled, we don't parallelize chunks.
    download->maxParallelChunks = 1;
  }

    // NOTE(apliscov): PS5 specific disable, because parallel chunk downloading
    // causes corruption of downloaded files.
#if defined(PLATFORM_PS5) && PLATFORM_PS5
    download->maxParallelChunks = 1;
#endif

  download->progressDelegate =
    MakeShared<FFileTransferProgressDelegate>(progress_delegate);

  if (opt_chunk_completion_delegate.IsSet()) {
    download->chunkCompletionDelegate =
      MakeShared<FChunkCompletionDelegate>(*opt_chunk_completion_delegate);
  }

  download->retries = kMaxPerDownloadRetries;
  download->allowResume = params.allow_resumable_download;
  return download;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
ChunkedFileDownloadServiceImpl::AssureDestinationFileDeleted(
  TSharedRef<FString> file_path) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  file_system_service_->FileExists(file_path)
    .Next([this, file_path, promise](bool exists) {

    if (!exists) {
      promise->SetValue(FCFCoreError());
      return;
    }

    file_system_service_->DeleteFile(file_path).Next([promise](bool deleted) {
      if (!deleted) {
        auto code = ECFCoreErrorCodes::FailedDeletingOutputFile;
        promise->SetValue(FCFCoreError(code));
      }
      else {
        promise->SetValue(FCFCoreError());
      }
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::FinalizeDownload(
  TSharedRef<FDownloadInProgress> downld,
  OptError opt_err) {

  if (downld->isFinalized) {
    return; 
  }

  downld->isFinalized = true;

  CancelActiveChunkDownloads(downld);

  if (opt_err.IsSet()) {
    FinalizeDownloadWithError(downld, opt_err);
    return;
  }

  auto future = file_system_service_->MoveFile(*downld->tempLocalFilename,
                                               *downld->localFilename);
  future.Next([this, downld](bool success) {
    if (success) {
      downld->promise.SetValue({});
      downloads_in_progress_.Remove(downld->id);
      return;
    }

    const FString& filename = *downld->tempLocalFilename;
    CFCORE_LOG(Error, TEXT("Failed renaming: %s"), *filename);

    auto code = ECFCoreErrorCodes::FileSystemError;
    FinalizeDownloadWithError(downld, FCFCoreError(code));
  });
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::StartNextChunkDownload(
  TSharedRef<FDownloadInProgress> downld) {

  FChunkDownloadInfo chunk_info;
  
  if (!downld->chunkDownloadInfos.Dequeue(chunk_info)) {
    return;
  }

  TMap<FString, FString> headers;
  api_helper_service_->GetApiRequestHeaders(headers);

  FChunkedFileDownloadBytesRange range = chunk_info.range;
  SetRangeHeader(range, headers);

  chunk_info.start_timestamp = FDateTime::UtcNow();

  int64 chunk_id = downld->next_chunk_id++;
  chunk_info.chunk_id = chunk_id;

  FChunkDownloadInfo& chunk_info_ref =
    downld->activeChunkDownloadsMap.Add(chunk_id, chunk_info);

  int64 http_request_id = DownloadChunk(downld, chunk_id, range, headers);

  chunk_info_ref.http_request_id = http_request_id;
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::CancelActiveChunkDownloads(
  TSharedRef<FDownloadInProgress> downld) {

  for (const auto& pair : downld->activeChunkDownloadsMap) {
    int64 http_request_id = pair.Value.http_request_id;
    http_file_service_->Abort(http_request_id);
  }
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::FinalizeDownloadWithError(
  const TSharedRef<FDownloadInProgress> downld,
  FCFCoreError err) {

  if (ShouldPreserveTempLocalFile(downld, err)) {
    downld->promise.SetValue(err);
    downloads_in_progress_.Remove(downld->id);
    return;
  }

  if (downld->chunkCompletionDelegate.IsValid()) {
    FChunkCompletionInfo completion_info;
    completion_info.successful = false;
    completion_info.allow_resumable_download = false;
    downld->chunkCompletionDelegate->ExecuteIfBound(completion_info);
  }

  AssureDestinationFileDeleted(downld->tempLocalFilename.ToSharedRef())
    .Next([this, downld, err](FCFCoreError) {

    downld->promise.SetValue(err);
    downloads_in_progress_.Remove(downld->id);
  });
}

// -----------------------------------------------------------------------------
bool ChunkedFileDownloadServiceImpl::ShouldPreserveTempLocalFile(
  const TSharedRef<FDownloadInProgress> download,
  FCFCoreError err) {

  if (!download->allowResume) {
    return false;
  }

  if (WasDownloadCancelledByUser(err)) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool ChunkedFileDownloadServiceImpl::WasDownloadCancelledByUser(
  FCFCoreError& err) {
  return err.code == ECFCoreErrorCodes::DownloadCancelled &&
    !lifetime_service_->IsShuttingDown();
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::LogDownloadDetails(
  const TSharedRef<FDownloadInProgress> download,
  const FChunkedDownloadParams& params) {

  CFCORE_LOG(Log,
    TEXT("%s download - %d parts for %s ..."),
    download->isResumedDownload ? TEXT("Resuming") : TEXT("Starting"),
    download->totalChunks,
    *download->directFileUrl);

  if (params.throttle_download_kbps > 0) {
    CFCORE_LOG(Log,
      TEXT("Throttling download speed to %d KB/s"),
      params.throttle_download_kbps);
    return;
  }

  const auto max_parallel_chunks = 
    settings_service_->GetSettings().downloads.maxParallelChunks;

  if (max_parallel_chunks == 1) {
    CFCORE_LOG(Log, TEXT("Downloading chunks sequentially"));
  } else {
    CFCORE_LOG(Log,
      TEXT("Downloading with up to %d parallel chunks"),
      max_parallel_chunks);
  }
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::DownloadNextChunks(
  TSharedRef<FDownloadInProgress> downld) {

  if (downld->cancelRequest) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::DownloadCancelled;
    FinalizeDownload(downld, FCFCoreError(code));
    return;
  }

  while (HasChunksToDownload(downld) && CanStartMoreChunks(downld)) {
    StartNextChunkDownload(downld);
  }

  if (IsDownloadFinished(downld)) {
    FinalizeDownload(downld, {});
  }
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::SetRangeHeader(
  const FChunkedFileDownloadBytesRange& range,
  TMap<FString, FString>& headers) {

  FString range_header_value = FString::Format(TEXT("bytes={0}-{1}"), {
    range.from_bytes,
    range.to_bytes
    });

  headers.Add(kHttpHeaderRange, range_header_value);
}

// -----------------------------------------------------------------------------
int64 ChunkedFileDownloadServiceImpl::DownloadChunk(
  TSharedRef<FDownloadInProgress> download,
  const int64 chunk_id,
  const FChunkedFileDownloadBytesRange& range,
  const TMap<FString, FString>& headers) {

  return http_file_service_->DownloadFile(
    download->directFileUrl,
    headers,
    IHttpFileService::FProgressDelegate::CreateLambda(
      [this, download, chunk_id, range](const HttpProgress& progress) {

      HandleChunkProgress(download, chunk_id, range, progress);
    }),
    IHttpFileService::FCompleteDelegate::CreateLambda(
      [this, download, chunk_id](const HttpResponse& response,
                               TSharedPtr<IBuffer> buffer) {

      HandleChunkCompletion(download, chunk_id, response, buffer);
    })
  );
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleChunkProgress(
  const TSharedRef<FDownloadInProgress> downld,
  const int64 chunk_id,
  const FChunkedFileDownloadBytesRange& range,
  const HttpProgress& http_progress) {

  if (downld->isFinalized) {
    return;
  }

  if (!downld->activeChunkDownloadsMap.Contains(chunk_id)) {
    CFCORE_LOG(Warning,
      TEXT("Received chunk progress for unknown chunk id=%lld"),
      chunk_id);
    return;
  }

  FChunkDownloadInfo& chunk_info = downld->activeChunkDownloadsMap[chunk_id];

  int64 delta = http_progress.transferred_bytes - chunk_info.transferred_bytes;
  delta = FMath::Max<int64>(delta, 0);
  int64 total_bytes = downld->totalTransferredBytes + delta;
  chunk_info.transferred_bytes += delta;

  FFileTransferProgress overall_progress;
  overall_progress.bytesPerSecond = CalcDownloadedBytesPerSecond(total_bytes,
                                                                 downld);

  overall_progress.transferredBytes = total_bytes;
  overall_progress.progress = GetRoundedProgress(total_bytes,
                                                 downld->fileSizeInBytes);

  downld->progressDelegate->ExecuteIfBound(overall_progress);
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleChunkCompletion(
  TSharedRef<FDownloadInProgress> download,
  const int64 chunk_id,
  const HttpResponse& response,
  TSharedPtr<IBuffer> buffer) {

  if (download->isFinalized) {
    return;
  }

  FChunkDownloadInfo* chunk_info_ptr =
    download->activeChunkDownloadsMap.Find(chunk_id);

  if (!chunk_info_ptr) {
    CFCORE_LOG(Warning,
      TEXT("Received chunk completion for unknown chunk id=%lld"),
      chunk_id);
    return;
  }

  FChunkDownloadInfo chunk_info = *chunk_info_ptr;
  auto opt_err = api_helper_service_->GetApiResponseError(response);
  if (opt_err.IsSet()) {
    HandleChunkCompletionError(download, chunk_info, *opt_err);
    return;
  }

  auto future = file_system_service_->WriteBuffer(
    download->tempLocalFilename.ToSharedRef(),
    buffer->GetData(),
    buffer->GetLength(),
    chunk_info.range.from_bytes);

  // NOTE(twolf): Must pass |buffer| to keep it alive
  future.Next([this, download, buffer, chunk_info](EFileSystemError file_err) {
    if (file_err != EFileSystemError::None) {
      HandleFileSystemError(download, file_err);
      return;
    }

    HandleChunkWriteCompletion(download, chunk_info);
  });
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleChunkCompletionError(
  TSharedRef<FDownloadInProgress> download,
  const FChunkDownloadInfo& chunk_info,
  const FCFCoreApiResponseError api_err) {

  if (download->isFinalized) {
    return;
  }

  if (download->cancelRequest) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::DownloadCancelled;
    FinalizeDownload(download, FCFCoreError(code));
    return;
  }

  download->retries--;

  FChunkDownloadInfo retry_chunk_info = chunk_info;
  retry_chunk_info.retries--;

  LogChunkFailure(download, retry_chunk_info, api_err);

  if (download->retries <= 0) {
    HandleDownloadRetriesExhausted(download, api_err);
    return;
  }

  if (retry_chunk_info.retries <= 0) {
    HandleChunkRetriesExhausted(download, chunk_info, api_err);
    return;
  }

  RetryChunkDownload(download, retry_chunk_info);
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::LogChunkFailure(
  const TSharedRef<FDownloadInProgress> download,
  const FChunkDownloadInfo& chunk_info,
  const FCFCoreApiResponseError& api_err) {
  
  CFCORE_LOG(Warning,
    TEXT("Failed downloading: %s "
         "(desc=%s, chunk retries left=%d, download retries left=%d)"),
    *download->directFileUrl,
    *api_err.description,
    chunk_info.retries,
    download->retries);
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleDownloadRetriesExhausted(
  TSharedRef<FDownloadInProgress> download,
  const FCFCoreApiResponseError& api_err) {
  
  CFCORE_LOG(Warning,
    TEXT("Download retry budget exhausted for: %s"),
    *download->directFileUrl);
    
  FCFCoreError error(ECFCoreErrorCodes::FailedToDownloadFile);
  error.description = api_err.description;
  FinalizeDownload(download, error);
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleChunkRetriesExhausted(
  TSharedRef<FDownloadInProgress> download,
  const FChunkDownloadInfo& chunk_info,
  const FCFCoreApiResponseError& api_err) {
  
  CFCORE_LOG(Warning,
    TEXT("Chunk retry budget exhausted for chunk at bytes %lld-%lld"),
    chunk_info.range.from_bytes,
    chunk_info.range.to_bytes);
    
  FCFCoreError error(ECFCoreErrorCodes::FailedToDownloadFile);
  error.description = api_err.description;
  FinalizeDownload(download, error);
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::RetryChunkDownload(
  TSharedRef<FDownloadInProgress> download,
  const FChunkDownloadInfo& chunk_info) {
  
  FChunkDownloadInfo retry_info = chunk_info;
  retry_info.http_request_id = 0;
  retry_info.transferred_bytes = 0;

  download->chunkDownloadInfos.Enqueue(retry_info);
  download->activeChunkDownloadsMap.Remove(chunk_info.chunk_id);
  DownloadNextChunks(download);
}

// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleFileSystemError(
  const TSharedRef<FDownloadInProgress> download,
  EFileSystemError filesys_error) {

  ENUM_TO_STR(str_filesys_error, filesys_error);

  FString desc = FString::Format(TEXT("Failed writing to: {0} (desc={1})"), {
    **download->localFilename,
    *str_filesys_error
  });

  CFCORE_LOG(Error, TEXT("%s"), *desc);

  ECFCoreErrorCodes code = ECFCoreErrorCodes::FileSystemError;
  FCFCoreError err(code);
  err.description = desc;
  FinalizeDownload(download, err);
}


// -----------------------------------------------------------------------------
void ChunkedFileDownloadServiceImpl::HandleChunkWriteCompletion(
  TSharedRef<FDownloadInProgress> download,
  const FChunkDownloadInfo& chunk_info) {

  if (download->isFinalized) {
    return;
  }

  int32 total_chunks = download->totalChunks;
  download->completedChunks++;

  FChunkDownloadInfo chunk_info_copy = chunk_info;
  download->activeChunkDownloadsMap.Remove(chunk_info.chunk_id);

  CFCORE_LOG(Log,
             TEXT("Successfully downloaded chunk %d of %d for %s"),
             download->completedChunks,
             total_chunks,
             *download->directFileUrl);

  if (download->chunkCompletionDelegate.IsValid()) {
    FChunkCompletionInfo completion_info;
    completion_info.allow_resumable_download = download->allowResume;
    completion_info.last_chunk = IsDownloadFinished(download);
    completion_info.file_size_in_bytes = download->fileSizeInBytes;
    completion_info.temp_file = *download->tempLocalFilename;
    completion_info.download_info = chunk_info_copy;
    download->chunkCompletionDelegate->ExecuteIfBound(completion_info);
  }


  FTimespan delta = FDateTime::UtcNow() - chunk_info_copy.start_timestamp;
  double wait_ms = FMath::Max<double>(1000 - delta.GetTotalMilliseconds(), 0);
  if ((!download->throttleChunkPerSecond) || (wait_ms == 0)) {
    DownloadNextChunks(download);
    return;
  }

  timer_service_->Delay(
    ITimerService::FTimerDelegate::CreateLambda([this, download] {
      DownloadNextChunks(download);
    }), wait_ms);
}

// -----------------------------------------------------------------------------
int64 ChunkedFileDownloadServiceImpl::CalcDownloadedBytesPerSecond(
  int64 total_bytes_downloaded,
  const TSharedRef<FDownloadInProgress> download) {

  download->sumBytesPerSecond +=
    total_bytes_downloaded - download->totalTransferredBytes;
  download->totalTransferredBytes = total_bytes_downloaded;

  FTimespan delta_time = FDateTime::Now() - download->lastReportTimestamp;
  double seconds = delta_time.GetTotalSeconds();
  if (seconds < 1) {
    return download->lastBytesPerSecondReported > 0 ?
      download->lastBytesPerSecondReported : download->sumBytesPerSecond;
  }

  int64 bytes_per_second = download->sumBytesPerSecond / seconds;
  download->lastBytesPerSecondReported = bytes_per_second;
  download->sumBytesPerSecond = 0;
  download->lastReportTimestamp = FDateTime::Now();
  return bytes_per_second;
}

// -----------------------------------------------------------------------------
int32 ChunkedFileDownloadServiceImpl::GetRoundedProgress(
  int64 downloaded_bytes,
  int64 file_size_in_bytes) {

  double unrounded = ((double)downloaded_bytes / file_size_in_bytes) * 100;
  int32 rounded = (int32)FGenericPlatformMath::RoundToDouble(unrounded);
  return rounded;
}

// -----------------------------------------------------------------------------
bool ChunkedFileDownloadServiceImpl::IsDownloadFinished(
  const TSharedRef<FDownloadInProgress> download) {

  bool no_active_chunks = download->activeChunkDownloadsMap.Num() == 0;
  bool no_pending_chunks = download->chunkDownloadInfos.IsEmpty();
  
  return no_active_chunks && no_pending_chunks;
}

// -----------------------------------------------------------------------------
bool ChunkedFileDownloadServiceImpl::HasChunksToDownload(
  const TSharedRef<FDownloadInProgress> download) const {
  
  return !download->chunkDownloadInfos.IsEmpty();
}

// -----------------------------------------------------------------------------
bool ChunkedFileDownloadServiceImpl::CanStartMoreChunks(
  const TSharedRef<FDownloadInProgress> download) const {
  
  return download->activeChunkDownloadsMap.Num() < download->maxParallelChunks;

}