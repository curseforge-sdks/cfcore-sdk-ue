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

#include "subscription_sync_service.h"

#include <library/models/installed_mod.h>

namespace cfcore {
class ICFCoreLibrary;
class ICFCoreApiAuthorized;
class ICFCoreAuthentication;
class IInstalledModsInfoService;
class ISettingsService;

// -----------------------------------------------------------------------------
class SubscriptionSyncServiceImpl : public ISubscriptionSyncService {
public:
  SubscriptionSyncServiceImpl(
    TSharedRef<ICFCoreLibrary> cfcore_library,
    TSharedRef<ICFCoreApiAuthorized> api_authorized,
    TSharedRef<ICFCoreAuthentication> cfcore_authentication,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
    TSharedRef<ISettingsService> settings_service);

// ISubscriptionSyncService
public:
  /**
   * As part of this, installed mod metadata is refreshed from the latest
   * subscribed mod details, regardless of planned actions.
   */
  virtual TFuture<GetSyncPlanResult> GetSyncPlan() override;

  virtual TFuture<SyncSubscriptionsResult> SyncSubscriptions(
    const TArray<FSubscriptionSyncPlanItem>& plan_items) override;

private:
  TFuture<FCFCoreError> GetInstalledMods(
    OUT TSharedRef<TArray<FInstalledMod>> out_installed_mods);

  TFuture<FCFCoreError> GetAllSubscriptions(
    OUT TSharedRef<TArray<FCFCoreMod>> out_subscriptions);

  TFuture<FCFCoreError> UpdateInstalledModDetails(
    const TArray<FCFCoreMod>& subscriptions);

  void GetAllSubsRecursive(int32 index,
                           TSharedRef<TArray<FCFCoreMod>> subscriptions,
                           TSharedRef<TPromise<FCFCoreError>> promise);

  TFuture<FCFCoreError> BuildSyncPlan(
    const TArray<FInstalledMod>& inst_mods,
    const TArray<FCFCoreMod>& subscriptions,
    OUT TArray<FSubscriptionSyncPlanItem>& out_items);

  void AddSubscribedModsToPlan(
    const TArray<FCFCoreMod>& subscriptions,
    const TMap<int64, FInstalledMod>& installed_mods_map,
    IN OUT TArray<FSubscriptionSyncPlanItem>& items);

  void AddUnsubscribedModsToPlan(
    const TMap<int64, FCFCoreMod>& subscriptions_map,
    const TArray<FInstalledMod>& installed_mods,
    IN OUT TArray<FSubscriptionSyncPlanItem>& items);

  TFuture<SyncSubscriptionsResult> ExecutePlan(
    const TArray<FSubscriptionSyncPlanItem>& plan_items);

  bool RequiresUpdate(const FCFCoreMod& subscribed_mod,
                      const FInstalledMod& installed_mod) const;

  bool IsActionAllowed(ESubscriptionSyncAction action) const;
  bool ShouldIgnoreInstalledMod(const FInstalledMod& installed_mod) const;

  TOptional<ESubscriptionSyncAction> ResolveOptionalSyncAction(
    const FCFCoreMod& subscribed_mod,
    const FInstalledMod* installed_mod) const;

  TFuture<FCFCoreError> InstallAsync(const FCFCoreMod& mod);
  TFuture<FCFCoreError> UninstallAsync(int64 mod_id);

  void AggregateResultItem(
    TArray<FSubscriptionSyncResultItem>& out_items,
    const FSubscriptionSyncPlanItem& plan_item,
    const FCFCoreError& err) const;

private:
  TSharedRef<ICFCoreLibrary> cfcore_library_;
  TSharedRef<ICFCoreApiAuthorized> api_authorized_;
  TSharedRef<ICFCoreAuthentication> cfcore_authentication_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
  TSharedRef<ISettingsService> settings_service_;
  
  // Guards against concurrent sync operations. Game-thread only.
  bool is_sync_in_progress_ = false;
};

} // namespace cfcore

Expose_TNameOf(cfcore::SubscriptionSyncServiceImpl)
