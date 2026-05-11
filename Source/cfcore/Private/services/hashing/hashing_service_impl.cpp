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
#include "hashing_service_impl.h"
#include "zlib.h" // crc32
#include <Async/TaskGraphInterfaces.h>
#include <Async/Async.h>

#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/filesystem/file_system_service.h>
#include <services/thread_pools_service.h>

namespace cfcore {
const int64 kMaxHashingReadChunkInBytes = 100 * 1024 * 1024;
const uint32_t kMurmur2Multiplex = 1540483477;

};

using namespace cfcore;

// -----------------------------------------------------------------------------
HashingServiceImpl::HashingServiceImpl(
  TSharedRef<IThreadPoolsService> thread_pools_service,
  TSharedRef<IFileSystemService> file_system_service) :
  io_thread_pool_(thread_pools_service->IO()),
  cpu_intensive_thread_pool_(thread_pools_service->Compression()),
  file_system_service_(file_system_service) {
}

// -----------------------------------------------------------------------------
FString HashingServiceImpl::Md5(TSharedPtr<IBuffer> buffer) {
  FString md5 = FMD5::HashBytes(buffer->GetData(), buffer->GetLength());
  return md5;
}

// -----------------------------------------------------------------------------
FString HashingServiceImpl::Sha1(TSharedPtr<IBuffer> buffer) {
  TArray<uint8> sha1_buffer;
  sha1_buffer.Reserve(FSHA1::DigestSize);
  FSHA1::HashBuffer((const void*)buffer->GetData(),
                    buffer->GetLength(),
                    sha1_buffer.GetData());

  FString sha1 = BytesToHex(sha1_buffer.GetData(), FSHA1::DigestSize);
  return sha1;
}

// -----------------------------------------------------------------------------
uint64 HashingServiceImpl::Murmur2(TSharedPtr<IBuffer> buffer) {
  const uint64 length = buffer->GetLength();
  const uint8* data = buffer->GetData();

  uint64 num1 = Murmur2ComputeNormalizedLength(buffer);

  uint32_t num2 = (uint64)1 ^ num1;
  uint32_t num3 = 0;
  int32 num4 = 0;

  for (uint64 index = 0; index < length; ++index) {
    unsigned char b = data[index];

    if (Murmur2IsWhitespaceChar(b)) {
      continue;
    }

    num3 |= (uint64)b << num4;
    num4 += 8;

    if (num4 != 32) {
      continue;
    }

    uint64 num6 = num3 * kMurmur2Multiplex;
    uint64 num7 = (num6 ^ num6 >> 24) * kMurmur2Multiplex;

    num2 = num2 * kMurmur2Multiplex ^ num7;
    num3 = 0;
    num4 = 0;
  }

  if (num4 > 0) {
    num2 = (num2 ^ num3) * kMurmur2Multiplex;
  }

  uint64 num6 = (num2 ^ num2 >> 13) * kMurmur2Multiplex;
  return num6 ^ num6 >> 15;
}

// -----------------------------------------------------------------------------
TFuture<HashResult> HashingServiceImpl::Md5(TSharedRef<FString> filename) {
  SHAREABLE_PROMISE(HashResult, promise);

  TSharedRef<FMD5> md5_gen = MakeShared<FMD5>();
  TSharedRef<TArray<uint8>> reusable_buffer = MakeShared<TArray<uint8>>();
  reusable_buffer->AddUninitialized(kMaxHashingReadChunkInBytes);

  StreamFileForHash(
    filename,
    reusable_buffer,
    [this, filename, md5_gen, promise, reusable_buffer](
      int64 bytes_read,
      bool done,
      EFileSystemError last_err) {

    if (last_err != EFileSystemError::None) {
      HandleFileSystemError(filename, last_err, promise);
      return MakeFulfilledPromise<FCFCoreError>(
        FCFCoreError(ECFCoreErrorCodes::FileSystemError)).GetFuture();
    }

    if (done) {
      uint8 digest[16];
      md5_gen->Final(digest);

      FString md5 = BytesToHex(digest, 16).ToLower();
      promise->SetValue(HashResult{ FCFCoreError(), md5 });
      return MakeFulfilledPromise<FCFCoreError>().GetFuture();
    }

    SHAREABLE_PROMISE(FCFCoreError, inner_promise);

    AsyncPool(*cpu_intensive_thread_pool_,
      [this, inner_promise, reusable_buffer, bytes_read, md5_gen]() {

      md5_gen->Update(reusable_buffer->GetData(), bytes_read);

      FCFCoreError no_error = FCFCoreError();
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(inner_promise, no_error);
    });

    return inner_promise->GetFuture();
  });

  return promise->GetFuture();
}


