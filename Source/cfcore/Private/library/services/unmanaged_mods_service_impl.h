/*MIT License

Copyright (c) 2025 Overwolf Ltd.

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
#include <Containers/Queue.h>

#include "unmanaged_mods_service.h"
#include <library/services/installation_commands/models/scanned_local_mod_item.h>
#include <library/services/mods_validation/mods_validation_service_delegate.h>

namespace cfcore {
class IInstalledModsInfoService;
class ISettingsService;
class IDirectoriesService;
class IInstallationCommandUtilsService;

// -----------------------------------------------------------------------------
class UnmanagedModsServiceImpl :
  public IUnmanagedModsService,
  public IModsValidationServiceDelegate,
  public TSharedFromThis<UnmanagedModsServiceImpl> {

public:
  UnmanagedModsServiceImpl(
    TSharedRef<IInstalledModsInfoService> installed_mods_info_system_service,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_user_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IDirectoriesService> dirs_service,
    TSharedRef<IInstallationCommandUtilsService> command_utils_service);

// IUnmanagedModsService
public:
  virtual TFuture<FCFCoreError> Initialize() override;

// IModsValidationServiceDelegate
public:
  virtual TFuture<bool> OnPerformQuickValidation(
    const FInstalledMod& installed_mod,
    bool& out_skip_other_validations) override;

private:
  TFuture<FCFCoreError> ScanUnmanagedMods();
  TFuture<FCFCoreError> RetrieveLocalRootModDirectories(
    TSharedRef<TArray<FScannedLocalModItem>> out_local_mod_items);
  TFuture<FCFCoreError> FilterExistingMods(
    const TSharedRef<TArray<FScannedLocalModItem>> local_mod_items,
    TSharedRef<TMap<FString, FInstalledMod>> out_invalid_unmanaged_mods,
    TSharedRef<TArray<FInstalledMod>> out_removed_unmanaged_mods);
  bool IsUnamangedModInvalid(const FInstalledMod& installed_mod);
  TFuture<FCFCoreError> UninstallRemovedUnmanagedMods(
    const TSharedRef<TArray<FInstalledMod>> missing_unmanaged_mods);
  TFuture<FCFCoreError> AddNewUnamanagedMods(
    const TSharedRef<TArray<FScannedLocalModItem>> local_mod_items,
    TSharedRef<TMap<FString, FInstalledMod>> out_invalid_unmanaged_mods);

  void OverrideExistingMod(
    TSharedRef<TMap<FString, FInstalledMod>>& invalid_unmanaged_mods,
    const FScannedLocalModItem& local_mod_item,
    FInstalledMod& installed_mod);

  FInstalledMod CreateUnmanagedInstalledMod(
    const FScannedLocalModItem& local_mod_item);

  bool IsUnamangedModsEnabled();
  bool IsHidden(const FScannedLocalModItem& item) const;
  bool ShouldExcludeFromScan(const FScannedLocalModItem& item) const;

private:
  TSharedRef<IInstalledModsInfoService> installed_mods_info_system_service_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_user_service_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IDirectoriesService> directories_service_;
  TSharedRef<IInstallationCommandUtilsService> command_utils_service_;

  FString local_library_file_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::UnmanagedModsServiceImpl)