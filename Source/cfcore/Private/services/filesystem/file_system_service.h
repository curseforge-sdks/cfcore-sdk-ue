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
#include <GenericPlatform/GenericPlatformFile.h>

#include "file_system_error.h"
#include "models/disk_space_info.h"
#include "models/buffered_file_options.h"

namespace cfcore {
class IPersistentLogFile;
class IPersistentBinaryFileReader;
class IPersistentBinaryFileWriter;
class IPersistentBinaryFile;

typedef TTuple<EFileSystemError, TSharedPtr<TArray<uint8>>> ReadBufferRes;
typedef TTuple<EFileSystemError, TSharedPtr<TArray64<uint8>>> ReadBuffer64Res;

// -----------------------------------------------------------------------------
class IFileSystemService {
public:
  virtual ~IFileSystemService() = default;

public:
  virtual TFuture<bool> FileExists(TSharedRef<FString> filename) = 0;

  virtual TFuture<FFileStatData> GetStat(TSharedRef<FString> path) = 0;

  /**
   * Supports both directories and files - unlike FileExists which will only
   * work for files
   */
  virtual TFuture<bool> PathExists(TSharedRef<FString> path) = 0;
  virtual TFuture<bool> DeleteFile(TSharedRef<FString> filename) = 0;
  virtual TFuture<bool> DeleteDirectory(TSharedRef<FString> directory) = 0;
  virtual TFuture<EFileSystemError> CreateDirectory(
    const FString& directory) = 0;

  virtual TFuture<EFileSystemError> ReadBuffer(
    TSharedRef<FString> filename,
    int64 file_offset_in_bytes,
    int64 bytes_to_read,
    TSharedRef<TArray<uint8>> buffer) = 0;

  virtual TFuture<TTuple<EFileSystemError, FString>> ReadText(
    const FString& filename) = 0;

  virtual TFuture<ReadBufferRes> ReadBuffer(const FString& filename) = 0;

  /**
   * Use wisely - if you have to load a lot of memory from disk.
   */
  virtual TFuture<ReadBuffer64Res> ReadBuffer64(
    TSharedRef<FString> filename) = 0;

  virtual TFuture<EFileSystemError> WriteText(const FString& filename,
                                              const FString& text) = 0;

  // NOTE(twolf): When passing data to |WriteBuffer| - it might (most probably)
  // be ran on a separate thread. Be cautious about scoping and using TSharedPtr
  // as you'll need to keep a reference alive (e.g. by passing it to the capture
  // clause by value and not by reference)
  virtual TFuture<EFileSystemError> WriteBuffer(
    TSharedRef<FString> filename,
    const uint8* data,
    int64 length_in_bytes,
    int64 file_offset_in_bytes = 0) = 0;

  virtual TFuture<bool> MoveFile(const FString& src_filename,
                                 const FString& dest_filename) = 0;
  virtual TFuture<bool> MoveDirectory(const FString& src_dir,
                                      const FString& dest_dir) = 0;

  /**
   * Given any path - this function will look at the root disk the path is on
   * and return information about it.
   */
  virtual TFuture<EFileSystemError> GetDiskSpaceInfo(
    TSharedRef<FString> path,
    IN OUT TSharedRef<DiskSpaceInfo> disk_space_info) = 0;

  /**
   * Get a list filenames in a directory, with an option to filter by filename
   * prefix
   */
  virtual TFuture<bool> GetFilesInDirectory(
    TSharedRef<FString> directory,
    TSharedRef<FString> file_prefix,
    OUT TSharedRef<TArray<FString>> files) = 0;

  /**
   * Same as |GetFilesInDirectory| but returns files that are in inner
   * directories as well as the top directory
   */
  virtual TFuture<bool> GetFilesInDirectoryRecursive(
    TSharedRef<FString> directory,
    TSharedRef<FString> file_prefix,
    OUT TSharedRef<TArray<FString>> files) = 0;

  virtual TSharedPtr<IPersistentLogFile> CreatePersistentLogFile(
    const FString& filename) = 0;

  virtual TSharedPtr<IPersistentBinaryFileReader>
    CreatePersistentBinaryFileReader(const FString& filename) = 0;
  virtual TSharedPtr<IPersistentBinaryFileWriter>
    CreatePersistentBinaryFileWriter(const FString& filename) = 0;
  virtual TSharedPtr<IPersistentBinaryFileReader>
    CreatePersistentBinaryFileReaderBuffered(
      const FString& filename,
      struct BufferedFileOptions& options) = 0;
  virtual TSharedPtr<IPersistentBinaryFileWriter>
    CreatePersistentBinaryFileWriterBuffered(
      const FString& filename,
      struct BufferedFileOptions& options) = 0;

  virtual TSharedPtr<IPersistentBinaryFile> CreatePersistentBinaryFile(
    const FString& filename,
    bool read) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IFileSystemService)