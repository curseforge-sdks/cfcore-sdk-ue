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
#include "file_system_service_impl.h"
#include <Async/Async.h>
#include <HAL/FileManager.h>
#include <Misc/Paths.h>
#include <Misc/FileHelper.h>

#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/io_throttling/io_throttling_service.h>
#include <services/thread_pools_service.h>

#include "persistent_file/persistent_log_file.h"
#include "persistent_file/persistent_binary_file_buffered.h"

#include "persistent_file/persistent_binary_file_internal.h"
#include "persistent_file/persistent_binary_file_reader.h"
#include "persistent_file/persistent_binary_file_writer.h"

namespace cfcore {

using ReadTextRes = TTuple<EFileSystemError, FString>;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
FileSystemServiceImpl::FileSystemServiceImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service,
  TSharedPtr<IThreadPoolsService> thread_pools_service,
  TSharedPtr<ClassFactory<IPersistentLogFile>> persistent_log_file_factory,
  TSharedPtr<PersistBinFileReaderInternalFactory> bin_file_reader_internal,
  TSharedPtr<PersistBinFileWriterInternalFactory> bin_file_writer_internal,
  TSharedPtr<PersistBinFileReaderBufferredInternalFactory>
    bin_file_reader_buffered_intrnl,
  TSharedPtr<PersistBinFileWriterBufferredInternalFactory>
    bin_file_writer_buffered_intrnl,
  TSharedPtr<PersistBinFileBuffFactory> persistnt_bin_file_buffered_factory) :
  io_throttling_service_(io_throttling_service),
  thread_pool_(thread_pools_service->IO()),
  persistent_log_file_factory_(persistent_log_file_factory),
  persistent_bin_file_reader_intrnl_factory_(bin_file_reader_internal),
  persistent_bin_file_writer_intrnl_factory_(bin_file_writer_internal),
  persistnt_bin_file_reader_buffered_factory_(bin_file_reader_buffered_intrnl),
  persistnt_bin_file_writer_buffered_factory_(bin_file_writer_buffered_intrnl),
  persistent_bin_file_buffered_factory_(persistnt_bin_file_buffered_factory) {
}

