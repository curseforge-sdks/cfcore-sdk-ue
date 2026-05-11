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
#include <Async/Future.h>

#include <subscription/services/models/get_sync_plan_result.h>
#include <subscription/services/models/sync_subscriptions_result.h>
#include <subscription/models/subscription_sync_plan_item.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ISubscriptionSyncService {
public:
  virtual ~ISubscriptionSyncService() = default;

public:
  /**
   * Returns a sync plan based on installed mods and user subscriptions.
   */
  virtual TFuture<GetSyncPlanResult> GetSyncPlan() = 0;

  /**
   * Executes all items in the given sync plan.
   */
  virtual TFuture<SyncSubscriptionsResult> SyncSubscriptions(
    const TArray<FSubscriptionSyncPlanItem>& plan_items) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ISubscriptionSyncService)
