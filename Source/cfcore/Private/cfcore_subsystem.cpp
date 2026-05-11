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
#include <cfcore_subsystem.h>
#include "common/cfcore_log.h"
#include <cfcore_context.h>
#include "editor/cfcore_editor_settings.h"

using namespace cfcore;

#define ASSERT_INITIALIZED(on_error) \
  if (!CFCoreContext::GetInstance()->IsInitialized()) { \
    on_error.ExecuteIfBound(CreateFailedToInitializeError()); \
    return; \
  }

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::Deinitialize() {
  Super::Deinitialize();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::Initialize(const FCFCoreSettings& Settings,
                                  const FCFCoreInitializeDelegate& OnInitialized,
                                  const FCFCoreErrorDelegate& OnError) {

  CFCoreContext::GetInstance()->Initialize(
    Settings,
    ICFCore::FInitializeDelegate::CreateLambda(
      [this, OnError, OnInitialized](TOptional<FCFCoreError> OptErr) {
        if (OptErr.IsSet()) {
          OnError.ExecuteIfBound(OptErr);
          return;
        }

        RegisterCFCoreLibraryDelegates();

        OnInitialized.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::Unitialize(
  const FCFCoreUninitializeDelegate& OnUninitialized,
  const FCFCoreErrorDelegate& OnError) {

  CFCoreContext::GetInstance()->Uninitialize(
    ICFCore::FUninitializeDelegate::CreateLambda(
      [OnError, OnUninitialized](TOptional<FCFCoreError> OptErr) {
        if (OptErr.IsSet()) {
          OnError.ExecuteIfBound(OptErr);
          return;
        }

        OnUninitialized.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::UpdateSettings(
  const FCFCoreUpdatableSettings& InSettings,
  const FCFCoreSuccessDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->UpdateSettings(
    InSettings,
    ICFCore::FUpdateSettingsDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<FCFCoreError>& OptErr) {
        if (OptErr.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(OptErr));
          return;
        }

        OnSuccess.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetGame(FCFCoreApiGetGameDelegate on_game,
                                  const FCFCoreErrorDelegate& on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetGame(
    ICFCoreApi::FGetGameDelegate::CreateLambda(
      [on_game, on_error](TOptional<FGame> opt_game,
                          TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_game.ExecuteIfBound(opt_game.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetVersionTypes(
  FCFCoreApiGetVersionTypesDelegate on_results,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetVersionTypes(
    ICFCoreApi::FGetVersionTypesDelegate::CreateLambda(
      [on_results, on_error](TOptional<TArray<FGameVersionType>> opt_ver_types,
                             TOptional<FCFCoreApiResponseError> opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_results.ExecuteIfBound(opt_ver_types.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetVersions(FCFCoreApiGetVersionsDelegate on_results,
                                      const FCFCoreErrorDelegate& on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetVersions(
    ICFCoreApi::FGetVersionsDelegate::CreateLambda(
      [on_results, on_error](TOptional<TArray<FGameVersionsByType>> opt_vers,
                             TOptional<FCFCoreApiResponseError> opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_results.ExecuteIfBound(opt_vers.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetCategories(
  FCFCoreGetCategoriesFilter filter,
  FCFCoreApiGetCategoriesDelegate on_results,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetCategories(
    filter,
    ICFCoreApi::FGetCategoriesDelegate::CreateLambda(
      [on_results, on_error](TOptional<TArray<FCategory>> opt_categories,
                             TOptional<FCFCoreApiResponseError> opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_results.ExecuteIfBound(opt_categories.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiSearchMods(FCFCoreSearchModsFilter filter,
                                     FCFCoreApiRequestPagination pagination,
                                     FCFCoreApiSearchModsDelegate on_results,
                                     const FCFCoreErrorDelegate& on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->SearchMods(
    filter,
    pagination,
    ICFCoreApi::FSearchModsDelegate::CreateLambda(
      [on_results, on_error](
        TOptional<TArray<FCFCoreMod>> opt_mods,
        TOptional<FCFCoreApiResponsePagination> opt_pagination,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_results.ExecuteIfBound(opt_mods.GetValue(),
                                  opt_pagination.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetModsHighlights(
  FCFCoreGetModsHighlightsFilter InFilter,
  FCFCoreApiGetModsHighlightsDelegate OnResults,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  PRAGMA_DISABLE_DEPRECATION_WARNINGS
  CFCoreContext::GetInstance()->Api()->GetModsHighlights(
  PRAGMA_ENABLE_DEPRECATION_WARNINGS
    InFilter,
    ICFCoreApi::FGetModsHighlightsDelegate::CreateLambda(
      [OnResults, OnError](
        TOptional<FModsHighlights> mods_highlights,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(mods_highlights.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetModsHighlightsV2(
  FCFCoreGetModsHighlightsFilter InFilter,
  FCFCoreApiGetModsHighlightsV2Delegate OnResults,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  PRAGMA_DISABLE_DEPRECATION_WARNINGS
  CFCoreContext::GetInstance()->Api()->GetModsHighlightsV2(
  PRAGMA_ENABLE_DEPRECATION_WARNINGS
    InFilter,
    ICFCoreApi::FGetModsHighlightsV2Delegate::CreateLambda(
      [OnResults, OnError](
        TOptional<FModsHighlightsV2> mods_highlights,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(mods_highlights.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetModsHighlightsV3(
  FCFCoreGetModsHighlightsFilter InFilter,
  FCFCoreApiGetModsHighlightsV3Delegate OnResults,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  PRAGMA_DISABLE_DEPRECATION_WARNINGS
  CFCoreContext::GetInstance()->Api()->GetModsHighlightsV3(
  PRAGMA_ENABLE_DEPRECATION_WARNINGS
    InFilter,
    ICFCoreApi::FGetModsHighlightsV3Delegate::CreateLambda(
      [OnResults, OnError](
        TOptional<FModsHighlightsV3> mods_highlights,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(mods_highlights.GetValue());
      }
    )
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetMods(const TArray<int64>& mod_ids,
                                  FCFCoreApiGetModsDelegate on_results,
                                  const FCFCoreErrorDelegate& on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetMods(
    mod_ids,
    ICFCoreApi::FGetModsDelegate::CreateLambda(
      [on_results, on_error](TOptional<TArray<FCFCoreMod>> opt_mods,
                             const TOptional<FCFCoreApiResponseError>& opt_er) {
        if (opt_er.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_er));
          return;
        }

        on_results.ExecuteIfBound(opt_mods.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetMod(int64 modId,
                                 FCFCoreApiGetModDelegate on_mod,
                                 const FCFCoreErrorDelegate& on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetMod(
    modId,
    ICFCoreApi::FGetModDelegate::CreateLambda(
      [on_mod, on_error](TOptional<FCFCoreMod> opt_mod,
                         TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_mod.ExecuteIfBound(opt_mod.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetModDescription(
  int64 modId,
  FCFCoreApiGetModDescDelegate on_mod_desc,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->GetModDescription(
    modId,
    ICFCoreApi::FGetModDescDelegate::CreateLambda(
      [on_mod_desc, on_error](TOptional<FString> opt_mod_desc,
                              TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_mod_desc.ExecuteIfBound(opt_mod_desc.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetFiles(const TArray<int64>& FileIds,
                                  FCFCoreApiGetFilesDelegate OnResults,
                                  FCFCoreErrorDelegate OnError) {

  ASSERT_INITIALIZED(OnError);

  const FCFCoreGetFilesFilter filter{ FileIds };

  CFCoreContext::GetInstance()->Api()->GetFiles(
    filter,
    ICFCoreApi::FGetFilesDelegate::CreateLambda(
      [OnResults, OnError](const TOptional<TArray<FFile>>& opt_files,
                           const TOptional<FCFCoreApiResponseError>& opt_er) {
        if (opt_er.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_er));
          return;
        }

        OnResults.ExecuteIfBound(opt_files.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiMatchPlatformFiles(
  const FMatchPlatformFilesFilter& filter,
  FCFCoreApiGetFilesDelegate OnResults,
  FCFCoreErrorDelegate OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->MatchPlatformFiles(
    filter,
    ICFCoreApi::FMatchPlatformFilesDelegate::CreateLambda(
      [OnResults, OnError](const TOptional<TArray<FFile>>& opt_files,
                           const TOptional<FCFCoreApiResponseError>& opt_er) {
        if (opt_er.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_er));
          return;
        }

        OnResults.ExecuteIfBound(opt_files.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetMe(FCFCoreApiGetMeDelegate OnResults,
                                const FCFCoreErrorDelegate& OnError) {
  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetMe(
    ICFCoreApiAuthorized::FGetMeDelegate::CreateLambda(
      [OnError, OnResults](const TOptional<FMe>& me,
                           TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(me.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetMyMods(FCFCoreApiGetModsDelegate OnResults,
                                    const FCFCoreErrorDelegate& OnError) {
  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetMyMods(
    ICFCoreApiAuthorized::FGetMyModsDelegate::CreateLambda(
      [OnError, OnResults](const TOptional<TArray<FCFCoreMod>>& mods,
                           TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(mods.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetMyRatings(
  FCFCoreApiGetMyRatingsDelegate OnResults,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetMyRatings(
    ICFCoreApiAuthorized::FGetMyRatingsDelegate::CreateLambda(
      [OnError, OnResults](const TOptional<FMyRatings>& ratings,
                           TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(ratings.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiUpdateRating(int64 modId,
                                       ECFCoreRatingVoteDirection direction,
                                       FCFCoreSuccessDelegate on_success,
                                       const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->UpdateRating(
    modId,
    direction,
    ICFCoreApiAuthorized::FUpdateRatingDelegate::CreateLambda(
      [on_success, on_error](TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_success.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiRemoveRating(int64 modId,
                                       FCFCoreSuccessDelegate on_success,
                                       const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->RemoveRating(
    modId,
    ICFCoreApiAuthorized::FRemoveRatingDelegate::CreateLambda(
      [on_success, on_error](TOptional<FCFCoreApiResponseError> opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_success.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetReportingReasons(
  FCFCoreApiGetReportingReasonsDelegate OnResults,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetReportingReasons(
    ICFCoreApiAuthorized::FGetReportingReasonsDelegate::CreateLambda(
      [OnResults, OnError](
        const TOptional<TArray<FReportingReason>>& opt_reasons,
        TOptional<FCFCoreApiResponseError> opt_err) {

        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResults.ExecuteIfBound(opt_reasons.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiReportMod(int64 modId,
                                    int64 reasonId,
                                    FString report,
                                    FCFCoreSuccessDelegate OnSuccess,
                                    const FCFCoreErrorDelegate& OnError) {
  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->ReportMod(
    modId,
    reasonId,
    report,
    ICFCoreApiAuthorized::FReportModDelegate::CreateLambda(
      [OnError, OnSuccess](const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnSuccess.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGenerateTempToken(
  FCFCoreApiGenerateTempTokenDelegate on_result,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->GenerateTempToken(
    ICFCoreApiAuthorized::FGenerateTempTokenDelegate::CreateLambda(
      [=](const TOptional<FString>& opt_temp_token,
          const TOptional<FCFCoreApiResponseError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_result.ExecuteIfBound(opt_temp_token.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetBlockedModsDetails(
  FCFCoreApiGetBlockedDetailsDelegate on_result,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetBlockedDetails(
    ICFCoreApiAuthorized::FGetBlockedDetailsDelegate::CreateLambda(
      [=](const TOptional<FBlockedDetails>& opt_blocked_server_details,
          const TOptional<FCFCoreApiResponseError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_result.ExecuteIfBound(opt_blocked_server_details.GetValue());
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiUnblockMods(
  const FUnblockModsRequest& Request,
  FCFCoreApiUnblockModsDelegate OnResult,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->UnblockMods(
    Request,
    ICFCoreApiAuthorized::FUnblockModsDelegate::CreateLambda(
      [=](const TOptional<FCFCoreApiResponseError>& opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        OnResult.ExecuteIfBound(true);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGeneratePremiumCheckoutUrl(
  const FGenPremiumCheckoutUrlRequest& Request,
  const FCFCoreApiGeneratePremiumCheckoutUrlDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->GeneratePremiumCheckoutUrl(
    Request,
    ICFCoreApiAuthorized::FGeneratePremiumCheckoutUrlDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<FString>& OutUrl,
                           const TOptional<FCFCoreApiResponseError>& OutErr) {
        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(*OutErr));
          return;
        }

        OnSuccess.ExecuteIfBound(*OutUrl);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiInitiatePurchase(
  const FInitiatePurchaseRequest& Request,
  const FCFCoreInitiatePurchaseDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->InitiatePurchase(
    Request,
    ICFCoreApiAuthorized::FInitiatePurchaseDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<FPurchaseDetails>& OutPurchDetails,
                           const TOptional<FCFCoreApiResponseError>& OutErr) {
        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(*OutErr));
          return;
        }

        OnSuccess.ExecuteIfBound(*OutPurchDetails);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiFinalizePurchase(
  const FFinalizePurchaseRequest& Request,
  const FCFCoreSuccessDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->FinalizePurchase(
    Request,
    ICFCoreApiAuthorized::FFinalizePurchaseDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<FCFCoreApiResponseError>& OutErr) {
        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(*OutErr));
          return;
        }

        OnSuccess.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiEntitlementsRedeemCode(
  const FEntitlementsRedeemCodeRequest& InRequest,
  const FCFCoreEntitlmentsRedeemCodeDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Api()->Authorized()->EntitlementsRedeemCode(
    InRequest,
    ICFCoreApiAuthorized::FEntitlementsRedeemCodeDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<FEntitlementCampaign>& OptCampaign,
                           const TOptional<FCFCoreApiResponseError>& OptErr) {
        if (OptErr.IsSet()) {
          OnError.ExecuteIfBound(FCFCoreError(*OptErr));
          return;
        }

        OnSuccess.ExecuteIfBound(*OptCampaign);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetModsSubscriptions(
  const FCFCoreApiRequestPagination& pagination,
  const FCFCoreApiGetModsSubscriptionsDelegate& on_subscriptions,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetModsSubscriptions(
    pagination,
    ICFCoreApiAuthorized::FGetModsSubscriptionsDelegate::CreateLambda(
      [on_subscriptions, on_error](
        const TOptional<TArray<FCFCoreMod>>& mods,
        const TOptional<FCFCoreApiResponsePagination>& pagination,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_subscriptions.ExecuteIfBound(*mods, *pagination);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiGetModsSubscriptionsIds(
  const FCFCoreApiRequestPagination& pagination,
  const FCFCoreApiGetModsSubscriptionsIdsDelegate& on_ids,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->GetModsSubscriptionsIds(
    pagination,
    ICFCoreApiAuthorized::FGetModsSubscriptionsIdsDelegate::CreateLambda(
      [on_ids, on_error](
        const TOptional<TArray<int64>>& ids,
        const TOptional<FCFCoreApiResponsePagination>& pagination,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_ids.ExecuteIfBound(*ids, *pagination);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiSubscribe(
  FSubscribeRequest request,
  const FCFCoreApiSubscribeDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->Subscribe(
    request,
    ICFCoreApiAuthorized::FSubscribeDelegate::CreateLambda(
      [on_success, on_error](
        const TOptional<FCFCoreMod>& mod,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_success.ExecuteIfBound(*mod);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiSubscribeMods(
  FSubscribeModsRequest request,
  const FCFCoreSuccessDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->SubscribeMods(
    request,
    ICFCoreApiAuthorized::FSubscribeModsDelegate::CreateLambda(
      [on_success, on_error](
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(opt_err));
          return;
        }

        on_success.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiUnsubscribe(
  FSubscribeRequest request,
  const FCFCoreSuccessDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->Unsubscribe(
    request,
    ICFCoreApiAuthorized::FUnsubscribeDelegate::CreateLambda(
      [on_success, on_error](const TOptional<FCFCoreApiResponseError>& err) {

        if (err.IsSet()) {
          on_error.ExecuteIfBound(FCFCoreError(err));
          return;
        }

        on_success.ExecuteIfBound();
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::EnableAutoManagement(
  const FCFCoreSuccessDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();
  if (!subscription) {
    on_error.ExecuteIfBound(CreateFailedToInitializeError());
    return;
  }

  subscription->EnableAutoManagement(
    ICFCoreSubscription::FAutoManagementDelegate::CreateLambda(
      [on_success, on_error](
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet() && opt_err->isError) {
        on_error.ExecuteIfBound(*opt_err);
        return;
      }

      on_success.ExecuteIfBound();
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::DisableAutoManagement() {
  if (!CFCoreContext::GetInstance()->IsInitialized()) {
    return;
  }

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();

  if (!subscription) {
    return;
  }

  subscription->DisableAutoManagement();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::SubscribeMod(
  const FSubscribeRequest& request,
  const FCFCoreSubscriptionSubscribeDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();

  if (!subscription) {
    on_error.ExecuteIfBound(CreateFailedToInitializeError());
    return;
  }

  subscription->Subscribe(
    request,
    ICFCoreSubscription::FSubscribeDelegate::CreateLambda(
      [on_success, on_error](const FCFCoreMod& mod,
                             const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet() && opt_err->isError) {
        on_error.ExecuteIfBound(*opt_err);
        return;
      }

      on_success.ExecuteIfBound(mod);
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::UnsubscribeMod(
  const FSubscribeRequest& request,
  const FCFCoreSuccessDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();

  if (!subscription) {
    on_error.ExecuteIfBound(CreateFailedToInitializeError());
    return;
  }

  subscription->Unsubscribe(
    request,
    ICFCoreSubscription::FUnsubscribeDelegate::CreateLambda(
      [on_success, on_error](const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet() && opt_err->isError) {
        on_error.ExecuteIfBound(*opt_err);
        return;
      }

      on_success.ExecuteIfBound();
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::SyncSubscriptions(
  const FCFCoreSubscriptionSyncDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();
  if (!subscription) {
    on_error.ExecuteIfBound(CreateFailedToInitializeError());
    return;
  }

  subscription->SyncSubscriptions(
    ICFCoreSubscription::FSyncSubscriptionsDelegate::CreateLambda(
      [on_success, on_error](
        const TArray<FSubscriptionSyncResultItem>& result_items,
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet() && opt_err->isError) {
        on_error.ExecuteIfBound(*opt_err);
        return;
      }

      on_success.ExecuteIfBound(result_items);
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetSyncPlan(
  const FCFCoreSubscriptionGetSyncPlanDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();
  if (!subscription) {
    on_error.ExecuteIfBound(CreateFailedToInitializeError());
    return;
  }

  subscription->GetSyncPlan(
    ICFCoreSubscription::FGetSyncPlanDelegate::CreateLambda(
      [on_success, on_error](
        const TArray<FSubscriptionSyncPlanItem>& plan_items,
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet() && opt_err->isError) {
        on_error.ExecuteIfBound(*opt_err);
        return;
      }

      on_success.ExecuteIfBound(plan_items);
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ExecuteSyncPlan(
  const TArray<FSubscriptionSyncPlanItem>& plan_items,
  const FCFCoreSubscriptionExecuteSyncPlanDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  ICFCoreSubscription* subscription =
    CFCoreContext::GetInstance()->Subscription();
  if (!subscription) {
    on_error.ExecuteIfBound(CreateFailedToInitializeError());
    return;
  }

  subscription->ExecuteSyncPlan(
    plan_items,
    ICFCoreSubscription::FExecuteSyncPlanDelegate::CreateLambda(
      [on_success, on_error](
        const TArray<FSubscriptionSyncResultItem>& result_items,
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet() && opt_err->isError) {
        on_error.ExecuteIfBound(*opt_err);
        return;
      }

      on_success.ExecuteIfBound(result_items);
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::ApiDiagnosticsInit(
  const FCFCoreApiDiagnosticsInitDelegate& on_success,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Api()->Authorized()->DiagnosticsInit(
    ICFCoreApiAuthorized::FDiagnosticsInitDelegate::CreateLambda(
      [on_success, on_error](const TOptional<FDiagnosticsInitData>& init_data,
                             const TOptional<FCFCoreApiResponseError>& err) {

      if (err.IsSet()) {
        on_error.ExecuteIfBound(FCFCoreError(err));
        return;
      }

      on_success.ExecuteIfBound(*init_data);
    })
  );
}


// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetInstalledMods(
  FCFCoreLibGetInstalledModsDelegate on_installed_mods,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->GetInstalledMods(
    ICFCoreLibrary::FGetInstalledModsDelegate::CreateLambda(
      [=](const TArray<FInstalledMod>& installed_mods) {
        on_installed_mods.ExecuteIfBound(installed_mods);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetSystemInstalledMods(
  FCFCoreLibGetInstalledModsDelegate on_installed_mods,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->GetSystemInstalledMods(
    ICFCoreLibrary::FGetInstalledModsDelegate::CreateLambda(
      [=](const TArray<FInstalledMod>& installed_mods) {
        on_installed_mods.ExecuteIfBound(installed_mods);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetModsDirInfo(
  FCFCoreGetModsDirInfoDelegate OnModsDirInfo,
  FCFCoreErrorDelegate OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Library()->GetModsDirInfo(
    ICFCoreLibrary::FGetModsDirInfoDelegate::CreateLambda(
      [=](const TOptional<FModsDirInfo>& opt_mods_dir_info,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet() && opt_err->isError) {
          OnError.ExecuteIfBound(opt_err);
        } else {
          OnModsDirInfo.ExecuteIfBound(*opt_mods_dir_info);
        }
      })
  );

}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::SynchronizeWithServer(
  FCFCoreSuccessDelegate on_success,
  const FCFCoreErrorDelegate& on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->SyncWithServer(
    ICFCoreLibrary::FSyncWithServerDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound();
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::PerformModsValidation(
  const TArray<FInstalledMod>& installed_mods,
  FCFCoreLibPerformModsValidationDelegate on_success,
  FCFCoreErrorDelegate on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->PerformModsValidation(
    installed_mods,
    ICFCoreLibrary::FPerformModsValidationDelegate::CreateLambda(
      [=](const TOptional<TArray<FInstalledMod>>& invalid_mods,
          const TOptional<FCFCoreError>& opt_err) {

        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound(invalid_mods.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::UpdateInstalledModsProperties(
  const TArray<FInstalledModProperties>& InInstalledModsProperties,
  FCFCoreSuccessDelegate OnSuccess,
  FCFCoreErrorDelegate OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Library()->UpdateInstalledModsProperties(
    InInstalledModsProperties,
    ICFCoreLibrary::FUpdateInstalledModsPropertiesDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet()) {
        OnError.ExecuteIfBound(opt_err);
      } else {
        OnSuccess.ExecuteIfBound();
      }
    })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::InstallMod(
  FCFCoreMod mod,
  const FCFCoreLibInstallProgressDelegate& on_progress,
  const FCFCoreLibInstallDelegate& on_installed,
  const FCFCoreErrorDelegate& on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->Install(
    mod,
    ICFCoreLibrary::FInstallProgressDelegate::CreateLambda(
      [=](const FLibraryProgress& progress) {
        on_progress.ExecuteIfBound(progress);
      }),
    ICFCoreLibrary::FInstalledDelegate::CreateLambda(
      [=](const TOptional<FInstalledMod>& opt_installed_mod,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_installed.ExecuteIfBound(opt_installed_mod.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::InstallModExtended(
  const FCFCoreMod& InMod,
  const FFile& InFile,
  const FInstallModAdditionalParams& InAdditionalParams,
  const FCFCoreLibInstallProgressDelegate& OnProgress,
  const FCFCoreLibInstallDelegate& OnInstalled,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Library()->Install(
    InMod,
    InFile,
    InAdditionalParams,
    ICFCoreLibrary::FInstallProgressDelegate::CreateLambda(
      [OnProgress](const FLibraryProgress& progress) {
        OnProgress.ExecuteIfBound(progress);
      }),
    ICFCoreLibrary::FInstalledDelegate::CreateLambda(
      [OnError, OnInstalled](const TOptional<FInstalledMod>& opt_installed_mod,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(opt_err);
        } else {
          OnInstalled.ExecuteIfBound(opt_installed_mod.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::CancelInstallation(int64 mod_id,
                                          FCFCoreSuccessDelegate on_success,
                                          FCFCoreErrorDelegate on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->CancelInstallation(
    mod_id,
    ICFCoreLibrary::FCancelInstallDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound();
        }
      }
    )
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::UninstallMod(int64 mod_id,
                                    FCFCoreLibUninstallDelegate on_uninstalled,
                                    FCFCoreErrorDelegate on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Library()->Uninstall(
    mod_id,
    ICFCoreLibrary::FUninstallDelegate::CreateLambda(
      [=](TOptional<FInstalledMod> opt_uninstalled_mod,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_uninstalled.ExecuteIfBound(opt_uninstalled_mod.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::CleanTempDir(FCFCoreSuccessDelegate OnSuccess,
                                    FCFCoreErrorDelegate OnError) {
  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Library()->CleanTempDir(
    ICFCoreLibrary::FCleanTempDirDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(opt_err);
        } else {
          OnSuccess.ExecuteIfBound();
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::AssureServerModsUpdated(
  const FAssureServerModsUpdatedParams& params,
  const FCFCoreLibClientServerUpdateProgressDelegate& OnProgress,
  const FCFCoreLibClientServerUpdateDelegate& OnUpdated,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  auto lib = CFCoreContext::GetInstance()->Library();

  lib->ClientServerLibrary()->AssureServerModsUpdated(
    params,
    ICFCoreClientServerLibrary::FModsUpdateProgressDelegate::CreateLambda(
      [=](const FModsUpdateProgress& UpdateProgress,
          const TOptional<FLibraryProgress>& ModInstallProgress,
          const TOptional<FCFCoreMod>& Mod) {

        if (ModInstallProgress.IsSet() && Mod.IsSet()) {
          OnProgress.ExecuteIfBound(UpdateProgress, *ModInstallProgress, *Mod);
        } else {
          OnProgress.ExecuteIfBound(UpdateProgress,
                                    FLibraryProgress(),
                                    FCFCoreMod());
        }
      }),
    ICFCoreClientServerLibrary::FModsUpdateDelegate::CreateLambda(
      [=](TOptional<TArray<FInstalledMod>> InstalledMods,
          TOptional<FCFCoreError> OptErr) {
        if (OptErr.IsSet()) {
          OnError.ExecuteIfBound(MoveTemp(OptErr.GetValue()));
        } else {
          OnUpdated.ExecuteIfBound(MoveTemp(InstalledMods.GetValue()));
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::AssureClientModsUpdated(
  const TArray<int64>& ServerFileIds,
  const FCFCoreLibClientServerUpdateProgressDelegate& OnProgress,
  const FCFCoreLibClientServerUpdateDelegate& OnUpdated,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  auto lib = CFCoreContext::GetInstance()->Library();
  lib->ClientServerLibrary()->AssureClientModsUpdated(
    ServerFileIds,
    ICFCoreClientServerLibrary::FModsUpdateProgressDelegate::CreateLambda(
      [=](const FModsUpdateProgress& UpdateProgress,
          const TOptional<FLibraryProgress>& ModInstallProgress,
          const TOptional<FCFCoreMod>& Mod) {

        if (ModInstallProgress.IsSet() && Mod.IsSet()) {
          OnProgress.ExecuteIfBound(UpdateProgress, *ModInstallProgress, *Mod);
        } else {
          OnProgress.ExecuteIfBound(UpdateProgress,
                                    FLibraryProgress(),
                                    FCFCoreMod());
        }
      }),
    ICFCoreClientServerLibrary::FModsUpdateDelegate::CreateLambda(
      [=](TOptional<TArray<FInstalledMod>> installed_mods,
          TOptional<FCFCoreError> OptErr) {
        if (OptErr.IsSet()) {
          OnError.ExecuteIfBound(MoveTemp(OptErr.GetValue()));
        } else {
          OnUpdated.ExecuteIfBound(MoveTemp(installed_mods.GetValue()));
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::IsAuthenticated(
  FCFCoreIsAuthenticatedDelegate on_is_auth) {

  if (!CFCoreContext::GetInstance()->IsInitialized()) {
    on_is_auth.ExecuteIfBound(false);
    return;
  }

  bool is_authenticated =
    CFCoreContext::GetInstance()->Authentication()->IsAuthenticated();

  on_is_auth.ExecuteIfBound(is_authenticated);
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetAuthTerms(FCFCoreGetAuthTerms on_success,
                                    FCFCoreErrorDelegate on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Authentication()->GetTerms(
    ICFCoreAuthentication::FGetTermsDelegate::CreateLambda(
      [=](const TOptional<FTerms>& opt_terms,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound(opt_terms.GetValue());
        }
      })
  );

}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::SendSecurityCode(const FString& email,
                                        FCFCoreSuccessDelegate on_success,
                                        FCFCoreErrorDelegate on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Authentication()->SendSecurityCode(
    email,
    ICFCoreAuthentication::FSendSecurityCodeDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound();
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GenerateAuthToken(const FString& email,
                                         int32 security_code,
                                         FCFCoreSuccessDelegate on_success,
                                         FCFCoreErrorDelegate on_error) {
  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Authentication()->GenerateAuthToken(
    email,
    security_code,
    ICFCoreAuthentication::FGenerateAuthTokenDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound();
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GenerateAuthTokenByExternalProvider(
    ECFCoreExternalAuthProvider provider,
    const FString& external_token,
    const FExternalAuthAdditionalInfo &additional_info,
    FCFCoreSuccessDelegate on_success,
    FCFCoreErrorDelegate on_error) {

  ASSERT_INITIALIZED(on_error);

  auto authentication = CFCoreContext::GetInstance()->Authentication();
  authentication->GenerateAuthTokenByExternalProvider(
    provider,
    external_token,
    additional_info,
    ICFCoreAuthentication::FGenerateAuthTokenDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound();
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::Logout(FCFCoreSuccessDelegate on_success,
                              FCFCoreErrorDelegate on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Authentication()->Logout(
    ICFCoreAuthentication::FLogoutDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound();
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::CreateMod(const FCreateModRequest& create_mod_request,
                                 const FString& avatar_image_filename,
                                 FCFCoreCreateModDelegate on_success,
                                 FCFCoreErrorDelegate on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Creation()->CreateMod(
    create_mod_request,
    avatar_image_filename,
    ICFCoreCreation::FCreateModDelegate::CreateLambda(
      [=](const TOptional<FCFCoreMod>& opt_mod,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound(opt_mod.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::UpdateMod(int64 mod_id,
                                 const FUpdateModRequest& update_mod_request,
                                 const FString& avatar_image_filename,
                                 FCFCoreUpdateModDelegate on_success,
                                 FCFCoreErrorDelegate on_error) {

  ASSERT_INITIALIZED(on_error);

  CFCoreContext::GetInstance()->Creation()->UpdateMod(
    mod_id,
    update_mod_request,
    avatar_image_filename,
    ICFCoreCreation::FUpdateModDelegate::CreateLambda(
      [=](const TOptional<FCFCoreMod>& opt_mod,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          on_error.ExecuteIfBound(opt_err);
        } else {
          on_success.ExecuteIfBound(opt_mod.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::CreateModFile(
  int64 ModId,
  const FCreateModFileRequest& CreateModFileRequest,
  const FString& LocalFilenameToUpload,
  const FCreateModFileRequestIdDelegate& OnCreateModFileRequestId,
  const FCreateModFileProgressDelegate& OnProgress,
  const FCreateModFileDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Creation()->CreateModFile(
    ModId,
    CreateModFileRequest,
    LocalFilenameToUpload,
    ICFCoreCreation::FCreateModFileRequestIdDelegate::CreateLambda(
      [=](int64 UploadId) {
        OnCreateModFileRequestId.ExecuteIfBound(UploadId);
      }),
    ICFCoreCreation::FFileTransferProgressDelegate::CreateLambda(
      [=](const FFileTransferProgress& Progress) {
        OnProgress.ExecuteIfBound(Progress);
      }),
    ICFCoreCreation::FCreateModFileDelegate::CreateLambda(
      [=](const TOptional<FUploadedModFile>& OptUploadedModFile,
          const TOptional<FCFCoreError>& OptError) {
        if (OptError.IsSet()) {
          OnError.ExecuteIfBound(OptError);
        } else {
          OnSuccess.ExecuteIfBound(OptUploadedModFile.GetValue());
        }
      }));
}


// -----------------------------------------------------------------------------
void UCFCoreSubsystem::CreateCookedModFile(
  int64 ModId,
  int64 SourceFileId,
  const FCreateCookedModFileRequest& CreateCookedModFileRequest,
  const FString& LocalFilenameToUpload,
  const FCreateModFileRequestIdDelegate& OnCreateModFileRequestId,
  const FCreateModFileProgressDelegate& OnProgress,
  const FCreateModFileDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->Creation()->CreateCookedModFile(
    ModId,
    SourceFileId,
    CreateCookedModFileRequest,
    LocalFilenameToUpload,
    ICFCoreCreation::FCreateModFileRequestIdDelegate::CreateLambda(
      [=](int64 file_request_id) {
        OnCreateModFileRequestId.ExecuteIfBound(file_request_id);
      }),
    ICFCoreCreation::FFileTransferProgressDelegate::CreateLambda(
      [=](const FFileTransferProgress& progress) {
        OnProgress.ExecuteIfBound(progress);
      }),
    ICFCoreCreation::FCreateModFileDelegate::CreateLambda(
      [=](const TOptional<FUploadedModFile>& opt_uploaded_mod_file,
          const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          OnError.ExecuteIfBound(opt_err);
        } else {
          OnSuccess.ExecuteIfBound(opt_uploaded_mod_file.GetValue());
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::OverridePublicKey(const FString& InPublicKeyPem,
                                         const FCFCoreSuccessDelegate& OnSuccess,
                                         const FCFCoreErrorDelegate& OnError) {
  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->PremiumMods()->OverridePublicKey(
    InPublicKeyPem);

  OnSuccess.ExecuteIfBound();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetMyPremiumMods(
  const FPremiumModsCheckDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->PremiumMods()->GetPremiumMods(
    ICFCorePremiumMods::FGetPremiumModsDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<TArray<int64>>& OutModIds,
                           const TOptional<FCFCoreError>& OutErr) {
        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(*OutErr);
          return;
        }

        OnSuccess.ExecuteIfBound(*OutModIds);
      })
  );

}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::GetMyPremiumModsV2(
  const FGetMyPremiumModsV2Delegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->PremiumMods()->GetPremiumModsV2(
    ICFCorePremiumMods::FGetPremiumModsV2Delegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<FOwnedPremiumMods>& OutOwnedMods,
                           const TOptional<FCFCoreError>& OutErr) {

        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(*OutErr);
          return;
        }

        OnSuccess.ExecuteIfBound(*OutOwnedMods);
      })
  );

}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::PremiumModsCheck(
  const TArray<int64>& InModIds,
  const FPremiumModsCheckDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->PremiumMods()->CheckMods(
    InModIds,
    ICFCorePremiumMods::FPremiumModsCheckDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<TArray<int64>>& OutModIds,
                           const TOptional<FCFCoreError>& OutErr) {
        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(*OutErr);
          return;
        }

        OnSuccess.ExecuteIfBound(*OutModIds);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::PremiumModsCheckDlc(
  const TArray<FString>& InEntitlementIds,
  const FPremiumModsCheckDlcDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->PremiumMods()->CheckDlc(
    InEntitlementIds,
    ICFCorePremiumMods::FCheckDlcDelegate::CreateLambda(
      [OnSuccess, OnError](const TOptional<TArray<int64>>& OutModIds,
                           const TOptional<FCFCoreError>& OutErr) {
        if (OutErr.IsSet()) {
          OnError.ExecuteIfBound(*OutErr);
          return;
        }

        OnSuccess.ExecuteIfBound(*OutModIds);
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::PollModPurchase(
  const FPollPurchaseParams& InParams,
  const FPollModPurchaseDelegate& OnPurchased,
  const FCFCoreErrorDelegate& OnError,
  const FCFCoreSuccessDelegate& OnPurchasePollingTimeout,
  const FCFCoreSuccessDelegate& OnPurchasePollingStopped) {

  ASSERT_INITIALIZED(OnError);

  CFCoreContext::GetInstance()->PremiumMods()->PollModPurchase(
    InParams,
    ICFCorePremiumMods::FPollPurchaseDelegate::CreateLambda(
      [OnPurchased, OnError, OnPurchasePollingTimeout, OnPurchasePollingStopped](
        const TOptional<TArray<int64>>& OutOwnedModIds,
        const TOptional<FCFCoreError>& OutErr) {

        if (!OutErr.IsSet()) {
          OnPurchased.ExecuteIfBound(*OutOwnedModIds);
          return;
        }

        switch (OutErr->code) {
          case ECFCoreErrorCodes::PurchasePollingTimeout:
            OnPurchasePollingTimeout.ExecuteIfBound();
            break;
          case ECFCoreErrorCodes::PurchasePollingStopped:
            OnPurchasePollingStopped.ExecuteIfBound();
            break;
          default:
            OnError.ExecuteIfBound(*OutErr);
            break;
        }
      })
  );
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::StopPurchasePolling() {
  if (!CFCoreContext::GetInstance()) {
    return;
  }

  CFCoreContext::GetInstance()->PremiumMods()->StopPurchasePolling();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::AnalyticsSendGamePlaySession(
  const FGamePlaySessionParams& InParams,
  const FCFCoreSuccessDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  bool success =
    CFCoreContext::GetInstance()->Analytics()->SendGamePlaySession(InParams);

  if (!success) {
    // NOTE(twolf): There is no real error we return here - unless we are
    // missing internal params - in that case, view logs.
    OnError.ExecuteIfBound(FCFCoreError(ECFCoreErrorCodes::ApiError));
    return;
  }

  OnSuccess.ExecuteIfBound();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::AnalyticsSendModBrowsingFunnelImpression(
  const FModBrowsingFunnelParams& InParams,
  const FCFCoreSuccessDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  ICFCoreAnalytics* analytics = CFCoreContext::GetInstance()->Analytics();
  bool success = analytics->SendModBrowsingFunnelImpression(InParams);

  if (!success) {
    OnError.ExecuteIfBound(FCFCoreError(ECFCoreErrorCodes::ApiError));
    return;
  }

  OnSuccess.ExecuteIfBound();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::AnalyticsSendModBrowsingFunnelAction(
  const FModBrowsingFunnelParams& InParams,
  const FCFCoreSuccessDelegate& OnSuccess,
  const FCFCoreErrorDelegate& OnError) {

  ASSERT_INITIALIZED(OnError);

  ICFCoreAnalytics* analytics = CFCoreContext::GetInstance()->Analytics();
  bool success = analytics->SendModBrowsingFunnelAction(InParams);

  if (!success) {
    OnError.ExecuteIfBound(FCFCoreError(ECFCoreErrorCodes::ApiError));
    return;
  }

  OnSuccess.ExecuteIfBound();
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::UtilsCompressionZipPaths(
  const TArray<FString>& InPathsToZip,
  const FString& InOutputZipFile,
  const FUtilsCompressionProgressDelegate& OnProgress,
  const FCFCoreSuccessDelegate& OnSuccess,
  const FUtilsCompressionErrorDelegate& OnError) {

  auto future = CFCoreContext::GetInstance()->Utils()->Compression()->Zip(
    MakeShared<TArray<FString>>(InPathsToZip),
    InOutputZipFile,
    ICompressionService::FProgressDelegate::CreateLambda(
      [OnProgress](const FCompressionProgress& Progress) {
        OnProgress.ExecuteIfBound(Progress);
      })
  );

  future.Next([OnSuccess, OnError](ECompressionError err) {
    if (err == ECompressionError::None) {
      OnSuccess.ExecuteIfBound();
      return;
    }

    OnError.ExecuteIfBound(err);
  });
}

// -----------------------------------------------------------------------------
FCFCoreError UCFCoreSubsystem::CreateFailedToInitializeError() {
  FCFCoreError err;
  err.code = ECFCoreErrorCodes::FailedToInitialize;
  err.isError = true;
  err.description = TEXT("Not initialized");
  return err;
}

// -----------------------------------------------------------------------------
void UCFCoreSubsystem::RegisterCFCoreLibraryDelegates() {
  ICFCoreLibrary* library = CFCoreContext::GetInstance()->Library();
  if (!library) {
    return;
  }

  library->OnModInstallProgress().Clear();
  library->OnModInstalled().Clear();

  library->OnModInstallProgress().AddLambda(
    [this](const FLibraryProgress& Progress) {

    OnModInstallProgress.Broadcast(Progress);
  });

  library->OnModInstalled().AddLambda(
    [this](const TOptional<FInstalledMod>& OptInstalledMod,
           const TOptional<FCFCoreError>& OptErr) {

    if (OptErr && OptErr->isError) {
      return;
    }
    if (OptInstalledMod) {
      OnModInstalled.Broadcast(OptInstalledMod.GetValue());
    }
  });
}
