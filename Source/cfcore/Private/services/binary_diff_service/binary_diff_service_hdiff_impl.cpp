/*MIT License

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
SOFTWARE.*/
#include "binary_diff_service_hdiff_impl.h"
#include <common/macros.h>
#include <common/future_utils.h>

#include <services/io_throttling/io_throttling_service.h>
#include <services/thread_pools_service.h>
#include <services/logger/logger_macros.h>
#include <services/hashing/hashing_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/filesystem/persistent_file/persistent_binary_file_reader.h>
#include <services/filesystem/persistent_file/persistent_binary_file_writer.h>
#include <services/filesystem/persistent_file/persistent_binary_file.h>

#include <Async/Async.h>

#include "internal/hdiff/cfcore_to_hdfiff_utils.h"

namespace cfcore {
constexpr uint64 kReadBufferPageSizeInBytes = 40 * 1024 * 1024;
constexpr uint64 kReadBufferMaxPages = 20;

constexpr uint64 kWriteBufferSizeInBytes = 500 * 1024 * 1024;

constexpr int32 kPatchCacheSize = 100 * 1024 * 1024;

typedef TPair<IBinaryDiffServiceInternalProgress*,
              IBinaryDiffService::FProgressDelegate> TContextPair;

// -----------------------------------------------------------------------------
static void hpatch_progress_handler(struct spatch_progressListener_t* listener,
                                    hpatch_uint progress,
                                    hpatch_StreamPos_t total_bytes_completed) {

  TContextPair* context = (TContextPair*)listener->context;
  context->Key->OnProgress(context->Value,
                           static_cast<int32>(progress),
                           total_bytes_completed);
}

};

using namespace cfcore;

