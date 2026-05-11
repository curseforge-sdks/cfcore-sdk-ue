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
#include "install_analytics_tagger_premium_mods_impl.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallAnalyticsTaggerPremiumModsImpl::InstallAnalyticsTaggerPremiumModsImpl(
  TSharedRef<ApiAuthDelegatesDispatcher> dispatcher) {

  listener_guid_ = FGuid::NewGuid();

  auto listener = MakeShared<CFCoreApiAuthorizedDelegates>();
  listener->get_my_premium_mods_v2.BindRaw(
    this,
    &InstallAnalyticsTaggerPremiumModsImpl::OnOwnedPremiumModsUpdated);

  dispatcher->Register(listener_guid_, listener);
}

// -----------------------------------------------------------------------------
void InstallAnalyticsTaggerPremiumModsImpl::Tag(
  const FHandleInstallContext& context,
  FModInstallAnalyticsParams& analytics_params) const {

  int64 mod_id = context.installed_mod.details.id;
  analytics_params.isTBYB = IsTrialMod(mod_id);
}

// -----------------------------------------------------------------------------
bool InstallAnalyticsTaggerPremiumModsImpl::IsTrialMod(int64 mod_id) const {
  if (!cached_.IsSet()) {
    return false;
  }

  return cached_->trialMods.ContainsByPredicate(
    [mod_id](const FUserTrialData& trial) {
      return trial.modId == mod_id && !trial.isExpired;
    });
}

// -----------------------------------------------------------------------------
void InstallAnalyticsTaggerPremiumModsImpl::OnOwnedPremiumModsUpdated(
  const TOptional<FOwnedPremiumMods>& opt_owned,
  const FString& /*signature*/,
  const TOptional<FCFCoreApiResponseError>& opt_err) {

  if (opt_err.IsSet() || !opt_owned.IsSet()) {
    return;
  }

  cached_ = opt_owned;
}
