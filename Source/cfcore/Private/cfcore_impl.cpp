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
#include "cfcore_impl.h"
#include <common/macros.h>
#include <common/cfcore_log.h>
#include <common/future_utils.h>

#include <services/logger/logger_context.h>

#include <services/lifetime/lifetime_internal_service.h>
#include <authentication/cfcore_authentication_internal.h>
#include <authentication/cfcore_authentication.h>
#include <creation/cfcore_creation.h>
#include <premium/cfcore_premium_mods.h>
#include <analytics/cfcore_analytics.h>
using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreImpl::CFCoreImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<ILifetimeInternalService> lifetime_internal_service,
  TSharedRef<ILifetimeService> lifetime_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<ICFCoreLibraryInternal> cfcore_lib_internal,
  TSharedRef<ICFCoreAuthenticationInternal> cfcore_auth_internal,
  TSharedRef<ICFCoreCreation> cfcore_creation,
  TSharedRef<ICFCorePremiumMods> premium_mods,
  TSharedRef<ICFCoreAnalytics> cfcore_analytics,
  TSharedRef<IUserContextService> user_context_service,
  TSharedRef<ICFCoreSubscriptionInternal> cfcore_subscription_internal,
  TSharedRef<ICFCoreUtils> cfcore_utils) :
  initialized_(false),
  settings_service_(settings_service),
  lifetime_internal_service_(lifetime_internal_service),
  lifetime_service_(lifetime_service),
  cfcore_api_(cfcore_api),
  cfcore_library_internal_(cfcore_lib_internal),
  cfcore_authentication_internal_(cfcore_auth_internal),
  cfcore_creation_(cfcore_creation),
  premium_mods_(premium_mods),
  cfcore_analytics_(cfcore_analytics),
  user_context_service_(user_context_service),
  cfcore_subscription_internal_(cfcore_subscription_internal),
  cfcore_utils_(cfcore_utils) {
}

// -----------------------------------------------------------------------------
void CFCoreImpl::Initialize(const FCFCoreSettings& settings,
                            FInitializeDelegate delegate) {

  TOptional<FCFCoreError> opt_err =
    settings_service_->ValidateSettings(settings);
  if (opt_err.IsSet()) {
    delegate.ExecuteIfBound(opt_err);
    return;
  }

  settings_service_->SetSettings(settings);

  initialized_ = false;
  LoggerContext::GetInstance()->Initialize();
  lifetime_internal_service_->Initializing();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this]() { return user_context_service_->Initialize(); },
      [this]() { return cfcore_library_internal_->Initialize(); },
      [this]() { return cfcore_authentication_internal_->Initialize(); },
      [this]() { return cfcore_subscription_internal_->Initialize(); }
  });

  FutureUtils::SequenceAll(functions, &FCFCoreError::isError)
    .Next([this, delegate](FCFCoreError err) {

    if (err.isError) {
      delegate.ExecuteIfBound(err);
      return;
    }

    lifetime_internal_service_->Initialized();
    initialized_ = true;

    delegate.ExecuteIfBound({});
  });
}

// -----------------------------------------------------------------------------
// TODO(twolf): let |cfcore_api_| know we are uninitialized so it cancels all
// outstanding requests
void CFCoreImpl::Uninitialize(FUninitializeDelegate delegate) {
  if (!initialized_) {
    delegate.ExecuteIfBound({});
    return;
  }

  lifetime_service_->Shutdown();
  initialized_ = false;
  delegate.ExecuteIfBound({});
}

// -----------------------------------------------------------------------------
void CFCoreImpl::UpdateSettings(const FCFCoreUpdatableSettings& settings,
                                FUpdateSettingsDelegate delegate) {
  if (!initialized_) {
    delegate.ExecuteIfBound(
      FCFCoreError(ECFCoreErrorCodes::FailedToInitialize));
    return;
  }

  settings_service_->UpdateUpdatableSettings(settings);
  delegate.ExecuteIfBound({});
}
