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
#include <CoreMinimal.h>

#include <api/cfcore_api_authorized_delegates.h>
#include <api/cfcore_api_delegates_dispatcher.h>

#include "install_analytics_tagger.h"

namespace cfcore {

using ApiAuthDelegatesDispatcher =
  CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>;

// -----------------------------------------------------------------------------
// Listens to GetMyPremiumModsV2 responses via the dispatcher, caches the
// result, and tags isTBYB on cf4s_mod_install analytic events.
class InstallAnalyticsTaggerPremiumModsImpl : public IInstallAnalyticsTagger {
public:
  explicit InstallAnalyticsTaggerPremiumModsImpl(
      TSharedRef<ApiAuthDelegatesDispatcher> dispatcher);
  virtual ~InstallAnalyticsTaggerPremiumModsImpl() = default;

// IInstallAnalyticsTagger
public:
  virtual void Tag(const FHandleInstallContext& context,
                   FModInstallAnalyticsParams& analytics_params) const override;

private:
  bool IsTrialMod(int64 mod_id) const;

  void OnOwnedPremiumModsUpdated(
      const TOptional<FOwnedPremiumMods>& opt_owned,
      const FString& signature,
      const TOptional<FCFCoreApiResponseError>& opt_err);

private:
  TOptional<FOwnedPremiumMods> cached_;
  FGuid listener_guid_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallAnalyticsTaggerPremiumModsImpl)
