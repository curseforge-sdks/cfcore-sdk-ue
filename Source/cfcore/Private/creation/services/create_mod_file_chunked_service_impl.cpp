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
#include "create_mod_file_chunked_service_impl.h"
#include <services/logger/logger_macros.h>
#include <common/cfcore_log.h>

namespace cfcore {
constexpr int64 kMaxChunkSizeInBytes = 100 * 1024 * 1024;
const int32 kMaxUploadChunkRetries = 3;
};

using namespace cfcore;

// -----------------------------------------------------------------------------
CreateModFileChunkedServiceImpl::CreateModFileChunkedServiceImpl(
  TSharedPtr<ILifetimeService> lifetime_service,
  TSharedPtr<ICFCoreApiCreation> api_creation,
  TSharedPtr<IFileSystemService> file_system_service) :
  lifetime_service_(lifetime_service),
  api_creation_(api_creation),
  file_system_service_(file_system_service) {

  on_shutdown_handle_ = lifetime_service_->OnShutdown().AddLambda([this]() {
    HandleShutdown();
  });
}

// -----------------------------------------------------------------------------
TFuture<ICreateModFileChunkedService::CreationResult>
CreateModFileChunkedServiceImpl::Create(
  int64 mod_id,
  const FCreateModFileRequest& create_mod_file_request,
  const FString& local_filename_to_upload,
  ICFCoreCreation::FCreateModFileRequestIdDelegate request_id_delegate,
  ICFCoreCreation::FFileTransferProgressDelegate progress_delegate) {

  TSharedRef<FCreationInProgress> creation = NewCreation(
    local_filename_to_upload,
    progress_delegate);

  creations_in_progress_.Add(creation->id, creation);

  request_id_delegate.ExecuteIfBound(creation->id);

  auto f = CreateModFileChunked(mod_id, create_mod_file_request, creation);
  f.Next([this, creation](TOptional<FModFileChunkedInfo> opt_info) {
    StartUploadingChunks(opt_info, creation);
  });

  return creation->promise.GetFuture();
}


