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
#include "cfcore_subscription_impl.h"

#include <common/cfcore_error.h>
#include <common/future_utils.h>
#include <common/macros.h>

#include <services/logger/logger_macros.h>

#include <api/cfcore_api_authorized.h>
#include <authentication/cfcore_authentication.h>
#include <library/cfcore_library.h>
#include <models/cfcore_settings_subscriptions.h>
#include <services/settings_service.h>
#include <services/timer/timer_service.h>
#include <subscription/services/subscription_sync_service.h>

#include <Misc/Optional.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreSubscriptionImpl::CFCoreSubscriptionImpl(
  TSharedRef<ICFCoreLibrary> cfcore_library,
  TSharedRef<ICFCoreApiAuthorized> api_authorized,
  TSharedRef<ITimerService> timer_service,
  TSharedRef<ICFCoreAuthentication> cfcore_authentication,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<ISubscriptionSyncService> subscription_sync_service,
  TSharedRef<ISubscribeInstalledModsService> sub_installed_mods_service) :
  cfcore_library_(cfcore_library),
  api_authorized_(api_authorized),
  timer_service_(timer_service),
  cfcore_authentication_(cfcore_authentication),
  settings_service_(settings_service),
  subscription_sync_service_(subscription_sync_service),
  sub_installed_mods_service_(sub_installed_mods_service),
  is_auto_management_enabled_(false),
  auto_management_interval_ms_(0) {
}

// ICFCoreSubscriptionInternal
// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::Initialize() {
  return MakeFulfilledPromise<FCFCoreError>().GetFuture();
}

