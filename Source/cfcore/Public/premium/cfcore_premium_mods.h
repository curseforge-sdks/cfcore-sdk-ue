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
#include "CoreMinimal.h"

#include <common/cfcore_error.h>
#include <api/models/mod_file_download_url.h>
#include <api/models/premium/owned_premium_mods.h>
#include <premium/models/poll_purchase_params.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ICFCorePremiumMods {
public:
  DECLARE_DELEGATE_TwoParams(FGetPremiumModsDelegate, const TOptional<TArray<int64>>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FGetPremiumModsV2Delegate, const TOptional<FOwnedPremiumMods>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FPremiumModsCheckDelegate, const TOptional<TArray<int64>>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FCheckDlcDelegate, const TOptional<TArray<int64>>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FGetModFileDownloadUrlDelegate, const TOptional<FModFileDownloadUrl>&, const TOptional<FCFCoreError>&);

  /**
   * Fired when PollModPurchase finishes — either because all mod ids are now
   * owned, the max duration elapsed, or StopPurchasePolling was called.
   * On success the owned mod ids are set and the error is unset.
   * On timeout/cancellation the error code is set accordingly.
   */
  DECLARE_DELEGATE_TwoParams(FPollPurchaseDelegate, const TOptional<TArray<int64>>&, const TOptional<FCFCoreError>&);

public:
  virtual ~ICFCorePremiumMods() = default;

public:
  /**
   * Use this as an alternative to setting the public key in the project
   * settings for security measures (e.g. not allowing ini overrides)
   */
  virtual void OverridePublicKey(const FString& public_key_pem) = 0;

  /**
   * Calls ICFCoreApiAuthorized::GetMyPremiumMods but also performs signature
   * checking.
   * @deprecated Use GetPremiumModsV2 instead.
   */
  UE_DEPRECATED(4.xx, "This function is deprecated. Use the GetPremiumModsV2 function instead.")
  virtual void GetPremiumMods(FGetPremiumModsDelegate delegate) = 0;

  /**
   * Calls ICFCoreApiAuthorized::GetMyPremiumModsV2.
   * Returns owned and trial premium mods for the current user.
   */
  virtual void GetPremiumModsV2(FGetPremiumModsV2Delegate delegate) = 0;

  /**
   * Calls ICFCoreApiAuthorized::PremiumModsCheck but also performs signature
   * checking.
   */
  virtual void CheckMods(const TArray<int64>& mod_ids,
                         FPremiumModsCheckDelegate delegate) = 0;

  /**
   * Calls ICFCoreApiAuthorized::DlcCheck but also performs signature
   * checking.
   */
  virtual void CheckDlc(const TArray<FString>& entitlement_ids,
                        FCheckDlcDelegate delegate) = 0;

  /**
   * The protected version of ICFCoreApiAuthorized::GetPremiumModFileDownloadUrl
   */
  virtual void GetModFileDownloadUrl(
    int64 mod_id,
    int64 file_id,
    FGetModFileDownloadUrlDelegate delegate) = 0;

  /**
   * Repeatedly calls CheckMods at the rate defined in |params| until all
   * requested mod ids are owned, the max duration elapses, or
   * StopPurchasePolling is called.
   *
   * This is designed to be used together with opening a browser purchase
   * page: start polling, open the store URL, and the delegate fires once the
   * purchase is confirmed (or the session times out).
   *
   * Only one polling session can be active at a time per instance. Starting
   * a new one while one is already running will first stop the previous one
   * (the previous delegate will NOT be fired).
   */
  virtual void PollModPurchase(const FPollPurchaseParams& params,
                               FPollPurchaseDelegate delegate) = 0;

  /**
   * Stops an active polling session started by PollModPurchase and fires
   * the delegate with a PurchasePollingStopped error.
   * Has no effect if no session is currently active.
   */
  virtual void StopPurchasePolling() = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCorePremiumMods)