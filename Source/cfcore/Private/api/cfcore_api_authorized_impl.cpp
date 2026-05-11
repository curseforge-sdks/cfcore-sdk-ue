/* MIT License

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
SOFTWARE. */
#include "cfcore_api_authorized_impl.h"

#include <common/constants.h>
#include "common/api_macros.h"

#include <services/logger/logger_macros.h>
#include <api/services/api_helper_service.h>
#include <services/settings_service.h>

#include <api/models/model_responses/mod_array_response.h>
#include <api/models/model_responses/mod_id_array_with_pagination_response.h>

#include <api/models/model_responses/auth_token_response.h>
#include <api/models/model_responses/reporting_reasons_response.h>
#include <api/models/model_responses/me_response.h>
#include <api/models/model_responses/my_ratings_response.h>
#include <api/models/model_responses/generate_temp_token_response.h>
#include <api/models/model_responses/get_blocked_details_response.h>
#include <api/models/model_responses/premium_mods_check_request_response.h>
#include <api/models/model_responses/owned_premium_mods_response.h>
#include <api/models/model_responses/generate_premium_checkout_url_response.h>
#include <api/models/model_responses/initiate_purchase_response.h>
#include <api/models/model_responses/entitlements_redeem_code_response.h>
#include <api/models/model_responses/check_dlc_response.h>
#include <api/models/model_responses/mod_response.h>
#include <api/models/model_responses/diagnostics_init_response.h>
#include <api/models/serializers/api_request_pagination_serializer.h>

#include "models/dtos/report_mod_request_dto.h"
#include "models/dtos/premium_mods_check_request_dto.h"
#include "models/dtos/check_dlc_request_dto.h"

