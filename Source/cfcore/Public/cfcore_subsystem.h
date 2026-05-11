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
#include <Subsystems/EngineSubsystem.h>
#include <common/cfcore_error.h>
#include <models/cfcore_settings.h>
#include <models/cfcore_updatable_settings.h>

#include <api/models/game.h>
#include <api/models/game_version_type.h>
#include <api/models/game_version_by_type.h>
#include <api/models/category.h>
#include <api/models/mod.h>
#include <api/models/file.h>
#include <api/models/reporting_reason.h>
#include <api/models/me.h>
#include <api/models/my_ratings.h>
#include <api/models/external_auth_additional_info.h>
#include <api/models/blocked_details.h>
#include <api/models/unblock_mods_request.h>
#include <api/models/gen_premium_checkout_url_request.h>
#include <api/models/initiate_purchase_request.h>
#include <api/models/finalize_purchase_request.h>
#include <api/models/entitlements_redeem_code_request.h>
#include <api/models/entitlement_campaign.h>
#include <api/models/highlights/mods_highlights.h>
#include <api/models/highlights/mods_highlights_v2.h>
#include <api/models/highlights/mods_highlights_v3.h>
#include <api/models/premium/purchase_details.h>
#include <api/models/terms/terms.h>
#include <api/models/creation/create_mod_request.h>
#include <api/models/creation/update_mod_request.h>
#include <api/models/creation/create_mod_file_request.h>
#include <api/models/creation/create_cooked_mod_file_request.h>
#include <api/models/file_transfer_progress.h>
#include <api/models/creation/uploaded_mod_file.h>
#include <api/models/subscribe_request.h>
#include <api/models/subscribe_mods_request.h>
#include <api/models/diagnostics_init_data.h>

#include <api/models/enums/rating_vote_direction.h>

#include <api/models/filters/get_categories_filter.h>
#include <api/models/filters/search_mods_filter.h>
#include <api/models/filters/match_platform_files_filter.h>
#include <api/models/filters/get_mods_highlights_filter.h>

#include <api/common/cfcore_api_request_pagination.h>
#include <api/common/cfcore_api_response_pagination.h>

#include <subscription/models/subscription_sync_plan_item.h>
#include <subscription/models/subscription_sync_result_item.h>

#include <library/models/installed_mod.h>
#include <library/models/mods_dir_info.h>
#include <library/models/library_progress.h>
#include <library/models/mods_update_progress.h>
#include <library/models/assure_server_mods_updated_params.h>
#include <library/models/install_mod_additional_params.h>
#include <library/models/installed_mod_properties.h>

#include <api/models/premium/owned_premium_mods.h>

#include <premium/models/poll_purchase_params.h>

#include <analytics/models/game_play_session_params.h>
#include <analytics/models/mod_browsing_funnel_params.h>

#include <utils/compression/models/compression_error.h>
#include <utils/compression/models/compression_progress.h>

#include "cfcore_subsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreErrorDelegate, const FCFCoreError&, error);
DECLARE_DYNAMIC_DELEGATE(FCFCoreInitializeDelegate);
DECLARE_DYNAMIC_DELEGATE(FCFCoreUninitializeDelegate);
DECLARE_DYNAMIC_DELEGATE(FCFCoreSuccessDelegate);

// Api
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetGameDelegate, const FGame&, game);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetVersionTypesDelegate, const TArray<FGameVersionType>&, version_types);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetVersionsDelegate, const TArray<FGameVersionsByType>&, versions);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetCategoriesDelegate, const TArray<FCategory>&, categories);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FCFCoreApiSearchModsDelegate, const TArray<FCFCoreMod>&, mods, const FCFCoreApiResponsePagination&, pagination);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetModsHighlightsDelegate, const FModsHighlights&, mods);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetModsHighlightsV2Delegate, const FModsHighlightsV2&, mods);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetModsHighlightsV3Delegate, const FModsHighlightsV3&, mods);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetModsDelegate, const TArray<FCFCoreMod>&, mods);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetModDelegate, const FCFCoreMod&, mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetModDescDelegate, const FString&, description);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetFilesDelegate, const TArray<FFile>&, files);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetReportingReasonsDelegate, const TArray<FReportingReason>&, reasons);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetMeDelegate, const FMe&, me);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetMyRatingsDelegate, const FMyRatings&, ratings);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGenerateTempTokenDelegate, const FString&, temp_user_token);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGetBlockedDetailsDelegate, const FBlockedDetails&, blocked_details);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiUnblockModsDelegate, bool, success);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiGeneratePremiumCheckoutUrlDelegate, const FString&, url);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreInitiatePurchaseDelegate, const FPurchaseDetails&, purchase_details);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreEntitlmentsRedeemCodeDelegate, const FEntitlementCampaign&, EntitlementCampaign);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FCFCoreApiGetModsSubscriptionsDelegate, const TArray<FCFCoreMod>&, mods, const FCFCoreApiResponsePagination&, pagination);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FCFCoreApiGetModsSubscriptionsIdsDelegate, const TArray<int64>&, ids, const FCFCoreApiResponsePagination&, pagination);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiSubscribeDelegate, const FCFCoreMod&, mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreApiDiagnosticsInitDelegate, const FDiagnosticsInitData&, init_data);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreSubscriptionSyncDelegate, const TArray<FSubscriptionSyncResultItem>&, result_items);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreSubscriptionGetSyncPlanDelegate, const TArray<FSubscriptionSyncPlanItem>&, plan_items);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreSubscriptionExecuteSyncPlanDelegate, const TArray<FSubscriptionSyncResultItem>&, result_items);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreSubscriptionSubscribeDelegate, const FCFCoreMod&, mod);


