/* MIT License

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
SOFTWARE. */
#include "chunk_completion_persistency_service_impl.h"
#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <library/services/installed_mods_info/installed_mods_info_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
ChunkCompletionPersistencyServiceImp::ChunkCompletionPersistencyServiceImp(
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service) :
  installed_mods_info_service_(installed_mods_info_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
ChunkCompletionPersistencyServiceImp::HandleChunkCompletion(
  FInstallModRequestPtr request,
  const FChunkCompletionInfo& chunk_completion_info) {

  FInstalledModDownloadInfo download_info;
  bool allow_resumes = chunk_completion_info.allow_resumable_download;

  if (allow_resumes && !chunk_completion_info.last_chunk) {
    const FFile& file = request->installedMod.latestUpdatedFile;
    download_info.canResume = true;
    download_info.fileId = file.id;
    download_info.fileSizeInBytes = chunk_completion_info.file_size_in_bytes;
    download_info.lastModified = FDateTime::UtcNow();
    download_info.tempFile = chunk_completion_info.temp_file;

    TArray<FDownloadedChunk>& current_downloaded =
      request->installedMod.downloadInfo.downloaded;

    AddLastChunkToDownloadedChunks(current_downloaded, chunk_completion_info);
    MergeDownloadedChunks(current_downloaded, download_info);
  }

  // persist in-memory as well
  request->installedMod.downloadInfo = download_info;

  FInternalInstalledModInfoProperties props;
  props.id = request->installedMod.id;
  props.downloadInfo = download_info;

  TArray<FInternalInstalledModInfoProperties> arr_props { props };
  return installed_mods_info_service_->SetInstalledProperties(arr_props);
}

// -----------------------------------------------------------------------------
void ChunkCompletionPersistencyServiceImp::AddLastChunkToDownloadedChunks(
  TArray<FDownloadedChunk>& downloaded,
  const FChunkCompletionInfo& chunk_completion_info) {

  FDownloadedChunk last_chunk;
  last_chunk.from_bytes = chunk_completion_info.download_info.range.from_bytes;
  last_chunk.to_bytes = chunk_completion_info.download_info.range.to_bytes;
  downloaded.Add(last_chunk);
}

// -----------------------------------------------------------------------------
// To reduce the amount of chunk information we save, we can merge sequential
// chunks into a single large one
void ChunkCompletionPersistencyServiceImp::MergeDownloadedChunks(
  TArray<FDownloadedChunk>& current_downloaded,
  FInstalledModDownloadInfo& download_info) {

  SortDownloadedChunks(current_downloaded);

  FDownloadedChunk current_chunk = current_downloaded[0];

  int64 prev_from = current_chunk.from_bytes, prev_to = current_chunk.to_bytes;

  for (int i = 1; i < current_downloaded.Num(); ++i) {
    current_chunk = current_downloaded[i];

    if (current_chunk.from_bytes > prev_to + 1) {
      FDownloadedChunk chunk;
      chunk.from_bytes = prev_from;
      chunk.to_bytes = prev_to;
      download_info.downloaded.Add(chunk);

      prev_from = current_chunk.from_bytes;
      prev_to = current_chunk.to_bytes;
    } else {
      prev_to = current_chunk.to_bytes;
    }
  }

  FDownloadedChunk chunk;
  chunk.from_bytes = prev_from;
  chunk.to_bytes = prev_to;
  download_info.downloaded.Add(chunk);
}

// -----------------------------------------------------------------------------
void ChunkCompletionPersistencyServiceImp::SortDownloadedChunks(
  TArray<FDownloadedChunk>& downloaded) {

  downloaded.Sort([](const auto& a, const auto& b) {
    return a.from_bytes < b.from_bytes;
  });
}
