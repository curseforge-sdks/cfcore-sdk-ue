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
#include "subscription_sync_service_impl.h"

#include <common/container_utils.h>
#include <common/future_utils.h>
#include <common/macros.h>

#include <services/logger/logger_macros.h>

#include <api/cfcore_api_authorized.h>
#include <authentication/cfcore_authentication.h>
#include <library/cfcore_library.h>
#include <library/models/enums/mod_install_origin.h>
#include <library/models/install_mod_additional_params.h>
#include <library/services/installed_mods_info/installed_mods_info_service.h>
#include <models/cfcore_settings.h>
#include <models/cfcore_settings_subscriptions.h>
#include <services/settings_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
SubscriptionSyncServiceImpl::SubscriptionSyncServiceImpl(
  TSharedRef<ICFCoreLibrary> cfcore_library,
  TSharedRef<ICFCoreApiAuthorized> api_authorized,
  TSharedRef<ICFCoreAuthentication> cfcore_authentication,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
  TSharedRef<ISettingsService> settings_service) :
  cfcore_library_(cfcore_library),
  api_authorized_(api_authorized),
  cfcore_authentication_(cfcore_authentication),
  installed_mods_info_service_(installed_mods_info_service),
  settings_service_(settings_service) {
}

// -----------------------------------------------------------------------------
TFuture<GetSyncPlanResult> SubscriptionSyncServiceImpl::GetSyncPlan() {
  if (!cfcore_authentication_->IsAuthenticated()) {
    FCFCoreError err(ECFCoreErrorCodes::UserNotAuthenticated);
    GetSyncPlanResult result;
    result.error = err;
    return MakeFulfilledPromise<GetSyncPlanResult>(result).GetFuture();
  }

  SHAREABLE_PROMISE(GetSyncPlanResult, promise);

  auto installed_mods = MakeShared<TArray<FInstalledMod>>();
  auto subscriptions = MakeShared<TArray<FCFCoreMod>>();
  auto items = MakeShared<TArray<FSubscriptionSyncPlanItem>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, installed_mods]() {
        return GetInstalledMods(installed_mods);
      },
      [this, subscriptions]() { 
        return GetAllSubscriptions(subscriptions);
      },
      [this, subscriptions]() {
        return UpdateInstalledModDetails(*subscriptions);
      },
      [this, installed_mods, subscriptions, items]() {
        return BuildSyncPlan(*installed_mods, *subscriptions, *items);
      }
  });

  auto future = FutureUtils::SequenceAll(functions, &FCFCoreError::isError);

  future.Next([promise, items](FCFCoreError err) {
    GetSyncPlanResult result;
    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to build subscription sync plan: %s"),
        *err.description);
      result.error = err;
      promise->SetValue(result);
      return;
    }

    result.items = *items;
    promise->SetValue(result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<SyncSubscriptionsResult> SubscriptionSyncServiceImpl::SyncSubscriptions(
  const TArray<FSubscriptionSyncPlanItem>& plan_items) {
  check(IsInGameThread());

  if (is_sync_in_progress_) {
    CFCORE_LOG(Warning,
      TEXT("Subscription sync already in progress, rejecting request"));

    FCFCoreError err(ECFCoreErrorCodes::SubscriptionSyncInProgress);
    SyncSubscriptionsResult result;
    result.error = err;
    return MakeFulfilledPromise<SyncSubscriptionsResult>(result).GetFuture();
  }

  is_sync_in_progress_ = true;

  return ExecutePlan(plan_items)
    .Next([this](SyncSubscriptionsResult result) {
      is_sync_in_progress_ = false;
      return result;
    });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscriptionSyncServiceImpl::UpdateInstalledModDetails(
  const TArray<FCFCoreMod>& subscriptions) {

  return installed_mods_info_service_->UpdateModDetails(subscriptions);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscriptionSyncServiceImpl::GetInstalledMods(
  OUT TSharedRef<TArray<FInstalledMod>> out_inst_mods) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_library_->GetInstalledMods(
    ICFCoreLibrary::FGetInstalledModsDelegate::CreateLambda(
      [promise, out_inst_mods](const TArray<FInstalledMod>& installed_mods) {

      *out_inst_mods = installed_mods;
      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscriptionSyncServiceImpl::GetAllSubscriptions(
  OUT TSharedRef<TArray<FCFCoreMod>> out_subscriptions) {
  if (!cfcore_authentication_->IsAuthenticated()) {
    FCFCoreError err(ECFCoreErrorCodes::UserNotAuthenticated);
    return MakeFulfilledPromise<FCFCoreError>(err).GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  GetAllSubsRecursive(0, out_subscriptions, promise);

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void SubscriptionSyncServiceImpl::GetAllSubsRecursive(
  int32 index,
  TSharedRef<TArray<FCFCoreMod>> out_subscriptions,
  TSharedRef<TPromise<FCFCoreError>> promise) {

  FCFCoreApiRequestPagination pagination;
  pagination.index = index;

  api_authorized_->GetModsSubscriptions(
    pagination,
    ICFCoreApiAuthorized::FGetModsSubscriptionsDelegate::CreateLambda(
      [this, out_subscriptions, promise](
        const TOptional<TArray<FCFCoreMod>>& opt_mods,
        const TOptional<FCFCoreApiResponsePagination>& opt_pagination,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

      if (opt_err.IsSet()) {
        promise->SetValue(FCFCoreError(opt_err));
        return;
      }

      out_subscriptions->Append(*opt_mods);

      int32 next_index = opt_pagination->index + opt_pagination->pageSize;
      if (next_index >= opt_pagination->totalCount) {
        promise->SetValue(FCFCoreError());
        return;
      }

      GetAllSubsRecursive(next_index, out_subscriptions, promise);
    })
  );
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscriptionSyncServiceImpl::BuildSyncPlan(
  const TArray<FInstalledMod>& inst_mods,
  const TArray<FCFCoreMod>& subscriptions,
  TArray<FSubscriptionSyncPlanItem>& out_plan) {

  auto subs_map = ContainerUtils::AsMap(subscriptions, &FCFCoreMod::id);
  TMap<int64, FInstalledMod> inst_mods_map;
  for (const FInstalledMod& installed_mod : inst_mods) {
    inst_mods_map.Add(installed_mod.details.id, installed_mod);
  }

  AddSubscribedModsToPlan(subscriptions, inst_mods_map, out_plan);
  AddUnsubscribedModsToPlan(subs_map, inst_mods, out_plan);

  return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
}

// -----------------------------------------------------------------------------
void SubscriptionSyncServiceImpl::AddSubscribedModsToPlan(
  const TArray<FCFCoreMod>& subscriptions,
  const TMap<int64, FInstalledMod>& installed_mods_map,
  IN OUT TArray<FSubscriptionSyncPlanItem>& items) {

  for (const FCFCoreMod& subbed_mod : subscriptions) {
    const FInstalledMod* inst_mod = installed_mods_map.Find(subbed_mod.id);

    TOptional<ESubscriptionSyncAction> opt_action =
      ResolveOptionalSyncAction(subbed_mod, inst_mod);

    if (!opt_action.IsSet() || !IsActionAllowed(*opt_action)) {
      continue;
    }

    FSubscriptionSyncPlanItem item;
    item.action = *opt_action;
    item.mod = subbed_mod;
    items.Add(item);
  }
}

// -----------------------------------------------------------------------------
void SubscriptionSyncServiceImpl::AddUnsubscribedModsToPlan(
  const TMap<int64, FCFCoreMod>& subscriptions_map,
  const TArray<FInstalledMod>& installed_mods,
  IN OUT TArray<FSubscriptionSyncPlanItem>& items) {

  if (!IsActionAllowed(ESubscriptionSyncAction::Uninstall)) {
    return;
  }

  for (const FInstalledMod& installed_mod : installed_mods) {
    if (ShouldIgnoreInstalledMod(installed_mod)) {
      continue;
    }

    if (subscriptions_map.Contains(installed_mod.details.id)) {
      continue;
    }

    FSubscriptionSyncPlanItem item;
    item.action = ESubscriptionSyncAction::Uninstall;
    item.mod = installed_mod.details;
    items.Add(item);
  }
}

// -----------------------------------------------------------------------------
bool SubscriptionSyncServiceImpl::IsActionAllowed(
  ESubscriptionSyncAction action) const {

  const FCFCoreSettingsSubscriptions subs_settings =
    settings_service_->GetSettings().subscriptions;

  switch (action) {
    case ESubscriptionSyncAction::Install:
      return subs_settings.allowInstallActions;
    case ESubscriptionSyncAction::Update:
      return subs_settings.allowUpdateActions;
    case ESubscriptionSyncAction::Uninstall:
      return subs_settings.allowUninstallActions;
    default:
      return false;
  }
}

// -----------------------------------------------------------------------------
bool SubscriptionSyncServiceImpl::ShouldIgnoreInstalledMod(
  const FInstalledMod& installed_mod) const {

  if (installed_mod.dynamicContent) {
    return true;
  }

  // NOTE(apliscov): This protects unmanaged mods from automatic uninstalls.
  const FCFCoreSettings& settings = settings_service_->GetSettings();
  return settings.unmanagedMods.enabled && installed_mod.unmanaged;
}

// -----------------------------------------------------------------------------
TOptional<ESubscriptionSyncAction>
SubscriptionSyncServiceImpl::ResolveOptionalSyncAction(
  const FCFCoreMod& subscribed_mod,
  const FInstalledMod* installed_mod) const {

  if (!installed_mod) {
    return ESubscriptionSyncAction::Install;
  }

  if (ShouldIgnoreInstalledMod(*installed_mod)) {
    return {};
  }

  if (RequiresUpdate(subscribed_mod, *installed_mod)) {
    return ESubscriptionSyncAction::Update;
  }

  return {};
}

TFuture<FCFCoreError> SubscriptionSyncServiceImpl::InstallAsync(
  const FCFCoreMod& mod) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  FInstallModAdditionalParams additional_params;
  additional_params.origin = EModInstallOrigin::SubscribeModule;

  cfcore_library_->Install(
    mod,
    FFile(),
    additional_params,
    ICFCoreLibrary::FInstallProgressDelegate(),
    ICFCoreLibrary::FInstalledDelegate::CreateLambda(
      [promise](const TOptional<FInstalledMod>&,
                const TOptional<FCFCoreError>& opt_err) {

      FCFCoreError err;
      if (opt_err.IsSet() && opt_err->isError) {
        err = *opt_err;
      }

      promise->SetValue(err);
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscriptionSyncServiceImpl::UninstallAsync(
  int64 mod_id) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_library_->Uninstall(
    mod_id,
    ICFCoreLibrary::FUninstallDelegate::CreateLambda(
      [promise](TOptional<FInstalledMod>,
                const TOptional<FCFCoreError>& opt_err) {

      FCFCoreError err;
      if (opt_err.IsSet() && opt_err->isError) {
        err = *opt_err;
      }

      promise->SetValue(err);
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<SyncSubscriptionsResult> SubscriptionSyncServiceImpl::ExecutePlan(
  const TArray<FSubscriptionSyncPlanItem>& plan_items) {

  if (plan_items.Num() == 0) {
    return MakeFulfilledPromise<SyncSubscriptionsResult>(
      SyncSubscriptionsResult()).GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  auto planned_items = MakeShared<TArray<FSubscriptionSyncPlanItem>>();

  for (const FSubscriptionSyncPlanItem& plan_item : plan_items) {
    planned_items->Add(plan_item);
    if (plan_item.action == ESubscriptionSyncAction::Uninstall) {
      futures->Add(UninstallAsync(plan_item.mod.id));
    } else {
      futures->Add(InstallAsync(plan_item.mod));
    }
  }

  SHAREABLE_PROMISE(SyncSubscriptionsResult, promise);

  FutureUtils::Parallel<FCFCoreError>(futures).Next(
    [this, promise, planned_items](TArray<FCFCoreError> errs) mutable {

    SyncSubscriptionsResult result;
    for (int32 i = 0; i < planned_items->Num(); i++) {
      AggregateResultItem(result.items, (*planned_items)[i], errs[i]);
    }

    promise->SetValue(result);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void SubscriptionSyncServiceImpl::AggregateResultItem(
  TArray<FSubscriptionSyncResultItem>& out_items,
  const FSubscriptionSyncPlanItem& plan_item,
  const FCFCoreError& err) const {

  FSubscriptionSyncResultItem item;
  item.modId = plan_item.mod.id;
  item.action = plan_item.action;
  item.error = err;
  item.status = ESubscriptionSyncItemStatus::Success;

  if (err.isError) {
    item.status = ESubscriptionSyncItemStatus::Failed;
  }

  out_items.Add(item);

  if (err.isError) {
    ENUM_TO_STR(action_str, plan_item.action);
    CFCORE_LOG(Warning,
      TEXT("Subscriptions sync failed (%s) - modId=%lld (%s)"),
      *action_str, plan_item.mod.id, *err.description);
  }
}

// -----------------------------------------------------------------------------
bool SubscriptionSyncServiceImpl::RequiresUpdate(
  const FCFCoreMod& subscribed_mod,
  const FInstalledMod& installed_mod) const {

  if (subscribed_mod.latestFiles.Num() <= 0) {
    return false;
  }

  const FFile& latest_file = subscribed_mod.latestFiles.Last();
  if (installed_mod.installedFile.id == latest_file.id) {
    return false;
  }

  return true;
}
