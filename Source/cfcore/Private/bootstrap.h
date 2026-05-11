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
#include <cfcore.h>
#include <common/macros.h>
#include "models/internal_settings.h"

#include <Misc/TypeContainer.h>
#include <Misc/QueuedThreadPool.h>

namespace cfcore {
class ILoggerService;

// -----------------------------------------------------------------------------
class CFCORE_API Bootstrap {
public:
  static void Create();

  // ---------------------------------------------------------------------------
  inline static TSharedPtr<ILoggerService> GetLoggerService() {
    return logger_service_;
  }

  // ---------------------------------------------------------------------------
  inline static TSharedPtr<ICFCore> GetCFCore() {
    return cfcore_;
  }

  // ---------------------------------------------------------------------------
#if WITH_DEV_AUTOMATION_TESTS
  template<class R>
  inline static TSharedPtr<R> GetService() {
    const FString service_name = TNameOf<R>::GetName();
    check(container_->IsRegistered<R>());
    return container_->GetInstance<R>();
  }
#endif

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Bootstrap);

private:
  static FInternalSettings CreateInternalSettings();
  static void RegisterBinaryDiffServices();
  static void RegisterCFCoreAnalyticsImpl();
  static void RegisterLanguageTestAnalyticsService();
  static void RegisterCFCoreApiImpl();
  static void RegisterCFCoreAuthenticationImpl();

  static void RegisterInstalledModInfoServices();
  static void RegisterUnmanagedModsService();
  static void RegisterInstalledModPendingStatusService();
  static void RegisterInstallCommands();
  static void RegisterInstallStrategies();
  static void RegisterCFCoreLibrary();
  static void RegisterCFCoreClientServerLibrary();
  static void RegisterStartupInfoLogging();
  static void RegisterCFCoreSubscription();

  static FString GetPluginVersion();
  static FString GetPlatformName();
  static TSharedPtr<FQueuedThreadPool> CreateLoggerThreadPool();
  static TSharedPtr<FQueuedThreadPool> CreateIOThreadPool();
  static TSharedPtr<FQueuedThreadPool> CreateCompressionThreadPool();

private:
  static TSharedPtr<TTypeContainer<>> container_;

  static TSharedPtr<ILoggerService> logger_service_;
  static TSharedPtr<ICFCore> cfcore_;
};

} // namespace cfcore