// ICFCoreSubscription
// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::EnableAutoManagement(
  FAutoManagementDelegate delegate) {

  const FCFCoreSettingsSubscriptions& sub_settings =
    settings_service_->GetSettings().subscriptions;

  if (is_auto_management_enabled_) {
    DisableAutoManagement();
  }

  auto_management_interval_ms_ = sub_settings.automaticModManagementIntervalMs;
  is_auto_management_enabled_ = true;

  CFCORE_LOG(Log,
    TEXT("Automatic mod management enabled (interval: %d ms)"),
    auto_management_interval_ms_);

  ScheduleNextAutoManagementRun();

  delegate.ExecuteIfBound({});
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::DisableAutoManagement() {
  if (!is_auto_management_enabled_) {
    return;
  }

  is_auto_management_enabled_ = false;
  auto_management_interval_ms_ = 0;

  if (pending_auto_management_timer_handle_.IsValid()) {
    timer_service_->CancelDelay(pending_auto_management_timer_handle_);
    pending_auto_management_timer_handle_.Reset();
  }

  CFCORE_LOG(Log, TEXT("Automatic mod management disabled"));
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::Subscribe(const FSubscribeRequest& request,
                                       FSubscribeDelegate delegate) {

  if (!cfcore_authentication_->IsAuthenticated()) {
    CFCORE_LOG(Warning,
               TEXT("Failed to subscribe - user is not authenticated"));
    FCFCoreError err(ECFCoreErrorCodes::UserNotAuthenticated);
    delegate.ExecuteIfBound({}, err);
    return;
  }

  auto subscribed_mod = MakeShared<FCFCoreMod>();
  auto installed_mod = MakeShared<FCFCoreMod>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>>{
      [this, request, subscribed_mod]() {
        return ApiSubscribeAsync(request, subscribed_mod);
      },
      [this, subscribed_mod, installed_mod]() {
        return InstallAsync(subscribed_mod, installed_mod);
      }
  });

  auto future = FutureUtils::SequenceAll(functions, &FCFCoreError::isError);

  future.Next([delegate, installed_mod](FCFCoreError err) {
    TOptional<FCFCoreError> opt_err;
    if (err.isError) {
      opt_err = err;
    }

    delegate.ExecuteIfBound(*installed_mod, opt_err);
  });
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::Unsubscribe(const FSubscribeRequest& request,
                                         FUnsubscribeDelegate delegate) {

  if (!cfcore_authentication_->IsAuthenticated()) {
    CFCORE_LOG(Warning,
      TEXT("Failed to unsubscribe - user is not authenticated"));
    FCFCoreError err(ECFCoreErrorCodes::UserNotAuthenticated);
    delegate.ExecuteIfBound(err);
    return;
  }

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>>{
      [this, request]() {
        return ApiUnsubscribeAsync(request);
      },
      [this, request]() {
        return UninstallAsync(request.modId);
      }
  });

  auto future = FutureUtils::SequenceAll(functions, &FCFCoreError::isError);

  future.Next([delegate](FCFCoreError err) {
    TOptional<FCFCoreError> opt_err;
    if (err.isError) {
      opt_err = err;
    }

    delegate.ExecuteIfBound(opt_err);
  });
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::GetModsSubscriptions(
  const FCFCoreApiRequestPagination& pagination,
  FGetModsSubscriptionsDelegate delegate) {

  if (!cfcore_authentication_->IsAuthenticated()) {
    CFCORE_LOG(Warning,
      TEXT("Failed to get mods subscriptions - user is not authenticated"));
    FCFCoreError err(ECFCoreErrorCodes::UserNotAuthenticated);
    delegate.ExecuteIfBound({}, {}, err);
    return;
  }

  api_authorized_->GetModsSubscriptions(
    pagination,
    ICFCoreApiAuthorized::FGetModsSubscriptionsDelegate::CreateLambda(
      [delegate](TOptional<TArray<FCFCoreMod>> opt_mod_array,
                 TOptional<FCFCoreApiResponsePagination> opt_pagination,
                 const TOptional<FCFCoreApiResponseError>& opt_api_err) {

      TOptional<FCFCoreError> opt_err;
      if (opt_api_err) {
        FString desc = opt_api_err.GetValue().description;
        CFCORE_LOG(Error, TEXT("Failed to get mods subscriptions (%s)"), *desc);
        opt_err = FCFCoreError(opt_api_err);
        delegate.ExecuteIfBound({}, {}, opt_err);
        return;
      }

      delegate.ExecuteIfBound(*opt_mod_array, *opt_pagination, opt_err);
    })
  );
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::SyncSubscriptions(
  FSyncSubscriptionsDelegate delegate) {

  auto result_items = MakeShared<TArray<FSubscriptionSyncResultItem>>();

  SyncSubscriptionsAsync(result_items)
    .Next([delegate, result_items](FCFCoreError err) {

    TOptional<FCFCoreError> opt_err;
    if (err.isError) {
      opt_err = err;
    }

    delegate.ExecuteIfBound(*result_items, opt_err);
  });
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::GetSyncPlan(FGetSyncPlanDelegate delegate) {
  subscription_sync_service_->GetSyncPlan()
    .Next([delegate](GetSyncPlanResult result) {

    TOptional<FCFCoreError> opt_err;
    if (result.error.isError) {
      CFCORE_LOG(Error,
        TEXT("Failed to get subscription sync plan (%s)"),
        *result.error.description);
      opt_err = result.error;
    }

    delegate.ExecuteIfBound(result.items, opt_err);
  });
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::ExecuteSyncPlan(
  const TArray<FSubscriptionSyncPlanItem>& plan_items,
  FExecuteSyncPlanDelegate delegate) {

  subscription_sync_service_->SyncSubscriptions(plan_items)
    .Next([delegate](SyncSubscriptionsResult result) {

    TOptional<FCFCoreError> opt_err;
    if (result.error.isError) {
      opt_err = result.error;
    }

    delegate.ExecuteIfBound(result.items, opt_err);
  });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::ApiSubscribeAsync(
  const FSubscribeRequest& request,
  OUT TSharedRef<FCFCoreMod> out_mod) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  api_authorized_->Subscribe(
    request,
    ICFCoreApiAuthorized::FSubscribeDelegate::CreateLambda(
      [out_mod, promise](
        const TOptional<FCFCoreMod>& opt_mod,
        const TOptional<FCFCoreApiResponseError>& opt_api_err) {

      if (opt_api_err) {
        FString desc = opt_api_err.GetValue().description;
        CFCORE_LOG(Error, TEXT("Failed to subscribe to mod (%s)"), *desc);
        promise->SetValue(FCFCoreError(opt_api_err));
        return;
      }

      if (opt_mod) {
        *out_mod = opt_mod.GetValue();
      }

      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::InstallAsync(
  const TSharedRef<FCFCoreMod> mod,
  OUT TSharedRef<FCFCoreMod> out_installed_mod) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_library_->Install(
    *mod,
    ICFCoreLibrary::FInstallProgressDelegate(),
    ICFCoreLibrary::FInstalledDelegate::CreateLambda(
      [mod, out_installed_mod, promise](
        const TOptional<FInstalledMod>& opt_installed_mod,
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err && opt_err->isError) {
        CFCORE_LOG(Error,
          TEXT("Failed to install after subscribe (%lld)"),
          mod->id);
        promise->SetValue(opt_err);
        return;
      } 

      CFCORE_LOG(Log,
        TEXT("Successfully installed mod (%lld) after subscribing"),
        mod->id);

      if (opt_installed_mod) {
        *out_installed_mod = opt_installed_mod->details;
      }

      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::ApiUnsubscribeAsync(
  const FSubscribeRequest& request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  api_authorized_->Unsubscribe(
    request,
    ICFCoreApiAuthorized::FUnsubscribeDelegate::CreateLambda(
      [promise](const TOptional<FCFCoreApiResponseError>& opt_api_err) {

      FCFCoreError err;
      if (opt_api_err) {
        FString desc = opt_api_err.GetValue().description;
        CFCORE_LOG(Error, TEXT("Failed to unsubscribe from mod (%s)"), *desc);
        err = FCFCoreError(opt_api_err);
      }

      promise->SetValue(err);
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::UninstallAsync(
  int64 mod_id) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_library_->Uninstall(
    mod_id,
    ICFCoreLibrary::FUninstallDelegate::CreateLambda(
      [mod_id, promise](
        TOptional<FInstalledMod>,
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err && opt_err->isError) {
        CFCORE_LOG(Error,
          TEXT("Failed to uninstall after unsubscribe (%lld)"),
          mod_id);
        promise->SetValue(*opt_err);
        return;
      }

      CFCORE_LOG(Log,
        TEXT("Successfully uninstalled mod (%lld) after unsubscribing"),
        mod_id);

      promise->SetValue(FCFCoreError());
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::SyncSubscriptionsAsync(
  OUT TSharedRef<TArray<FSubscriptionSyncResultItem>> out_result_items) {

  auto plan_items = MakeShared<TArray<FSubscriptionSyncPlanItem>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, plan_items]() {
        return GetSyncPlanItemsAsync(plan_items);
      },
      [this, plan_items, out_result_items]() {
        return SyncPlanItemsAsync(plan_items, out_result_items);
      }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::GetSyncPlanItemsAsync(
  OUT TSharedRef<TArray<FSubscriptionSyncPlanItem>> out_plan_items) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  subscription_sync_service_->GetSyncPlan()
    .Next([out_plan_items, promise](GetSyncPlanResult result) {
      if (result.error.isError) {
        CFCORE_LOG(Warning, TEXT("Failed to build subscriptions sync plan"));
        promise->SetValue(result.error);
        return;
      }

      *out_plan_items = result.items;
      promise->SetValue(FCFCoreError());
    });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreSubscriptionImpl::SyncPlanItemsAsync(
  const TSharedRef<TArray<FSubscriptionSyncPlanItem>> plan_items,
  OUT TSharedRef<TArray<FSubscriptionSyncResultItem>> out_result_items) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  subscription_sync_service_->SyncSubscriptions(*plan_items)
    .Next([out_result_items, promise](SyncSubscriptionsResult result) {

      if (result.error.isError) {
        CFCORE_LOG(Warning, TEXT("Failed to sync subscriptions"));
      }

      *out_result_items = result.items;
      promise->SetValue(result.error);
    });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void CFCoreSubscriptionImpl::ScheduleNextAutoManagementRun() {
  if (!is_auto_management_enabled_) {
    return;
  }

  pending_auto_management_timer_handle_ = timer_service_->Delay(
    ITimerService::FTimerDelegate::CreateLambda([this]() {
      pending_auto_management_timer_handle_.Reset();

      if (!is_auto_management_enabled_) {
        return;
      }

      auto result_items = MakeShared<TArray<FSubscriptionSyncResultItem>>();

      SyncSubscriptionsAsync(result_items)
        .Next([this](FCFCoreError) {
          ScheduleNextAutoManagementRun();
        });
    }),
    static_cast<uint64_t>(auto_management_interval_ms_)
  );
}
