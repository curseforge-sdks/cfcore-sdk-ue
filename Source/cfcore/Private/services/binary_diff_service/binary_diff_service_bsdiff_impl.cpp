/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include "binary_diff_service_bsdiff_impl.h"
#include <common/macros.h>
#include <common/future_utils.h>

#include <services/io_throttling/io_throttling_service.h>
#include <services/thread_pools_service.h>
#include <services/logger/logger_macros.h>
#include <services/hashing/hashing_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/filesystem/persistent_file/persistent_binary_file.h>

#include "internal/bsdiff/streams.h"

#include <Async/Async.h>

namespace cfcore {
};

using namespace cfcore;
using namespace bsdiff;

// -----------------------------------------------------------------------------
BinaryDiffServiceBSDiffImpl::BinaryDiffServiceBSDiffImpl(
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
TFuture<FCFCoreError> BinaryDiffServiceBSDiffImpl::ApplyBinaryPatch(
  const FString& old_file,
  const FString& patch_file,
  const FString& output_file,
  FProgressDelegate progress_delegate) {

  // TODO(apliscov): extract method(s) to reduce function size
  SHAREABLE_PROMISE(FCFCoreError, promise);

  CFCORE_LOG(Log, TEXT("Applying patch for %s..."), *output_file);

  auto crc32 = MakeShared<uint32_t>(0);
  auto patch_stream = MakeShared<courgette::SourceStream>();

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  futures->Add(CalcCrc32(old_file, crc32));
  futures->Add(InitSourceStream(patch_file, patch_stream));
  futures->Add(AssureFileDeleted(output_file));

  auto future = FutureUtils::Parallel<FCFCoreError>(futures);

  future.Next([this,
               promise,
               crc32,
               patch_stream,
               old_file,
               output_file,
               progress_delegate](TArray<FCFCoreError> errs) {

    for (FCFCoreError& err : errs) {
      if (err.isError) {
        promise->SetValue(err);
        return;
      }
    }

    AsyncPool(*thread_pool_,
              [this,
               promise,
               old_file,
               crc32,
               patch_stream,
               output_file,
               progress_delegate]() {

      FCFCoreError err = ApplyBinaryPatch(old_file,
                                          *crc32,
                                          patch_stream,
                                          output_file,
                                          progress_delegate);

      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, err);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceBSDiffImpl::CalcOutputFileSize(
  const FString& patch_file,
  TSharedRef<int64> output_files_total_size) {

  SHAREABLE_PROMISE(FCFCoreError, promise)
  ECFCoreErrorCodes code = ECFCoreErrorCodes::BindiffInvalidHeader;
  auto patch_stream = MakeShared<courgette::SourceStream>();
  auto future = InitHeaderSourceStream(patch_file, patch_stream);
  future.Next([this,
               promise,
               code,
               patch_stream = patch_stream,
               output_files_total_size](FCFCoreError err) {

    if (err.isError) {
      promise->SetValue(err);
      return;
    }

    MBSPatchHeader patch_header;
    if (!ReadHeader(patch_stream, &patch_header)) {
      promise->SetValue(FCFCoreError(code));
      return;
    }

    *output_files_total_size += patch_header.dlen;
    promise->SetValue(FCFCoreError());
    return;
  });
  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceBSDiffImpl::CalcCrc32(
  const FString& file,
  TSharedRef<uint32_t> crc) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto file_shared = MakeShared<FString>(file);
  hashing_service_->Crc32(file_shared).Next([promise, crc](HashResult res) {
    if (res.Key.isError) {
      promise->SetValue(res.Key);
      return;
    }

    const TCHAR* start = GetData(res.Value);
    TCHAR* end;
    *crc = (uint32_t)FCString::Strtoui64(start, &end, 10);
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceBSDiffImpl::InitSourceStream(
  const FString& file,
  TSharedRef<courgette::SourceStream> stream) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto file_shared = MakeShared<FString>(file);
  filesystem_service_->ReadBuffer64(file_shared)
    .Next([stream, promise](ReadBuffer64Res res) {

    if (res.Key != EFileSystemError::None) {
      ENUM_TO_STR(str_filesys_error, res.Key);
      CFCORE_LOG(Error, TEXT("Failed reading patch file: %s"), *str_filesys_error);
      ECFCoreErrorCodes code = ECFCoreErrorCodes::FileSystemError;
      promise->SetValue(FCFCoreError(code));
      return;
    }

    TSharedPtr<TArray64<uint8>> buffer = res.Value;
    stream->Init(res.Value);
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceBSDiffImpl::InitHeaderSourceStream(
  const FString& file,
  TSharedRef<courgette::SourceStream> stream) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto buffer = MakeShared<TArray<uint8>>();
  buffer->SetNumUninitialized(kMaxSizeOfBufferForHeaderInBytes);
  auto file_shared = MakeShared<FString>(file);

  filesystem_service_->GetStat(file_shared).Next(
    [this, stream, promise, buffer, file_shared](FFileStatData stat) {

    int64 size =
      FMath::Min(stat.FileSize, (int64)kMaxSizeOfBufferForHeaderInBytes);

    filesystem_service_->ReadBuffer(file_shared,0,size,buffer).Next(
      [stream, promise, buffer](EFileSystemError err) {

      if (err != EFileSystemError::None) {
        ENUM_TO_STR(str_filesys_error, err);
        CFCORE_LOG(Error, TEXT("Failed reading patch file: %s"), *str_filesys_error);
        ECFCoreErrorCodes code = ECFCoreErrorCodes::FileSystemError;
        promise->SetValue(FCFCoreError(code));
        return;
      }

      TArray64<uint8> buffer64(*buffer);
      auto buffer64_ref = MakeShared<TArray64<uint8>>(buffer64);
      stream->Init(buffer64_ref);
      promise->SetValue(FCFCoreError());
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> BinaryDiffServiceBSDiffImpl::AssureFileDeleted(
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
FCFCoreError BinaryDiffServiceBSDiffImpl::ApplyBinaryPatch(
  const FString& old_file,
  uint32_t old_crc32,
  TSharedRef<courgette::SourceStream> patch_stream,
  const FString& output_file,
  FProgressDelegate progress_delegate) {

  check(!IsInGameThread());

  ECFCoreErrorCodes ecode = ECFCoreErrorCodes::BindiffInvalidHeader;

  MBSPatchHeader patch_header;
  if (!ReadHeader(patch_stream, &patch_header)) {
    return FCFCoreError(ecode);
  }

  if (~old_crc32 != patch_header.scrc32) {
    CFCORE_LOG(Error, TEXT("Bsdiff crc32 doesn't match"));
    return FCFCoreError(ecode);
  }

  TSharedPtr<IPersistentBinaryFile> old_persist_file =
    filesystem_service_->CreatePersistentBinaryFile(*old_file, true);

  if (old_persist_file->TotalSizeInBytes() != patch_header.slen) {
    CFCORE_LOG(Error, TEXT("Bsdiff slen doesn't match old file length"));
    return FCFCoreError(ecode);
  }

  TSharedPtr<IPersistentBinaryFile> new_persist_file =
    filesystem_service_->CreatePersistentBinaryFile(*output_file, false);

  BSDiffStatus bsdiff_res = PerformPatch(&patch_header,
                                         patch_stream,
                                         old_persist_file.ToSharedRef(),
                                         new_persist_file.ToSharedRef(),
                                         progress_delegate);

  if (patch_header.dlen != new_persist_file->TotalSizeInBytes()) {
    CFCORE_LOG(Error, TEXT("output file doesn't match header length"));
    return FCFCoreError(ECFCoreErrorCodes::BindiffFailedToApplyPatch);
  }

  if (bsdiff_res == BSDiffStatus::OK) {
    return FCFCoreError();
  }


  CFCORE_LOG(Error, TEXT("Failed to apply bsdiff patch (err=%d)"), bsdiff_res);
  return FCFCoreError(ECFCoreErrorCodes::BindiffFailedToApplyPatch);
}

// -----------------------------------------------------------------------------
bool BinaryDiffServiceBSDiffImpl::ReadHeader(
  TSharedRef<courgette::SourceStream> stream,
  MBSPatchHeader* header) {

  if (!stream->Read(header->tag, sizeof(header->tag))) {
    CFCORE_LOG(Error, TEXT("Failed reading bsdiff header tag"));
    return false;
  }

  if (!stream->ReadVarint32(&header->slen)) {
    CFCORE_LOG(Error, TEXT("Failed reading bsdiff header slen"));
    return false;
  }

  if (!stream->ReadVarint32(&header->scrc32)) {
    CFCORE_LOG(Error, TEXT("Failed reading bsdiff header scrc32"));
    return false;
  }

  if (!stream->ReadVarint32(&header->dlen)) {
    CFCORE_LOG(Error, TEXT("Failed reading bsdiff header dlen"));
    return false;
  }

  // The string will have a NUL terminator that we don't use, hence '-1'.
  static_assert(sizeof(MBS_PATCH_HEADER_TAG) - 1 == sizeof(header->tag),
    "MBS_PATCH_HEADER_TAG must match header field size");
  if (memcmp(header->tag, MBS_PATCH_HEADER_TAG, 8) != 0) {
    CFCORE_LOG(Error, TEXT("Bsdiff header tag invalid"));
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
BSDiffStatus BinaryDiffServiceBSDiffImpl::PerformPatch(
  MBSPatchHeader* header,
  TSharedRef<courgette::SourceStream> patch_stream,
  TSharedRef<IPersistentBinaryFile> old_file,
  TSharedRef<IPersistentBinaryFile> output_file,
  FProgressDelegate progress_delegate) {

  int64 old_start = 0;
  int64 old_end = old_file->TotalSizeInBytes();

  courgette::SourceStreamSet patch_streams;
  if (!patch_streams.Init(&patch_stream.Get()))
    return READ_ERROR;

  courgette::SourceStream* control_stream_copy_counts =
    patch_streams.stream(0);
  courgette::SourceStream* control_stream_extra_counts =
    patch_streams.stream(1);
  courgette::SourceStream* control_stream_seeks = patch_streams.stream(2);
  courgette::SourceStream* diff_skips = patch_streams.stream(3);
  courgette::SourceStream* diff_bytes = patch_streams.stream(4);
  courgette::SourceStream* extra_bytes = patch_streams.stream(5);

  const uint8_t* extra_start = extra_bytes->Buffer();
  const uint8_t* extra_end = extra_start + extra_bytes->Remaining();
  const uint8_t* extra_position = extra_start;
  extra_bytes->Skip(extra_bytes->Remaining());

  int64 old_position = old_start;

  //if (header->dlen && !new_stream->Reserve(header->dlen))
  //  return MEM_ERROR;

  uint32_t pending_diff_zeros = 0;
  if (!diff_skips->ReadVarint32(&pending_diff_zeros))
    return UNEXPECTED_ERROR;

  TArray64<uint8_t> old_file_read_buffer;
  old_file_read_buffer.AddUninitialized(1); // reading 1 byte at a time
  int64 processed_bytes = 0;

  while (!control_stream_copy_counts->Empty()) {
    uint32_t copy_count, extra_count;
    int32_t seek_adjustment;
    if (!control_stream_copy_counts->ReadVarint32(&copy_count))
      return UNEXPECTED_ERROR;
    if (!control_stream_extra_counts->ReadVarint32(&extra_count))
      return UNEXPECTED_ERROR;
    if (!control_stream_seeks->ReadVarint32Signed(&seek_adjustment))
      return UNEXPECTED_ERROR;

    // Byte-wise arithmetically add bytes from old file to bytes from the diff
    // block.
    if (copy_count > static_cast<size_t>(old_end - old_position))
      return UNEXPECTED_ERROR;

    // Add together bytes from the 'old' file and the 'diff' stream.
    for (size_t i = 0; i < copy_count; ++i) {
      uint8_t diff_byte = 0;
      if (pending_diff_zeros) {
        --pending_diff_zeros;
      } else {
        if (!diff_skips->ReadVarint32(&pending_diff_zeros))
          return UNEXPECTED_ERROR;
        if (!diff_bytes->Read(&diff_byte, 1))
          return UNEXPECTED_ERROR;
      }

      TArrayView64<uint8> old_file_read_view(old_file_read_buffer.GetData(),
                                             old_file_read_buffer.Num());

      if (!old_file->Read(old_position + i, 1, old_file_read_view)) {
        return UNEXPECTED_ERROR;
      }

      uint8_t byte = old_file_read_buffer[0] + diff_byte;

      if (!output_file->Write(&byte, 1))
        return MEM_ERROR;
    }

    old_position += copy_count;

    // Copy bytes from the extra block.
    if (extra_count > static_cast<size_t>(extra_end - extra_position))
      return UNEXPECTED_ERROR;

    if (!output_file->Write(extra_position, extra_count))
      return MEM_ERROR;

    extra_position += extra_count;

    processed_bytes += copy_count;
    processed_bytes += extra_count;
    HandlePatchingProgress(processed_bytes,  progress_delegate);

    // "seek" forwards (or backwards) in oldfile.
    if (old_position + seek_adjustment < old_start ||
        old_position + seek_adjustment > old_end)
      return UNEXPECTED_ERROR;

    old_position += seek_adjustment;
  }

  if (!control_stream_copy_counts->Empty() ||
      !control_stream_extra_counts->Empty() || !control_stream_seeks->Empty() ||
      !diff_skips->Empty() || !diff_bytes->Empty() || !extra_bytes->Empty())
    return UNEXPECTED_ERROR;

  output_file->Flush();
  return OK;
}

// -----------------------------------------------------------------------------
void BinaryDiffServiceBSDiffImpl::HandlePatchingProgress(
  int32 transferred_bytes,
  FProgressDelegate on_progress) {

  AsyncTask(ENamedThreads::GameThread, [on_progress, transferred_bytes]() {
    FFileTransferProgress overall_patch_progress;
    overall_patch_progress.transferredBytes = transferred_bytes;
    on_progress.ExecuteIfBound(overall_patch_progress);
  });
}