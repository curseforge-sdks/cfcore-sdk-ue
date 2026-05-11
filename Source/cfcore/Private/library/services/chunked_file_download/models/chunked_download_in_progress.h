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
#include <Containers/Queue.h>

#include <library/services/chunked_file_download/chunked_file_download_service.h>
#include <api/common/cfcore_api_response_error.h>

#include "chunk_download_info.h"

namespace cfcore {
using FFileTransferProgressDelegate =
  IChunkedFileDownloadService::FFileTransferProgressDelegate;

using FChunkCompletionDelegate =
  IChunkedFileDownloadService::FChunkCompletionDelegate;

// -----------------------------------------------------------------------------
// TODO(twolf): Change the member variables to underscore convention - as this
// isn't a UStruct
struct FDownloadInProgress {
  int64 id = 0;

  /**
   * Full filename path on disk
   */
  TSharedPtr<FString> localFilename;

  /**
   * For writing partial chunks, until we get the full file
   */
  TSharedPtr<FString> tempLocalFilename;

  TSharedPtr<FFileTransferProgressDelegate> progressDelegate;
  TSharedPtr<FChunkCompletionDelegate> chunkCompletionDelegate;

  FString directFileUrl;

  int64 fileSizeInBytes = 0;

  /**
   * The size of each download chunk
   */
  uint64 maxChunkSizeInBytes;

  bool allowResume = false;
  bool isResumedDownload = false;
  // This contains the download plan - the chunks to download
  TQueue<FChunkDownloadInfo> chunkDownloadInfos;

  int32 totalChunks = 0;
  int32 completedChunks = 0;
  int32 retries = 0;

  int32 maxParallelChunks = 1;

  int64 next_chunk_id = 1;
  // Maps Chunk IDs to their info
  TMap<int64, FChunkDownloadInfo> activeChunkDownloadsMap;

  bool isFinalized = false;

  /**
   * Allows to cancel the upload operation
   */
  bool cancelRequest = false;

  /**
   * When set, we wait 1 second between consecutive chunk requests
   */
  bool throttleChunkPerSecond = false;

  TPromise<FCFCoreError> promise;

  /**
   * Used for tracking transfer rates (kbps)
   */
  int64 totalTransferredBytes = 0;
  int64 sumBytesPerSecond = 0;
  int64 lastBytesPerSecondReported = 0;
  FDateTime lastReportTimestamp = FDateTime::MinValue();
};

}