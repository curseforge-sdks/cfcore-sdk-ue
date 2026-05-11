/* MIT License

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
SOFTWARE. */
#include "fingerprints_service_impl.h"
#include <common/macros.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/hashing/hashing_service.h>
#include <services/buffer/buffer_impl.h>
#include <services/settings_service.h>

#include <Serialization/JsonTypes.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
FingerprintsServiceImpl::FingerprintsServiceImpl(
  TSharedRef<IDirectoriesService> dirs_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<IHashingService> hashing_service,
  TSharedRef<ISettingsService> settings_service) :
  dirs_service_(dirs_service),
  file_system_service_(file_system_service),
  hashing_service_(hashing_service),
  settings_service_(settings_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> FingerprintsServiceImpl::CalcFingerprints(
  FInstalledMod installed_mod,
  const TArray<FFileModule>& server_modules_detail,
  OUT TSharedRef<ModuleFingerprintArray> fingerprints) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  if (!IsInstalledFileValid(installed_mod)) {
    int64 id = installed_mod.details.id;
    CFCORE_LOG(Warning, TEXT("installed mod is invalid (%lld)"), id);
    return promise->GetFuture();
  }

  if (server_modules_detail.Num() == 0) {
    promise->SetValue(FCFCoreError());
    return promise->GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<FModuleFingerprint>>>();

  FString mods_dir = FPaths::Combine(dirs_service_->GetModsRootDirectory(),
                                     installed_mod.pathOnDisk);

  for (const FFileModule& module : server_modules_detail) {
    auto path = FPaths::Combine(mods_dir, module.name);
    futures->Add(CalcModuleFingerprint(module, path));
  }

  auto future = FutureUtils::Parallel<FModuleFingerprint>(futures);
  future.Next([this, promise, fingerprints](TArray<FModuleFingerprint> res) {
    for (const FModuleFingerprint& module_fingerprint : res) {
      fingerprints->Add(module_fingerprint);
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FModuleFingerprint> FingerprintsServiceImpl::CalcModuleFingerprint(
  const FFileModule& module,
  const FString& path) {

  SHAREABLE_PROMISE(FModuleFingerprint, promise);

  auto shared_path = MakeShared<FString>(path);
  file_system_service_->GetStat(shared_path)
    .Next([this, module, promise, shared_path](FFileStatData stat) {

    if (stat.bIsDirectory) {
      CalcDirectoryFingerprint(module, shared_path, promise);
    } else {
      CalcFileFingerprint(module, shared_path, promise);
    }
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void FingerprintsServiceImpl::CalcFileFingerprint(
  const FFileModule& module,
  TSharedRef<FString> path,
  TSharedRef<TPromise<FModuleFingerprint>> promise) {

  file_system_service_->FileExists(path)
    .Next([this, module, path, promise](bool exists) {

    if (!exists) {
      CFCORE_LOG(Error, TEXT("module path doesn't exist (%s)"), **path);
      promise->SetValue(CreateModuleFingerprint(module, 0));
      return;
    }

    auto files = MakeShared<FJsonSerializableArray>();
    files->Add(*path);
    CalcCombinedFingerprint(files, module, promise);
  });
}

// -----------------------------------------------------------------------------
void FingerprintsServiceImpl::CalcDirectoryFingerprint(
  const FFileModule& module,
  TSharedRef<FString> path,
  TSharedRef<TPromise<FModuleFingerprint>> promise) {

  auto files = MakeShared<TArray<FString>>();
  auto prefix = MakeShared<FString>("");
  file_system_service_->GetFilesInDirectoryRecursive(path, prefix, files)
    .Next([this, module, promise, files](bool success) {

    if (!success) {
      promise->SetValue(CreateModuleFingerprint(module, 0));
      return;
    }

    CalcCombinedFingerprint(files, module, promise);
  });
}

// -----------------------------------------------------------------------------
void FingerprintsServiceImpl::CalcCombinedFingerprint(
  TSharedRef<TArray<FString>> files,
  const FFileModule& module,
  TSharedRef<TPromise<FModuleFingerprint>> promise) {

  if (files->Num() == 0) {
    CFCORE_LOG(Error, TEXT("No files found for module"));
    promise->SetValue(CreateModuleFingerprint(module, 0));
    return;
  }

  files->Sort();

  auto futures = MakeShared<TArray<TFuture<HashResult>>>();
  for (const FString& file : *files) {
    if (IsIgnoredFile(file)) {
      continue;
    }

    futures->Add(hashing_service_->Murmur2(MakeShared<FString>(file)));
  }

  if (futures->Num() == 0) {
    CFCORE_LOG(Error, TEXT("No files found for module"));
    promise->SetValue(CreateModuleFingerprint(module, 0));
    return;
  }

  auto future = FutureUtils::Parallel<HashResult>(futures);
  future.Next([this, module, promise](TArray<HashResult> hashes) {
    TStringBuilder<1024> fingerprints_builder;

    if (!CombineFingerprintHashResults(hashes, fingerprints_builder)) {
      promise->SetValue(CreateModuleFingerprint(module, 0));
      return;
    }

    auto buffer = CreateBuffer(fingerprints_builder);

    uint64 fingerprint = hashing_service_->Murmur2(buffer);
    promise->SetValue(CreateModuleFingerprint(module, fingerprint));
  });
}

// -----------------------------------------------------------------------------
bool FingerprintsServiceImpl::CombineFingerprintHashResults(
  const TArray<HashResult>& hashes,
  TStringBuilder<1024>& fingerprints_builder) {

  TArray<uint64> fingerprints;

  for (int i = 0; i < hashes.Num(); ++i) {
    const FCFCoreError& err = hashes[i].Key;
    const FString& murmur_hash_str = hashes[i].Value;

    if (err.isError) {
      return false;
    }

    fingerprints.Add(ToNumericMurmurHash(murmur_hash_str));
  }

  // Sort numerically
  fingerprints.Sort();

  for (uint64 fingerprint : fingerprints) {
    fingerprints_builder.Append(FString::Printf(TEXT("%llu"), fingerprint));
  }
  return true;
}


// -----------------------------------------------------------------------------
TSharedRef<IBuffer> FingerprintsServiceImpl::CreateBuffer(
  const TStringBuilder<1024>& fingerprints_builder) {

  // Convert the string to a UTF-8 encoded TArray<uint8>
  FTCHARToUTF8 utf8_converter(fingerprints_builder.ToString());

  auto arr_combined_fingerprints = MakeShared<TArray64<uint8>>();
  arr_combined_fingerprints->SetNumUninitialized(utf8_converter.Length());
  FMemory::Memcpy(arr_combined_fingerprints->GetData(),
    utf8_converter.Get(),
    utf8_converter.Length());

  // TODO(twolf): Use ClassFactory<IBuffer>
  auto buffer = MakeShared<BufferImpl>(arr_combined_fingerprints);
  return buffer;
}

// -----------------------------------------------------------------------------
FModuleFingerprint FingerprintsServiceImpl::CreateModuleFingerprint(
  const FFileModule& module,
  uint64 calculated_fingerprint) {

  FModuleFingerprint module_fingerprint;
  module_fingerprint.success = calculated_fingerprint != 0;
  module_fingerprint.is_valid = calculated_fingerprint == module.fingerprint;
  module_fingerprint.module = module;
  module_fingerprint.fingerprint = calculated_fingerprint;

  return MoveTemp(module_fingerprint);
}

// -----------------------------------------------------------------------------
bool FingerprintsServiceImpl::IsInstalledFileValid(const FInstalledMod& mod) {
  if ((mod.installedFile.id == 0) ||
      (mod.installedFile.modId != mod.details.id)) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
uint64 FingerprintsServiceImpl::ToNumericMurmurHash(const FString& str_hash) {
  if (str_hash.IsEmpty()) {
    return 0;
  }

  const TCHAR* start = GetData(str_hash);
  TCHAR* end;

  uint64 hash = FCString::Strtoui64(start, &end, 10);
  return hash;
}

// -----------------------------------------------------------------------------
bool FingerprintsServiceImpl::IsIgnoredFile(const FString& file) {
  FString lower_case_filename = FPaths::GetCleanFilename(file).ToLower();

  return settings_service_->GetSettings()
                            .ignoredDynamicModFiles
                            .Contains(lower_case_filename);
}
