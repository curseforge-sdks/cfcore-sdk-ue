/*MIT License

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
SOFTWARE.*/
#pragma once
#include <CoreMinimal.h>
#include <GenericPlatform/GenericPlatformFile.h>

#include "mods_validation_service.h"
#include "mods_validation_service_delegate.h"
#include <common/delegate_broadcaster.h>

#include <library/services/fingerprints_service.h>

namespace cfcore {
class IDirectoriesService;
class IFileSystemService;
class IInstalledModsInfoService;
class ICFCoreApi;
class IFeaturePhasingService;
class ISettingsService;

// -----------------------------------------------------------------------------
class ModsValidationServiceImpl : public IModsValidationService {
public:
  ModsValidationServiceImpl(
    TSharedRef<IDirectoriesService> dirs_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
    TSharedRef<IFingerprintsService> fingerprints_service,
    TSharedRef<ICFCoreApi> cfcore_api,
    TSharedRef<IFeaturePhasingService> feature_phasing_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IDelegateBroadcaster<IModsValidationServiceDelegate>> delegates);

// IModsValidationService
public:
  virtual TFuture<FCFCoreError> PerformQuickValidation(
    TSharedRef<TArray<FInstalledMod>> installed_mods) override;

  virtual TFuture<FCFCoreError> PerformValidation(
    const TArray<FInstalledMod>& installed_mods) override;

private:
  TFuture<bool> PerformModulesExistValidation(const FInstalledMod& mod);
  TFuture<bool> PerformFileSizeValidation(const FInstalledMod& mod);
  TFuture<bool> PerformModValidation(const FInstalledMod& mod,
                                     const FFile& server_file_details);

  bool ContainsInvalidFingerprint(
    int64 mod_id,
    TSharedRef<ModuleFingerprintArray> fingerprints);

  bool IsInstalledFileValid(const FInstalledMod& mod);

  void SetModInvalid(int64 mod_id,
                     const FString& reason,
                     TSharedPtr<TPromise<bool>> promise);

  void SetModValid(int64 mod_id,
                   const FString& reason,
                   TSharedPtr<TPromise<bool>> promise);

  bool IsValidFingerprint(const FFileModule& module, uint64 fingerprint);

  TFuture<bool> GetModFiles(const FInstalledMod& mod,
                            TSharedRef<TArray<FString>> mod_files);

  TFuture<TArray<FFileStatData>> GetModFileStats(
    TSharedRef<TArray<FString>> mod_files);

  bool IsIgnoredFile(const FString& filename);

  bool ShouldValidateMod(const FInstalledMod& mod);

private:
  TSharedRef<IDirectoriesService> dirs_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
  TSharedRef<IFingerprintsService> fingerprints_service_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<IFeaturePhasingService> feature_phasing_service_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IDelegateBroadcaster<IModsValidationServiceDelegate>> delegates_;
};

} // namespace cfcore