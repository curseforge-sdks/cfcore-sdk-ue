/*MIT License

Copyright (c) 2023 Overwolf Ltd.

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
#include "premium_mod_purchase_polling_service_impl.h"

#include <services/timer/timer_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
PremiumModPurchasePollingServiceImpl::PremiumModPurchasePollingServiceImpl(
    TSharedPtr<ITimerService> timer_service) :
  timer_service_(timer_service) {
}

// -----------------------------------------------------------------------------
void PremiumModPurchasePollingServiceImpl::Start(
    const FPollPurchaseParams& params,
    FCheckModsFunc check_mods_func,
    ICFCorePremiumMods::FPollPurchaseDelegate delegate) {
  StopInternal();

  poll_state_ = FPollState{params, check_mods_func, delegate,
                           FDateTime::UtcNow()};

  ScheduleNextOrTimeout();
}

// -----------------------------------------------------------------------------
void PremiumModPurchasePollingServiceImpl::Stop() {
  StopAndNotify(ECFCoreErrorCodes::PurchasePollingStopped);
}

// -----------------------------------------------------------------------------
void PremiumModPurchasePollingServiceImpl::ScheduleNextOrTimeout() {
  if (!poll_state_.IsSet()) {
    return;
  }

  const FPollState& state = poll_state_.GetValue();
  double elapsed =
    (FDateTime::UtcNow() - state.start_time).GetTotalSeconds();

  if (elapsed >= state.params.maxDurationSeconds) {
    StopAndNotify(ECFCoreErrorCodes::PurchasePollingTimeout);
    return;
  }

  uint64_t delay_ms =
    static_cast<uint64_t>(state.params.intervalSeconds * 1000.0f);

  pending_timer_handle_ = timer_service_->Delay(
    ITimerService::FTimerDelegate::CreateRaw(
      this,
      &PremiumModPurchasePollingServiceImpl::OnPollTimerFired),
    delay_ms);
}

// -----------------------------------------------------------------------------
void PremiumModPurchasePollingServiceImpl::OnPollTimerFired() {
  if (!poll_state_.IsSet()) {
    return;
  }

  TArray<int64> mod_ids = poll_state_.GetValue().params.modIds;
  FCheckModsFunc check_mods_func = poll_state_.GetValue().check_mods_func;

  check_mods_func.ExecuteIfBound(
    mod_ids,
    ICFCorePremiumMods::FPremiumModsCheckDelegate::CreateLambda(
      [this, mod_ids](
        const TOptional<TArray<int64>>& opt_owned,
        const TOptional<FCFCoreError>& opt_err) {

        OnCheckModsResult(mod_ids, opt_owned, opt_err);
      }));
}

// -----------------------------------------------------------------------------
void PremiumModPurchasePollingServiceImpl::OnCheckModsResult(
    const TArray<int64>& mod_ids,
    const TOptional<TArray<int64>>& opt_owned,
    const TOptional<FCFCoreError>& opt_err) {
  if (!poll_state_.IsSet()) {
    return;
  }

  if (opt_err.IsSet()) {
    // Propagate API / signature errors immediately.
    ICFCorePremiumMods::FPollPurchaseDelegate saved_delegate = StopInternal();
    saved_delegate.ExecuteIfBound({}, opt_err);
    return;
  }

  if (!opt_owned.IsSet()) {
    ScheduleNextOrTimeout();
    return;
  }

  // Check that every requested mod id is present in the response.
  const TArray<int64>& owned = opt_owned.GetValue();
  bool all_owned = true;
  for (int64 id : mod_ids) {
    if (!owned.Contains(id)) {
      all_owned = false;
      break;
    }
  }

  if (all_owned) {
    ICFCorePremiumMods::FPollPurchaseDelegate saved_delegate = StopInternal();
    saved_delegate.ExecuteIfBound(owned, {});
    return;
  }

  ScheduleNextOrTimeout();
}

// -----------------------------------------------------------------------------
ICFCorePremiumMods::FPollPurchaseDelegate
PremiumModPurchasePollingServiceImpl::StopInternal() {
  if (!poll_state_.IsSet()) {
    return ICFCorePremiumMods::FPollPurchaseDelegate{};
  }

  timer_service_->CancelDelay(pending_timer_handle_);
  pending_timer_handle_.Reset();

  ICFCorePremiumMods::FPollPurchaseDelegate saved_delegate =
    poll_state_.GetValue().delegate;
  poll_state_.Reset();

  return saved_delegate;
}

// -----------------------------------------------------------------------------
void PremiumModPurchasePollingServiceImpl::StopAndNotify(
    ECFCoreErrorCodes stop_code) {
  ICFCorePremiumMods::FPollPurchaseDelegate delegate = StopInternal();
  delegate.ExecuteIfBound({}, FCFCoreError(stop_code));
}