// -----------------------------------------------------------------------------
TFuture<HashResult> HashingServiceImpl::Sha1(TSharedRef<FString> filename) {
  SHAREABLE_PROMISE(HashResult, promise);

  TSharedRef<FSHA1> sha1_gen = MakeShared<FSHA1>();
  TSharedRef<TArray<uint8>> reusable_buffer = MakeShared<TArray<uint8>>();
  reusable_buffer->AddUninitialized(kMaxHashingReadChunkInBytes);

  StreamFileForHash(
    filename,
    reusable_buffer,
    [this, filename, sha1_gen, promise, reusable_buffer](
      int64 bytes_read,
      bool done,
      EFileSystemError last_err) {

    if (last_err != EFileSystemError::None) {
      HandleFileSystemError(filename, last_err, promise);
      return MakeFulfilledPromise<FCFCoreError>(
        FCFCoreError(ECFCoreErrorCodes::FileSystemError)).GetFuture();
    }

    if (done) {
      TArray<uint8> sha1_hash;
      sha1_hash.Reserve(FSHA1::DigestSize);
      sha1_gen->Final();
      sha1_gen->GetHash(sha1_hash.GetData());

      FString sha1 =
        BytesToHex(sha1_hash.GetData(), FSHA1::DigestSize).ToLower();
      promise->SetValue(HashResult{ FCFCoreError(), sha1 });
      return MakeFulfilledPromise<FCFCoreError>().GetFuture();
    }

    SHAREABLE_PROMISE(FCFCoreError, inner_promise);

    AsyncPool(*cpu_intensive_thread_pool_,
      [this, inner_promise, reusable_buffer, bytes_read, sha1_gen]() {

      sha1_gen->Update(reusable_buffer->GetData(), bytes_read);

      FCFCoreError no_error = FCFCoreError();
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(inner_promise, no_error);
    });

    return inner_promise->GetFuture();
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<HashResult> HashingServiceImpl::Crc32(TSharedRef<FString> filename) {

  TSharedRef<uint32_t> crc = MakeShared<uint32_t>();
  TSharedRef<TArray<uint8>> reusable_buffer = MakeShared<TArray<uint8>>();
  reusable_buffer->AddUninitialized(kMaxHashingReadChunkInBytes);

  SHAREABLE_PROMISE(HashResult, promise);

  StreamFileForHash(
    filename,
    reusable_buffer,
    [this, filename, crc, promise, reusable_buffer](int64 bytes_read,
                                                    bool done,
                                                    EFileSystemError last_err) {

    if (last_err != EFileSystemError::None) {
      HandleFileSystemError(filename, last_err, promise);
      return MakeFulfilledPromise<FCFCoreError>(
        FCFCoreError(ECFCoreErrorCodes::FileSystemError)).GetFuture();
    }

    if (done) {
      FString str_crc = FString::Format(TEXT("{0}"), { *crc });
      promise->SetValue(HashResult{ FCFCoreError(), str_crc });
      return MakeFulfilledPromise<FCFCoreError>().GetFuture();
    }

    SHAREABLE_PROMISE(FCFCoreError, inner_promise);

    AsyncPool(*cpu_intensive_thread_pool_,
      [this, inner_promise, reusable_buffer, bytes_read, crc]() {

      *crc = crc32(*crc, reusable_buffer->GetData(), bytes_read);

      FCFCoreError no_error = FCFCoreError();
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(inner_promise, no_error);
    });

    return inner_promise->GetFuture();
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<HashResult> HashingServiceImpl::Murmur2(
  TSharedRef<FString> filename) {

  SHAREABLE_PROMISE(HashResult, promise);

  TSharedRef<TArray<uint8>> reusable_buffer = MakeShared<TArray<uint8>>();
  reusable_buffer->AddUninitialized(kMaxHashingReadChunkInBytes);

  Murmur2ComputeNormalizedLength(filename, reusable_buffer).Next(
    [this, filename, promise, reusable_buffer](HashResult len_res) {

    if (len_res.Key.isError) {
      promise->SetValue(len_res);
      return;
    }

    uint64 normalized_len = FCString::Strtoui64(*len_res.Value, nullptr, 10);
    Murmur2ComputeHash(filename, normalized_len, reusable_buffer).Next(
      [promise, filename](HashResult hash_res) {

      promise->SetValue(hash_res);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
uint64 HashingServiceImpl::Murmur2ComputeNormalizedLength(
  TSharedPtr<IBuffer> buffer) {

  uint64 normalized_len = 0;
  const int64 length = buffer->GetLength();
  const uint8* data = buffer->GetData();

  for (int64 index = 0; index < length; ++index) {
    if (!Murmur2IsWhitespaceChar(data[index])) {
      ++normalized_len;
    }
  }

  return normalized_len;

  return 0;
}

// -----------------------------------------------------------------------------
TFuture<HashResult> HashingServiceImpl::Murmur2ComputeNormalizedLength(
  TSharedRef<FString> filename,
  TSharedRef<TArray<uint8>> reusable_buffer) {

  SHAREABLE_PROMISE(HashResult, promise);

  TSharedRef<uint64> normalized_len = MakeShared<uint64>();

  StreamFileForHash(
    filename,
    reusable_buffer,
    [this, filename, normalized_len, promise, reusable_buffer](
      int64 bytes_read,
      bool done,
      EFileSystemError last_err) {

    if (last_err != EFileSystemError::None) {
      HandleFileSystemError(filename, last_err, promise);
      return MakeFulfilledPromise<FCFCoreError>(
        FCFCoreError(ECFCoreErrorCodes::FileSystemError)).GetFuture();
    }

    if (done) {
      FString str_len = FString::Format(TEXT("{0}"), { *normalized_len });
      promise->SetValue(HashResult{ FCFCoreError(), str_len });
      return MakeFulfilledPromise<FCFCoreError>().GetFuture();
    }

    SHAREABLE_PROMISE(FCFCoreError, inner_promise);

    AsyncPool(*cpu_intensive_thread_pool_,
      [this, inner_promise, reusable_buffer, bytes_read, normalized_len]() {

      const uint8* data = reusable_buffer->GetData();

      for (int64 index = 0; index < bytes_read; ++index) {
        if (!Murmur2IsWhitespaceChar(data[index])) {
          (*normalized_len)++;
        }
      }

      FCFCoreError no_error = FCFCoreError();
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(inner_promise, no_error);
    });

    return inner_promise->GetFuture();
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<HashResult> HashingServiceImpl::Murmur2ComputeHash(
  TSharedRef<FString> filename,
  uint64 normalized_file_len,
  TSharedRef<TArray<uint8>> reusable_buffer) {

  SHAREABLE_PROMISE(HashResult, promise);

  auto num2 = MakeShared<uint32_t>((uint64)1 ^ normalized_file_len);
  auto num3 = MakeShared<uint32_t>(0);
  auto num4 = MakeShared<int32>(0);

  StreamFileForHash(
    filename,
    reusable_buffer,
    [this, filename, promise, reusable_buffer, num2, num3, num4](
      int64 bytes_read,
      bool done,
      EFileSystemError last_err) {

    if (last_err != EFileSystemError::None) {
      HandleFileSystemError(filename, last_err, promise);
      return MakeFulfilledPromise<FCFCoreError>(
        FCFCoreError(ECFCoreErrorCodes::FileSystemError)).GetFuture();
    }

    if (done) {
      if (*num4 > 0) {
        *num2 = (*num2 ^ *num3) * kMurmur2Multiplex;
      }

      uint64 num6 = (*num2 ^ *num2 >> 13) * kMurmur2Multiplex;
      uint64 fingerprint = num6 ^ num6 >> 15;

      FString str_len = FString::Format(TEXT("{0}"), { fingerprint });
      promise->SetValue(HashResult{ FCFCoreError(), str_len });
      return MakeFulfilledPromise<FCFCoreError>().GetFuture();
    }

    SHAREABLE_PROMISE(FCFCoreError, inner_promise);

    AsyncPool(*cpu_intensive_thread_pool_,
      [this, inner_promise, reusable_buffer, bytes_read, num2, num3, num4]() {

      const uint8* data = reusable_buffer->GetData();
      for (int64 index = 0; index < bytes_read; ++index) {
        unsigned char b = data[index];

        if (Murmur2IsWhitespaceChar(b)) {
          continue;
        }

        *num3 |= (uint64)b << *num4;
        *num4 += 8;

        if (*num4 != 32) {
          continue;
        }

        uint64 num6 = *num3 * kMurmur2Multiplex;
        uint64 num7 = (num6 ^ num6 >> 24) * kMurmur2Multiplex;

        *num2 = *num2 * kMurmur2Multiplex ^ num7;
        *num3 = 0;
        *num4 = 0;
      }

      FCFCoreError no_error = FCFCoreError();
      SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(inner_promise, no_error);
    });

    return inner_promise->GetFuture();
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool HashingServiceImpl::Murmur2IsWhitespaceChar(char chr) {
  return chr == 9 || chr == 10 || chr == 13 || chr == 32;
}

// -----------------------------------------------------------------------------
void HashingServiceImpl::StreamFileForHash(
  TSharedRef<FString> filename,
  TSharedRef<TArray<uint8>> reusable_buffer,
  //(int64 bytes_read, bool done, EFileSystemError last_error)
  TFunction<TFuture<FCFCoreError>(int64, bool, EFileSystemError)> callback) {

  file_system_service_->GetStat(filename).Next(
    [this, filename, reusable_buffer, callback](FFileStatData file_stats) {

    if (!file_stats.bIsValid) {
      callback(0, true, EFileSystemError::FailedToReadFromDisk)
        .Next([](FCFCoreError) {});
      return;
    }

    InternalStreamFileForHash(filename,
                              0,
                              file_stats.FileSize,
                              reusable_buffer,
                              callback);
  });
}

// -----------------------------------------------------------------------------
void HashingServiceImpl::InternalStreamFileForHash(
  TSharedRef<FString> filename,
  int64 bytes_read,
  int64 total_bytes,
  TSharedRef<TArray<uint8>> reusable_buffer,
  TFunction<TFuture<FCFCoreError>(int64, bool, EFileSystemError)> callback) {

  // Check if we are done
  if (bytes_read == total_bytes) {
    callback(0, true, EFileSystemError::None).Next([](FCFCoreError) {});
    return;
  }

  // Read buffer chunk
  int bytes_to_read = FMath::Min(kMaxHashingReadChunkInBytes,
                                 total_bytes - bytes_read);

  file_system_service_->ReadBuffer(filename,
                                   bytes_read,
                                   bytes_to_read,
                                   reusable_buffer)
    .Next([this,
           callback,
           bytes_to_read,
           filename,
           bytes_read,
           total_bytes,
           reusable_buffer](EFileSystemError filesys_err) {

    if (filesys_err != EFileSystemError::None) {
      callback(0, true, filesys_err);
      return;
    }

    callback(bytes_to_read, false, EFileSystemError::None)
      .Next([this,
             filename,
             bytes_read,
             bytes_to_read,
             total_bytes,
             reusable_buffer,
             callback](FCFCoreError err) {

      if (err.isError) {
        return;
      }

      InternalStreamFileForHash(filename,
                                bytes_read + bytes_to_read,
                                total_bytes,
                                reusable_buffer,
                                callback);
    });
  });
}


// -----------------------------------------------------------------------------
void HashingServiceImpl::HandleFileSystemError(
  TSharedRef<FString> filename,
  EFileSystemError filesys_err,
  TSharedRef<TPromise<HashResult>> promise) {

  ENUM_TO_STR(str_filesys_error, filesys_err);

  FString msg = FString::Format(TEXT("Failed hash check: {0} (desc={1})"), {
    *filename,
    *str_filesys_error
  });

  CFCORE_LOG(Error, TEXT("%s"), *msg);

  FCFCoreError err(ECFCoreErrorCodes::FileSystemError);
  err.description = msg;
  promise->SetValue(HashResult{ err, "" });
}
