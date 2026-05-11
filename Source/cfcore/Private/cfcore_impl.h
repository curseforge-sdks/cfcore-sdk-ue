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
#include "cfcore.h"

#include <common/macros.h>
#include <services/settings_service.h>
#include <api/cfcore_api.h>
#include <library/cfcore_library_internal.h>
#include <library/cfcore_library.h>
#include <services/lifetime/lifetime_service.h>
#include <authentication/cfcore_authentication_internal.h>
#include <services/user_context/user_context_service.h>
#include <subscription/cfcore_subscription_internal.h>

namespace cfcore {
class ILifetimeInternalService;
class ICFCoreAuthenticationInternal;

// -----------------------------------------------------------------------------
class CFCoreImpl : public ICFCore {
public:
  CFCoreImpl(TSharedRef<ISettingsService> settings_service,
             TSharedRef<ILifetimeInternalService> lifetime_internal_service,
             TSharedRef<ILifetimeService> lifetime_service,
             TSharedRef<ICFCoreApi> cfcore_api,
             TSharedRef<ICFCoreLibraryInternal> cfcore_library_internal,
             TSharedRef<ICFCoreAuthenticationInternal> cfcore_auth_internal,
             TSharedRef<ICFCoreCreation> cfcore_creation,
             TSharedRef<ICFCorePremiumMods> premium_mods,
             TSharedRef<ICFCoreAnalytics> cfcore_analytics,
             TSharedRef<IUserContextService> user_context_service,
             TSharedRef<ICFCoreSubscriptionInternal> cfcore_sub_internal,
             TSharedRef<ICFCoreUtils> cfcore_utils);

public:
  virtual bool IsInitialized() override {
    return initialized_;
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreApi* Api() override {
    if (!initialized_) {
      return nullptr;
    }

    return &cfcore_api_.Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreLibrary* Library() {
    if (!initialized_) {
      return nullptr;
    }

    return cfcore_library_internal_->GetLibrary().Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreAuthentication* Authentication() {
    if (!initialized_) {
      return nullptr;
    }

    return cfcore_authentication_internal_->GetAuthentication().Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreCreation* Creation() {
    if (!initialized_) {
      return nullptr;
    }

    return &cfcore_creation_.Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCorePremiumMods* PremiumMods() {
    if (!initialized_) {
      return nullptr;
    }

    return &premium_mods_.Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreAnalytics* Analytics() override {
    if (!initialized_) {
      return nullptr;
    }

    return &cfcore_analytics_.Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreSubscription* Subscription() override {
    if (!initialized_) {
      return nullptr;
    }

    return cfcore_subscription_internal_->GetSubscription().Get();
  }

  // ---------------------------------------------------------------------------
  virtual ICFCoreUtils* Utils() override {
    return &cfcore_utils_.Get();
  }

public:
  virtual void Initialize(const FCFCoreSettings& settings,
                          FInitializeDelegate delegate) override;

  virtual void Uninitialize(FUninitializeDelegate delegate) override;

  virtual void UpdateSettings(const FCFCoreUpdatableSettings& settings,
                              FUpdateSettingsDelegate delegate) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CFCoreImpl);

private:
  bool initialized_;

  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<ILifetimeInternalService> lifetime_internal_service_;
  TSharedRef<ILifetimeService> lifetime_service_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<ICFCoreLibraryInternal> cfcore_library_internal_;
  TSharedRef<ICFCoreAuthenticationInternal> cfcore_authentication_internal_;
  TSharedRef<ICFCoreCreation> cfcore_creation_;
  TSharedRef<ICFCorePremiumMods> premium_mods_;
  TSharedRef<ICFCoreAnalytics> cfcore_analytics_;
  TSharedRef<IUserContextService> user_context_service_;
  TSharedRef<ICFCoreSubscriptionInternal> cfcore_subscription_internal_;
  TSharedRef<ICFCoreUtils> cfcore_utils_;
};

} // namespace cfcore
