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
#include "CoreMinimal.h"
#include "installed_mods_info_service.h"

namespace cfcore {
class ISettingsService;
class IUserContextService;

// -----------------------------------------------------------------------------
class InstalledModsInfoUserServiceImpl : public IInstalledModsInfoService {
public:
  InstalledModsInfoUserServiceImpl(
    TSharedRef<IInstalledModsInfoService> decorated_installed_mods_info,
    TSharedRef<IUserContextService> user_context_service,
    TSharedRef<ISettingsService> settings_service);

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

  void SetInstalledProperties(const FInternalInstalledModInfoProperties& props,
                              FInstalledMod* stored_mod);

  virtual TFuture<FCFCoreError> SetUnmanagedInstalledMods(
    const TArray<FInstalledMod>& installed_mods) override;

  virtual TFuture<InstallModsInfoResult> Uninstall(int64 mod_id) override;
  virtual TFuture<InstallModsInfoResult> Uninstall(const FGuid& id) override;

  virtual const FFile* GetLatestFile(const FCFCoreMod& mod) override;

  virtual TFuture<FCFCoreError> UpdateModDetails(
    const TArray<FCFCoreMod>& mods) override;

private:
  TFuture<FCFCoreError> AssureLibraryMigrated();
  TFuture<FCFCoreError> AssureStaleModsRemoved();
  TFuture<FCFCoreError> MigrateLibrary();
  TFuture<FCFCoreError> RegisterUserToMods(
    const TArray<FInstalledMod>& installed_mods);

  void ApplyUserStateToInstalledMod(FInstalledMod& installed_mod);

  TFuture<FCFCoreError> LinkUserToMod(FInstalledMod& installed_mod);
  TFuture<FCFCoreError> UnlinkUserFromMod(FInstalledMod& installed_mod);

  bool IsModRegistered(const FInstalledMod& installed_mod);

  TFuture<FCFCoreError> RegisterModToUser(const FInstalledMod& installed_mod);
  TFuture<FCFCoreError> UnregisterModFromUser(
    const FInstalledMod& installed_mod);

  TFuture<FCFCoreError> RegisterUserToMod(FInstalledMod& installed_mod);
  TFuture<FCFCoreError> UnregisterUserFromMod(FInstalledMod& installed_mod);

  TFuture<FCFCoreError> SetDefaultModState(FInstalledMod& inst_mod);

  FString GetUserContextId();

  TFuture<FCFCoreError> SaveInstalledModUsers(
    const FInstalledMod& installed_mod);

  TFuture<FCFCoreError> SetUserModStates(
    const TArray<FInternalInstalledModInfoProperties>& props_array);

  TFuture<FCFCoreError> RegisterUnmanagedModsToUser(
    const TArray<FInstalledMod>& unmanaged_mods);

private:
  TSharedRef<IInstalledModsInfoService> decorated_installed_mods_info_;
  TSharedRef<IUserContextService> user_context_service_;
  TSharedRef<ISettingsService> settings_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstalledModsInfoUserServiceImpl)