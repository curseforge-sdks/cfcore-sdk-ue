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
#include "cfcore_analytics_impl.h"

#include <api/services/feature_phasing_service.h>

#include <services/settings_service.h>
#include <services/user_context/user_context_service.h>
#include <services/crypto_service/crypto_service.h>
#include <services/http/http_service.h>

#include <api/common/url_search_params.h>
#include <services/logger/logger_macros.h>

#include <common/defines.h>

#include <JsonUtilities.h>
#include <GenericPlatform/GenericPlatformHttp.h>

namespace cfcore {
const char kUrlParamCurrentVersion[] = "CurrentVersion";
const char kUrlParamPartnerId[] = "PartnerID";
const char kUrlParamValue[] = "Value";
const char kUrlParamGameId[] = "InGame";
const char kUrlParamMuid[] = "muid";
const char kUrlParamUsername[] = "UserName";
const char kUrlParamExtra[] = "Extra";

const char kJsonParamPlatform[] = "platform";
const char kJsonParamModId[] = "modId";

const char kEventGamePlaySession[] = "cf4s_gameplay_session";
const char kEventModBrowseFunnelImpression[] = "cf4s_modbrowse_impression";
const char kEventModBrowseFunnelAction[] = "cf4s_modbrws_funnel_action";
const char kEventModInstall[] = "cf4s_mod_install";
const char kEventModLangTestAction[] = "cf4s_lang_test_action";

// Map analytics category to a feature flag - you don't have to have a mapping
// for ALL categories (e.g. if you don't want to place one under a feature flag)
const TMap<ECFCoreAnalyticsCategory, int32 FMePhasingData::*>
kCategoryToPhasingField {
  { ECFCoreAnalyticsCategory::UserEngagement,
    &FMePhasingData::analyticsUserEngagement },
  { ECFCoreAnalyticsCategory::PerformanceAndStability,
    &FMePhasingData::analyticsPerfAndStability }
};

const TMap<FString, ECFCoreAnalyticsCategory> kAnalyticsEventToCategory{
  { kEventGamePlaySession, ECFCoreAnalyticsCategory::UserEngagement },
  { kEventModBrowseFunnelImpression, ECFCoreAnalyticsCategory::UserEngagement },
  { kEventModBrowseFunnelAction, ECFCoreAnalyticsCategory::UserEngagement },
  { kEventModInstall, ECFCoreAnalyticsCategory::PerformanceAndStability },
  { kEventModLangTestAction, ECFCoreAnalyticsCategory::UserEngagement}
};
}

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreAnalyticsImpl::CFCoreAnalyticsImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IUserContextService> user_context_service,
  TSharedRef<ICryptoService> crypto_service,
  TSharedRef<IHttpService> http_service,
  TSharedRef<IFeaturePhasingService> feature_phasing_service) :
  settings_service_(settings_service),
  user_context_service_(user_context_service),
  crypto_service_(crypto_service),
  http_service_(http_service),
  feature_phasing_service_(feature_phasing_service) {

}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::SendGamePlaySession(
  const FGamePlaySessionParams& params) {

  return SendAnalytic(kEventGamePlaySession, params);
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::SendModBrowsingFunnelImpression(
  const FModBrowsingFunnelParams& params) {

  return SendAnalytic(kEventModBrowseFunnelImpression, params, true);
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::SendModBrowsingFunnelAction(
  const FModBrowsingFunnelParams& params) {

  return SendAnalytic(kEventModBrowseFunnelAction, params, true);
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::SendModInstall(
  const FModInstallAnalyticsParams& params) {

  return SendAnalytic(kEventModInstall, params, true);
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::SendLanguageTestAction(
  const FLanguageTestActionParams& params) {

  return SendAnalytic(kEventModLangTestAction, params, true);
}

// -----------------------------------------------------------------------------
template<typename InStructType>
bool CFCoreAnalyticsImpl::SendAnalytic(FString event_name,
  const InStructType& params,
  bool clean_empty_fields/*= false*/) {

  if (!IsCategoryEnabled(event_name)) {
    return true;
  }

  UrlSearchParams search_params;
  search_params.Append("Name", event_name);

  FillDefaultSearchParams(&search_params);

  TSharedPtr<FJsonObject> json_obj = FJsonObjectConverter::UStructToJsonObject(
    params,
    0,
    0);

  FillDefaultExtraParams(json_obj);

  if (clean_empty_fields) {
    RemoveEmptyJsonFields(json_obj);
  }

  return SendAnalytic(&search_params, json_obj);
}

// -----------------------------------------------------------------------------
void CFCoreAnalyticsImpl::FillDefaultSearchParams(UrlSearchParams* params) {
  const FInternalSettings& internal_settings =
    settings_service_->GetInternalSettings();
  const FCFCoreSettings& settings = settings_service_->GetSettings();

  params->Append(kUrlParamPartnerId, 0);
  params->Append(kUrlParamValue, 0);
  params->Append(kUrlParamCurrentVersion, internal_settings.pluginVersion);
  params->Append(kUrlParamGameId, settings.gameId);

  FString user_name = GetCFUserId();
  if (!user_name.IsEmpty()) {
    params->Append(kUrlParamUsername, user_name);
  }
}

// -----------------------------------------------------------------------------
void CFCoreAnalyticsImpl::FillDefaultExtraParams(
  TSharedPtr<FJsonObject> extra_json) {

  const FInternalSettings& internal_settings =
    settings_service_->GetInternalSettings();

  TSharedPtr<FJsonValue> value = GetJsonValueString(internal_settings.platform);
  extra_json->SetField(kJsonParamPlatform, value);

  FString machineId = user_context_service_->SharedContextInfo().machineId;
  value = GetJsonValueString(machineId);
  extra_json->SetField(kUrlParamMuid, value);
}

// -----------------------------------------------------------------------------
FString CFCoreAnalyticsImpl::GetCFUserId() const {
  FString auth_token = user_context_service_->UserContextInfo().authToken;
  if (auth_token.IsEmpty()) {
    return "";
  }

  FJwtPayload payload;
  if (!crypto_service_->ExtractJwtPayload(auth_token, payload)) {
    return "";
  }

  return FString::Printf(TEXT("%lld"), payload.sub);
}

// -----------------------------------------------------------------------------
TSharedPtr<FJsonValue> CFCoreAnalyticsImpl::GetJsonValueString(
  const FString& str) {

  TSharedPtr<FJsonValue> platform_value = MakeShared<FJsonValueString>(str);
  return platform_value;
}

// -----------------------------------------------------------------------------
TSharedPtr<FJsonValue> CFCoreAnalyticsImpl::GetJsonValueNumber(int64 num) {
  TSharedPtr<FJsonValue> platform_value = MakeShared<FJsonValueNumber>(num);
  return platform_value;
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::SendAnalytic(
  UrlSearchParams* search_params,
  TSharedPtr<FJsonObject> extra_json) {


  FString extra;
  if (!Serialize(extra_json, extra)) {
    CFCORE_LOG(Error, TEXT("Failed to serialize analytics payload"));
    return false;
  }

  // NOTE(twolf): We are required to encode extra twice
  extra = FGenericPlatformHttp::UrlEncode(extra);
  extra = FGenericPlatformHttp::UrlEncode(extra);

  FString url = FString::Format(TEXT("{0}?{1}&{2}={3}"), {
    settings_service_->GetInternalSettings().analyticsUrl,
    search_params->ToString(),
    FString(kUrlParamExtra),
    extra
  });

  TMap<FString, FString> headers;
  http_service_->Get(
    url,
    headers,
    IHttpService::FCompleteDelegate::CreateLambda(
      [](const HttpResponse& http_response) {}
    )
  );

  return true;
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::Serialize(TSharedPtr<FJsonObject> json,
                                    OUT FString& output) {

  auto json_writer = TJsonWriterFactory<
    FString::ElementType,
    TCondensedJsonPrintPolicy<TCHAR>>::Create(&output, 0);

  bool success = FJsonSerializer::Serialize(json.ToSharedRef(),
                                            json_writer);
  json_writer->Close();

  return success;
}

// -----------------------------------------------------------------------------
void CFCoreAnalyticsImpl::RemoveEmptyJsonFields(TSharedPtr<FJsonObject> json) {
  TArray<FString> fields_removal_list;

  for (auto& json_obj : json->Values) {
    switch (json_obj.Value->Type) {
    case EJson::Array:
      if (json_obj.Value->AsArray().Num() == 0) {
        fields_removal_list.Add(json_obj.Key);
      }
    case EJson::String:
      if (json_obj.Value->AsString().IsEmpty()) {
        fields_removal_list.Add(json_obj.Key);
      }
      break;
    case EJson::Number:
      if (json_obj.Value->AsNumber() == CF_INVALID_NUMBER) {
        fields_removal_list.Add(json_obj.Key);
      }
      break;
    }
  }

  for (const FString& field_name : fields_removal_list) {
    json->RemoveField(field_name);
  }
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::IsCategoryEnabled(FString event_name) {
  check(kAnalyticsEventToCategory.Contains(event_name));
  if (!kAnalyticsEventToCategory.Contains(event_name)) {
    CFCORE_LOG(Error, TEXT("Unknown analytics event name"));
    return false;
  }

  const FCFCoreSettings& settings = settings_service_->GetSettings();
  ECFCoreAnalyticsCategory category = kAnalyticsEventToCategory[event_name];

  if (!IsCategoryFeatureFlagEnabledForUser(category)) {
    return false;
  }

  switch (category) {
  case ECFCoreAnalyticsCategory::PerformanceAndStability:
    return settings.analytics.performanceAndStability;
  case ECFCoreAnalyticsCategory::UserEngagement:
    return settings.analytics.userEngagement;
  }

  return false;
}

// -----------------------------------------------------------------------------
bool CFCoreAnalyticsImpl::IsCategoryFeatureFlagEnabledForUser(
  ECFCoreAnalyticsCategory category) {

  if (!kCategoryToPhasingField.Contains(category)) {
    // By default, don't block categories that aren't behind a flag - this
    // allows us to remove feature flags in an easier way (just remove from the
    // kCategoryToPhasingField const)
    return true;
  }

  int32 FMePhasingData::* phasing_field = kCategoryToPhasingField[category];

  if (!feature_phasing_service_->IsFeatureEnabledForUser(phasing_field)) {
    return false;
  }

  return true;
}
