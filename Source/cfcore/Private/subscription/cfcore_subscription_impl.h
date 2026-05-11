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
#include <subscription/cfcore_subscription.h>
#include "cfcore_subscription_internal.h"

namespace cfcore {

class ICFCoreLibrary;
class ICFCoreApiAuthorized;
class ITimerService;
class ICFCoreAuthentication;
class ISettingsService;
class ISubscribeInstalledModsService;
class ISubscriptionSyncService;

// -----------------------------------------------------------------------------
class CFCoreSubscriptionImpl : public ICFCoreSubscription,
                               public ICFCoreSubscriptionInternal,
                               public TSharedFromThis<CFCoreSubscriptionImpl> {
public:
  CFCoreSubscriptionImpl(
    TSharedRef<ICFCoreLibrary> cfcore_library,
    TSharedRef<ICFCoreApiAuthorized> api_authorized,
    TSharedRef<ITimerService> timer_service,
    TSharedRef<ICFCoreAuthentication> cfcore_authentication,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<ISubscriptionSyncService> subscription_sync_service,
    TSharedRef<ISubscribeInstalledModsService> sub_installed_mods_service);
  virtual ~CFCoreSubscriptionImpl() = default;

// ICFCoreSubscriptionInternal
public:
  virtual TSharedPtr<ICFCoreSubscription> GetSubscription() override {
    return this->AsShared();
  }

  virtual TFuture<FCFCoreError> Initialize() override;

// ICFCoreSubscription
public:
  virtual void EnableAutoManagement(
    FAutoManagementDelegate delegate) override;

  virtual void DisableAutoManagement() override;

  virtual void Subscribe(const FSubscribeRequest& request,
                         FSubscribeDelegate delegate) override;

  virtual void Unsubscribe(const FSubscribeRequest& request,
                           FUnsubscribeDelegate delegate) override;

  virtual void GetModsSubscriptions(
    const FCFCoreApiRequestPagination& pagination,
    FGetModsSubscriptionsDelegate delegate) override;

  virtual void SyncSubscriptions(FSyncSubscriptionsDelegate delegate) override;

  virtual void GetSyncPlan(FGetSyncPlanDelegate delegate) override;

  virtual void ExecuteSyncPlan(
    const TArray<FSubscriptionSyncPlanItem>& plan_items,
    FExecuteSyncPlanDelegate delegate) override;

private:
  TFuture<FCFCoreError> ApiSubscribeAsync(
    const FSubscribeRequest& request,
    OUT TSharedRef<FCFCoreMod> out_mod);

  TFuture<FCFCoreError> InstallAsync(
    const TSharedRef<FCFCoreMod> mod,
    OUT TSharedRef<FCFCoreMod> out_installed_mod);

  TFuture<FCFCoreError> ApiUnsubscribeAsync(const FSubscribeRequest& request);

  TFuture<FCFCoreError> UninstallAsync(int64 mod_id);

  TFuture<FCFCoreError> SyncSubscriptionsAsync(
    OUT TSharedRef<TArray<FSubscriptionSyncResultItem>> out_result_items);

  TFuture<FCFCoreError> GetSyncPlanItemsAsync(
    OUT TSharedRef<TArray<FSubscriptionSyncPlanItem>> out_plan_items);

  TFuture<FCFCoreError> SyncPlanItemsAsync(
    const TSharedRef<TArray<FSubscriptionSyncPlanItem>> plan_items,
    OUT TSharedRef<TArray<FSubscriptionSyncResultItem>> out_result_items);

  void ScheduleNextAutoManagementRun();

private:
  TSharedRef<ICFCoreLibrary> cfcore_library_;
  TSharedRef<ICFCoreApiAuthorized> api_authorized_;
  TSharedRef<ITimerService> timer_service_;
  TSharedRef<ICFCoreAuthentication> cfcore_authentication_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<ISubscriptionSyncService> subscription_sync_service_;
  TSharedRef<ISubscribeInstalledModsService> sub_installed_mods_service_;

  bool is_auto_management_enabled_;
  int32 auto_management_interval_ms_;
  FDelegateHandle pending_auto_management_timer_handle_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreSubscriptionImpl)
