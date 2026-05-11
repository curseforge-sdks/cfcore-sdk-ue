/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include <common/macros.h>

#include <api/cfcore_api_authorized.h>
#include "cfcore_api_delegates_dispatcher.h"
#include "cfcore_api_authorized_delegates.h"

namespace cfcore {
class ISettingsService;
class IApiHelperService;

using ApiAuthDelegatesDispatcher =
  CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>;

// -----------------------------------------------------------------------------
class CFCoreApiAuthorizedImpl : public ICFCoreApiAuthorized {

 public:
   CFCoreApiAuthorizedImpl(
     TSharedRef<ISettingsService> settings_service,
     TSharedRef<IApiHelperService> api_helper_service,
     TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher);

// -----------------------------------------------------------------------------
// ICFCoreApiAuthorized
public:
  virtual void GetMe(FGetMeDelegate delegate) override;

  virtual void GetMyMods(FGetMyModsDelegate delegate) override;

  virtual void GetMyRatings(FGetMyRatingsDelegate delegate) override;

  virtual void UpdateRating(int64 mod_id,
                            ECFCoreRatingVoteDirection direction,
                            FUpdateRatingDelegate delegate) override;

  virtual void RemoveRating(int64 mod_id,
                            FRemoveRatingDelegate delegate) override;

  virtual void GetReportingReasons(
    FGetReportingReasonsDelegate delegate) override;

  virtual void ReportMod(int64 mod_id,
                         int64 reasonId,
                         FString report,
                         FReportModDelegate delegate) override;

  virtual void GenerateTempToken(FGenerateTempTokenDelegate delegate) override;

  virtual void GetBlockedDetails(
    FGetBlockedDetailsDelegate delegate) override;

  virtual void UnblockMods(const FUnblockModsRequest& request,
                           FUnblockModsDelegate delegate) override;

  virtual void GetMyPremiumMods(FGetMyPreimumModsDelegate delegate) override;

  virtual void GetMyPremiumModsV2(FGetMyPremiumModsV2Delegate delegate) override;

  virtual void PremiumModsCheck(const TArray<int64>& mod_ids,
                                FPremiumModsCheckDelegate delegate) override;

  virtual void GeneratePremiumCheckoutUrl(
    const FGenPremiumCheckoutUrlRequest& request,
    FGeneratePremiumCheckoutUrlDelegate delegate) override;

  virtual void InitiatePurchase(const FInitiatePurchaseRequest& request,
                                FInitiatePurchaseDelegate delegate) override;

  virtual void FinalizePurchase(const FFinalizePurchaseRequest& request,
                                FFinalizePurchaseDelegate delegate) override;

  virtual void EntitlementsRedeemCode(
    const FEntitlementsRedeemCodeRequest& request,
    FEntitlementsRedeemCodeDelegate delegate) override;

  virtual void CheckDlc(const TArray<FString>& entitlement_ids,
                        FCheckDlcDelegate delegate) override;

  virtual void GetModsSubscriptions(
    const FCFCoreApiRequestPagination& pagination,
    FGetModsSubscriptionsDelegate delegate) override;

  virtual void GetModsSubscriptionsIds(
    const FCFCoreApiRequestPagination& pagination,
    FGetModsSubscriptionsIdsDelegate delegate) override;

  virtual void Subscribe(const FSubscribeRequest& request,
                         FSubscribeDelegate delegate) override;

  virtual void SubscribeMods(const FSubscribeModsRequest& request,
                             FSubscribeModsDelegate delegate) override;

  virtual void Unsubscribe(const FSubscribeRequest& request,
                           FUnsubscribeDelegate delegate) override;

  virtual void DiagnosticsInit(FDiagnosticsInitDelegate delegate) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CFCoreApiAuthorizedImpl);

  void InitializeSettings();

  void LogMe(const TOptional<FMe>& opt_me);

private:
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IApiHelperService> api_helper_service_;
  TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher_;

  FString api_base_url_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreApiAuthorizedImpl)