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
#pragma once
#include <CoreMinimal.h>
#include "fingerprints_service.h"
#include <services/hashing/hashing_service.h>

namespace cfcore {
class IDirectoriesService;
class IFileSystemService;
class IHashingService;
class ISettingsService;

// -----------------------------------------------------------------------------
class FingerprintsServiceImpl : public IFingerprintsService {
public:
  FingerprintsServiceImpl(TSharedRef<IDirectoriesService> dirs_service,
                          TSharedRef<IFileSystemService> file_system_service,
                          TSharedRef<IHashingService> hashing_service,
                          TSharedRef<ISettingsService> settings_service);

// IFingerprintsService
public:
  virtual TFuture<FCFCoreError> CalcFingerprints(
    FInstalledMod installed_mod,
    const TArray<FFileModule>& server_modules_detail,
    OUT TSharedRef<ModuleFingerprintArray> fingerprints) override;

private:
  TFuture<FModuleFingerprint> CalcModuleFingerprint(const FFileModule& module,
                                                    const FString& path);
  void CalcFileFingerprint(const FFileModule& module,
                           TSharedRef<FString> path,
                           TSharedRef<TPromise<FModuleFingerprint>> promise);
  void CalcDirectoryFingerprint(
    const FFileModule& module,
    TSharedRef<FString> path,
    TSharedRef<TPromise<FModuleFingerprint>> promise);
  void CalcCombinedFingerprint(
    TSharedRef<TArray<FString>> files,
    const FFileModule& module,
    TSharedRef<TPromise<FModuleFingerprint>> promise);
  bool CombineFingerprintHashResults(
    const TArray<HashResult>& hashes,
    TStringBuilder<1024>& combined_fingerprints_builder);
  TSharedRef<IBuffer> CreateBuffer(
    const TStringBuilder<1024>& combined_fingerprints_builder);

  FModuleFingerprint CreateModuleFingerprint(const FFileModule& module,
                                             uint64 calculated_fingerprint);
  bool IsInstalledFileValid(const FInstalledMod& mod);
  uint64 ToNumericMurmurHash(const FString& str_hash);
  bool IsIgnoredFile(const FString& file);

private:
  TSharedRef<IDirectoriesService> dirs_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IHashingService> hashing_service_;
  TSharedRef<ISettingsService> settings_service_;
};

} // namespace cfcore