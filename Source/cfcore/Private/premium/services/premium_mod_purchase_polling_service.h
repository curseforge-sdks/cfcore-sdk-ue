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

#include <premium/cfcore_premium_mods.h>
#include <premium/models/poll_purchase_params.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IPremiumModPurchasePollingService {
public:
  /**
   * Callable injected by the owner to perform the actual (signature-verified)
   * mod ownership check. Decouples the polling logic from ICFCorePremiumMods.
   */
  DECLARE_DELEGATE_TwoParams(FCheckModsFunc,
      const TArray<int64>& /*mod_ids*/,
      ICFCorePremiumMods::FPremiumModsCheckDelegate /*result_delegate*/);

public:
  virtual ~IPremiumModPurchasePollingService() = default;

public:
  /**
   * Starts a polling session. If a session is already active it is silently
   * replaced (its delegate will NOT be fired).
   *
   * @param params          Polling configuration (mod ids, interval, timeout).
   * @param check_mods_func Callable that performs the ownership check.
   * @param delegate        Fired on success, timeout, or API error.
   */
  virtual void Start(
      const FPollPurchaseParams& params,
      FCheckModsFunc check_mods_func,
      ICFCorePremiumMods::FPollPurchaseDelegate delegate) = 0;

  /**
   * Stops the active polling session and fires the delegate with
   * PurchasePollingStopped. Has no effect if no session is active.
   */
  virtual void Stop() = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IPremiumModPurchasePollingService)