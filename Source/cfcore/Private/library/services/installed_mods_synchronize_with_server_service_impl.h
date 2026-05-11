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
#include "installed_mods_synchronize_with_server_service.h"

namespace cfcore {
class ISettingsService;
class IInstalledModsInfoService;
class ICFCoreApi;
class IModsInstallationQueueService;

// -----------------------------------------------------------------------------
class InstalledModsSynchronizeWithServerServiceImpl :
  public IInstalledModsSynchronizeWithServerService {
public:
  InstalledModsSynchronizeWithServerServiceImpl(
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
    TSharedRef<ICFCoreApi> cfcore_api,
    TSharedRef<IModsInstallationQueueService> installation_queue_service);

// IInstalledModsSynchronizeWithServerService
public:
  virtual TFuture<FCFCoreError> Sync() override;

  virtual TFuture<FCFCoreError> GetModDetailsFromServer(
    const FCFCoreGetModsFilter& filter,
    TSharedRef<TArray<FCFCoreMod>> out_mod_details) override;


private:
  bool IsModValid(const FCFCoreMod& mod);

  void FillModsNotBeingInstalled(
    const TArray<FCFCoreMod>& mods,
    const TSharedRef<TArray<FCFCoreMod>>& out_mod_details);

private:
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<IModsInstallationQueueService> installation_queue_service_;
};

} // namespace cfcore