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
#include "dynamic_content_service_impl.h"
#include <common/constants.h>
#include <common/macros.h>
#include <common/container_utils.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include "installed_mods_info/installed_mods_info_service.h"
#include <services/settings_service.h>
#include <premium/cfcore_premium_mods.h>
#include <library/services/installed_mods_synchronize_with_server_service.h>

namespace cfcore {
using InstallModsInfoResult = IInstalledModsInfoService::InstallModsInfoResult;

const EInstalledModStatus kShouldInstallStatuses[] = {
  EInstalledModStatus::Pending,
  EInstalledModStatus::OutOfDate,
  EInstalledModStatus::Invalid,
  EInstalledModStatus::Uninstalled
};
}

using namespace cfcore;

// -----------------------------------------------------------------------------
DynamicContentServiceImpl::DynamicContentServiceImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
  TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service,
  TSharedRef<ICFCorePremiumMods> cfcore_premium_mods) :
  settings_service_(settings_service),
  installed_mods_info_service_(installed_mods_info_service),
  mods_sync_service_(mods_sync_service),
  cfcore_premium_mods_(cfcore_premium_mods) {
}

// -----------------------------------------------------------------------------
TFuture<TOptional<FInstalledMod>> DynamicContentServiceImpl::HandleInstall(
  TSharedRef<FInstallModParams> install_params) {

  if (install_params->additional_params.dynamicContent) {
    install_params->additional_params.tracking.FindOrAdd(
      kTrackingDynamicContent,
      kTrue);
    install_params->additional_params.origin =
      EModInstallOrigin::DynamicDownload;
  }

  TOptional<FInstalledMod> installed_mod =
    installed_mods_info_service_->GetInstalledMod(install_params->mod.id);
  if (!installed_mod.IsSet()) {
    return MakeFulfilledPromise<TOptional<FInstalledMod>>().GetFuture();
  }

  if (install_params->file.id != installed_mod->installedFile.id) {
    return MakeFulfilledPromise<TOptional<FInstalledMod>>().GetFuture();
  }

  if (ShouldInstallModBasedOnStatus(*installed_mod)) {
    return MakeFulfilledPromise<TOptional<FInstalledMod>>().GetFuture();
  }

  bool is_dynamic_content = install_params->additional_params.dynamicContent;
  if (is_dynamic_content == installed_mod->dynamicContent) {
    return MakeFulfilledPromise<TOptional<FInstalledMod>>().GetFuture();
  }

  SHAREABLE_PROMISE(TOptional<FInstalledMod>, promise);

  const FInstallModAdditionalParams& params = install_params->additional_params;
  installed_mods_info_service_->SetInstalled(*installed_mod, params)
    .Next([promise](InstallModsInfoResult info_res) {

    if (info_res.Key.isError) {
      promise->SetValue({});
    } else {
      promise->SetValue(info_res.Value);
    }
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DynamicContentServiceImpl::CheckPremiumOwnership() {
  TSharedRef<TSet<int64>> dynamic_content_ids =
    GetModIdsForPremiumOwnershipCheck();

  if (dynamic_content_ids->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  // TODO(twolf): We could have only queried mods that aren't premium -
  // (requiring dynamic_content_mod_ids to be of type TArray<FCFCoreMod>), this
  // can reduce server calls and bandwidth (in theory).
  auto mods_details = MakeShared<TArray<FCFCoreMod>>();
  FetchModDetails(dynamic_content_ids->Array(), mods_details)
    .Next([this, promise, dynamic_content_ids, mods_details](FCFCoreError err) {

    if (mods_details->Num() < dynamic_content_ids->Num()) {
      CFCORE_LOG(Error, TEXT("Got less mods details from server than client"));
      promise->SetValue(FCFCoreError(ECFCoreErrorCodes::MissingModsDetails));
      return;
    }

    RemoveNonPremiumMods(dynamic_content_ids, mods_details);
    if (dynamic_content_ids->Num() == 0) {
      promise->SetValue(FCFCoreError());
      return;
    }

    AssurePremiumOwnership(dynamic_content_ids).Next([promise](FCFCoreError e) {
      promise->SetValue(e);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool DynamicContentServiceImpl::ShouldInstallModBasedOnStatus(
  const FInstalledMod& installed_mod) {

  for (const EInstalledModStatus status : kShouldInstallStatuses) {
    if (installed_mod.status == status) {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
TSharedRef<TSet<int64>>
DynamicContentServiceImpl::GetModIdsForPremiumOwnershipCheck() {
  TArray<FInstalledMod> installed_mods;
  installed_mods_info_service_->GetInstalledMods(installed_mods);

  auto dynamic_content_mod_ids = MakeShared<TSet<int64>>();

  for (FInstalledMod& installed_mod : installed_mods) {
    if (IsModForPremiumOwnershipCheck(installed_mod)) {
      dynamic_content_mod_ids->Add(installed_mod.details.id);
    }
  }

  return dynamic_content_mod_ids;
}

// -----------------------------------------------------------------------------
bool DynamicContentServiceImpl::IsModForPremiumOwnershipCheck(
  const FInstalledMod& installed_mod) {

  // We only care about dynamicContent that are false (meaning installed)
  if (installed_mod.dynamicContent) {
    return false;
  }

  const TSet<int64>& dymaic_content_category_ids =
    settings_service_->GetSettings().dynamicContentCategoryIds;

  // NOTE(twolf): The assumption here is that the game treats mods belonging to
  // the Dynamic Content Category Ids differently than a regular mod - so even
  // if the user edits and changes the mod's category - it will just mean that
  // the game won't treat it as such and it won't work.
  // For example - when joining a server, if your mod is not of the Skin/CC
  // category, then it will just not load.
  for (const FCategory& category : installed_mod.details.categories) {
    if (dymaic_content_category_ids.Contains(category.id)) {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DynamicContentServiceImpl::FetchModDetails(
  const TArray<int64>& mod_ids,
  TSharedRef<TArray<FCFCoreMod>> mods_details) {

  FCFCoreGetModsFilter filter;
  filter.modIds = mod_ids;
  return mods_sync_service_->GetModDetailsFromServer(filter, mods_details);
}

// -----------------------------------------------------------------------------
void DynamicContentServiceImpl::RemoveNonPremiumMods(
  TSharedRef<TSet<int64>> dynamic_content_ids,
  TSharedRef<TArray<FCFCoreMod>> mods_details) {

  for (const FCFCoreMod& mod_details : *mods_details) {
    if (!mod_details.premiumDetails.isPremium) {
      dynamic_content_ids->Remove(mod_details.id);
      continue;
    }
  }
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> DynamicContentServiceImpl::AssurePremiumOwnership(
  TSharedRef<TSet<int64>> dynamic_content_ids) {

  if (dynamic_content_ids->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_premium_mods_->CheckMods(
    dynamic_content_ids->Array(),
    ICFCorePremiumMods::FPremiumModsCheckDelegate::CreateLambda(
      [this, promise, dynamic_content_ids](
        const TOptional<TArray<int64>>& owned_ids,
        const TOptional<FCFCoreError>& err) {

        if (err.IsSet() && err->isError) {
          promise->SetValue(err);
          return;
        }

        HandleUnownedPremiumMods(dynamic_content_ids, *owned_ids)
          .Next([promise](bool has_unowned_premium_mods) {

          if (has_unowned_premium_mods) {
            auto code = ECFCoreErrorCodes::ModsNotOwnedByUser;
            promise->SetValue(FCFCoreError(code));
          } else {
            promise->SetValue(FCFCoreError());
          }
        });
      }));

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> DynamicContentServiceImpl::HandleUnownedPremiumMods(
  TSharedRef<TSet<int64>> dynamic_content_ids,
  const TArray<int64>& owned_mod_ids) {

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  TSet<int64> owned_ids_set = ContainerUtils::AsSet(owned_mod_ids);

  for (int64 dynamic_content_id : *dynamic_content_ids) {
    if (!owned_ids_set.Contains(dynamic_content_id)) {
      CFCORE_LOG(
        Error,
        TEXT("Found dynamic content not owned by user - modId=%lld"),
        dynamic_content_id);

      FInternalInstalledModInfoProperties props;
      props.modId = dynamic_content_id;
      props.dynamicContent = true; // reset dynamicContent to true (unowned)
      TArray<FInternalInstalledModInfoProperties> props_arr{props};
      futures->Add(installed_mods_info_service_->SetInstalledProperties(
        props_arr));
    }
  }

  bool found_unowned_premium_mod = (futures->Num() > 0);
  if (!found_unowned_premium_mod) {
    return MakeFulfilledPromise<bool>(false).GetFuture();
  }

  SHAREABLE_PROMISE(bool, promise);

  auto future = FutureUtils::Parallel<FCFCoreError>(futures);
  future.Next([promise](TArray<FCFCoreError> errors) {
    bool found_unowned_premium_mod = true;
    promise->SetValue(found_unowned_premium_mod);
  });

  return promise->GetFuture();
}