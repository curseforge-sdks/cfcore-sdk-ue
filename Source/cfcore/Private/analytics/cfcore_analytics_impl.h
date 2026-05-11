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
#include <analytics/cfcore_analytics.h>
#include "cfcore_analytics_internal.h"
#include "models/enums/cfcore_analytics_category.h"

class FJsonObject;
class FJsonValue;

namespace cfcore {
class ISettingsService;
class IUserContextService;
class ICryptoService;
class IHttpService;
class UrlSearchParams;
class IFeaturePhasingService;

// -----------------------------------------------------------------------------
class CFCoreAnalyticsImpl : public ICFCoreAnalyticsInternal,
  public ICFCoreAnalytics,
  public TSharedFromThis<CFCoreAnalyticsImpl> {
public:
  CFCoreAnalyticsImpl(
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IUserContextService> user_context_service,
    TSharedRef<ICryptoService> crypto_service,
    TSharedRef<IHttpService> http_service,
    TSharedRef<IFeaturePhasingService> feature_phasing_service);

  virtual ~CFCoreAnalyticsImpl() = default;

  // ICFCoreAnalytics
public:
  virtual bool SendGamePlaySession(
    const FGamePlaySessionParams& params) override;

  virtual bool SendModBrowsingFunnelImpression(
    const FModBrowsingFunnelParams& params) override;

  virtual bool SendModBrowsingFunnelAction(
    const FModBrowsingFunnelParams& params) override;

  // ICFCoreAnalyticsInternal
public:
  virtual TSharedRef<ICFCoreAnalytics> PublicInterface() override {
    return StaticCastSharedRef<ICFCoreAnalytics>(this->AsShared());
  }

  virtual bool SendModInstall(
    const FModInstallAnalyticsParams& params) override;

  virtual bool SendLanguageTestAction(
    const FLanguageTestActionParams& params) override;

private:
  FString GetCFUserId() const;

  template<typename InStructType>
  bool SendAnalytic(FString event_name,
                    const InStructType& params,
                    bool clean_empty_fields = false);

  bool SendAnalytic(UrlSearchParams* params,
                    TSharedPtr<FJsonObject> extra_json);

  void FillDefaultSearchParams(UrlSearchParams* params);
  void FillDefaultExtraParams(TSharedPtr<FJsonObject> extra_json);

  TSharedPtr<FJsonValue> GetJsonValueString(const FString& str);
  TSharedPtr<FJsonValue> GetJsonValueNumber(int64 num);
  bool Serialize(TSharedPtr<FJsonObject> json, OUT FString& output);
  void RemoveEmptyJsonFields(TSharedPtr<FJsonObject> json);
  bool IsCategoryEnabled(FString event_name);
  bool IsCategoryFeatureFlagEnabledForUser(ECFCoreAnalyticsCategory category);

private:
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IUserContextService> user_context_service_;
  TSharedRef<ICryptoService> crypto_service_;
  TSharedRef<IHttpService> http_service_;
  TSharedRef<IFeaturePhasingService> feature_phasing_service_;
};

} // namespace cfcore