// -----------------------------------------------------------------------------
TFuture<FFileStatData> FileSystemServiceImpl::GetStat(
  TSharedRef<FString> path) {

  SHAREABLE_PROMISE(FFileStatData, promise);

  AsyncPool(*thread_pool_, [promise, path]() {
    IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();

    FFileStatData file_stat_data = file.GetStatData(*(path.Get()));

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, file_stat_data);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::FileExists(TSharedRef<FString> filename) {
  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [promise, filename]() {
    bool exists = IFileManager::Get().FileExists(*(filename.Get()));

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, exists);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::PathExists(
  TSharedRef<FString> path) {

  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [promise, path]() {
    IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();

    FFileStatData file_stat_data = file.GetStatData(*(path.Get()));
    bool exists = file_stat_data.bIsValid;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, exists);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::DeleteFile(TSharedRef<FString> filename) {

  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [promise, filename]() {
    bool success = IFileManager::Get().Delete(*(filename).Get(), false, true);
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, success);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::DeleteDirectory(
  TSharedRef<FString> directory) {

  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [promise, directory]() {
    bool success = IFileManager::Get().DeleteDirectory(*(directory).Get(),
                                                       false,
                                                       true);
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, success);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<EFileSystemError> FileSystemServiceImpl::CreateDirectory(
  const FString& directory) {

  SHAREABLE_PROMISE(EFileSystemError, promise);

  AsyncPool(*thread_pool_, [this, directory, promise]() {
    EFileSystemError result = EFileSystemError::None;

    if (!AssureDirectoryExists(directory)) {
      result = EFileSystemError::FailedToCreateDirectory;
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<EFileSystemError> FileSystemServiceImpl::ReadBuffer(
  TSharedRef<FString> filename,
  int64 file_offset_in_bytes,
  int64 bytes_to_read,
  TSharedRef<TArray<uint8>> buffer) {

  if (MAX_int32 < bytes_to_read) {
    return MakeFulfilledPromise<EFileSystemError>(
      EFileSystemError::FailedToReadFromDisk).GetFuture();
  }

  SHAREABLE_PROMISE(EFileSystemError, promise);

  AsyncPool(*thread_pool_,
            [filename, bytes_to_read, file_offset_in_bytes, buffer, promise]() {

    IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
    TUniquePtr<IFileHandle> file_handle(platfrm_file.OpenRead(*filename.Get()));

    EFileSystemError result = EFileSystemError::None;

    if (!file_handle.IsValid()) {
      result = EFileSystemError::FailedToReadFromDisk;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
      return;
    }


    int64 current = file_handle->Tell();

    file_handle->SeekFromEnd(0);
    int64 total_size = file_handle->Tell();

    if (file_offset_in_bytes + bytes_to_read > total_size) {
      result = EFileSystemError::FailedReadOutOfBounds;
    } else {
      file_handle->Seek(file_offset_in_bytes);

      if (!file_handle->Read(buffer->GetData(), bytes_to_read)) {
        result = EFileSystemError::FailedToReadFromDisk;
      }
    }

    file_handle->Seek(current);

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<ReadTextRes> FileSystemServiceImpl::ReadText(const FString& filename) {
  SHAREABLE_PROMISE(ReadTextRes, promise);

  AsyncPool(*thread_pool_, [filename, promise]() {
    ReadTextRes result;
    if (!FFileHelper::LoadFileToString(result.Value, *filename)) {
      result.Key = EFileSystemError::FailedToReadFromDisk;
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<ReadBufferRes> FileSystemServiceImpl::ReadBuffer(
  const FString& filename) {

  SHAREABLE_PROMISE(ReadBufferRes, promise);

  AsyncPool(*thread_pool_, [filename, promise]() {
    ReadBufferRes result;
    IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
    TUniquePtr<IFileHandle> file_handle(platfrm_file.OpenRead(*filename));

    result.Key = EFileSystemError::FailedToReadFromDisk;

    if (file_handle.IsValid()) {
      if (MAX_int32 < file_handle->Size()) {
        result.Key = EFileSystemError::FailedFileToLarge;
        SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
        return;
      }

      result.Value = MakeShared<TArray<uint8>>();
      result.Value->AddUninitialized(file_handle->Size());

      if (file_handle->Read(result.Value->GetData(), file_handle->Size())) {
        result.Key = EFileSystemError::None;
      }
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<ReadBuffer64Res> FileSystemServiceImpl::ReadBuffer64(
  TSharedRef<FString> filename) {

  SHAREABLE_PROMISE(ReadBuffer64Res, promise);

  AsyncPool(*thread_pool_, [filename, promise]() {
    ReadBuffer64Res result;
    IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
    TUniquePtr<IFileHandle> file_handle(platfrm_file.OpenRead(**filename));

    result.Key = EFileSystemError::FailedToReadFromDisk;

    if (!file_handle.IsValid()) {
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
      return;
    }

    result.Value = MakeShared<TArray64<uint8>>();
    result.Value->AddUninitialized(file_handle->Size());

    if (file_handle->Read(result.Value->GetData(), file_handle->Size())) {
      result.Key = EFileSystemError::None;
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<EFileSystemError> FileSystemServiceImpl::WriteText(
  const FString& filename,
  const FString& text) {

  SHAREABLE_PROMISE(EFileSystemError, promise);

  AsyncPool(*thread_pool_, [this, filename, text, promise]() {
    EFileSystemError result = EFileSystemError::None;

    if (!AssureFileDirectoryExists(filename)) {
      result = EFileSystemError::FailedToCreateDirectory;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
      return;
    }

    TArray<FString> arr({ text });

    auto encoding = FFileHelper::EEncodingOptions::ForceUTF8;
    if (!FFileHelper::SaveStringArrayToFile(arr, *filename, encoding)) {
      result = EFileSystemError::FailedToSaveToDisk;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
      return;
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<EFileSystemError> FileSystemServiceImpl::WriteBuffer(
  TSharedRef<FString> filename,
  const uint8* data,
  int64 length_in_bytes,
  int64 file_offset_in_bytes/*= 0*/) {

  SHAREABLE_PROMISE(EFileSystemError, promise);

  AsyncPool(
    *thread_pool_,
    [this, filename, data, length_in_bytes, file_offset_in_bytes, promise]() {

    if (!AssureFileDirectoryExists(*filename)) {
      EFileSystemError err = EFileSystemError::FailedToCreateDirectory;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
      return;
    }

    IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
    TUniquePtr<IFileHandle> file_handle(platfrm_file.OpenWrite(**filename,
                                                               true));

    if (!file_handle.IsValid()) {
      uint32 gle = FGenericPlatformMisc::GetLastError();
      CFCORE_LOG(Error, TEXT("Can't open %s (GLE=%d)"), **filename, gle);

      EFileSystemError err = EFileSystemError::FailedToOpenFileForWrite;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
      return;
    }

    file_handle->Seek(file_offset_in_bytes);

    bool success = WriteBufferWithThrottling(
      file_handle,
      data,
      length_in_bytes);

    EFileSystemError result = EFileSystemError::None;
    if (!success) {
      result = EFileSystemError::FailedToSaveToDisk;
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::MoveFile(const FString& src_filename,
                                              const FString& dest_filename) {
  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [src_filename, dest_filename, promise]() {
    IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();
    FFileStatData file_stat_data = file.GetStatData(*src_filename);

    bool success = false;
    if (file_stat_data.bIsValid && !file_stat_data.bIsDirectory) {
      // NOTE(twolf): Trying to move a directory can result in the destination
      // being deleted and will fail anyway
      success = file.MoveFile(*dest_filename, *src_filename);
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, success);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::MoveDirectory(const FString& src_dir,
                                                   const FString& dest_dir) {
  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [src_dir, dest_dir, promise]() {
    IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();
    FFileStatData file_stat_data = file.GetStatData(*src_dir);

    bool success = false;

    if (file_stat_data.bIsValid && file_stat_data.bIsDirectory) {
      success = file.CopyDirectoryTree(*dest_dir, *src_dir, true);
    }

    if (success) {
      file.DeleteDirectoryRecursively(*src_dir);
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, success);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<EFileSystemError> FileSystemServiceImpl::GetDiskSpaceInfo(
  TSharedRef<FString> path,
  IN OUT TSharedRef<DiskSpaceInfo> disk_space_info) {

  SHAREABLE_PROMISE(EFileSystemError, promise);

  AsyncPool(*thread_pool_, [this, path, disk_space_info, promise]() {
    EFileSystemError result = EFileSystemError::None;

    bool exists = AssureDirectoryExists(*path);
    if (!exists) {
      result = EFileSystemError::FailedToCreateDirectory;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
      return;
    }

    bool success = FPlatformMisc::GetDiskTotalAndFreeSpace(
      *(path.Get()),
      disk_space_info->total_size_in_bytes,
      disk_space_info->free_size_int_bytes);

    if (!success) {
      result = EFileSystemError::FailedGettingDiskSpaceInfo;
    }

    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::GetFilesInDirectory(
  TSharedRef<FString> dir,
  TSharedRef<FString> file_prefix,
  OUT TSharedRef<TArray<FString>> files) {

  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [dir, file_prefix, files, promise]() {
    EFileSystemError result = EFileSystemError::None;

    bool success = IFileManager::Get().IterateDirectory(
      **dir,
      [file_prefix, dir, files](const TCHAR* filename_or_dir, bool is_dir) {

      if (is_dir) {
        return true;
      }

      FString filename = FPaths::GetCleanFilename(filename_or_dir);
      if (!file_prefix->IsEmpty() && !filename.StartsWith(*file_prefix)) {
        return true;
      }

      files->Add(filename_or_dir);
      return true;
    });

    if (!success) {
      result = EFileSystemError::FailedToReadFromDisk;
    }

    success = result == EFileSystemError::None;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, success);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> FileSystemServiceImpl::GetFilesInDirectoryRecursive(
  TSharedRef<FString> dir,
  TSharedRef<FString> file_prefix,
  OUT TSharedRef<TArray<FString>> files) {

  SHAREABLE_PROMISE(bool, promise);

  AsyncPool(*thread_pool_, [dir, file_prefix, files, promise]() {
    EFileSystemError result = EFileSystemError::None;

    bool success = IFileManager::Get().IterateDirectoryRecursively(
      **dir,
      [file_prefix, dir, files](const TCHAR* filename_or_dir, bool is_dir) {

      if (is_dir) {
        return true;
      }

      FString filename = FPaths::GetCleanFilename(filename_or_dir);
      if (!file_prefix->IsEmpty() && !filename.StartsWith(*file_prefix)) {
        return true;
      }

      files->Add(filename_or_dir);
      return true;
    });

    if (!success) {
      result = EFileSystemError::FailedToReadFromDisk;
    }

    success = result == EFileSystemError::None;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, success);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TSharedPtr<IPersistentLogFile> FileSystemServiceImpl::CreatePersistentLogFile(
  const FString& filename) {

  TSharedPtr<IPersistentLogFile> file = persistent_log_file_factory_->Create();
  file->Initialize(filename);
  return file;
}

// -----------------------------------------------------------------------------
TSharedPtr<IPersistentBinaryFileReader>
FileSystemServiceImpl::CreatePersistentBinaryFileReader(const FString& file) {
  TSharedPtr<PersistentBinFileReaderInternal> file_internal =
    persistent_bin_file_reader_intrnl_factory_->Create();

  if (!file_internal->Open(file)) {
    CFCORE_LOG(Error, TEXT("Failed to open persistent binary file %s"), *file);
    return nullptr;
  }

  return file_internal->AsType();
}

// -----------------------------------------------------------------------------
TSharedPtr<IPersistentBinaryFileWriter>
FileSystemServiceImpl::CreatePersistentBinaryFileWriter(const FString& file) {
  TSharedPtr<PersistentBinFileWriterInternal> file_internal =
    persistent_bin_file_writer_intrnl_factory_->Create();

  if (!file_internal->Open(file)) {
    CFCORE_LOG(Error, TEXT("Failed to open persistent binary file %s"), *file);
    return nullptr;
  }

  return file_internal->AsType();
}

// -----------------------------------------------------------------------------
TSharedPtr<IPersistentBinaryFileReader>
FileSystemServiceImpl::CreatePersistentBinaryFileReaderBuffered(
  const FString& filename,
  struct BufferedFileOptions& options) {

  auto file_reader = CreatePersistentBinaryFileReader(filename);
  if (!file_reader.IsValid()) {
    return nullptr;
  }

  TSharedRef<PersistentBinFileReaderBufferedInternal> internal =
    persistnt_bin_file_reader_buffered_factory_->Create();

  internal->Initialize(file_reader.ToSharedRef(), options);
  return internal->AsType();
}

// -----------------------------------------------------------------------------
TSharedPtr<IPersistentBinaryFileWriter>
FileSystemServiceImpl::CreatePersistentBinaryFileWriterBuffered(
  const FString& filename,
  struct BufferedFileOptions& options) {

  auto file_writer = CreatePersistentBinaryFileWriter(filename);
  if (!file_writer.IsValid()) {
    return nullptr;
  }

  TSharedRef<PersistentBinFileWriterBufferedInternal> internal =
    persistnt_bin_file_writer_buffered_factory_->Create();

  internal->Initialize(file_writer.ToSharedRef(), options);
  return internal->AsType();
}


// -----------------------------------------------------------------------------
TSharedPtr<IPersistentBinaryFile>
FileSystemServiceImpl::CreatePersistentBinaryFile(const FString& filename,
                                                  bool read) {

  TSharedPtr<IPersistentBinaryFile> file =
    persistent_bin_file_buffered_factory_->Create();

  if (read) {
    file->OpenRead(filename);
  } else {
    file->OpenWrite(filename);
  }

  return file;
}

// -----------------------------------------------------------------------------
bool FileSystemServiceImpl::AssureFileDirectoryExists(const FString& filename) {
  FString path, name, extension;
  FPaths::Split(filename, path, name, extension);

  return AssureDirectoryExists(path);
}

// -----------------------------------------------------------------------------
bool FileSystemServiceImpl::AssureDirectoryExists(const FString& directory) {
  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  if (PlatformFile.DirectoryExists(*directory)) {
    return true;
  }

  if (PlatformFile.CreateDirectoryTree(*directory)) {
    return true;
  }

  CFCORE_LOG(Error, TEXT("Unable to create a directory %s"), *directory);
  return true;
}


// -----------------------------------------------------------------------------
bool FileSystemServiceImpl::WriteBufferWithThrottling(
  const TUniquePtr<IFileHandle>& file_handle,
  const uint8* data,
  int64 length) {

  int64 written_bytes = 0;
  int64 left_to_write = length - written_bytes;
  bool success = true;

  while (success && (left_to_write > 0)) {
    int64 bytes_to_write =
      io_throttling_service_->SleepForNextQuotaInBytes(left_to_write);

    success = file_handle->Write(&data[written_bytes], bytes_to_write);
    if (success) {
      success = file_handle->Flush();
    }

    written_bytes += bytes_to_write;
    left_to_write = length - written_bytes;
  }

  return success;
}