// Library
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreLibGetInstalledModsDelegate, const TArray<FInstalledMod>&, installed_mods);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreGetModsDirInfoDelegate, const FModsDirInfo&, ModsDirInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreLibInstallDelegate, const FInstalledMod&, installed_mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreLibInstallProgressDelegate, const FLibraryProgress&, progress);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreLibUninstallDelegate, const FInstalledMod&, uninstalled_mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreLibPerformModsValidationDelegate, const TArray<FInstalledMod>&, invalid_installed_mods);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFCoreLibOnModInstallProgressDelegate, const FLibraryProgress&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFCoreLibModInstalledDelegate, const FInstalledMod&, InstalledMod);

// Library ClientServer
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FCFCoreLibClientServerUpdateProgressDelegate, const FModsUpdateProgress&, UpdateProgress, const FLibraryProgress&, ModInstallProgress, const FCFCoreMod&, Mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreLibClientServerUpdateDelegate, const TArray<FInstalledMod>&, installed_mods);

// Authentication
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreGetAuthTerms, const FTerms&, terms);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreIsAuthenticatedDelegate, bool, is_authenticated);

// Creation
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreCreateModDelegate, const FCFCoreMod&, mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCFCoreUpdateModDelegate, const FCFCoreMod&, mod);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCreateModFileRequestIdDelegate, int64, mod_file_request_id);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCreateModFileProgressDelegate, const FFileTransferProgress&, progress);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCreateModFileDelegate, const FUploadedModFile&, uploaded_file);

// Premium Mods
DECLARE_DYNAMIC_DELEGATE_OneParam(FPremiumModsCheckDelegate, const TArray<int64>&, ModIds);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPremiumModsCheckDlcDelegate, const TArray<int64>&, ModIds);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPollModPurchaseDelegate, const TArray<int64>&, OwnedModIds);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyPremiumModsV2Delegate, const FOwnedPremiumMods&, OwnedPremiumMods);

// Utils
DECLARE_DYNAMIC_DELEGATE_OneParam(FUtilsCompressionProgressDelegate, const FCompressionProgress&, Progress);
DECLARE_DYNAMIC_DELEGATE_OneParam(FUtilsCompressionErrorDelegate, ECompressionError, CompressionError);

