/*MIT License

Copyright (c) 2022 Overwolf Ltd.

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
#include <Misc/QueuedThreadPool.h>
#include <utils/compression/compression_service.h>

#include "internal/zip_buffer_handle.h"
#include <services/filesystem/persistent_file/persistent_binary_file_reader.h>

namespace cfcore {
class IThreadPoolsService;
class IFileSystemService;
class IBuffer;

// -----------------------------------------------------------------------------
class CompressionServiceImpl : public ICompressionService {
public:
  CompressionServiceImpl(TSharedPtr<IThreadPoolsService> thread_pools_service,
                 TSharedPtr<IFileSystemService> file_system_service,
                 uint64 buffer_size = 31457280);

public:
  virtual TFuture<ECompressionError> Zip(
    const TSharedRef<TArray<FString>> files_to_zip,
    const FString& output_zip_file,
    FProgressDelegate on_progress) override;

  virtual TFuture<ECompressionError> Unzip(
    const FString& zip_file,
    const FString& output_folder,
    FProgressDelegate on_progress) override;

private:
  void ZipInternal(const TSharedRef<TArray<FString>> paths_to_zip,
                   const FString& output_zip_file,
                   FProgressDelegate on_progress,
                   TSharedPtr<TPromise<ECompressionError>> promise);

  void UnzipInternal(const FString& zip_file,
                     const FString& output_folder,
                     FProgressDelegate on_progress,
                     TSharedPtr<TPromise<ECompressionError>> promise);

  FString ComputeCommonRoot(const TArray<FString>& paths);

  bool CompressFile(const FString& abs_file_path,
                    const FString& input_root,
                    TSharedPtr<ZipBufferHandle> writer,
                    TArray<uint8>& chunk);

  TSharedPtr<IPersistentBinaryFileReader> CreateBufferedReader(
    const FString& filename);

  TSharedRef<TArray<FString>> FlattenInputs(
    const TSharedRef<TArray<FString>> inputs);

  void PerformZip(TSharedRef<TArray<FString>> inputs,
                  TSharedPtr<ZipBufferHandle> writer,
                  FProgressDelegate on_progress,
                  TSharedPtr<TPromise<ECompressionError>> promise);

  void ChooseZipParamsForFile(const FString& path,
                              int32& out_level,
                              int32& out_method);

  bool ShouldStoreUncompressed(const FString& path);

  ECompressionError ExtractFile(TSharedPtr<class UnzipBufferHandle> zip,
                                const FString& filename,
                                const FString& output_folder);

  FString SanitizeZipEntryName(const FString& in_path, const FString& base_dir);
  void ComputeCommonRoot(FString& base, const FString& current);
  void HandleZipProgress(FProgressDelegate on_progress,
                         FCompressionProgress& progress,
                         int32 done,
                         int32 total);

private:
  TSharedPtr<FQueuedThreadPool> thread_pool_;
  TSharedPtr<class IFileSystemService> file_system_service_;
  uint64 buffer_size_;
};

} // namespace cfcore
