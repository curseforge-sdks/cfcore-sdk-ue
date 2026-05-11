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
#include "CoreMinimal.h"
#include "installed_mods_info_service.h"
#include <api/cfcore_api.h>
#include <services/settings_service.h>
#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>
#include "models/installed_mod_array.h"

#include <common/delegate_broadcaster.h>
#include "installed_mods_info_service_delegate.h"

namespace cfcore {

using TModsInfoBroadcaster =
  IDelegateBroadcaster<IInstalledModsInfoServiceDelegate>;

// -----------------------------------------------------------------------------
class InstalledModsInfoSystemServiceImpl : public IInstalledModsInfoService {
public:
  InstalledModsInfoSystemServiceImpl(
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IDirectoriesService> dirs_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<ICFCoreApi> cfcore_api,
    TSharedRef<TModsInfoBroadcaster> delegates);

// IInstalledModsService
public:
  virtual TFuture<FCFCoreError> Initialize() override;

  virtual TOptional<FInstalledMod> GetInstalledMod(int64 mod_id) override;
  virtual TOptional<FInstalledMod> GetInstalledMod(const FGuid& id) override;

  virtual void GetInstalledMods(
    TArray<FInstalledMod>& out_installed_mods) override;

  virtual TFuture<InstallModsInfoResult> SetInstalling(
    TSharedRef<FInstallModParams> install_params) override;

  virtual TFuture<InstallModsInfoResult> SetInstalled(
    const FInstalledMod& installed_mod,
    const FInstallModAdditionalParams& additional_params) override;
  virtual TFuture<InstallModsInfoResult> SetInstallFailed(
    const FInstalledMod& installed_mod) override;

  virtual TFuture<FCFCoreError> SetInstalledProperties(
    const TArray<FInternalInstalledModInfoProperties>& properties) override;

  virtual TFuture<FCFCoreError> SetUnmanagedInstalledMods(
    const TArray<FInstalledMod>& installed_mods) override;

  virtual TFuture<InstallModsInfoResult> Uninstall(int64 mod_id) override;
  virtual TFuture<InstallModsInfoResult> Uninstall(const FGuid& id) override;

  virtual const FFile* GetLatestFile(const FCFCoreMod& mod) override;

  virtual TFuture<FCFCoreError> UpdateModDetails(
    const TArray<FCFCoreMod>& mod_details) override;

private:
  void SetLocalLibraryFile();
  TFuture<FCFCoreError> LoadInstalledModsOnDisk();
  TFuture<FCFCoreError> SaveInstalledModsToDisk();

  void UpdateInstalledMod(FInstalledMod& installed, const FCFCoreMod& updated_mod);
  const FFile* GetMainFile(const FCFCoreMod& mod);
  bool IsModValid(const FCFCoreMod& mod);
  FInstalledMod* GetInstalledModRefById(const FGuid& id);
  FInstalledMod* GetInstalledModRefByModId(int64 mod_id);
  bool RemoveInstalledModByModId(int64 id);
  bool RemoveInstalledModById(const FGuid& id);
  bool WasPreviouslyInstalled(const FInstalledMod& installed_mod);

  void SaveChangesAndTrigger(
    const FInstalledMod& installed_mod,
    TSharedPtr<TPromise<InstallModsInfoResult>> promise);

  TFuture<InstallModsInfoResult> CreateError(
    ECFCoreErrorCodes code,
    const FInstalledMod& installed_mod);

  void SetAdditionalParams(
    FInstalledMod& installed_mod,
    const FInstallModAdditionalParams& additional_params);

  void FillInstalledModProperties(
    FInstalledMod* stored_mod,
    const FInternalInstalledModInfoProperties& props);

private:
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IDirectoriesService> directories_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<TModsInfoBroadcaster> delegates_;

  FInstalledModArray installed_mods_;
  FString local_library_file_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstalledModsInfoSystemServiceImpl)