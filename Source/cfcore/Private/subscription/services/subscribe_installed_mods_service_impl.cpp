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
#include "subscribe_installed_mods_service_impl.h"
#include <common/constants.h>
#include <common/macros.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include <services/user_context/user_context_service.h>
#include <api/cfcore_api_authorized.h>

namespace cfcore {
};

using namespace cfcore;

// -----------------------------------------------------------------------------
SubscribeInstalledModsServiceImpl::SubscribeInstalledModsServiceImpl(
  TSharedRef<IUserContextService> user_context_service,
  TSharedRef<ICFCoreApiAuthorized> api_authorized,
  TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
  authorized_delegates_dispatcher,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service) :
  user_context_service_(user_context_service),
  api_authorized_(api_authorized),
  delegates_(MakeShared<CFCoreApiAuthorizedDelegates>()),
  installed_mods_info_service_(installed_mods_info_service) {

  RegisterDelegates(authorized_delegates_dispatcher);
}

// -----------------------------------------------------------------------------
void SubscribeInstalledModsServiceImpl::RegisterDelegates(
  TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
  authorized_delegates_dispatcher) {

  delegates_->get_me = ICFCoreApiAuthorized::FGetMeDelegate::CreateLambda(
    [this](const TOptional<FMe>& opt_me,
      const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          return;
        }

        TryPerformingSubscriptions(opt_me.GetValue());
    });

  authorized_delegates_dispatcher->Register(FGuid::NewGuid(), delegates_);
}

// -----------------------------------------------------------------------------
void SubscribeInstalledModsServiceImpl::TryPerformingSubscriptions(
  const FMe&) {

  if (user_context_service_->UserContextInfo().subscribedToInstalledMods) {
    return;
  }

  PerformSubscriptions();
}

// -----------------------------------------------------------------------------
void SubscribeInstalledModsServiceImpl::PerformSubscriptions() {
  TSharedRef<TArray<int64>> subbed_mods_ids = MakeShared<TArray<int64>>();
  auto future = GetAllSubscribedMods(subbed_mods_ids);

  future.Next([this, subbed_mods_ids](FCFCoreError err) {
    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to get all subscribed mods"));
      return;
    }

    TSharedRef<TArray<int64>> mods_ids_to_sub =
      GetUnsubscribedInstalledModsIds(subbed_mods_ids);

    SubscribeMods(mods_ids_to_sub).Next([this](FCFCoreError err) {
      if (err.isError) {
        CFCORE_LOG(Error, TEXT("Failed to auto-subscribe to installed mods"));
      }

      user_context_service_->SetSubscribedToInstalledMods()
        .Next([](FCFCoreError err) {

        if (err.isError) {
          CFCORE_LOG(Error, TEXT("Failed to set subscribed to installed mods"));
        }

        CFCORE_LOG(Log, TEXT("Successfully auto-subscribed to installed mods"));
      });
    });
  });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscribeInstalledModsServiceImpl::GetAllSubscribedMods(
  TSharedRef<TArray<int64>> out_subscribed_mods_ids,
  const int32 index/* = 0 */) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  FCFCoreApiRequestPagination pagination;
  pagination.index = index;
  api_authorized_->GetModsSubscriptionsIds(
    pagination,
    ICFCoreApiAuthorized::FGetModsSubscriptionsIdsDelegate::CreateLambda(
      [this, promise, out_subscribed_mods_ids](
        const TOptional<TArray<int64>>& opt_subbed_mods_ids,
        const TOptional<FCFCoreApiResponsePagination>& opt_pagination,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

      if (opt_err.IsSet()) {
        CFCORE_LOG(Error, TEXT("Failed to get mods subscriptions IDs"));
        promise->SetValue(FCFCoreError(ECFCoreErrorCodes::ApiError));
        return;
      }

      out_subscribed_mods_ids->Append(opt_subbed_mods_ids.GetValue());

      int32 next_index = opt_pagination->index + opt_pagination->pageSize;
      if (next_index >= opt_pagination->totalCount) {
        promise->SetValue(FCFCoreError());
        return;
      }

      GetAllSubscribedMods(out_subscribed_mods_ids, next_index)
        .Next([this, promise](FCFCoreError err) {

        promise->SetValue(err);
        return;
      });
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SubscribeInstalledModsServiceImpl::SubscribeMods(
  TSharedRef<TArray<int64>> mods_ids_to_sub) {

  if (mods_ids_to_sub->Num() <= 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TArray<int64> current_batch = PopCurrentBatch(mods_ids_to_sub);

  FSubscribeModsRequest request{ current_batch };
  api_authorized_->SubscribeMods(
    request,
    ICFCoreApiAuthorized::FSubscribeModsDelegate::CreateLambda(
      [this, promise, mods_ids_to_sub](
        const TOptional<FCFCoreApiResponseError>& opt_err) {

      if (opt_err.IsSet()) {
        promise->SetValue(FCFCoreError(ECFCoreErrorCodes::ApiError));
        return;
      }

      SubscribeMods(mods_ids_to_sub).Next([promise](FCFCoreError err) {
        promise->SetValue(err);
      });
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TSharedRef<TArray<int64>>
SubscribeInstalledModsServiceImpl::GetUnsubscribedInstalledModsIds(
  TSharedRef<TArray<int64>> subscribed_mods_ids) {

  TArray<FInstalledMod> installed_mods;
  installed_mods_info_service_->GetInstalledMods(installed_mods);

  TArray<int64> installed_mods_ids;
  for (const FInstalledMod& installed_mod : installed_mods) {
    if (installed_mod.unmanaged) {
      continue;
    }

    installed_mods_ids.Add(installed_mod.details.id);
  }

  TArray<int64> mods_ids_for_sub = installed_mods_ids.FilterByPredicate(
    [subscribed_mods_ids](const int64& installed_mod_id) {
      return !subscribed_mods_ids->Contains(installed_mod_id);
    }
  );

  return MakeShared<TArray<int64>>(mods_ids_for_sub);
}

// -----------------------------------------------------------------------------
TArray<int64> SubscribeInstalledModsServiceImpl::PopCurrentBatch(
  TSharedRef<TArray<int64>> mods_ids) {

  TArray<int64> current_batch;
  int32 current_batch_size =
  FMath::Min(static_cast<int>(kMaxSubscriptionsPerBatch), mods_ids->Num());

  const int64* mods_ids_ptr = mods_ids->GetData();
  current_batch.Append(mods_ids_ptr, current_batch_size);
  mods_ids->RemoveAt(0, current_batch_size);

  return current_batch;
}
