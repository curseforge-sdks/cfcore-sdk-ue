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
#pragma once
#include <CoreMinimal.h>

#include <api/cfcore_api_delegates_dispatcher.h>
#include <api/cfcore_api_authorized_delegates.h>
#include <api/cfcore_api_delegates.h>

namespace cfcore {

class ICFCoreAnalyticsInternal;
class IFeaturePhasingService;
class ISettingsService;

using ApiDelegatesDispatcher =
  CFCoreApiDelegatesDispatcher<CFCoreApiDelegates>;

using ApiAuthDelegatesDispatcher =
  CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>;

// -----------------------------------------------------------------------------
class LanguageTestAnalyticsServiceImpl {

public:
  LanguageTestAnalyticsServiceImpl(
    TSharedRef<ICFCoreAnalyticsInternal> cfcore_analytics,
    TSharedRef<IFeaturePhasingService> feature_phasing_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher,
    TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher);

private:
  void RegisterDelegates(
    TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher,
    TSharedRef<ApiAuthDelegatesDispatcher> api_auth_delegates_dispatcher);

  void OnGetMod(
    const TOptional<FCFCoreMod>& mod,
    const TOptional<FCFCoreApiResponseError>& error);

  void OnGetModDescription(
    const TOptional<FString>& description,
    const TOptional<FCFCoreApiResponseError>& error);

  void OnGetMyPremiumMods(
    const TOptional<TArray<int64>>& mods_ids,
    const FString& signature,
    const TOptional<FCFCoreApiResponseError>& error);

  void OnGeneratePremiumCheckoutUrl(
    const TOptional<FString>& url_result,
    const TOptional<FCFCoreApiResponseError>& error);

  void OnFinalizePurchase(
    const TOptional<FCFCoreApiResponseError>& error);

  void TrySendLangTestAction(bool is_purchase);
  void SendLangTestAction(const FCFCoreMod& mod, bool is_purchase);

private:
  TSharedRef<ICFCoreAnalyticsInternal> cfcore_analytics_;
  TSharedRef<IFeaturePhasingService> feature_phasing_service_;
  TSharedRef<ISettingsService> settings_service_;

  TSharedRef<CFCoreApiDelegates> api_delegates_;
  TSharedRef<CFCoreApiAuthorizedDelegates> api_auth_delegates_;

  TOptional<FCFCoreMod> last_mod_;
  TArray<int64> my_premium_mods_ids_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::LanguageTestAnalyticsServiceImpl)