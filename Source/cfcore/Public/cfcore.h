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
#include <Templates/UniquePtr.h>
#include <common/cfcore_error.h>
#include <api/cfcore_api.h>
#include <library/cfcore_library.h>
#include <authentication/cfcore_authentication.h>
#include <creation/cfcore_creation.h>
#include <premium/cfcore_premium_mods.h>
#include <analytics/cfcore_analytics.h>
#include <subscription/cfcore_subscription.h>
#include <utils/cfcore_utils.h>
#include <models/cfcore_settings.h>
#include <models/cfcore_updatable_settings.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ICFCore {
public:
  DECLARE_DELEGATE_OneParam(FInitializeDelegate, TOptional<FCFCoreError>);
  DECLARE_DELEGATE_OneParam(FUninitializeDelegate, TOptional<FCFCoreError>);
  DECLARE_DELEGATE_OneParam(FUpdateSettingsDelegate,
                            TOptional<FCFCoreError>);

public:
  virtual ~ICFCore() = default;

public:
  virtual bool IsInitialized() = 0;

  virtual ICFCoreApi* Api() = 0;
  virtual ICFCoreLibrary* Library() = 0;
  virtual ICFCoreAuthentication* Authentication() = 0;
  virtual ICFCoreCreation* Creation() = 0;
  virtual ICFCorePremiumMods* PremiumMods() = 0;
  virtual ICFCoreAnalytics* Analytics() = 0;
  virtual ICFCoreSubscription* Subscription() = 0;
  virtual ICFCoreUtils* Utils() = 0;

public:
  virtual void Initialize(const FCFCoreSettings& settings,
                          FInitializeDelegate delegate) = 0;

  virtual void Uninitialize(FUninitializeDelegate delegate) = 0;

  virtual void UpdateSettings(const FCFCoreUpdatableSettings& settings,
                              FUpdateSettingsDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCore)
