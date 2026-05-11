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
#include <CoreMinimal.h>
#include <Async/Future.h>
#include <Misc/Paths.h>

#include <library/cfcore_client_server_library.h>

#include <library/models/install_mod_additional_params.h>

#include <api/models/mod.h>
#include <api/models/file.h>
#include <api/models/filters/get_mods_filter.h>

#include <common/cfcore_error.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IClientServerLibraryShared {
public:
  using QueryModsResult = TTuple<FCFCoreError, TOptional<TArray<FCFCoreMod>>>;
  using ModFilePair = TTuple<FCFCoreMod, FFile>;

  virtual ~IClientServerLibraryShared() = default;

public:
  virtual TFuture<QueryModsResult> QueryMods(
    const FCFCoreGetModsFilter& filter) = 0;

  /**
   * Returns a TArray<FInstalledMod> where each item has an up-to-date status
   * based on local validations.
   */
  virtual TFuture<TArray<FInstalledMod>> GetInstalledByIdsWithValidations(
    TSharedRef<TArray<ModFilePair>> mods_file_pairs) = 0;

  virtual TFuture<TArray<FInstalledMod>> GetInstalledByIds(
    TSharedRef<TArray<int64>> mod_ids) = 0;

  virtual bool ShouldInstallModByState(const FInstalledMod* installed_mod) = 0;

  virtual TFuture<FCFCoreError> PerformModsValidation(
    TSharedRef<TArray<FInstalledMod>> mods,
    TSharedRef<TArray<ModFilePair>> mods_file_pairs) = 0;

  /**
   * NOTE(twolf): We assume QueryMods was called prior to calling this function.
   * Otherwise, we might reject mods/files based on a old server status.
   */
  virtual void AssureModsUpdated(
    TSharedRef<TArray<ModFilePair>> mod_file_pairs,
    const FInstallModAdditionalParams& additional_params,
    TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> on_prgs,
    TSharedRef<ICFCoreClientServerLibrary::FModsUpdateDelegate> on_update) = 0;
};

class ISettingsService;
class ICFCoreLibrary;
class IInstalledModsSynchronizeWithServerService;
class IModsValidationService;

// -----------------------------------------------------------------------------
class ClientServerLibrarySharedImpl : public IClientServerLibraryShared {
public:
  using GetInstalledModsResult =
    TTuple<FCFCoreError, TOptional<TArray<FInstalledMod>>>;

  ClientServerLibrarySharedImpl(
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<ICFCoreLibrary> cfcore_library,
    TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service,
    TSharedRef<IModsValidationService> mods_validation_service);
  virtual ~ClientServerLibrarySharedImpl() = default;

// IClientServerLibraryShared
public:
  TFuture<QueryModsResult> QueryMods(
    const FCFCoreGetModsFilter& filter) override;

  TFuture<TArray<FInstalledMod>> GetInstalledByIdsWithValidations(
    TSharedRef<TArray<ModFilePair>> mods_file_pairs) override;

  TFuture<TArray<FInstalledMod>> GetInstalledByIds(
    TSharedRef<TArray<int64>> mod_ids) override;
  bool ShouldInstallModByState(const FInstalledMod* installed_mod) override;
  TFuture<FCFCoreError> PerformModsValidation(
    TSharedRef<TArray<FInstalledMod>> mods,
    TSharedRef<TArray<ModFilePair>> mods_file_pairs) override;

  void AssureModsUpdated(
    TSharedRef<TArray<ModFilePair>> mod_file_pairs,
    const FInstallModAdditionalParams& additional_params,
    TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> on_prgs,
    TSharedRef<ICFCoreClientServerLibrary::FModsUpdateDelegate> on_update)
      override;

private:
  // TFuture<bool> RunValidation();

  TFuture<TArray<ModFilePair>> GetModsToInstall(
    TSharedRef<TArray<ModFilePair>> mods_file_pairs);

  TFuture<FCFCoreError> InstallMods(
    const TArray<ModFilePair>& mod_file_pairs,
    const FInstallModAdditionalParams& additional_params,
    TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> progrs);

  TFuture<FCFCoreError> InstallMod(
    const ModFilePair& mod_file_pair,
    const FInstallModAdditionalParams& additional_params,
    TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> progrs);

  TFuture<GetInstalledModsResult> ValidateModsInstallation(
    TSharedRef<TArray<ModFilePair>> mod_file_pairs);

  bool ShouldInstallModByFile(const FFile& file,
                              const FInstalledMod* installed_mod);
private:
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<ICFCoreLibrary> cfcore_library_;
  TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service_;
  TSharedRef<IModsValidationService> mods_validation_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IClientServerLibraryShared)
Expose_TNameOf(cfcore::ClientServerLibrarySharedImpl)