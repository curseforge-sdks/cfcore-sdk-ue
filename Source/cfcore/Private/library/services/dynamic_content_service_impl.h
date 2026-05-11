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
#include "dynamic_content_service.h"

namespace cfcore {
class ISettingsService;
class IInstalledModsInfoService;
class IInstalledModsSynchronizeWithServerService;
class ICFCorePremiumMods;

// -----------------------------------------------------------------------------
class DynamicContentServiceImpl : public IDynamicContentService {
public:
  DynamicContentServiceImpl(
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
    TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service,
    TSharedRef<ICFCorePremiumMods> cfcore_premium_mods);

// IDynamicContentService
public:
  virtual TFuture<TOptional<FInstalledMod>> HandleInstall(
    TSharedRef<FInstallModParams> install_params) override;

  virtual TFuture<FCFCoreError> CheckPremiumOwnership() override;

private:
  bool ShouldInstallModBasedOnStatus(const FInstalledMod& installed_mod);
  TSharedRef<TSet<int64>> GetModIdsForPremiumOwnershipCheck();
  bool IsModForPremiumOwnershipCheck(const FInstalledMod& installed_mod);

  TFuture<FCFCoreError> FetchModDetails(
    const TArray<int64>& mod_ids,
    TSharedRef<TArray<FCFCoreMod>> mods_details);
  void RemoveNonPremiumMods(TSharedRef<TSet<int64>> dynamic_content_ids,
                            TSharedRef<TArray<FCFCoreMod>> mods_details);
  TFuture<FCFCoreError> AssurePremiumOwnership(
    TSharedRef<TSet<int64>> dynamic_content_ids);

  // Returns true if there are Unowned premium mods
  TFuture<bool> HandleUnownedPremiumMods(
    TSharedRef<TSet<int64>> dynamic_content_ids,
    const TArray<int64>& owned_mod_ids);

private:
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
  TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service_;
  TSharedRef<ICFCorePremiumMods> cfcore_premium_mods_;
};

} // namespace cfcore