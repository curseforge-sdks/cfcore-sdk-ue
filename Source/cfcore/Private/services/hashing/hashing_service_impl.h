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
#include <Async/Future.h>
#include <Misc/QueuedThreadPool.h>

#include "hashing_service.h"
#include <services/filesystem/file_system_error.h>

namespace cfcore {

class IThreadPoolsService;
class IFileSystemService;
class IPersistentBinaryFile;

// -----------------------------------------------------------------------------
class HashingServiceImpl : public IHashingService {
public:
  HashingServiceImpl(TSharedRef<IThreadPoolsService> thread_pools_service,
                     TSharedRef<IFileSystemService> file_system_service);

// IHashingService
public:
  virtual FString Md5(TSharedPtr<IBuffer> buffer) override;
  virtual FString Sha1(TSharedPtr<IBuffer> buffer) override;
  virtual uint64 Murmur2(TSharedPtr<IBuffer> buffer) override;

  virtual TFuture<HashResult> Md5(TSharedRef<FString> filename) override;
  virtual TFuture<HashResult> Sha1(TSharedRef<FString> filename) override;
  virtual TFuture<HashResult> Crc32(TSharedRef<FString> filename) override;
  virtual TFuture<HashResult> Murmur2(TSharedRef<FString> file) override;

private:
  uint64 Murmur2ComputeNormalizedLength(TSharedPtr<IBuffer> buffer);
  TFuture<HashResult> Murmur2ComputeNormalizedLength(
    TSharedRef<FString> filename,
    TSharedRef<TArray<uint8>> reusable_buffer);
  TFuture<HashResult> Murmur2ComputeHash(
    TSharedRef<FString> filename,
    uint64 normalized_file_length,
    TSharedRef<TArray<uint8>> reusable_buffer);
  bool Murmur2IsWhitespaceChar(char chr);

  // TODO(twolf): Consider moving this to FileSystemService
  void StreamFileForHash(
    TSharedRef<FString> filename,
    TSharedRef<TArray<uint8>> reusable_buffer,
    TFunction<TFuture<FCFCoreError>(int64, bool, EFileSystemError)> callback);

  void InternalStreamFileForHash(
    TSharedRef<FString> filename,
    int64 bytes_read,
    int64 total_bytes,
    TSharedRef<TArray<uint8>> reusable_buffer,
    TFunction<TFuture<FCFCoreError>(int64, bool, EFileSystemError)> callback);

  void HandleFileSystemError(TSharedRef<FString> filename,
                             EFileSystemError filesys_err,
                             TSharedRef<TPromise<HashResult>> promise);

private:
  TSharedPtr<FQueuedThreadPool> io_thread_pool_;
  TSharedPtr<FQueuedThreadPool> cpu_intensive_thread_pool_;
  TSharedRef<IFileSystemService> file_system_service_;
};

} // namespace cfcore