// -----------------------------------------------------------------------------
UCLASS(MinimalAPI)
class UCFCoreSubsystem : public UEngineSubsystem {
	GENERATED_BODY()

public:
  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

public:
  // ---------------------------------------------------------------------------
  // Api
  UFUNCTION(BlueprintCallable, DisplayName="Initialize", Category="cfcore", Meta=(AutoCreateRefTerm="OnInitialized, OnError"))
  CFCORE_API void Initialize(const FCFCoreSettings& Settings,
                             const FCFCoreInitializeDelegate& OnInitialized,
                             const FCFCoreErrorDelegate& OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Uninitialize", Category="cfcore", Meta=(AutoCreateRefTerm="OnUninitialized, OnError"))
  CFCORE_API void Unitialize(const FCFCoreUninitializeDelegate& OnUninitialized,
                             const FCFCoreErrorDelegate& OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Update Settings", Category="cfcore", Meta=(AutoCreateRefTerm="OnSuccess, OnError"))
  CFCORE_API void UpdateSettings(const FCFCoreUpdatableSettings& InSettings,
                                 const FCFCoreSuccessDelegate& OnSuccess,
                                 const FCFCoreErrorDelegate& OnError);

  // https://docs.curseforge.com/#get-game
  UFUNCTION(BlueprintCallable, DisplayName="Get Game Info", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetGame(FCFCoreApiGetGameDelegate on_game,
                             const FCFCoreErrorDelegate& on_error);

  // https://docs.curseforge.com/#get-version-types
  UFUNCTION(BlueprintCallable, DisplayName="Get Version Types Info", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetVersionTypes(FCFCoreApiGetVersionTypesDelegate on_results,
                                     const FCFCoreErrorDelegate& on_error);

  // https://docs.curseforge.com/#get-versions
  UFUNCTION(BlueprintCallable, DisplayName="Get Versions Info", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetVersions(FCFCoreApiGetVersionsDelegate on_results,
                                 const FCFCoreErrorDelegate& on_error);

  // https://docs.curseforge.com/#get-categories
  UFUNCTION(BlueprintCallable, DisplayName="Get Categories Info", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetCategories(FCFCoreGetCategoriesFilter filter,
                                   FCFCoreApiGetCategoriesDelegate on_results,
                                   const FCFCoreErrorDelegate& on_error);

  // https://docs.curseforge.com/#search-mods
  UFUNCTION(BlueprintCallable, DisplayName="Search Mods Info", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiSearchMods(FCFCoreSearchModsFilter filter,
                                FCFCoreApiRequestPagination pagination,
                                FCFCoreApiSearchModsDelegate on_results,
                                const FCFCoreErrorDelegate& on_error);

  /**
   * This API is used for creating a "Discover" or "Homepage" interface - where
   * mods and categories can be promoted.
   * It requires enablement on console.curseforge.com.
   *
   * NOTE: This function is now deprecated - please use ApiGetModsHighlightsV2
   */
  UFUNCTION(BlueprintCallable, DisplayName="Get Mods Highlights", Category="cfcore|Api", Meta=(AutoCreateRefTerm="OnError", DeprecatedFunction, DeprecationMessage = "This function is deprecated. Use the GetModsHighlightsV3 function instead."))
  CFCORE_API void ApiGetModsHighlights(FCFCoreGetModsHighlightsFilter InFilter,
                                       FCFCoreApiGetModsHighlightsDelegate OnResults,
                                       const FCFCoreErrorDelegate& OnError);

  /**
   * This API is used for creating a "Discover" or "Homepage" interface - where
   * mods and categories can be promoted.
   * It requires enablement on console.curseforge.com.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Get Mods Highlights V2", Category="cfcore|Api", Meta=(AutoCreateRefTerm="OnError", DeprecatedFunction, DeprecationMessage = "This function is deprecated. Use the GetModsHighlightsV3 function instead."))
  CFCORE_API void ApiGetModsHighlightsV2(
    FCFCoreGetModsHighlightsFilter InFilter,
    FCFCoreApiGetModsHighlightsV2Delegate OnResults,
    const FCFCoreErrorDelegate& OnError);

  /**
   * This API is used for creating a "Discover" or "Homepage" interface - where
   * mods and categories can be promoted.
   * It requires enablement on console.curseforge.com.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Get Mods Highlights V3", Category="cfcore|Api", Meta=(AutoCreateRefTerm="OnError"))
  CFCORE_API void ApiGetModsHighlightsV3(
    FCFCoreGetModsHighlightsFilter InFilter,
    FCFCoreApiGetModsHighlightsV3Delegate OnResults,
    const FCFCoreErrorDelegate& OnError);

  // https://docs.curseforge.com/#get-mods
  UFUNCTION(BlueprintCallable, DisplayName="Get Mods Info By Ids", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetMods(const TArray<int64>& modIds,
                             FCFCoreApiGetModsDelegate on_results,
                             const FCFCoreErrorDelegate& on_error);

  // https://docs.curseforge.com/#get-mod
  UFUNCTION(BlueprintCallable, DisplayName="Get Mod Info By Id", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetMod(int64 modId,
                            FCFCoreApiGetModDelegate on_mod,
                            const FCFCoreErrorDelegate& on_error);

  // https://docs.curseforge.com/#get-mod-description
  UFUNCTION(BlueprintCallable, DisplayName="Get Mod Description By Id", Category="cfcore|Api", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetModDescription(int64 modId,
                                       FCFCoreApiGetModDescDelegate on_mod_desc,
                                       const FCFCoreErrorDelegate& on_error);

  /**
   * https://docs.curseforge.com/#get-files
   */
  UFUNCTION(BlueprintCallable, DisplayName="Get Files Info By Ids", Category="cfcore|Api", Meta=(AutoCreateRefTerm="OnError"))
  CFCORE_API void ApiGetFiles(const TArray<int64>& FileIds,
                              FCFCoreApiGetFilesDelegate OnResults,
                              FCFCoreErrorDelegate OnError);

  /**
   * When joining a server - given the server's file id list, match the file ids
   * that are required for the client (based on the platform of the client).
   */
  UFUNCTION(BlueprintCallable, DisplayName="Match Platform Files By Ids", Category="cfcore|Api", Meta=(AutoCreateRefTerm="OnError"))
  CFCORE_API void ApiMatchPlatformFiles(const FMatchPlatformFilesFilter& filter,
                                        FCFCoreApiGetFilesDelegate OnResults,
                                        FCFCoreErrorDelegate OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Get Me", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetMe(FCFCoreApiGetMeDelegate on_results,
                           const FCFCoreErrorDelegate& on_error);

  /**
   * Get all mods that the user created or is a member of, for the running game.
   *
   * NOTE: Although the delegates recieves FCFCoreMod objects - they will only
   * contain information about the mods and NOT information about mod files.
   * Should you want to retrieve file information - you can use the mod ids and
   * call GetMods
   */
  UFUNCTION(BlueprintCallable, DisplayName="Get My Mods", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetMyMods(FCFCoreApiGetModsDelegate on_results,
                               const FCFCoreErrorDelegate& on_error);

  // Get the mod ids for which the user has thumbed-up
  UFUNCTION(BlueprintCallable, DisplayName="Get My Ratings", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetMyRatings(FCFCoreApiGetMyRatingsDelegate on_results,
                                  const FCFCoreErrorDelegate& on_error);

  /**
   * Update the rating of a mod for the current user(up - vote / down - vote)
   */
  UFUNCTION(BlueprintCallable, DisplayName="Update Mod Rating", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_success, on_error"))
  CFCORE_API void ApiUpdateRating(int64 modId,
                                  ECFCoreRatingVoteDirection direction,
                                  FCFCoreSuccessDelegate on_success,
                                  const FCFCoreErrorDelegate& on_error);

  /**
   * Remove the rating of a mod for the current user
   */
  UFUNCTION(BlueprintCallable, DisplayName="Remove Mod Rating", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_success, on_error"))
  CFCORE_API void ApiRemoveRating(int64 modId,
                                  FCFCoreSuccessDelegate on_success,
                                  const FCFCoreErrorDelegate& on_error);

  // Gets an array of mod reporting reasons that can be used with the report api
  UFUNCTION(BlueprintCallable, DisplayName="Get Reporting Reasons", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetReportingReasons(
    FCFCoreApiGetReportingReasonsDelegate on_results,
    const FCFCoreErrorDelegate& on_error);

  // Reports a mod
  UFUNCTION(BlueprintCallable, DisplayName="Report Mod", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiReportMod(int64 modId,
                               int64 reasonId,
                               FString report,
                               FCFCoreSuccessDelegate on_reasons,
                               const FCFCoreErrorDelegate& on_error);

  /**
   * Generate a temp user token that can be used to authenticate user in
   * external services - for example when reporting a mod with our web-based
   * reporting pages.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Generate Temp User Token", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGenerateTempToken(
    FCFCoreApiGenerateTempTokenDelegate on_result,
    const FCFCoreErrorDelegate& on_error);

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
  UFUNCTION(BlueprintCallable, DisplayName="Get Blocked Mods Details", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void ApiGetBlockedModsDetails(
    FCFCoreApiGetBlockedDetailsDelegate on_result,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Allows to unblock mods - see "Get Blocked Mods Details" for getting blocked
   * mods.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Unblock Mods", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="OnError"))
  CFCORE_API void ApiUnblockMods(const FUnblockModsRequest& Request,
                                 FCFCoreApiUnblockModsDelegate OnResult,
                                 const FCFCoreErrorDelegate& OnError);

  /**
   * Generate a checkout url to purchase mods by their ids.
   * This is currently relevant only for PC/Steam and the url should be opened
   * in the user's default browser.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Generate Premium Checkout URL", Category="cfcore|Api Authorized", Meta=(AutoCreateRefTerm="OnError"))
  CFCORE_API void ApiGeneratePremiumCheckoutUrl(
    const FGenPremiumCheckoutUrlRequest& Request,
    const FCFCoreApiGeneratePremiumCheckoutUrlDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
   * When purchasing a mod on a device (e.g. console store), call this to get
   * specific information about the mod to be purchased.
   * Once the local device purchase is complete, call FinalizePurchase.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Initiate Purchase", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiInitiatePurchase(
    const FInitiatePurchaseRequest& Request,
    const FCFCoreInitiatePurchaseDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
   * After the purchase is complete on the device, call this function so that
   * the purchased mod is attributed to the user.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Finalize Purchase", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiFinalizePurchase(
    const FFinalizePurchaseRequest& Request,
    const FCFCoreSuccessDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
   * Redeems a code from the game's entitlement system (found on https://console.curseforge.com).
   * A successful result could mean the user will now own a premium mod (depending on the entitlement type)
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Entitlements Redeem Code", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiEntitlementsRedeemCode(
    const FEntitlementsRedeemCodeRequest& InRequest,
    const FCFCoreEntitlmentsRedeemCodeDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
   * Gets the mods that the user is subscribed to.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get Mods Subscriptions", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiGetModsSubscriptions(
    const FCFCoreApiRequestPagination& pagination,
    const FCFCoreApiGetModsSubscriptionsDelegate& on_subscriptions,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Returns the user's subscribed mods' IDs.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get Mods Subscriptions Ids", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiGetModsSubscriptionsIds(
    const FCFCoreApiRequestPagination& pagination,
    const FCFCoreApiGetModsSubscriptionsIdsDelegate& on_subscriptions_ids,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Subscribes to a mod.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Subscribe", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiSubscribe(
    FSubscribeRequest request,
    const FCFCoreApiSubscribeDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Subscribes to a list of mods.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Subscribe Mods", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiSubscribeMods(
    FSubscribeModsRequest request,
    const FCFCoreSuccessDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Unsubscribes from a mod.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Unsubscribe", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void ApiUnsubscribe(
    FSubscribeRequest request,
    const FCFCoreSuccessDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Initializes diagnostics by getting an upload URL.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Diagnostics Init", Category = "cfcore|Api Authorized", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void ApiDiagnosticsInit(
    const FCFCoreApiDiagnosticsInitDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  // ---------------------------------------------------------------------------
  // Subscriptions

  /**
   * Enables automatic subscription management.
   * While enabled, the SDK periodically checks the user's subscriptions,
   * compares them with the installed mods, and applies the allowed actions.
   *
   * This should not be used during gameplay. Prefer using it while the user is
   * in menus, launchers, or web UI.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Enable Auto Management", Category = "cfcore|Subscription", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void EnableAutoManagement(
    const FCFCoreSuccessDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Disables automatic subscription management.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Disable Auto Management", Category = "cfcore|Subscription")
  CFCORE_API void DisableAutoManagement();

  /**
   * Subscribes to a mod and installs it after subscribing succeeds.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Subscribe", Category = "cfcore|Subscription", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void SubscribeMod(
    const FSubscribeRequest& request,
    const FCFCoreSubscriptionSubscribeDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Unsubscribes from a mod and uninstalls it after unsubscribing succeeds.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Unsubscribe", Category = "cfcore|Subscription", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void UnsubscribeMod(
    const FSubscribeRequest& request,
    const FCFCoreSuccessDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Sync installed mods with the user's subscriptions.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Sync Subscriptions", Category = "cfcore|Subscription", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void SyncSubscriptions(
    const FCFCoreSubscriptionSyncDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Builds a sync plan based on installed mods and current subscriptions.
   * Useful for inspecting the plan before executing it.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get Sync Plan", Category = "cfcore|Subscription", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void GetSyncPlan(
    const FCFCoreSubscriptionGetSyncPlanDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Executes all actions from the current subscriptions sync plan.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Execute Sync Plan", Category = "cfcore|Subscription", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void ExecuteSyncPlan(
    const TArray<FSubscriptionSyncPlanItem>& plan_items,
    const FCFCoreSubscriptionExecuteSyncPlanDelegate& on_success,
    const FCFCoreErrorDelegate& on_error);

  // ---------------------------------------------------------------------------
  // Library
  UPROPERTY(BlueprintAssignable, Category = "cfcore|Library")
  FCFCoreLibOnModInstallProgressDelegate OnModInstallProgress;

  UPROPERTY(BlueprintAssignable, Category = "cfcore|Library")
  FCFCoreLibModInstalledDelegate OnModInstalled;


  UFUNCTION(BlueprintCallable, DisplayName="Get Installed Mods", Category="cfcore|Library", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void GetInstalledMods(
    FCFCoreLibGetInstalledModsDelegate on_installed_mods,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Get all mods installed on the machine.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get Global Installed Mods", Category = "cfcore|Library", Meta = (AutoCreateRefTerm = "on_error"))
  CFCORE_API void GetSystemInstalledMods(
    FCFCoreLibGetInstalledModsDelegate on_installed_mods,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Get information about the mods directory - like the actual directory on
   * disk and free disk space available.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get Mods Directory Info", Category = "cfcore|Library", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void GetModsDirInfo(
    FCFCoreGetModsDirInfoDelegate OnModsDirInfo,
    FCFCoreErrorDelegate OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Synchronize Installed Mods with Server", Category="cfcore|Library", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void SynchronizeWithServer(FCFCoreSuccessDelegate on_success,
                                        const FCFCoreErrorDelegate& on_error);

  /**
   * Performs local validation (for corruption/replacement/deletion) of
   * installed mods.
   *
   * Any mod that is invalid will be returned via the delegate and the local
   *
   * status of the mod will be set to Invalid and saved to the state of
   * the installed mods on disk - meaning, you may also check for invalid mods
   * by calling |GetInstalledMods| and checking each mod's status.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Perform Mods Validation", Category="cfcore|Library", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void PerformModsValidation(const TArray<FInstalledMod>& installed_mods,
                                        FCFCoreLibPerformModsValidationDelegate on_success,
                                        FCFCoreErrorDelegate on_error);

  /**
   * Update the properties of an array of installed mods.
   *
   * This is used to update the installed mod's properties, like if it is
   * enabled or the load order of the mod.
  */
  UFUNCTION(BlueprintCallable, DisplayName = "Update Installed Mods Properties", Category = "cfcore|Library", Meta = (AutoCreateRefTerm = "OnError"))
  virtual void UpdateInstalledModsProperties(
    const TArray<FInstalledModProperties>& InInstalledModsProperties,
    FCFCoreSuccessDelegate OnSuccess,
    FCFCoreErrorDelegate OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Install Mod", Category="cfcore|Library", Meta=(AutoCreateRefTerm="on_progress, on_installed, on_error"))
  CFCORE_API void InstallMod(
    FCFCoreMod mod,
    const FCFCoreLibInstallProgressDelegate& on_progress,
    const FCFCoreLibInstallDelegate& on_installed,
    const FCFCoreErrorDelegate& on_error);

  /**
   * Install mod with the option to specify a file to be installed (not the
   * latest) and other parameters.
   *
   * InFile is optional - if the File.id is 0, it will install the latest file,
   * which is similar to calling "Install Mod"
   */
  UFUNCTION(BlueprintCallable, DisplayName="Install Mod Extended", Category="cfcore|Library", Meta=(AutoCreateRefTerm="InFile, OnProgress, OnInstalled, OnError"))
  CFCORE_API void InstallModExtended(
    const FCFCoreMod& InMod,
    const FFile& InFile,
    const FInstallModAdditionalParams& InAdditionalParams,
    const FCFCoreLibInstallProgressDelegate& OnProgress,
    const FCFCoreLibInstallDelegate& OnInstalled,
    const FCFCoreErrorDelegate& OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Cancel Mod Installation", Category="cfcore|Library", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void CancelInstallation(int64 mod_id,
                                     FCFCoreSuccessDelegate on_success,
                                     FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Uninstall Mod", Category="cfcore|Library", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void UninstallMod(int64 mod_id,
                               FCFCoreLibUninstallDelegate on_uninstalled,
                               FCFCoreErrorDelegate on_error);

  /**
   * This will clean all stale files/folders from the temp directory to free up
   * some diskspace - it will not delete any files that are currently being
   * downloaded/installed
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Clean Temp Directory", Category = "cfcore|Library", Meta = (AutoCreateRefTerm = "OnError"))
  CFCORE_API void CleanTempDir(FCFCoreSuccessDelegate OnSuccess,
                               FCFCoreErrorDelegate OnError);

  // ---------------------------------------------------------------------------
  // Library - ClientServer

  /**
   * On a game server (for multi-platform mods) - assure we are up to date with
   * mods including a validity check of the mods.
   */
  UFUNCTION(BlueprintCallable, DisplayName="Assure Server Mods Updated", Category="cfcore|Library|ClientServer", Meta=(AutoCreateRefTerm="OnError"))
  CFCORE_API void AssureServerModsUpdated(
    const FAssureServerModsUpdatedParams& params,
    const FCFCoreLibClientServerUpdateProgressDelegate& OnProgress,
    const FCFCoreLibClientServerUpdateDelegate& OnUpdated,
    const FCFCoreErrorDelegate& OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Assure Client Mods Updated", Category="cfcore|Library|ClientServer", Meta=(AutoCreateRefTerm="OnError"))
  virtual void AssureClientModsUpdated(
    const TArray<int64>& ServerFileIds,
    const FCFCoreLibClientServerUpdateProgressDelegate& OnProgress,
    const FCFCoreLibClientServerUpdateDelegate& OnUpdated,
    const FCFCoreErrorDelegate& OnError);

  // ---------------------------------------------------------------------------
  // Authentication
  UFUNCTION(BlueprintCallable, DisplayName="Is Authenticated", Category="cfcore|Authentication")
  CFCORE_API void IsAuthenticated(FCFCoreIsAuthenticatedDelegate on_is_auth);

  UFUNCTION(BlueprintCallable, DisplayName="Get Authentication Terms", Category="cfcore|Authentication")
  CFCORE_API void GetAuthTerms(FCFCoreGetAuthTerms on_success,
                               FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Send Security Code Email", Category="cfcore|Authentication")
  CFCORE_API void SendSecurityCode(const FString& email,
                                   FCFCoreSuccessDelegate on_success,
                                   FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Generate Auth Token from Email Code", Category="cfcore|Authentication", Meta=(AutoCreateRefTerm="on_success, on_error"))
  CFCORE_API void GenerateAuthToken(const FString& email,
                                    int32 security_code,
                                    FCFCoreSuccessDelegate on_success,
                                    FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Generate Auth Token for an external provider", Category="cfcore|Authentication", Meta=(AutoCreateRefTerm="on_success, on_error"))
  CFCORE_API void GenerateAuthTokenByExternalProvider(
    ECFCoreExternalAuthProvider provider,
    const FString& external_token,
    const FExternalAuthAdditionalInfo& additional_info,
    FCFCoreSuccessDelegate on_success,
    FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Logout", Category="cfcore|Authentication", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void Logout(FCFCoreSuccessDelegate on_success,
                         FCFCoreErrorDelegate on_error);

  // ---------------------------------------------------------------------------
  // Creation
  UFUNCTION(BlueprintCallable, DisplayName="Create New Mod", Category="cfcore|Creation", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void CreateMod(const FCreateModRequest& create_mod_request,
                            const FString& avatar_image_filename,
                            FCFCoreCreateModDelegate on_success,
                            FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Update Existing Mod", Category="cfcore|Creation", Meta=(AutoCreateRefTerm="on_error"))
  CFCORE_API void UpdateMod(int64 mod_id,
                            const FUpdateModRequest& update_mod_request,
                            const FString& avatar_image_filename,
                            FCFCoreUpdateModDelegate on_success,
                            FCFCoreErrorDelegate on_error);

  UFUNCTION(BlueprintCallable, DisplayName="Create Mod File", Category="cfcore|Creation", Meta=(AutoCreateRefTerm="on_create_mod_file_request_id, on_progress, on_error"))
  CFCORE_API void CreateModFile(
    int64 ModId,
    const FCreateModFileRequest& CreateModFileRequest,
    const FString& LocalFilenameToUpload,
    const FCreateModFileRequestIdDelegate& OnCreateModFileRequestId,
    const FCreateModFileProgressDelegate& OnProgress,
    const FCreateModFileDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  UFUNCTION(BlueprintCallable, DisplayName="Create Cooked Mod File", Category="cfcore|Creation", Meta=(AutoCreateRefTerm="on_create_mod_file_request_id, on_progress, on_error"))
  CFCORE_API void CreateCookedModFile(
    int64 ModId,
    int64 SourceFileId,
    const FCreateCookedModFileRequest& CreateCookedModFileRequest,
    const FString& LocalFilenameToUpload,
    const FCreateModFileRequestIdDelegate& OnCreateModFileRequestId,
    const FCreateModFileProgressDelegate& OnProgress,
    const FCreateModFileDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  // ---------------------------------------------------------------------------
  // Premium Mods

  /**
   * Use this as an alternative to setting the public key in the project
   * settings for security measures (e.g. not allowing ini overrides).
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Override Public Key", Category = "cfcore|Premium Mods")
  CFCORE_API void OverridePublicKey(const FString& InPublicKeyPem,
                                    const FCFCoreSuccessDelegate& OnSuccess,
                                    const FCFCoreErrorDelegate& OnError);

  /**
   * Returns the mod ids that are owned by the user - meaning the user can
   * install them. Mods that are not in the list will be required to be
   * purchased first.
   * @deprecated Use GetMyPremiumModsV2 instead.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get My Premium Mods", Category = "cfcore|Premium Mods", Meta = (DeprecatedFunction, DeprecationMessage = "Use GetMyPremiumModsV2 instead."))
  CFCORE_API void GetMyPremiumMods(const FPremiumModsCheckDelegate& OnSuccess,
                                   const FCFCoreErrorDelegate& OnError);

  /**
   * Returns owned and trial premium mods for the user.
   * Uses the v2 endpoint which also includes trial mod data.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Get My Premium Mods V2", Category = "cfcore|Premium Mods")
  CFCORE_API void GetMyPremiumModsV2(const FGetMyPremiumModsV2Delegate& OnSuccess,
                                     const FCFCoreErrorDelegate& OnError);

  /**
   * Given an array of mod ids, this method will return the mod ids that are
   * owned by the user - meaning the user can install them. Mods that are not in
   * the list will be required to be purchased first.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Premium Mods Check", Category = "cfcore|Premium Mods")
  CFCORE_API void PremiumModsCheck(const TArray<int64>& InModIds,
                                   const FPremiumModsCheckDelegate& OnSuccess,
                                   const FCFCoreErrorDelegate& OnError);

  /**
   * Given an array of entitlement IDs, this method will return the mod ids of
   * premium-mod-as-DLCs that the user owns.
   * In order to use this, you have to contact us first.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Premium Mods Check Dlc", Category = "cfcore|Premium Mods")
  CFCORE_API void PremiumModsCheckDlc(
    const TArray<FString>& InEntitlementIds,
    const FPremiumModsCheckDlcDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
   * Repeatedly calls PremiumModsCheck at a set interval until all requested
   * mod ids are owned by the user, the max polling duration elapses, or
   * StopPurchasePolling is called.
   *
   * Intended use: open the store checkout URL (e.g. from
   * ApiGeneratePremiumCheckoutUrl), then call this so the UI reacts as soon
   * as the purchase is confirmed.
   *
   * OnPurchasePollingTimeout and OnPurchasePollingStopped are fired instead of
   * OnError when the session ends due to timeout or manual cancellation.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Poll Mod Purchase", Category = "cfcore|Premium Mods", Meta = (AutoCreateRefTerm = "OnError, OnPurchasePollingTimeout, OnPurchasePollingStopped"))
  CFCORE_API void PollModPurchase(
    const FPollPurchaseParams& InParams,
    const FPollModPurchaseDelegate& OnPurchased,
    const FCFCoreErrorDelegate& OnError,
    const FCFCoreSuccessDelegate& OnPurchasePollingTimeout,
    const FCFCoreSuccessDelegate& OnPurchasePollingStopped);

  /**
   * Stops an active polling session started by PollModPurchase.
   * The OnPurchasePollingStopped delegate passed to PollModPurchase will be
   * fired. Has no effect if no session is currently active.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Stop Purchase Polling", Category = "cfcore|Premium Mods")
  CFCORE_API void StopPurchasePolling();

  // ---------------------------------------------------------------------------
  // Analytics

  /**
   * Send this after a game session ends so that it appears in your console
   * reports.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Send Game Play Session Analytic", Category = "cfcore|Analytics")
  CFCORE_API void AnalyticsSendGamePlaySession(
    const FGamePlaySessionParams& InParams,
    const FCFCoreSuccessDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
   * Used for sending mod browser analytics.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Send Mod Browsing Funnel Impression Analytic", Category = "cfcore|Analytics")
  CFCORE_API void AnalyticsSendModBrowsingFunnelImpression(
    const FModBrowsingFunnelParams& InParams,
    const FCFCoreSuccessDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  /**
  * Used for sending mod browser analytics.
  */
  UFUNCTION(BlueprintCallable, DisplayName = "Send Mod Browsing Funnel Action Analytic", Category = "cfcore|Analytics")
  CFCORE_API void AnalyticsSendModBrowsingFunnelAction(
    const FModBrowsingFunnelParams& InParams,
    const FCFCoreSuccessDelegate& OnSuccess,
    const FCFCoreErrorDelegate& OnError);

  // ---------------------------------------------------------------------------
  // Utils

  /**
   * Zips the specified paths into a single zip file.
   */
  UFUNCTION(BlueprintCallable, DisplayName = "Zip Paths", Category = "cfcore|Utils|Compression")
  CFCORE_API void UtilsCompressionZipPaths(
    const TArray<FString>& InPathsToZip,
    const FString& InOutputZipFile,
    const FUtilsCompressionProgressDelegate& OnProgress,
    const FCFCoreSuccessDelegate& OnSuccess,
    const FUtilsCompressionErrorDelegate& OnError);

private:
  FCFCoreError CreateFailedToInitializeError();

  void RegisterCFCoreLibraryDelegates();
};
