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
#pragma once
#include <CoreMinimal.h>
#include "binary_diff_service.h"
#include "internal/hdiff/HPatch/patch.h"

namespace cfcore {
class IIOThrottlingService;
class IThreadPoolsService;
class IHashingService;
class IFileSystemService;

// -----------------------------------------------------------------------------
class IBinaryDiffServiceInternalProgress {
public:
  virtual ~IBinaryDiffServiceInternalProgress() = default;
  virtual void OnProgress(IBinaryDiffService::FProgressDelegate delegate,
                          int32 progress,
                          hpatch_StreamPos_t total_bytes) = 0;
};

// -----------------------------------------------------------------------------
class BinaryDiffServiceHDiffImpl : public IBinaryDiffService,
                                   public IBinaryDiffServiceInternalProgress {
public:
  BinaryDiffServiceHDiffImpl(
    TSharedPtr<IIOThrottlingService> io_throttling_service,
    TSharedPtr<IThreadPoolsService> thread_pools_service,
    TSharedPtr<IHashingService> hashing_service,
    TSharedPtr<IFileSystemService> filesystem_service);

// IBinaryDiffService
public:
  // Currently, we load patch_file into memory, so try to keep it under
  // 100MB.
  virtual TFuture<FCFCoreError> ApplyBinaryPatch(
    const FString& old_file,
    const FString& patch_file,
    const FString& output_file,
    FProgressDelegate progress_delegate) override;

  virtual TFuture<FCFCoreError> CalcOutputFileSize(
    const FString& patch_file,
    TSharedRef<int64> output_files_total_size) override;

// IBinaryDiffServiceInternalProgress
public:
  virtual void OnProgress(IBinaryDiffService::FProgressDelegate delegate,
                          int32 progress,
                          hpatch_StreamPos_t total_bytes) override;

private:
  bool GetPatchInfo(
    const hpatch_TStreamInput& patch_stream,
    hpatch_singleCompressedDiffInfo& out_single_compressed_diff_info);

  TFuture<FCFCoreError> AssureFileDeleted(const FString& file);

  TFuture<FCFCoreError> PerformPatchAsync(const FString& old_file,
                                          const FString& patch_file,
                                          const FString& output_file,
                                          FProgressDelegate progress_delegate);
  FCFCoreError PerformPatch(const FString& old_file,
                                     const FString& patch_file,
                                     const FString& output_file,
                                     FProgressDelegate progress_delegate);

  void HandlePatchingProgress(int32 progress,
                              int64 transferred_bytes,
                              FProgressDelegate progress_delegate);

  spatch_progressListener_t CreateProgressListener(FProgressDelegate& delegate);
  void DestroyProgressListener(spatch_progressListener_t& listener);

private:
  TSharedPtr<IIOThrottlingService> io_throttling_service_;
  TSharedPtr<FQueuedThreadPool> thread_pool_;
  TSharedPtr<IHashingService> hashing_service_;
  TSharedPtr<IFileSystemService> filesystem_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::BinaryDiffServiceHDiffImpl)