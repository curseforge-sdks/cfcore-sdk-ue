/* MIT License

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
SOFTWARE. */
#include "installed_mods_synchronize_with_server_service_impl.h"
#include <common/constants.h>
#include <common/macros.h>

#include <services/logger/logger_macros.h>

#include <services/settings_service.h>
#include <library/services/installed_mods_info/installed_mods_info_service.h>
#include <api/cfcore_api.h>
#include <library/services/mods_installation_queue_service.h>

namespace cfcore {
}

using namespace cfcore;

// -----------------------------------------------------------------------------
InstalledModsSynchronizeWithServerServiceImpl::
InstalledModsSynchronizeWithServerServiceImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<IModsInstallationQueueService> installation_queue_service) :
  settings_service_(settings_service),
  installed_mods_info_service_(installed_mods_info_service),
  cfcore_api_(cfcore_api),
  installation_queue_service_(installation_queue_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsSynchronizeWithServerServiceImpl::Sync() {
  FCFCoreGetModsFilter filter;

  TArray<FInstalledMod> installed_mods;
  installed_mods_info_service_->GetInstalledMods(installed_mods);

  for (const FInstalledMod& installed_mod : installed_mods) {
    if (!IsModValid(installed_mod.details)) {
      continue;
    }

    filter.modIds.Add(installed_mod.details.id);
  }

  if (filter.modIds.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto mod_details = MakeShared<TArray<FCFCoreMod>>();
  return GetModDetailsFromServer(filter, mod_details);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsSynchronizeWithServerServiceImpl::GetModDetailsFromServer(
  const FCFCoreGetModsFilter& filter,
  TSharedRef<TArray<FCFCoreMod>> out_mod_details) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_api_->GetMods(
    filter,
    ICFCoreApi::FGetModsDelegate::CreateLambda(
      [this, promise, out_mod_details](
        TOptional<TArray<FCFCoreMod>> mods,
        const TOptional<FCFCoreApiResponseError>& err) {

        if (err.IsSet()) {
          const FString& desc = err->description;
          CFCORE_LOG(Error, TEXT("Error querying server mods: %s"), *desc);
          promise->SetValue(FCFCoreError(err));
          return;
        }

        FillModsNotBeingInstalled(*mods, out_mod_details);

        installed_mods_info_service_->UpdateModDetails(*mods)
          .Next([promise, out_mod_details](FCFCoreError err) {

          promise->SetValue(err);
        });
      })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstalledModsSynchronizeWithServerServiceImpl::IsModValid(
  const FCFCoreMod& mod) {

  if (mod.id == 0) {
    return false;
  }

  if (mod.gameId != settings_service_->GetSettings().gameId) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
void InstalledModsSynchronizeWithServerServiceImpl::FillModsNotBeingInstalled(
  const TArray<FCFCoreMod>& mods,
  const TSharedRef<TArray<FCFCoreMod>>& out_mod_details) {

  for (const FCFCoreMod& mod : mods) {
    // Skip mods that are currently in the installation queue - we don't want to
    // overwrite the latestUpdatedFile or status of the mod while it is being
    // updating/installed
    FInstallModRequestPtr request;
    if (installation_queue_service_->PeekByModId(mod.id, request)) {
      continue;
    }

    out_mod_details->Add(mod);
  }
}

