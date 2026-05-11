/*MIT License

Copyright (c) 2026 Overwolf Ltd.

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
#include "language_test_analytics_service_impl.h"

#include <common/constants.h>
#include <analytics/cfcore_analytics_internal.h>
#include <analytics/models/language_test_action_params.h>
#include <api/services/feature_phasing_service.h>
#include <services/settings_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
LanguageTestAnalyticsServiceImpl::LanguageTestAnalyticsServiceImpl(
  TSharedRef<ICFCoreAnalyticsInternal> cfcore_analytics,
  TSharedRef<IFeaturePhasingService> feature_phasing_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher,
  TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher) :
  cfcore_analytics_(cfcore_analytics),
  feature_phasing_service_(feature_phasing_service),
  settings_service_(settings_service),
  api_delegates_(MakeShared<CFCoreApiDelegates>()),
  api_auth_delegates_(MakeShared<CFCoreApiAuthorizedDelegates>()) {

  RegisterDelegates(api_delegates_dispatcher, api_auth_delegates_dispatcher);
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::RegisterDelegates(
  TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher,
  TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher) {

  api_delegates_->get_mod = ICFCoreApi::FGetModDelegate::CreateLambda(
    [this](const TOptional<FCFCoreMod>& mod,
           const TOptional<FCFCoreApiResponseError>& error) {
      OnGetMod(mod, error);
    });

  api_delegates_->get_mod_description =
  ICFCoreApi::FGetModDescDelegate::CreateLambda(
    [this](const TOptional<FString>& description,
           const TOptional<FCFCoreApiResponseError>& error) {
      OnGetModDescription(description, error);
    });

  api_auth_delegates_->get_my_premium_mods =
  ICFCoreApiAuthorized::FGetMyPreimumModsDelegate::CreateLambda(
    [this](const TOptional<TArray<int64>>& mods_ids,
           const FString& signature,
           const TOptional<FCFCoreApiResponseError>& error) {
      OnGetMyPremiumMods(mods_ids, signature, error);
    });

  api_auth_delegates_->get_my_premium_mods_v2 =
    ICFCoreApiAuthorized::FGetMyPremiumModsV2Delegate::CreateLambda(
      [this](const TOptional<FOwnedPremiumMods>& owned_mods,
        const FString& signature,
        const TOptional<FCFCoreApiResponseError>& error) {

        if (!owned_mods.IsSet()) {
          return;
        }

        OnGetMyPremiumMods(owned_mods->ownedMods, signature, error);
      });

  api_auth_delegates_->generate_premium_checkout_url =
  ICFCoreApiAuthorized::FGeneratePremiumCheckoutUrlDelegate::CreateLambda(
    [this](const TOptional<FString>& url_result,
           const TOptional<FCFCoreApiResponseError>& error) {
      OnGeneratePremiumCheckoutUrl(url_result, error);
    });

  api_auth_delegates_->finalize_purchase =
  ICFCoreApiAuthorized::FFinalizePurchaseDelegate::CreateLambda(
    [this](const TOptional<FCFCoreApiResponseError>& error) {
      OnFinalizePurchase(error);
    });

  api_delegates_dispatcher->Register(FGuid::NewGuid(), api_delegates_);
  api_auth_delegates_dispatcher->Register(FGuid::NewGuid(), api_auth_delegates_);
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::OnGetMod(
  const TOptional<FCFCoreMod>& mod,
  const TOptional<FCFCoreApiResponseError>& error) {

  if (!mod.IsSet() || error.IsSet()) {
    return;
  }

  last_mod_ = mod.GetValue();
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::OnGetModDescription(
  const TOptional<FString>& description,
  const TOptional<FCFCoreApiResponseError>& error) {

  if (!description.IsSet() || error.IsSet()) {
    return;
  }

  TrySendLangTestAction(false);
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::OnGetMyPremiumMods(
  const TOptional<TArray<int64>>& mods_ids,
  const FString& signature,
  const TOptional<FCFCoreApiResponseError>& error) {

  if (!mods_ids.IsSet() || error.IsSet()) {
    return;
  }

  my_premium_mods_ids_ = mods_ids.GetValue();
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::OnGeneratePremiumCheckoutUrl(
  const TOptional<FString>& url_result,
  const TOptional<FCFCoreApiResponseError>& error) {

  if (!url_result.IsSet() || error.IsSet()) {
    return;
  }

  TrySendLangTestAction(true);
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::OnFinalizePurchase(
  const TOptional<FCFCoreApiResponseError>& error) {

  if (error.IsSet()) {
    return;
  }

  TrySendLangTestAction(true);
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::TrySendLangTestAction(
  bool is_purchase) {

  if (!last_mod_.IsSet()) {
    return;
  }

  const FCFCoreMod& cached_mod = last_mod_.GetValue();

  if (!cached_mod.premiumDetails.isPremium) {
    return;
  }

  SendLangTestAction(cached_mod, is_purchase);
}

// -----------------------------------------------------------------------------
void LanguageTestAnalyticsServiceImpl::SendLangTestAction(
  const FCFCoreMod& mod,
  bool is_purchase) {

  FLanguageTestActionParams params;

  bool is_test_group = feature_phasing_service_->IsFeatureEnabledForUser(
    &FMePhasingData::useTranslations);

  params.modId = mod.id;
  params.group = is_test_group ? kTestGroup : kControlGroup;
  params.selectedLanguage = settings_service_->GetSettings().defaultLanguage;
  params.action = is_purchase ? kPurchaseAction : kPageViewAction;
  params.ownsMod = my_premium_mods_ids_.Contains(mod.id);

  cfcore_analytics_->SendLanguageTestAction(params);
}