// -----------------------------------------------------------------------------
BinaryDiffServiceHDiffImpl::BinaryDiffServiceHDiffImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service,
  TSharedPtr<IThreadPoolsService> thread_pools_service,
  TSharedPtr<IHashingService> hashing_service,
  TSharedPtr<IFileSystemService> filesystem_service) :
  io_throttling_service_(io_throttling_service),
  thread_pool_(thread_pools_service->IO()),
  hashing_service_(hashing_service),
  filesystem_service_(filesystem_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceHDiffImpl::ApplyBinaryPatch(
  const FString& old,
  const FString& patch,
  const FString& output,
  FProgressDelegate delegate) {

  CFCORE_LOG(Log, TEXT("Applying patch for %s..."), *output);

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, output]() {
        return AssureFileDeleted(output);
      },
      [this, old, patch, output, delegate]() {
        return PerformPatchAsync(old, patch, output, delegate);
      }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceHDiffImpl::CalcOutputFileSize(
  const FString& patch_file,
  TSharedRef<int64> output_files_total_size) {

  SHAREABLE_PROMISE(FCFCoreError, promise)

  AsyncPool(*thread_pool_,
            [this, promise, patch_file, output_files_total_size]() {

    TSharedPtr<IPersistentBinaryFileReader> patch_persist_file =
      filesystem_service_->CreatePersistentBinaryFileReader(patch_file);
    if (!patch_persist_file.IsValid()) {
      FCFCoreError err(ECFCoreErrorCodes::FileSystemError);
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
      return;
    }

    hpatch_TStreamInput patch_stream;
    CFCoreToHDiffUtils::FileToStreamInput(patch_persist_file, patch_stream);

    hpatch_singleCompressedDiffInfo single_compressed_diff_info;
    FCFCoreError err;
    if (!GetPatchInfo(patch_stream, single_compressed_diff_info)) {
      err = FCFCoreError(ECFCoreErrorCodes::BindiffInvalidHeader);
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
      return;
    }

    *output_files_total_size = single_compressed_diff_info.newDataSize;

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void BinaryDiffServiceHDiffImpl::OnProgress(
  IBinaryDiffService::FProgressDelegate delegate,
  int32 progress,
  hpatch_StreamPos_t total_bytes) {

  HandlePatchingProgress(progress, static_cast<int64>(total_bytes), delegate);
}

// -----------------------------------------------------------------------------
bool BinaryDiffServiceHDiffImpl::GetPatchInfo(
  const hpatch_TStreamInput& patch_stream,
  hpatch_singleCompressedDiffInfo& out_single_compressed_diff_info) {

  hpatch_BOOL res = getSingleCompressedDiffInfo(
    &out_single_compressed_diff_info,
    &patch_stream,
    0);

  if (hpatch_FALSE == res) {
    CFCORE_LOG(Error, TEXT("Failed reading patch file header"));
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceHDiffImpl::AssureFileDeleted(
  const FString& file) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto file_shared = MakeShared<FString>(file);
  filesystem_service_->FileExists(file_shared)
    .Next([this, promise, file_shared](bool exists) {

    if (!exists) {
      promise->SetValue(FCFCoreError());
      return;
    }

    filesystem_service_->DeleteFile(file_shared)
      .Next([promise, file_shared](bool success) {

      if (!success) {
        CFCORE_LOG(Error, TEXT("Failed deleting existing: %s"), **file_shared);
        promise->SetValue(FCFCoreError(ECFCoreErrorCodes::FileSystemError));
        return;
      }

      promise->SetValue(FCFCoreError());
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceHDiffImpl::PerformPatchAsync(
  const FString& old,
  const FString& patch,
  const FString& output,
  FProgressDelegate delegate) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  AsyncPool(*thread_pool_, [this, promise, old, patch, output, delegate]() {
    FCFCoreError err = PerformPatch(old, patch, output, delegate);
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
FCFCoreError BinaryDiffServiceHDiffImpl::PerformPatch(
  const FString& old,
  const FString& patch,
  const FString& output,
  FProgressDelegate delegate) {

  hpatch_TStreamInput patch_stream;
  TSharedPtr<IPersistentBinaryFileReader> patch_file =
    filesystem_service_->CreatePersistentBinaryFileReader(patch);
  if (!patch_file.IsValid()) {
    return FCFCoreError(ECFCoreErrorCodes::FileSystemError);
  }
  CFCoreToHDiffUtils::FileToStreamInput(patch_file, patch_stream);

  hpatch_singleCompressedDiffInfo single_compressed_diff_info;
  if (!GetPatchInfo(patch_stream, single_compressed_diff_info)) {
    return FCFCoreError(ECFCoreErrorCodes::BindiffInvalidHeader);
  }

  hpatch_TStreamInput old_stream;
  hpatch_TStreamOutput output_strm;

  BufferedFileOptions readopt = {
    kReadBufferPageSizeInBytes,
    kReadBufferMaxPages
  };

  TSharedPtr<IPersistentBinaryFileReader> old_file =
    filesystem_service_->CreatePersistentBinaryFileReaderBuffered(old, readopt);
  if (!old_file.IsValid()) {
    return FCFCoreError(ECFCoreErrorCodes::FileSystemError);
  }
  CFCoreToHDiffUtils::FileToStreamInput(old_file, old_stream);

  BufferedFileOptions writeopt = { kWriteBufferSizeInBytes, 1 };
  TSharedPtr<IPersistentBinaryFileWriter> output_file =
    filesystem_service_->CreatePersistentBinaryFileWriterBuffered(output,
                                                                  writeopt);
  if (!output_file.IsValid()) {
    return FCFCoreError(ECFCoreErrorCodes::FileSystemError);
  }

  hpatch_StreamPos_t new_size = single_compressed_diff_info.newDataSize;
  CFCoreToHDiffUtils::FileToStreamOutput(output_file, new_size, output_strm);

  TArray64<uint8> temp_cache;
  temp_cache.AddUninitialized(kPatchCacheSize);

  spatch_progressListener_t progressListener = CreateProgressListener(delegate);

  bool result = (bool)patch_single_compressed_diff(
    &output_strm,
    &old_stream,
    &patch_stream,
    single_compressed_diff_info.diffDataPos,
    single_compressed_diff_info.uncompressedSize,
    single_compressed_diff_info.compressedSize,
    0, // decompress_plugin
    single_compressed_diff_info.coverCount,
    (size_t)single_compressed_diff_info.stepMemSize,
    temp_cache.GetData(),
    temp_cache.GetData() + temp_cache.Num(),
    0,
    &progressListener);

  if (!result || !output_file->Flush()) {
    CFCORE_LOG(Error, TEXT("Output file size doesn't match expected size"));
    return FCFCoreError(ECFCoreErrorCodes::BindiffFailedToApplyPatch);
  }

  return FCFCoreError();
}

// -----------------------------------------------------------------------------
void BinaryDiffServiceHDiffImpl::HandlePatchingProgress(
  int32 progress,
  int64 transferred_bytes,
  FProgressDelegate on_progress) {

  AsyncTask(ENamedThreads::GameThread,
            [on_progress, progress, transferred_bytes]() {

    FFileTransferProgress overall_patch_progress;
    overall_patch_progress.progress = progress;
    overall_patch_progress.transferredBytes = transferred_bytes;
    on_progress.ExecuteIfBound(overall_patch_progress);
  });
}

// -----------------------------------------------------------------------------
spatch_progressListener_t BinaryDiffServiceHDiffImpl::CreateProgressListener(
  IBinaryDiffService::FProgressDelegate& delegate) {

  spatch_progressListener_t progressListener = { 0 };

  auto context =
    new TPair<IBinaryDiffServiceInternalProgress*, FProgressDelegate>();
  context->Key = this;
  context->Value = delegate;

  progressListener.context = context;
  progressListener.onProgress = hpatch_progress_handler;

  return progressListener;
}

// -----------------------------------------------------------------------------
void BinaryDiffServiceHDiffImpl::DestroyProgressListener(
  spatch_progressListener_t& listener) {
  if (listener.context) {
    delete (TContextPair*)listener.context;
    listener.context = nullptr;
  }
}