// -----------------------------------------------------------------------------
TFuture<ICreateModFileChunkedService::CreationResult>
CreateModFileChunkedServiceImpl::CreateCooked(
  int64 mod_id,
  int64 source_file_id,
  const FCreateCookedModFileRequest& create_cooked_mod_file_request,
  const FString& local_filename_to_upload,
  ICFCoreCreation::FCreateModFileRequestIdDelegate request_id_delegate,
  ICFCoreCreation::FFileTransferProgressDelegate progress_delegate) {

  TSharedRef<FCreationInProgress> creation = NewCreation(
    local_filename_to_upload,
    progress_delegate);

  creations_in_progress_.Add(creation->id, creation);

  request_id_delegate.ExecuteIfBound(creation->id);

  auto f = CreateCookedModFileChunked(
    mod_id,
    source_file_id,
    create_cooked_mod_file_request,
    creation);

  f.Next([this, creation](TOptional<FModFileChunkedInfo> opt_info) {
    StartUploadingChunks(opt_info, creation);
  });

  return creation->promise.GetFuture();
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::CancelCreation(int64 creation_id) {
  TSharedRef<FCreationInProgress>* creation =
    creations_in_progress_.Find(creation_id);

  if (creation == nullptr) {
    CFCORE_LOG(
      Error,
      TEXT("Request to cancel upload failed - can't find creation (%lld)"),
      creation_id);

    return;
  }

  (*creation)->cancelRequest = true;
  api_creation_->CancelCreation((*creation)->cancelId);
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::HandleShutdown() {
  for (const auto& pair : creations_in_progress_) {
    CancelCreation(pair.Key);
  }
}

// -----------------------------------------------------------------------------
TSharedRef<FCreationInProgress> CreateModFileChunkedServiceImpl::NewCreation(
  const FString& local_filename_to_upload,
  ICFCoreCreation::FFileTransferProgressDelegate& progress_delegate) {

  TSharedRef<FCreationInProgress> creation = MakeShared<FCreationInProgress>();
  static int64 unique_id = 1;
  creation->id = unique_id++;
  creation->localFilename = MakeShared<FString>(local_filename_to_upload);
  creation->buffer = MakeShared<TArray<uint8>>();

  creation->progressDelegate =
    MakeShared<ICFCoreCreation::FFileTransferProgressDelegate>(
      progress_delegate);

  creation->retries = kMaxUploadChunkRetries;

  return creation;
}

// -----------------------------------------------------------------------------
TFuture<TOptional<FModFileChunkedInfo>>
CreateModFileChunkedServiceImpl::CreateModFileChunked(
  int64 mod_id,
  const FCreateModFileRequest& create_mod_file_request,
  TSharedRef<FCreationInProgress> creation) {

  SHAREABLE_PROMISE(TOptional<FModFileChunkedInfo>, promise);

  auto f = file_system_service_->GetStat(creation->localFilename.ToSharedRef());
  f.Next([this, create_mod_file_request, creation, mod_id, promise](
    FFileStatData stat_data) {

    if (!stat_data.bIsValid) {
      auto code = ECFCoreErrorCodes::MissingFileInformation;
      auto err = FCFCoreError(code);
      err.description = FString::Format(
        TEXT("Please assure the local file exists on disk ({0})"), {
          *creation->localFilename
      });

      FinalizeCreation(creation, err);
      promise->SetValue({});
      return;
    }

    creation->fileSizeInBytes = stat_data.FileSize;
    FCreateModFileRequest request_with_length = create_mod_file_request;
    request_with_length.fileLength = stat_data.FileSize;

    api_creation_->CreateModFileChunked(
      mod_id,
      request_with_length,
      ICFCoreApiCreation::FCreateModFileChunkedDelegate::CreateLambda(
        [this, creation, promise](
          const TOptional<FModFileChunkedInfo>& opt_info,
          const TOptional<FCFCoreApiResponseError>& opt_err) {

          ValidateModFileChunkedInfo(creation, opt_info, opt_err, promise);
        })
    );
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<TOptional<FModFileChunkedInfo>>
CreateModFileChunkedServiceImpl::CreateCookedModFileChunked(
  int64 mod_id,
  int64 source_file_id,
  const FCreateCookedModFileRequest& create_cooked_request,
  TSharedRef<FCreationInProgress> creation) {

  SHAREABLE_PROMISE(TOptional<FModFileChunkedInfo>, promise);

  auto f = file_system_service_->GetStat(creation->localFilename.ToSharedRef());
  f.Next([this,
          mod_id,
          create_cooked_request,
          creation,
          source_file_id,
          promise](FFileStatData stat_data) {

    if (!stat_data.bIsValid) {
      auto code = ECFCoreErrorCodes::MissingFileInformation;
      FinalizeCreation(creation, FCFCoreError(code));
      promise->SetValue({});
      return;
    }

    creation->fileSizeInBytes = stat_data.FileSize;
    FCreateCookedModFileRequest request_with_length = create_cooked_request;
    request_with_length.fileLength = stat_data.FileSize;

    api_creation_->CreateCookedModFileChunked(
      mod_id,
      source_file_id,
      request_with_length,
      ICFCoreApiCreation::FCreateModFileChunkedDelegate::CreateLambda(
        [this, creation, promise](
          const TOptional<FModFileChunkedInfo>& opt_info,
          const TOptional<FCFCoreApiResponseError>& opt_err) {

          ValidateModFileChunkedInfo(creation, opt_info, opt_err, promise);
        }));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::ValidateModFileChunkedInfo(
  TSharedRef<FCreationInProgress> creation,
  TOptional<FModFileChunkedInfo> opt_info,
  const TOptional<FCFCoreApiResponseError>& opt_err,
  TSharedRef<TPromise<TOptional<FModFileChunkedInfo>>> promise) {

  if (opt_err.IsSet()) {
    FinalizeCreation(creation, FCFCoreError(opt_err));
    promise->SetValue({});
    return;
  }

  if (opt_info->uploadId.IsEmpty()) {
    auto code = ECFCoreErrorCodes::ApiError;
    FCFCoreApiResponseError api_err;
    api_err.failedToParseServerResponse = true;
    api_err.description = "Missing uploadId from server";
    FinalizeCreation(creation, FCFCoreError(api_err));

    CFCORE_LOG(Error, TEXT("Missing uploadId from server"));
    promise->SetValue({});
    return;
  }

  promise->SetValue(opt_info);
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::StartUploadingChunks(
  TOptional<FModFileChunkedInfo> opt_info,
  TSharedRef<FCreationInProgress> creation) {

  if (!opt_info.IsSet()) {
    return; // we failed and update the creation promise internally
  }

  creation->uploadedModFileInfo.fileId = opt_info->fileId;
  creation->uploadedModFileInfo.directory = opt_info->directory;
  creation->uploadedModFileInfo.filename = opt_info->fileName;

  creation->chunksMetadata.chunkedUploadId = opt_info->uploadId;

  CalcTotalChunks(creation);
  UploadNextChunk(creation);
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::CalcTotalChunks(
  TSharedRef<FCreationInProgress> creation) {

  double chunks_unrounded =
    (creation->fileSizeInBytes / (double)kMaxChunkSizeInBytes);
  int32 total = (int32)FGenericPlatformMath::CeilToDouble(chunks_unrounded);
  creation->chunksMetadata.totalChunks = total;
  creation->chunksMetadata.chunkIndex = 1;
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::UploadNextChunk(
  TSharedRef<FCreationInProgress> creation) {

  if (creation->cancelRequest) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UploadCancelled;
    FinalizeCreation(creation, FCFCoreError(code));
    return;
  }

  int32 currentChunk = creation->chunksMetadata.chunkIndex;
  int32 totalChunks = creation->chunksMetadata.totalChunks;

  int64 file_offset = (currentChunk - 1) * kMaxChunkSizeInBytes;
  int64 bytes_to_read = kMaxChunkSizeInBytes;
  if (IsLastChunk(currentChunk, totalChunks)) {
    bytes_to_read = creation->fileSizeInBytes - file_offset;
  }

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
  creation->buffer->SetNumUninitialized(bytes_to_read, EAllowShrinking::No);
#else
  bool allow_shrinking = false;
  creation->buffer->SetNumUninitialized(bytes_to_read, allow_shrinking);
#endif

  auto future = file_system_service_->ReadBuffer(
    creation->localFilename.ToSharedRef(),
    file_offset,
    bytes_to_read,
    creation->buffer.ToSharedRef());
  future.Next([this, creation](EFileSystemError filesys_error) {
    if (filesys_error != EFileSystemError::None) {
      HandleFileSystemError(creation, filesys_error);
      return;
    }

    int64 file_id = api_creation_->UploadModFileChunk(
      creation->chunksMetadata,
      creation->buffer,
      ICFCoreApiCreation::FFileTransferProgressDelegate::CreateLambda(
        [this, creation](const FFileTransferProgress& chunk_progress) {
          HandleChunkProgress(creation, chunk_progress);
        }),
      ICFCoreApiCreation::FUploadModFileChunkDelegate::CreateLambda(
        [this, creation](bool success,
                         const TOptional<FCFCoreApiResponseError>& opt_err) {
          if (opt_err.IsSet()) {
            HandleUploadChunkError(creation, *opt_err);
            return;
          }

          HandleUploadChunkCompletion(creation);
        }));

    creation->cancelId = file_id;
  });
}


// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::HandleFileSystemError(
  const TSharedRef<FCreationInProgress>& creation,
  EFileSystemError filesys_error) {

  ENUM_TO_STR(str_filesys_error, filesys_error);

  CFCORE_LOG(Error,
             TEXT("Failed uploading: %s (desc=%s)"),
             **creation->localFilename,
             *str_filesys_error);

  ECFCoreErrorCodes code = ECFCoreErrorCodes::FileSystemError;

  FinalizeCreation(creation, FCFCoreError(code));
}

// -----------------------------------------------------------------------------
bool CreateModFileChunkedServiceImpl::IsLastChunk(
  int32 current_chunk,
  int32 total_chunks) {

  return current_chunk == total_chunks;
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::HandleChunkProgress(
  const TSharedRef<FCreationInProgress>& creation,
  const FFileTransferProgress& chunk_progress) {

  FFileTransferProgress overall_progress = chunk_progress;

  const FModFileChunkMetadata& metadata = creation->chunksMetadata;
  int64 total_bytes = (metadata.chunkIndex - 1) * kMaxChunkSizeInBytes;
  total_bytes += chunk_progress.transferredBytes;
  overall_progress.transferredBytes = total_bytes;

  double unrounded = ((double)total_bytes / creation->fileSizeInBytes) * 100;
  int32 rounded = (int32)FGenericPlatformMath::RoundToDouble(unrounded);
  overall_progress.progress = rounded;

  creation->previousProgress = overall_progress.progress;
  creation->progressDelegate->ExecuteIfBound(overall_progress);
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::HandleUploadChunkError(
  TSharedRef<FCreationInProgress> creation,
  const FCFCoreApiResponseError& err) {

  if (!err.serverUnreachable) {
    FinalizeCreation(creation, FCFCoreError(err));
    return;
  }

  creation->retries--;

  CFCORE_LOG(Error,
             TEXT("Failed uploading: %s (desc=%s, retries left=%d)"),
             **creation->localFilename,
             *err.description,
             creation->retries);

  if (creation->retries <= 0) {
    FinalizeCreation(creation, FCFCoreError(err));
    return;
  }

  UploadNextChunk(creation);
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::HandleUploadChunkCompletion(
  TSharedRef<FCreationInProgress> creation) {

  FModFileChunkMetadata& chunk_metadata = creation->chunksMetadata;
  int32 currentChunk = chunk_metadata.chunkIndex;
  int32 totalChunks = chunk_metadata.totalChunks;

  // Done
  if (IsLastChunk(currentChunk, totalChunks)) {
    CFCORE_LOG(Log,
               TEXT("Successfully completed chunked upload for: %s"),
               **creation->localFilename);

    FinalizeCreation(creation, {});
    return;
  }

  CFCORE_LOG(Log,
             TEXT("Successfully uploaded chunk %d of %d for %s"),
             chunk_metadata.chunkIndex,
             chunk_metadata.totalChunks,
             **creation->localFilename);

  chunk_metadata.chunkIndex++;
  creation->retries = kMaxUploadChunkRetries;

  UploadNextChunk(creation);
}

// -----------------------------------------------------------------------------
void CreateModFileChunkedServiceImpl::FinalizeCreation(
  const TSharedRef<FCreationInProgress>& creation,
  TOptional<FCFCoreError> opt_err) {

  ICreateModFileChunkedService::CreationResult result{
    opt_err,
    creation->uploadedModFileInfo
  };

  creation->promise.SetValue(result);
  creations_in_progress_.Remove(creation->id);
}