/*MIT License

Copyright (c) 2025 Overwolf Ltd.

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
#include <Async/Future.h>

#include "subscribe_installed_mods_service.h"

#include <library/services/installed_mods_info/installed_mods_info_service.h>
#include <api/cfcore_api_delegates_dispatcher.h>
#include <api/cfcore_api_authorized_delegates.h>

#include <api/models/me.h>

namespace cfcore {
class IUserContextService;
class ICFCoreApiAuthorized;

// -----------------------------------------------------------------------------
class SubscribeInstalledModsServiceImpl :
  public ISubscribeInstalledModsService {
public:
  SubscribeInstalledModsServiceImpl(
    TSharedRef<IUserContextService> user_context_service,
    TSharedRef<ICFCoreApiAuthorized> api_authorized,
    TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
      authorized_delegates_dispatcher,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service);

private:
  void RegisterDelegates(
    TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
      authorized_delegates_dispatcher);

  void TryPerformingSubscriptions(const FMe&);
  TFuture<FCFCoreError> GetAllSubscribedMods(
    TSharedRef<TArray<int64>> out_subscribed_mods,
    const int32 index=0);

  void PerformSubscriptions();
  TFuture<FCFCoreError> SubscribeMods(
    TSharedRef<TArray<int64>> subscribe_mods_ids);

  TSharedRef<TArray<int64>> GetUnsubscribedInstalledModsIds(
    TSharedRef<TArray<int64>> subscribed_mods_ids);

  TArray<int64> PopCurrentBatch(TSharedRef<TArray<int64>> mods_ids);

private:
  TSharedRef<IUserContextService> user_context_service_;
  TSharedRef<ICFCoreApiAuthorized> api_authorized_;
  TSharedRef<CFCoreApiAuthorizedDelegates> delegates_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
};

} // namespace cfcore