#include <JsonUtilities.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
// static
CFCoreApiAuthorizedImpl::CFCoreApiAuthorizedImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IApiHelperService> api_helper_service,
  TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher) :
  settings_service_(settings_service),
  api_helper_service_(api_helper_service),
  api_auth_delegates_dispatcher_(api_auth_delegates_dispatcher) {

  settings_service->OnSettingsUpdated().AddLambda([this]() {
    InitializeSettings();
  });

  InitializeSettings();
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetMe(FGetMeDelegate delegate) {
  FString url = FString::Format(kApiUrlGetMe, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER(
    FMeResponse,
    FGetMeDelegate,
    FGetMeDelegate::CreateLambda([this, delegate](
      const TOptional<FMe>& opt_me,
      const TOptional<FCFCoreApiResponseError>& opt_err) {

      LogMe(opt_me);

      for (auto api_delegate : api_auth_delegates_dispatcher_->Delegates()) {
        api_delegate->get_me.ExecuteIfBound(opt_me, opt_err);
      }

      delegate.ExecuteIfBound(opt_me, opt_err);
    })
  );

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetMyMods(FGetMyModsDelegate delegate) {
  FString url = FString::Format(kApiUrlGetMyMods, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER(FModArrayResponse,
                                         FGetMyModsDelegate,
                                         delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetMyRatings(FGetMyRatingsDelegate delegate) {
  FString url = FString::Format(kApiUrlGetMyRatings, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER(FMyRatingsResponse,
                                         FGetMyRatingsDelegate,
                                         delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::UpdateRating(int64 mod_id,
                                           ECFCoreRatingVoteDirection direction,
                                           FUpdateRatingDelegate delegate) {

  FString vote_action = kApiActionUpVote;
  if (direction == ECFCoreRatingVoteDirection::DownVote) {
    vote_action = kApiActionDownVote;
  }

  FString url = FString::Format(kApiUrlPutUpdateRating, {
    api_base_url_,
    mod_id,
    vote_action
  });

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(FUpdateRatingDelegate, delegate);
  api_helper_service_->PerformPut(url, "", handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::RemoveRating(int64 mod_id,
                                           FRemoveRatingDelegate delegate) {

  FString url = FString::Format(kApiUrlDeleteRemoveRating, {
    api_base_url_,
    mod_id
  });

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(FRemoveRatingDelegate, delegate);
  api_helper_service_->PerformDelete(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetReportingReasons(
  FGetReportingReasonsDelegate delegate) {

  FString url = FString::Format(kApiUrlGetReportingReasons, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER(FReportingReasonsResponse,
                                         FGetReportingReasonsDelegate,
                                         delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::ReportMod(int64 mod_id,
                                        int64 reasonId,
                                        FString report,
                                        FReportModDelegate delegate) {

  FString url = FString::Format(kApiUrlPostReportMod, {
    api_base_url_,
    mod_id
  });

  FReportModRequestDto dto;
  dto.reportReasonId = reasonId;
  dto.reportText = report;
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(FReportModDelegate,
                                               delegate);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GenerateTempToken(
  FGenerateTempTokenDelegate delegate) {

  FString url = FString::Format(kApiUrlGetGenerateTempToken, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER(FGenerateTempTokenResponse,
                                         FGenerateTempTokenDelegate,
                                         delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetBlockedDetails(
  FGetBlockedDetailsDelegate delegate) {

  FString url = FString::Format(kApiUrlGetBlockedDetails, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER(FGetBlockedDetailsResponse,
                                         FGetBlockedDetailsDelegate,
                                         delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::UnblockMods(const FUnblockModsRequest& request,
                                          FUnblockModsDelegate delegate) {

  FString url = FString::Format(kApiUrlPostUnblockMods, { api_base_url_ });

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(FUnblockModsDelegate, delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetMyPremiumMods(
  FGetMyPreimumModsDelegate delegate) {

  FString url = FString::Format(kApiUrlGetMyPremiumMods, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER_WITH_SIGNATURE(
    FPremiumModsCheckRequestResponse,
    FGetMyPreimumModsDelegate,
    FGetMyPreimumModsDelegate::CreateLambda([this, delegate](
      const TOptional<TArray<int64>>& mods_ids,
      const FString& signature,
      const TOptional<FCFCoreApiResponseError>& error) {

      for (auto api_delegate : api_auth_delegates_dispatcher_->Delegates()) {
        api_delegate->get_my_premium_mods.ExecuteIfBound(mods_ids,
                                                         signature,
                                                         error);
      }

      delegate.ExecuteIfBound(mods_ids, signature, error);
    })
  );

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetMyPremiumModsV2(
  FGetMyPremiumModsV2Delegate delegate) {

  FString url = FString::Format(kApiUrlGetMyPremiumModsV2, { api_base_url_ });

  auto handler = CREATE_RESPONSE_HANDLER_WITH_SIGNATURE(
    FOwnedPremiumModsResponse,
    FGetMyPremiumModsV2Delegate,
    FGetMyPremiumModsV2Delegate::CreateLambda([this, delegate](
      const TOptional<FOwnedPremiumMods>& owned_mods,
      const FString& signature,
      const TOptional<FCFCoreApiResponseError>& error) {

      for (auto api_delegate : api_auth_delegates_dispatcher_->Delegates()) {
        api_delegate->get_my_premium_mods_v2.ExecuteIfBound(owned_mods,
                                                            signature,
                                                            error);
      }

      delegate.ExecuteIfBound(owned_mods, signature, error);
    })
  );

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::PremiumModsCheck(
  const TArray<int64>& mod_ids,
  FPremiumModsCheckDelegate delegate) {

  FString url = FString::Format(kApiUrlPostPremiumModsCheck, {
    api_base_url_
  });

  FPremiumModsCheckRequestDto dto{ mod_ids };
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER_WITH_SIGNATURE(
    FPremiumModsCheckRequestResponse,
    FPremiumModsCheckDelegate,
    delegate);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GeneratePremiumCheckoutUrl(
  const FGenPremiumCheckoutUrlRequest& request,
  FGeneratePremiumCheckoutUrlDelegate delegate) {

  FString url = FString::Format(kApiUrlPostGeneratePremiumCheckoutUrl, {
    api_base_url_
  });

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(
    FGeneratePremiumCheckoutUrlResponse,
    FGeneratePremiumCheckoutUrlDelegate,
    FGeneratePremiumCheckoutUrlDelegate::CreateLambda([this, request, delegate](
      const TOptional<FString>& url_result,
      const TOptional<FCFCoreApiResponseError>& error) {

      for (auto api_delegate : api_auth_delegates_dispatcher_->Delegates()) {
        api_delegate->generate_premium_checkout_url.ExecuteIfBound(
          url_result,
          error
        );
      }

      delegate.ExecuteIfBound(url_result, error);
    })
  );

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::InitiatePurchase(
  const FInitiatePurchaseRequest& request,
  FInitiatePurchaseDelegate delegate) {

  FString url = FString::Format(kApiUrlPostInitiatePurchase, { api_base_url_ });

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FInitiatePurchaseResponse,
                                         FInitiatePurchaseDelegate,
                                         delegate);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::FinalizePurchase(
  const FFinalizePurchaseRequest& request,
  FFinalizePurchaseDelegate delegate) {

  FString url = FString::Format(kApiUrlPostFinalizePurchase, { api_base_url_ });

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(
    FFinalizePurchaseDelegate,
    FFinalizePurchaseDelegate::CreateLambda([this, request, delegate](
      const TOptional<FCFCoreApiResponseError>& error) {

      for (auto api_delegate : api_auth_delegates_dispatcher_->Delegates()) {
        api_delegate->finalize_purchase.ExecuteIfBound(error);
      }

      delegate.ExecuteIfBound(error);
    })
  );

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::EntitlementsRedeemCode(
  const FEntitlementsRedeemCodeRequest& request,
  FEntitlementsRedeemCodeDelegate delegate) {

  FString url = FString::Format(kApiUrlPostEntitlementsRedeemCode, {
    api_base_url_
  });

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FEntitlementsRedeemCodeResponse,
                                         FEntitlementsRedeemCodeDelegate,
                                         delegate);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::CheckDlc(const TArray<FString>& entitlement_ids,
                                       FCheckDlcDelegate delegate) {

  FString url = FString::Format(kApiUrlPostDlcCheck, {
    api_base_url_
  });

  FCheckDlcRequestDto dto{ entitlement_ids };
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER_WITH_SIGNATURE(FCheckDlcResponse,
                                                        FCheckDlcDelegate,
                                                        delegate);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetModsSubscriptions(
  const FCFCoreApiRequestPagination& pagination,
  FGetModsSubscriptionsDelegate delegate) {

  FString url = FString::Format(kApiUrlGetModsSubscriptions, {
    api_base_url_
  });

  auto handler = CREATE_RESPONSE_HANDLER_WITH_PAGINATION(
    FModArrayResponse,
    FGetModsSubscriptionsDelegate,
    delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::GetModsSubscriptionsIds(
  const FCFCoreApiRequestPagination& pagination,
  FGetModsSubscriptionsIdsDelegate delegate) {

  FString url = FString::Format(kApiUrlGetModsSubscriptionsIds, {
    api_base_url_
  });

  auto handler = CREATE_RESPONSE_HANDLER_WITH_PAGINATION(
    FModIdArrayWithPaginationResponse,
    FGetModsSubscriptionsIdsDelegate,
    delegate);

  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::Subscribe(const FSubscribeRequest& request,
                                        FSubscribeDelegate delegate) {

  FString url = FString::Format(kApiUrlPostSubscribe, {
    api_base_url_,
    request.modId
  });

  auto handler = CREATE_RESPONSE_HANDLER(FModResponse,
                                         FSubscribeDelegate,
                                         delegate);

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::SubscribeMods(
  const FSubscribeModsRequest& request,
  FSubscribeModsDelegate delegate) {

  FString url = FString::Format(kApiUrlPostSubscribeMods, { api_base_url_ });

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(FSubscribeModsDelegate,
                                               delegate);

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::Unsubscribe(const FSubscribeRequest& request,
                                          FUnsubscribeDelegate delegate) {

  FString url = FString::Format(kApiUrlPostUnsubscribe, {
    api_base_url_,
    request.modId
  });

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(
    FUnsubscribeDelegate,
    delegate);

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(request, json_data);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::DiagnosticsInit(
  FDiagnosticsInitDelegate delegate) {

  const FString url = FString::Format(kApiUrlPostDiagnosticsInit, {
    api_base_url_
  });

  // NOTE(apliscov): Optionally, metadata can be sent in the request
  FString json_data;

  auto handler = CREATE_RESPONSE_HANDLER(FDiagnosticsInitResponse,
    FDiagnosticsInitDelegate,
    delegate);

  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::InitializeSettings() {
  api_base_url_ = settings_service_->GetInternalSettings().apiBaseUrl;
}

// -----------------------------------------------------------------------------
void CFCoreApiAuthorizedImpl::LogMe(const TOptional<FMe>& opt_me) {
  if (!opt_me.IsSet()) {
    return;
  }

  CFCORE_LOG(Log,
    TEXT("GetMe called. id:%lld, email:%s"),
    opt_me->id,
    *opt_me->email);
}