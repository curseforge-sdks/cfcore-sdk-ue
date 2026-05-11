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
#include "installation_command_verify_hash_impl.h"

#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/hashing/hashing_service.h>
#include "installation_command_utils_service.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandVerifyHashImpl::InstallationCommandVerifyHashImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<IHashingService> hashing_service) :
  utils_service_(utils_service),
  hashing_service_(hashing_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandVerifyHashImpl::Run(
  FInstallModRequestPtr request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  int64 mod_id = request->modId;
  CFCORE_LOG(Log, TEXT("Verifying mod (%lld) download ..."), mod_id);

  TArray<FFileHash> hashes;
  if (request->state.file_info.IsValid()) {
    hashes = request->state.file_info->hashes;
  } else if (request->state.file_delta_diff_info.IsValid())  {
    hashes = request->state.file_delta_diff_info->hashes;
  }

  if (hashes.Num() == 0) {
    CFCORE_LOG(Warning, TEXT("Mod (%lld) has no hashes"), mod_id);
    promise->SetValue({});
    return promise->GetFuture();
  }

  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::Validating);

  auto local_file = MakeShared<FString>(request->state.downloaded_mod_file);

  FString expected_hash = hashes[0].value;
  ECFCoreHashAlgo hash_algo = hashes[0].algo;

  auto result_handler = GetHashResultHandler(mod_id, expected_hash, promise);

  if (hash_algo == ECFCoreHashAlgo::Md5) {
    hashing_service_->Md5(local_file).Next(result_handler);
  } else if (hash_algo == ECFCoreHashAlgo::Sha1) {
    hashing_service_->Sha1(local_file).Next(result_handler);
  } else {
    CFCORE_LOG(Error, TEXT("Mod (%lld) has unknown hash algo"), mod_id);
    auto code = ECFCoreErrorCodes::DownloadedFileHasInvalidHash;
    promise->SetValue(FCFCoreError(code));
  }

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandVerifyHashImpl::Cancel(FInstallModRequestPtr request) {
  return true;
}

// -----------------------------------------------------------------------------
TFunction<void(HashResult)>
InstallationCommandVerifyHashImpl::GetHashResultHandler(
  int64 mod_id,
  const FString& expected_hash_value,
  TSharedPtr<TPromise<FCFCoreError>> promise) {

  return [mod_id, expected_hash_value, promise](HashResult result) {
    FCFCoreError err(ECFCoreErrorCodes::DownloadedFileHasInvalidHash);

    if (result.Key.isError) {
      err.description = result.Key.description;
      CFCORE_LOG(Error, TEXT("Mod (%lld):%s"), mod_id, *err.description);
      promise->SetValue(err);
      return;
    }

    if (!expected_hash_value.Equals(result.Value, ESearchCase::IgnoreCase)) {
      err.description = FString::Format(TEXT("Expected hash={0} but got {1}"), {
        *expected_hash_value,
        *result.Value
      });

      CFCORE_LOG(Error, TEXT("Mod (%lld):%s"), mod_id, *err.description);

      promise->SetValue(err);
      return;
    }

    promise->SetValue(FCFCoreError());
  };
}