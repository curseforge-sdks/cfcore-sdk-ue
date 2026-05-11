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
#include "compression_service_impl.h"

#include <services/thread_pools_service.h>
#include <services/filesystem/file_system_service.h>

#include <services/logger/logger_macros.h>
#include <services/buffer/buffer.h>
#include "internal/zip_buffer_handle.h"
#include "internal/unzip_buffer_handle.h"
#include "internal/minizip/zip.h"

#include "common/macros.h"
#include "common/constants.h"

#include <Async/Async.h>
#include <Misc/Paths.h>

namespace cfcore {
  constexpr uint64 kZipReadBufferPageSizeInBytes = 40 * 1024 * 1024;
  constexpr uint64 kZipReadBufferMaxPages = 4;
  constexpr uint64 kZipStreamChunkBytes = 40 * 1024 * 1024;
  constexpr int32 kZipMethodStore = 0;
  constexpr int64 kFourGigabytesInBytes = 1ll << 32;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
CompressionServiceImpl::CompressionServiceImpl(
  TSharedPtr<IThreadPoolsService> thread_pools_service,
  TSharedPtr<IFileSystemService> file_system_service,
  uint64 buffer_size) :
  thread_pool_(thread_pools_service->Compression()),
  file_system_service_(file_system_service),
  buffer_size_(buffer_size) {
}

// -----------------------------------------------------------------------------
TFuture<ECompressionError> CompressionServiceImpl::Zip(
  const TSharedRef<TArray<FString>> files_to_zip,
  const FString& output_zip_file,
  FProgressDelegate on_progress) {

  SHAREABLE_PROMISE(ECompressionError, promise);

  AsyncPool(*thread_pool_,
            [this, files_to_zip, output_zip_file, on_progress, promise]() {
    ZipInternal(files_to_zip, output_zip_file, on_progress, promise);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<ECompressionError> CompressionServiceImpl::Unzip(
  const FString& zip_file,
  const FString& output_folder,
  FProgressDelegate on_progrs) {

  SHAREABLE_PROMISE(ECompressionError, promise);

  AsyncPool(*thread_pool_,
            [this, zip_file, output_folder, on_progrs, promise]() {
    UnzipInternal(zip_file, output_folder, on_progrs, promise);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void CompressionServiceImpl::ZipInternal(
  const TSharedRef<TArray<FString>> paths_to_zip,
  const FString& output_zip_file,
  FProgressDelegate on_progress,
  TSharedPtr<TPromise<ECompressionError>> promise) {

  if (paths_to_zip->Num() == 0) {
    CFCORE_LOG(Warning, TEXT("No input files to zip"));
    auto code = ECompressionError::None;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
    return;
  }

  TSharedRef<TArray<FString>> flat_inputs = FlattenInputs(paths_to_zip);
  if (flat_inputs->Num() == 0) {
    CFCORE_LOG(Warning, TEXT("Failed to zip - no files found"));
    auto code = ECompressionError::None;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
    return;
  }

  if (flat_inputs->Contains(output_zip_file)) {
    flat_inputs->Remove(output_zip_file);
  }

  TSharedPtr<ZipBufferHandle> writer = ZipBufferHandle::FromFile(
    output_zip_file,
    nullptr);

  if (!writer.IsValid()) {
    CFCORE_LOG(Error, TEXT("Failed to create zip file %s"), *output_zip_file);
    auto code = ECompressionError::FailedToWriteFile;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
    return;
  }

  PerformZip(flat_inputs, writer, on_progress, promise);
}

// -----------------------------------------------------------------------------
void CompressionServiceImpl::UnzipInternal(
  const FString& zip_file,
  const FString& output_folder,
  FProgressDelegate on_progress,
  TSharedPtr<TPromise<ECompressionError>> promise) {

  TSharedPtr<UnzipBufferHandle> zip = UnzipBufferHandle::FromFile(zip_file);
  if (!zip.IsValid()) {
    CFCORE_LOG(Error, TEXT("Failed to read zip data"));
    auto err = ECompressionError::FailedToReadZip;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
    return;
  }

  auto infos = zip->GetZipInfos();

  FCompressionProgress progress;
  // NOTE(twolf): Set to -1 so that we also send progress 0
  progress.progress = -1;

  int32 num_of_infos = infos.Num();
  int32 info_count = 0;

  for (const TPair<FString, UnzipBufferHandle::FZLibFileInfo>& pair : infos) {
    info_count++;

    if (pair.Value.is_directory) {
      FString directory = FPaths::Combine(output_folder, pair.Key);
      auto future = file_system_service_->CreateDirectory(directory);
      future.Wait();
      continue;
    }

    progress.file = pair.Key;
    HandleZipProgress(on_progress, progress, info_count, num_of_infos);

    ECompressionError err = ExtractFile(zip, pair.Key, output_folder);
    if (err != ECompressionError::None) {
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
      return;
    }
  }

  auto code = ECompressionError::None;
  SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
}

// -----------------------------------------------------------------------------
FString CompressionServiceImpl::ComputeCommonRoot(
  const TArray<FString>& paths) {

  if (paths.Num() == 0) {
    return FString();
  }

  FString base = FPaths::GetPath(paths[0]);
  for (const FString& path : paths) {
    if (path == paths[0]) {
      continue;
    }

    ComputeCommonRoot(base, path);

    if (base.IsEmpty()) {
      break;
    }
  }
  return base;
}

// -----------------------------------------------------------------------------
bool CompressionServiceImpl::CompressFile(const FString& abs_file_path,
                                          const FString& input_root,
                                          TSharedPtr<ZipBufferHandle> writer,
                                          TArray<uint8>& chunk) {

  if (!writer.IsValid() || chunk.Num() <= 0) {
    return false;
  }

  TSharedPtr<IPersistentBinaryFileReader> reader =
    CreateBufferedReader(abs_file_path);
  if (!reader.IsValid()) {
    return false;
  }

  const int64 file_size = reader->TotalSizeInBytes();
  if (file_size < 0) {
    return false;
  }

  int32 compression_level = 0;
  int32 zip_method = 0;
  ChooseZipParamsForFile(abs_file_path, compression_level, zip_method);

  // if size is equal to or greater than 4GB, toggle zip64
  const bool zip64 = file_size >= kFourGigabytesInBytes;
  FString entry_name = SanitizeZipEntryName(abs_file_path, input_root);

  if (!writer->OpenNewEntry(entry_name, zip_method, compression_level, zip64)) {
    return false;
  }

  int64 offset = 0;
  while (offset < file_size) {
    const int64 to_read =
      FMath::Min<int64>(kZipStreamChunkBytes, file_size - offset);

    TArrayView64<uint8> view(chunk.GetData(), to_read);

    const bool read_success = reader->Read(offset, to_read, view);
    if (!read_success) {
      writer->CloseEntry();
      return false;
    }

    const bool write_success = writer->Write(view.GetData(), to_read);
    if (!write_success) {
      writer->CloseEntry();
      return false;
    }

    offset += to_read;
  }

  if (!writer->CloseEntry()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
TSharedRef<TArray<FString>> CompressionServiceImpl::FlattenInputs(
  const TSharedRef<TArray<FString>> inputs) {

  auto flat_inputs = MakeShared<TArray<FString>>();
  auto file_prefix = MakeShared<FString>(TEXT(""));
  flat_inputs->Reserve(inputs->Num());

  for (const FString& path : *inputs) {
    auto shared_path = MakeShared<FString>(path);
    TFuture<bool> file_exists = file_system_service_->FileExists(shared_path);

    file_exists.Wait();
    if (file_exists.Get()) {
      flat_inputs->Add(path);
      continue;
    }

    auto path_exists = file_system_service_->PathExists(shared_path);
    path_exists.Wait();

    if (path_exists.Get()) {
      TSharedRef<TArray<FString>> rec = MakeShared<TArray<FString>>();
      auto ok = file_system_service_->GetFilesInDirectoryRecursive(
        shared_path,
        file_prefix,
        rec
      );

      ok.Wait();
      if (ok.Get()) {
        flat_inputs->Append(*rec);
      }
    }
  }

  return flat_inputs;
}

// -----------------------------------------------------------------------------
void CompressionServiceImpl::PerformZip(
  TSharedRef<TArray<FString>> inputs,
  TSharedPtr<ZipBufferHandle> writer,
  FProgressDelegate on_progress,
  TSharedPtr<TPromise<ECompressionError>> promise) {

  // Use same chunk for all zip files to save on allocations
  TSharedRef<TArray<uint8>> chunk = MakeShared<TArray<uint8>>();
  chunk->SetNumUninitialized(kZipStreamChunkBytes);

  const FString input_root = ComputeCommonRoot(inputs.Get());

  FCompressionProgress progress;
  progress.progress = -1;
  int32 done = 0;

  for (const FString& abs_file_path : inputs.Get()) {
    progress.file = abs_file_path;
    HandleZipProgress(on_progress, progress, done, inputs->Num());

    const bool current_file_zip_success = CompressFile(abs_file_path,
                                                       input_root,
                                                       writer,
                                                       chunk.Get());

    if (!current_file_zip_success) {
      writer->Close();
      auto code = ECompressionError::FailedToWriteFile;
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
      return;
    }

    done += 1;
  }

  if (!writer->Close()) {
    auto code = ECompressionError::FailedToWriteFile;
    SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
    return;
  }

  auto code = ECompressionError::None;
  SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, code);
}
// -----------------------------------------------------------------------------
ECompressionError CompressionServiceImpl::ExtractFile(
  TSharedPtr<UnzipBufferHandle> zip,
  const FString& filename,
  const FString& output_folder) {

  if (!zip->OpenCurrentFile(*filename)) {
    CFCORE_LOG(Error, TEXT("Failed to open zip file %s"), *filename);
    return ECompressionError::FailedToExtractFile;
  }

  auto output = MakeShared<FString>(FPaths::Combine(output_folder, filename));

  TArray<uint8> buffer;
  buffer.SetNumUninitialized(buffer_size_);

  ECompressionError result = ECompressionError::None;
  int64 write_offset = 0;

  while (true) {
    int64 read = zip->ReadCurrentFile(buffer);

    if (read < 0) {
      CFCORE_LOG(Error, TEXT("Failed to read zip file %s"), *filename);
      result = ECompressionError::FailedToExtractFile;
      break;
    }

    if (read == 0) {
      break;
    }

    auto future = file_system_service_->WriteBuffer(output,
      buffer.GetData(),
      read,
      write_offset);
    future.Wait();
    if (future.Get() != EFileSystemError::None) {
      CFCORE_LOG(Error, TEXT("Failed to write zip file %s"), *filename);
      result = ECompressionError::FailedToExtractFile;
      break;
    }

    write_offset += read;
  }

  zip->CloseCurrentFile();
  return result;
}

// -----------------------------------------------------------------------------
void CompressionServiceImpl::ComputeCommonRoot(FString& base,
                                               const FString& current) {

  while (!current.IsEmpty() && !base.IsEmpty()
    && !current.StartsWith(base, ESearchCase::IgnoreCase)) {
    base = FPaths::GetPath(base);
  }
}

// -----------------------------------------------------------------------------
FString CompressionServiceImpl::SanitizeZipEntryName(const FString& in_path,
                                                     const FString& base_dir) {

  FString rel = in_path;
  if (!base_dir.IsEmpty()) {
    FPaths::MakePathRelativeTo(rel, *base_dir);
  }

  rel.ReplaceInline(TEXT("\\"), TEXT("/"));

  while (rel.StartsWith(TEXT("/"))) {
    rel.RightChopInline(1);
  }

  FPaths::CollapseRelativeDirectories(rel);
  return rel;
}

// -----------------------------------------------------------------------------
TSharedPtr<IPersistentBinaryFileReader>
CompressionServiceImpl::CreateBufferedReader(
  const FString& filename) {

  BufferedFileOptions options;
  options.buffer_page_size_in_bytes = kZipReadBufferPageSizeInBytes;
  options.max_buffer_pages = kZipReadBufferMaxPages;

  return file_system_service_->CreatePersistentBinaryFileReaderBuffered(
    filename,
    options);
}

// -----------------------------------------------------------------------------
void CompressionServiceImpl::ChooseZipParamsForFile(const FString& path,
                                                    int32& out_level,
                                                    int32& out_method) {
  if (ShouldStoreUncompressed(path)) {
    out_level = Z_NO_COMPRESSION;
    out_method = kZipMethodStore;
    return;
  }

  out_level = Z_BEST_SPEED;
  out_method = Z_DEFLATED;
}

// -----------------------------------------------------------------------------
bool CompressionServiceImpl::ShouldStoreUncompressed(const FString& path) {
  const FString ext = FPaths::GetExtension(path, true).ToLower();

  // Already-compressed or poorly-compressible formats.
  TSet<FString> exts_to_store = {
    TEXT(".zip"),
    TEXT(".7z"),
    TEXT(".rar"),
    TEXT(".png"),
    TEXT(".jpg"),
    TEXT(".jpeg"),
    TEXT(".webp"),
    TEXT(".mp4"),
    TEXT(".mov"),
    TEXT(".avi"),
    TEXT(".mp3"),
    TEXT(".ogg"),
    TEXT(".pdf"),
    TEXT(".pak"),
    TEXT(".uasset"),
    TEXT(".umap"),
    TEXT(".bin"),
    TEXT(".tgz")
  };

  if (exts_to_store.Contains(ext)) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
void CompressionServiceImpl::HandleZipProgress(FProgressDelegate on_progress,
                                               FCompressionProgress& progress,
                                               int32 done,
                                               int32 total) {

  int32 new_progress = (done / (float)total) * 100;
  if (progress.progress != new_progress) {
    progress.progress = new_progress;
    AsyncTask(ENamedThreads::GameThread, [on_progress, progress]() {
      on_progress.ExecuteIfBound(progress);
    });
  }
}
