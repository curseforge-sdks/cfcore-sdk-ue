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
#pragma once
#include <CoreMinimal.h>

#include "premium_mod_purchase_polling_service.h"
#include <common/macros.h>

namespace cfcore {
class ITimerService;

// -----------------------------------------------------------------------------
class PremiumModPurchasePollingServiceImpl
    : public IPremiumModPurchasePollingService {
public:
  explicit PremiumModPurchasePollingServiceImpl(
      TSharedPtr<ITimerService> timer_service);
  virtual ~PremiumModPurchasePollingServiceImpl() = default;

// IPremiumModPurchasePollingService
public:
  virtual void Start(
      const FPollPurchaseParams& params,
      FCheckModsFunc check_mods_func,
      ICFCorePremiumMods::FPollPurchaseDelegate delegate) override;

  virtual void Stop() override;

private:
  void ScheduleNextOrTimeout();
  void OnPollTimerFired();
  void OnCheckModsResult(const TArray<int64>& mod_ids,
                         const TOptional<TArray<int64>>& opt_owned,
                         const TOptional<FCFCoreError>& opt_err);

  ICFCorePremiumMods::FPollPurchaseDelegate StopInternal();
  void StopAndNotify(ECFCoreErrorCodes stop_code);

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(PremiumModPurchasePollingServiceImpl);

private:
  TSharedPtr<ITimerService> timer_service_;

  struct FPollState {
    FPollPurchaseParams params;
    FCheckModsFunc check_mods_func;
    ICFCorePremiumMods::FPollPurchaseDelegate delegate;
    FDateTime start_time;
  };
  TOptional<FPollState> poll_state_;
  FDelegateHandle pending_timer_handle_;
};

} // namespace cfcore
