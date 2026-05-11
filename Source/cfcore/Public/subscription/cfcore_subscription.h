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

#include <api/models/mod.h>
#include <api/models/subscribe_request.h>
#include <api/common/cfcore_api_request_pagination.h>
#include <api/common/cfcore_api_response_pagination.h>

#include <common/cfcore_error.h>

#include <subscription/models/subscription_sync_plan_item.h>
#include <subscription/models/subscription_sync_result_item.h>

#include <Misc/Optional.h>

namespace cfcore {

class ICFCoreSubscription {
public:
  DECLARE_DELEGATE_ThreeParams(FGetModsSubscriptionsDelegate, const TArray<FCFCoreMod>&, const FCFCoreApiResponsePagination&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FSyncSubscriptionsDelegate, const TArray<FSubscriptionSyncResultItem>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FGetSyncPlanDelegate, const TArray<FSubscriptionSyncPlanItem>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FExecuteSyncPlanDelegate, const TArray<FSubscriptionSyncResultItem>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FSubscribeDelegate, const FCFCoreMod&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FUnsubscribeDelegate, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FAutoManagementDelegate, const TOptional<FCFCoreError>&);

public:
  virtual ~ICFCoreSubscription() = default;

public:
  /**
   * Enables automatic subscription management.
   * While enabled, the SDK periodically syncs installed mods with the user's
   * subscriptions, using only the allowed actions.
   * If it is already enabled, it will restart with the current settings.
   */
  virtual void EnableAutoManagement(FAutoManagementDelegate delegate) = 0;

  /**
   * Disables automatic subscription management.
   * Any pending timer that has not fired yet will be cancelled.
   * If a sync is already in progress, it will be allowed to finish, but no
   * further automatic syncs will be scheduled afterward.
   */
  virtual void DisableAutoManagement() = 0;

  /**
   * Subscribes to a mod and installs it after subscribing succeeds.
   */
  virtual void Subscribe(const FSubscribeRequest& request,
                         FSubscribeDelegate delegate) = 0;

  /**
   * Unsubscribes from a mod and uninstalls it after unsubscribing succeeds.
   */
  virtual void Unsubscribe(const FSubscribeRequest& request,
                           FUnsubscribeDelegate delegate) = 0;

  /**
   * Gets the user's mods subscriptions from the CurseForge API.
   */
  virtual void GetModsSubscriptions(
    const FCFCoreApiRequestPagination& pagination,
    FGetModsSubscriptionsDelegate delegate) = 0;

  /**
   * Builds the current sync plan and executes it immediately.
   * Use this when you do not need to inspect the plan before applying it.
   */
  virtual void SyncSubscriptions(FSyncSubscriptionsDelegate delegate) = 0;

  /**
   * Returns a sync plan based on installed mods and user subscriptions.
   * Use this when you want to inspect the planned actions first.
   */
  virtual void GetSyncPlan(FGetSyncPlanDelegate delegate) = 0;

  /**
   * Executes all items in the given sync plan.
   * Use this when you already have a plan and want full control over when it
   * is applied.
   */
  virtual void ExecuteSyncPlan(
    const TArray<FSubscriptionSyncPlanItem>& plan_items,
    FExecuteSyncPlanDelegate delegate) = 0;

};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreSubscription)
