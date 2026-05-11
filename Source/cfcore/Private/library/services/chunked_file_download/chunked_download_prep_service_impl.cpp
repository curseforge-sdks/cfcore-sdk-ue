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
#include "chunked_download_prep_service_impl.h"

#include <common/macros.h>
#include <common/constants.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include <api/services/api_helper_service.h>
#include <services/http/http_service.h>
#include <services/filesystem/file_system_service.h>

#include <api/common/url_search_params.h>

namespace cfcore {
using TChunkedSeqFuncArray = TArray<TSequenceFunction<FCFCoreError>>;
using OptApiError = TOptional<FCFCoreApiResponseError>;

constexpr int32 kMaxPerChunkRetries = 3;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
ChunkedDownloadPrepServiceImpl::ChunkedDownloadPrepServiceImpl(
  TSharedRef<IApiHelperService> api_helper_service,
  TSharedRef<IHttpService> http_service,
  TSharedRef<IFileSystemService> file_system_service) :
  api_helper_service_(api_helper_service),
  http_service_(http_service),
  file_system_service_(file_system_service){
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ChunkedDownloadPrepServiceImpl::PrepareDownloadParams(
  const FChunkedDownloadParams& params,
  TSharedRef<FDownloadInProgress> downld) {

  auto functions = MakeShared<TChunkedSeqFuncArray>(TChunkedSeqFuncArray {
    [this, downld]() {
      return AssureDestinationFileDeleted(downld->localFilename.ToSharedRef());
    },
    [this, params, downld]() {
      return SetIsResumedDownload(params, downld);
    },
    [this, params, downld]() {
      return GetRemoteFileSize(params, downld);
    },
    [this, params, downld]() {
      return GenerateChunkDownloadsPlan(params, downld);
    },
    [this, params, downld]() {
      return CleanupStaleResumableFile(params, downld);
    }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
ChunkedDownloadPrepServiceImpl::AssureDestinationFileDeleted(
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
TFuture<FCFCoreError> ChunkedDownloadPrepServiceImpl::SetIsResumedDownload(
  const FChunkedDownloadParams& params,
  TSharedRef<FDownloadInProgress> download) {

  if (!IsValidResumableDownload(params.existing_download_info)) {
    download->isResumedDownload = false;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TSharedRef<bool> temp_exists = MakeShared<bool>(false);
  CheckIfFileExists(download->tempLocalFilename.ToSharedRef(), temp_exists)
    .Next([this, promise, params, download, temp_exists](FCFCoreError err) {

    if (err.isError) {
      return promise->SetValue(err);
    }

    const FString& old_temp_filename = params.existing_download_info.tempFile;
    FString new_temp_filename = *download->tempLocalFilename;
    bool same_temp_files =
      old_temp_filename.Equals(new_temp_filename, ESearchCase::IgnoreCase);

    int32 num_chunks = params.existing_download_info.downloaded.Num();

    if (num_chunks > 0 && !same_temp_files) {
      CFCORE_LOG(
        Warning,
        TEXT("Cannot resume download - temp file has changed from %s to %s"),
        *old_temp_filename,
        *new_temp_filename);
    }

    download->isResumedDownload = *temp_exists &&
                                  same_temp_files &&
                                  (num_chunks > 0);

    return promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool ChunkedDownloadPrepServiceImpl::IsValidResumableDownload(
  const FInstalledModDownloadInfo& existing) {

  return !existing.tempFile.IsEmpty() &&
    existing.downloaded.Num() > 0 &&
    existing.fileSizeInBytes > 0;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ChunkedDownloadPrepServiceImpl::CheckIfFileExists(
  TSharedRef<FString> file_path,
  TSharedRef<bool> out_exists) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  file_system_service_->FileExists(file_path)
    .Next([promise, out_exists](bool exists) {

    *out_exists = exists;
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}


// -----------------------------------------------------------------------------
// Send an HTTP head request to the downld url so that:
// 1. We add a downld count for the mod file
// 2. We retrieve the actual content length on the server and fill |downld|
// with this value
TFuture<FCFCoreError> ChunkedDownloadPrepServiceImpl::GetRemoteFileSize(
  const FChunkedDownloadParams& params,
  TSharedRef<FDownloadInProgress> downld) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TMap<FString, FString> headers;
  api_helper_service_->GetApiRequestHeaders(headers);

  FString head_request_url = GetHeaderRequestUrlWithTracking(params, downld);

  http_service_->Head(
    head_request_url,
    headers,
    IHttpService::FCompleteDelegate::CreateLambda(
      [this, promise, params, downld](const HttpResponse& response) {

      auto opt_err = FillDownloadWithFileSize(response, downld);
      if (opt_err.IsSet()) {
        promise->SetValue(FCFCoreError(opt_err));
        return;
      }

      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
FString ChunkedDownloadPrepServiceImpl::GetHeaderRequestUrlWithTracking(
  const FChunkedDownloadParams& params,
  TSharedRef<FDownloadInProgress> download) {

  TMap<FString, FString> tracking = params.tracking;
  if (download->isResumedDownload) {
    tracking.Add(kTrackingResumableDownload, TEXT("true"));
  }

  FString tracking_params = GetTrackingParamsString(tracking);
  if (tracking_params.IsEmpty()) {
    return params.head_request_url;
  }

  FString separator = "?";
  if (params.head_request_url.Contains("?")) {
    separator = "&";
  }

  return FString::Format(TEXT("{0}{1}{2}"), {
    *params.head_request_url,
    *separator,
    *tracking_params
  });
}

// -----------------------------------------------------------------------------
FString ChunkedDownloadPrepServiceImpl::GetTrackingParamsString(
  const TMap<FString, FString>& tracking) {

  if (tracking.Num() == 0) {
    return TEXT("");
  }

  UrlSearchParams url_search_params;
  for (const TPair<FString, FString>& pair : tracking) {
    url_search_params.Append(pair.Key, pair.Value);
  }

  return url_search_params.ToString();
}

// -----------------------------------------------------------------------------
OptApiError ChunkedDownloadPrepServiceImpl::FillDownloadWithFileSize(
  const HttpResponse& res,
  TSharedRef<FDownloadInProgress> downld) {

  OptApiError api_err = api_helper_service_->GetApiResponseError(res);
  if (api_err.IsSet()) {
    return api_err;
  }

  if (!res.allHeaders.Contains(kHttpHeaderContentLength)) {
    FCFCoreApiResponseError err;
    err.failedToParseServerResponse = true;
    err.description =
      FString::Format(TEXT("Response is missing {0} header"), {
        kHttpHeaderContentLength,
      });

    CFCORE_LOG(Error, TEXT("%s"), *err.description);
    return OptApiError(err);
  }

  FString len_str = res.allHeaders[kHttpHeaderContentLength];
  downld->fileSizeInBytes = FCString::Strtoi64(*len_str, nullptr, 10);
  return {};
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
ChunkedDownloadPrepServiceImpl::GenerateChunkDownloadsPlan(
  const FChunkedDownloadParams& params,
  TSharedRef<FDownloadInProgress> downld) {

  downld->chunkDownloadInfos.Empty();
  downld->totalChunks = 0;

  bool resumable_download_size_matches =
    params.existing_download_info.fileSizeInBytes == downld->fileSizeInBytes;
  downld->isResumedDownload &= resumable_download_size_matches;

  TArray<FDownloadedChunk> downloaded_chunks =
    params.existing_download_info.downloaded;
  if (!downld->isResumedDownload) {
    // resuming a downld only if we have a temp file to resume from
    downloaded_chunks.Empty();
  }

  downloaded_chunks.Sort([](const auto& a, const auto& b) {
    return a.from_bytes < b.from_bytes;
  });

  int64 current_file_position = 0;
  for (const FDownloadedChunk& chunk : downloaded_chunks) {
    if (chunk.from_bytes == current_file_position) {
      current_file_position = chunk.to_bytes + 1;
      continue;
    }

    // else, we have a gap to fill (chunk.from_bytes > current_file_position)
    int64 gap_size = chunk.from_bytes - current_file_position;
    GenerateChunkDownloadsPlanForGap(gap_size, current_file_position, downld);
    current_file_position = chunk.to_bytes + 1;
  }

  // finalize the plan
  int64 gap_size = downld->fileSizeInBytes - current_file_position;
  GenerateChunkDownloadsPlanForGap(gap_size, current_file_position, downld);

  return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
}

// -----------------------------------------------------------------------------
void ChunkedDownloadPrepServiceImpl::GenerateChunkDownloadsPlanForGap(
  int64 gap_size,
  int64 current_file_position,
  TSharedRef<FDownloadInProgress> downld) {

  double number_of_chunks_unrounded =
    (gap_size / (double)downld->maxChunkSizeInBytes);

  int32 number_of_chunks = (int32)FGenericPlatformMath::CeilToDouble(
    number_of_chunks_unrounded);

  for (int i = 0; i < number_of_chunks; i++) {
    int64 bytes_to_download = downld->maxChunkSizeInBytes;
    int64 bytes_downloaded = i * bytes_to_download;
    if (bytes_downloaded + bytes_to_download > gap_size) {
      bytes_to_download = gap_size - bytes_downloaded;
    }

    FChunkedFileDownloadBytesRange range;
    range.from_bytes = current_file_position + bytes_downloaded;
    range.to_bytes = range.from_bytes + (bytes_to_download - 1);

    FChunkDownloadInfo chunk_info;
    chunk_info.range = range;
    chunk_info.retries = kMaxPerChunkRetries;
    downld->chunkDownloadInfos.Enqueue(chunk_info);
    downld->totalChunks++;
  }
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ChunkedDownloadPrepServiceImpl::CleanupStaleResumableFile(
  const FChunkedDownloadParams& params,
  const TSharedRef<FDownloadInProgress> download) {

  if (download->isResumedDownload) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto functions = MakeShared<TChunkedSeqFuncArray>(TChunkedSeqFuncArray{
    [this, download]() {
      auto temp = download->tempLocalFilename.ToSharedRef();
      return AssureDestinationFileDeleted(temp);
    },
    [this, params]() {
      // Here, we will cleanup any temp file that doesn't match the current -
      // this might happen if the user has a resumable download, yet the mod was
      // update on the server
      auto temp = MakeShared<FString>(params.existing_download_info.tempFile);
      return AssureDestinationFileDeleted(temp);
    }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}