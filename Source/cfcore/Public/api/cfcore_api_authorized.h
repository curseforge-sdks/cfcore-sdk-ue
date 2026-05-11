/*MIT License

Copyright (c) 2022 Overwolf Ltd.

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
#include <api/common/cfcore_api_response_error.h>
#include <api/common/cfcore_api_request_pagination.h>
#include <api/common/cfcore_api_response_pagination.h>

#include "models/me.h"
#include "models/mod.h"
#include "models/enums/rating_vote_direction.h"
#include "models/reporting_reason.h"
#include "models/blocked_details.h"
#include "models/unblock_mods_request.h"
#include "models/my_ratings.h"
#include "models/gen_premium_checkout_url_request.h"
#include "models/initiate_purchase_request.h"
#include "models/finalize_purchase_request.h"
#include "models/premium/purchase_details.h"
#include "models/premium/owned_premium_mods.h"
#include "models/entitlement_campaign.h"
#include "models/entitlements_redeem_code_request.h"
#include "models/subscribe_request.h"
#include "models/subscribe_mods_request.h"
#include "models/diagnostics_init_data.h"

namespace cfcore {

// -----------------------------------------------------------------------------
// APIs that require a valid token.
// These APIs can only be called after being authorized (authenticated)
class ICFCoreApiAuthorized {
public:
  DECLARE_DELEGATE_TwoParams(FGetMeDelegate, const TOptional<FMe>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetMyModsDelegate, const TOptional<TArray<FCFCoreMod>>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetMyRatingsDelegate, const TOptional<FMyRatings>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FUpdateRatingDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FRemoveRatingDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetReportingReasonsDelegate, const TOptional<TArray<FReportingReason>>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FReportModDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGenerateTempTokenDelegate, const TOptional<FString>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetBlockedDetailsDelegate, const TOptional<FBlockedDetails>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FUnblockModsDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FGetMyPreimumModsDelegate, const TOptional<TArray<int64>>&, const FString&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FGetMyPremiumModsV2Delegate, const TOptional<FOwnedPremiumMods>&, const FString&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FPremiumModsCheckDelegate, const TOptional<TArray<int64>>&, const FString&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FCheckDlcDelegate, const TOptional<TArray<int64>>&, const FString&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGeneratePremiumCheckoutUrlDelegate, const TOptional<FString>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FInitiatePurchaseDelegate, const TOptional<FPurchaseDetails>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FFinalizePurchaseDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FEntitlementsRedeemCodeDelegate, const TOptional<FEntitlementCampaign>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FGetModsSubscriptionsDelegate, const TOptional<TArray<FCFCoreMod>>&, const TOptional<FCFCoreApiResponsePagination>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FGetModsSubscriptionsIdsDelegate, const TOptional<TArray<int64>>&, const TOptional<FCFCoreApiResponsePagination>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FSubscribeDelegate, const TOptional<FCFCoreMod>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FSubscribeModsDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FUnsubscribeDelegate, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FDiagnosticsInitDelegate, const TOptional<FDiagnosticsInitData>&, const TOptional<FCFCoreApiResponseError>&);


public:
  virtual ~ICFCoreApiAuthorized() = default;

public:
  /**
   * Get information about the authenticated user (the user profile)
   */
  virtual void GetMe(FGetMeDelegate delegate) = 0;

  /**
   * Get all mods that the user created or is a member of, for the running game.
   *
   * NOTE: Although the delegates recieves FCFCoreMod objects - they will only
   * contain information about the mods and NOT information about mod files.
   * Should you want to retrieve file information - you can use the mod ids and
   * call GetMods
   */
  virtual void GetMyMods(FGetMyModsDelegate delegate) = 0;

  /**
   * Returns the modIds (of the current game) for which I've upvoted and
   * downvoted (in two seprate arrays)
   */
  virtual void GetMyRatings(FGetMyRatingsDelegate delegate) = 0;

  virtual void UpdateRating(int64 mod_id,
                            ECFCoreRatingVoteDirection direction,
                            FUpdateRatingDelegate delegate) = 0;

  virtual void RemoveRating(int64 mod_id, FRemoveRatingDelegate delegate) = 0;

  virtual void GetReportingReasons(FGetReportingReasonsDelegate delegate) = 0;

  virtual void ReportMod(int64 mod_id,
                         int64 reasonId,
                         FString report,
                         FReportModDelegate delegate) = 0;

  /**
   * Generate a temp user token that can be used to authenticate user in
   * external services - for example when reporting a mod with our web-based
   * reporting pages.
   */
  virtual void GenerateTempToken(FGenerateTempTokenDelegate delegate) = 0;

  /**
   * This function relates to modded game servers. A user can report a server
   * and/or mods a server so that they either block the server or all servers
   * that contain blocked mods.
   *
   * The function will return:
   *   1. A list of server ids that have been blocked by the user
   *   2. A list of mod ids that have been blocked by the user in the context
   *      of modded servers.
   *
   * Both lists are in the context of the current game only.
   */
  virtual void GetBlockedDetails(
    FGetBlockedDetailsDelegate delegate) = 0;

  /**
   * Unblock mods/servers that were blocked by the current user for
   * the current game - by passing the ids.
   */
  virtual void UnblockMods(const FUnblockModsRequest& request,
                           FUnblockModsDelegate delegate) = 0;

  /**
   * Returns a list of mods ids owned by the current user for the current game.
   * Optionally protected by a signature.
   * @deprecated Use GetMyPremiumModsV2 instead.
   */
  UE_DEPRECATED(4.xx, "This function is deprecated. Use the GetMyPremiumModsV2 function instead.")
  virtual void GetMyPremiumMods(FGetMyPreimumModsDelegate delegate) = 0;

  /**
   * Returns owned and trial premium mods for the current user and game.
   * Uses /v2/users/me/premium-mods.
   */
  virtual void GetMyPremiumModsV2(FGetMyPremiumModsV2Delegate delegate) = 0;

  /**
   * Given an array of mod ids - check which ones are owned by the current user.
   * Returns a signature to be validated for extra security.
   */
  virtual void PremiumModsCheck(const TArray<int64>& mod_ids,
                                FPremiumModsCheckDelegate delegate) = 0;

  /**
   * Generate a checkout url to purchase mods by their ids.
   *
   * NOTE: This is only applicable for platforms that support direct web browser
   * purchase flows (usually PC only).
   * For consoles, use the InitiatePurchase/FinalizePurchase functions.
   */
  virtual void GeneratePremiumCheckoutUrl(
    const FGenPremiumCheckoutUrlRequest& request,
    FGeneratePremiumCheckoutUrlDelegate delegate) = 0;

  /**
   * When purchasing a mod on a device (e.g. console store), call this to get
   * specific information about the mod to be purchased.
   * Once the local device purchase is complete, call FinalizePurchase.
   */
  virtual void InitiatePurchase(const FInitiatePurchaseRequest& request,
                                FInitiatePurchaseDelegate delegate) = 0;


  /**
   * After the purchase is complete on the device, call this function so that
   * the purchased mod is attributed to the user.
   */
  virtual void FinalizePurchase(const FFinalizePurchaseRequest& request,
                                FFinalizePurchaseDelegate delegate) = 0;

  /**
   * Redeem a code with the CF entitlement system. This might result in
   * having the user get ownership of premium content.
   */
  virtual void EntitlementsRedeemCode(
    const FEntitlementsRedeemCodeRequest& request,
    FEntitlementsRedeemCodeDelegate delegate) = 0;

  /**
   * Check which premium mods the user owns based on external DLC entitlements.
   * This might result in the user getting ownership of premium mods.
   * Returns a signature to be validated for extra security.
   */
  virtual void CheckDlc(const TArray<FString>& entitlement_ids,
                        FCheckDlcDelegate delegate) = 0;

  /**
   * Returns a list of mods that the user is subscribed to.
   */
  virtual void GetModsSubscriptions(
    const FCFCoreApiRequestPagination& pagination,
    FGetModsSubscriptionsDelegate delegate) = 0;

  /**
   * Returns the user's subscribed mods' IDs.
   */
  virtual void GetModsSubscriptionsIds(
    const FCFCoreApiRequestPagination& pagination,
    FGetModsSubscriptionsIdsDelegate delegate) = 0;

  /**
   * Subscribes to a mod.
   */
  virtual void Subscribe(const FSubscribeRequest& request,
                         FSubscribeDelegate delegate) = 0;

  /**
   * Subscribes to a list of mods (Max 50).
   */
  virtual void SubscribeMods(const FSubscribeModsRequest& request,
                             FSubscribeModsDelegate delegate) = 0;

  /**
   * Unsubscribes from a mod.
   */
  virtual void Unsubscribe(const FSubscribeRequest& request,
                           FUnsubscribeDelegate delegate) = 0;

  /**
   * For internal use - initializes diagnostics by getting an upload URL.
   */
  virtual void DiagnosticsInit(FDiagnosticsInitDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreApiAuthorized)