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
#include "bootstrap.h"
#include <common/constants.h>
#include <common/delegate_dispatcher.h>

#include "class_factory.h"
#include <services/thread_pools_service_impl.h>
#include <services/persistent_model.h>

#include <utils/cfcore_utils_impl.h>
#include <cfcore_impl.h>
#include <services/lifetime/lifetime_service_impl.h>
#include <services/timer/timer_service_impl.h>
#include <services/settings_service_impl.h>
#include <services/logger/logger_service_impl.h>
#include <services/logger/logger_output_file_service_impl.h>

#include <services/logger/startup_info_logging/startup_info_logging.h>

#include <services/logger/startup_info_logging/startup_info_logging_impl_null.h>

#ifdef PLATFORM_WINDOWS
#include <services/logger/startup_info_logging/startup_info_logging_impl_win.h>
#endif

#include <services/hashing/hashing_service_impl.h>
#include <services/directories_service_impl.h>
#include <services/http/http_service_impl.h>
#include <services/http/http_file_service_impl.h>
#include <services/io_throttling/io_throttling_service_impl.h>
#include <services/filesystem/file_system_service_impl.h>
#include <services/filesystem/persistent_file/persistent_log_file_impl.h>
#include <services/filesystem/persistent_file/persistent_binary_file_reader_impl.h>
#include <services/filesystem/persistent_file/persistent_binary_file_writer_impl.h>
#include <services/filesystem/persistent_file/persistent_binary_file_reader_buffered_impl.h>
#include <services/filesystem/persistent_file/persistent_binary_file_writer_buffered_impl.h>
#include <services/filesystem/persistent_file/persistent_binary_file_impl.h>
#include <services/filesystem/persistent_file/persistent_binary_file_buffered_impl.h>
#include <services/compression/compression_service_impl.h>
#include <services/user_context/shared_context_service_impl.h>
#include <services/user_context/user_context_service_impl.h>
#include <services/crypto_service/crypto_service_impl.h>

#include <services/binary_diff_service/binary_diff_service_bsdiff_impl.h>
#include <services/binary_diff_service/binary_diff_service_hdiff_impl.h>

#include <analytics/cfcore_analytics_internal.h>
#include <analytics/cfcore_analytics_impl.h>
#include <analytics/language_test_analytics_service_impl.h>

#include <api/services/api_helper_service_impl.h>
#include <api/cfcore_api_authorized_impl.h>
#include <api/cfcore_api_authorized_delegates.h>
#include <api/cfcore_api_impl.h>
#include <api/cfcore_api_delegates.h>
#include <api/services/user_phasing_percent_service_impl.h>
#include <api/services/feature_phasing_service_impl.h>
#include <api/services/diagnostics_service_impl.h>


#include <library/services/installation_commands/installation_command_utils_service_impl.h>
#include <library/services/installation_commands/models/diff_method_to_service_map.h>

#include <library/services/installation_commands/models/enums/installation_commands.h>
#include <library/services/installation_commands/chunk_completion_persistency_service_impl.h>
#include <library/services/installation_commands/installation_command_download_impl.h>
#include <library/services/installation_commands/installation_command_verify_hash_impl.h>
#include <library/services/installation_commands/installation_command_unzip_impl.h>
#include <library/services/installation_commands/installation_command_cleanup_impl.h>
#include <library/services/installation_commands/installation_command_move_temp_output_dir_impl.h>
#include <library/services/installation_commands/installation_command_delete_output_dir_impl.h>
#include <library/services/installation_commands/installation_command_download_delta_impl.h>
#include <library/services/installation_commands/installation_command_apply_delta_patch_impl.h>
#include <library/services/installation_commands/installation_command_validate_installed_mod_impl.h>
#include <library/services/installation_commands/installation_command_delta_diff_supported_impl.h>

#include <library/services/installation_commands/visitors/installation_command_visitor_initialize.h>

#include <library/services/installation_commands/installation_commands_runner_service_impl.h>

#include <library/services/installation_strategies/installation_strategy_impl.h>

#include <library/services/installed_mods_info/installed_mods_info_service_delegate.h>
#include <library/services/installed_mods_info/installed_mods_info_system_service_impl.h>
#include <library/services/installed_mods_info/installed_mods_info_user_service_impl.h>
#include <library/services/installed_mods_info/get_installed_mods_service_impl.h>
#include <library/services/chunked_file_download/chunked_file_download_service_impl.h>
#include <library/services/chunked_file_download/chunked_download_prep_service_impl.h>
#include <library/services/mods_installation_queue_service_impl.h>
#include <library/services/mods_cleanup/mods_cleanup_service_impl.h>
#include <library/services/mods_installation_service_impl.h>
#include <library/services/mods_validation/mods_validation_service_impl.h>
#include <library/services/dynamic_content_service_impl.h>
#include <library/services/fingerprints_service_impl.h>
#include <library/services/installed_mod_pending_status_service_impl.h>
#include <library/services/unmanaged_mods_service_impl.h>

#include <library/services/client_server_library/client_server_library_shared_impl.h>
#include <library/services/client_server_library/client_server_library_server_impl.h>
#include <library/services/client_server_library/client_server_library_client_impl.h>

#include <library/services/installed_mods_synchronize_with_server_service_impl.h>

#include <library/cfcore_library_delegate.h>
#include <library/library_implementations/cfcore_outer_library_impl.h>
#include <library/library_implementations/cfcore_library_impl.h>
#include <library/library_implementations/cfcore_progress_library_impl.h>
#include <library/services/install_analytics_tagger/install_analytics_tagger_premium_mods_impl.h>
#include <library/cfcore_client_server_library_impl.h>

#include <authentication/cfcore_authentication_impl.h>
#include <creation/services/create_mod_file_chunked_service_impl.h>
#include <creation/cfcore_creation_impl.h>

#include <premium/cfcore_premium_mods_impl.h>
#include <premium/services/premium_mod_purchase_polling_service_impl.h>

#include <subscription/cfcore_subscription_impl.h>
#include <subscription/services/subscribe_installed_mods_service_impl.h>
#include <subscription/services/subscription_sync_service_impl.h>

#include <Interfaces/IPluginManager.h>

#ifndef UE_PLUGIN_NAME
#define UE_PLUGIN_NAME "cfcore"
#endif

namespace cfcore {
// TODO(twolf): Consider using the actual settings passed by user
const uint32 kMaxConcurrentCompressionThreads = 1;
const uint32 kIOThreadPoolSize = 1;
}

using namespace cfcore;

TSharedPtr<TTypeContainer<>> Bootstrap::container_;
TSharedPtr<ILoggerService> Bootstrap::logger_service_;
TSharedPtr<ICFCore> Bootstrap::cfcore_;

// NOTE(twolf): Used to generate dynamic objects so that the compiler won't
// optimize them out - see more in |ClassFactory|'s header file
#define CREATE_DUMMY_INSTANCE(T) { \
  auto dummy = container_->GetInstance<T>(); \
}

#define REGISTER_DELEGATE_DISPATCHER(DelegateInterface) { \
  container_->RegisterClass<DelegateDispatcher<DelegateInterface>, DelegateDispatcher<DelegateInterface>>(ETypeContainerScope::Process); \
\
  container_->RegisterFactory<IDelegateRegistrar<DelegateInterface>>( \
    []() -> TSharedRef<IDelegateRegistrar<DelegateInterface>> { \
      auto impl = container_->GetInstance<DelegateDispatcher<DelegateInterface>>(); \
      return StaticCastSharedRef<IDelegateRegistrar<DelegateInterface>>(impl); \
    } \
  ); \
\
  container_->RegisterFactory<IDelegateBroadcaster<DelegateInterface>>( \
    []() -> TSharedRef<IDelegateBroadcaster<DelegateInterface>> { \
      auto impl = container_->GetInstance<DelegateDispatcher<DelegateInterface>>(); \
      return StaticCastSharedRef<IDelegateBroadcaster<DelegateInterface>>(impl); \
    } \
  ); \
}

// -----------------------------------------------------------------------------
// NOTE(twolf): When adding a new service to the dependency injection, remember:
// 1. To add an Expose_TNameOf(IService) above
// 2. Have the service implementation inherit the service interface first,
// before other interfaces/classes - seems to cause issues with the vptr
//
// NOTE(twolf): Read docs in TypeContainer.h for more info on how this works.
void Bootstrap::Create() {
  container_ = MakeShared<TTypeContainer<>>();

  FInternalSettings internal_settings = Bootstrap::CreateInternalSettings();
  TSharedRef<ISettingsService> settings_service =
    MakeShared<SettingsServiceImpl>(internal_settings);
  container_->RegisterInstance<ISettingsService>(settings_service);

  container_->RegisterInstance<IThreadPoolsService>(
    MakeShared<ThreadPoolsServiceImpl>(CreateLoggerThreadPool(),
                                       CreateIOThreadPool(),
                                       CreateCompressionThreadPool()));

  container_->RegisterClass<IDirectoriesService, DirectoriesServiceImpl, ISettingsService>(ETypeContainerScope::Process);

  // Lifetime service
  container_->RegisterClass<LifetimeServiceImpl, LifetimeServiceImpl>(ETypeContainerScope::Process);
  container_->RegisterFactory<ILifetimeInternalService>(
    []() -> TSharedRef<ILifetimeInternalService> {
      auto impl = container_->GetInstance<LifetimeServiceImpl>();
      return StaticCastSharedRef<ILifetimeInternalService>(impl);
    }
  );
  container_->RegisterFactory<ILifetimeService>(
    []() -> TSharedRef<ILifetimeService> {
      auto impl = container_->GetInstance<LifetimeServiceImpl>();
      return StaticCastSharedRef<ILifetimeService>(impl);
    }
  );

  container_->RegisterClass<ITimerService, TimerServiceImpl, ILifetimeService>(ETypeContainerScope::Process);
  container_->RegisterClass<IIOThrottlingService, IOThrottlingServiceImpl, ISettingsService>(ETypeContainerScope::Process);

  container_->RegisterClass<IPersistentLogFile, PersistentLogFileImpl>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<IPersistentLogFile>>(MakeShared<ClassFactory<IPersistentLogFile>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<IPersistentLogFile>);

  container_->RegisterClass<PersistentBinFileReaderInternal, PersistentBinaryFileReaderImpl, IIOThrottlingService>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<PersistentBinFileReaderInternal>>(MakeShared<ClassFactory<PersistentBinFileReaderInternal>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<PersistentBinFileReaderInternal>);

  container_->RegisterClass<PersistentBinFileWriterInternal, PersistentBinaryFileWriterImpl, IIOThrottlingService>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<PersistentBinFileWriterInternal>>(MakeShared<ClassFactory<PersistentBinFileWriterInternal>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<PersistentBinFileWriterInternal>);

  container_->RegisterClass<PersistentBinFileReaderBufferedInternal, PersistentBinaryFileReaderBufferedImpl, IIOThrottlingService>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<PersistentBinFileReaderBufferedInternal>>(MakeShared<ClassFactory<PersistentBinFileReaderBufferedInternal>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<PersistentBinFileReaderBufferedInternal>);

  container_->RegisterClass<PersistentBinFileWriterBufferedInternal, PersistentBinaryFileWriterBufferedImpl, IIOThrottlingService>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<PersistentBinFileWriterBufferedInternal>>(MakeShared<ClassFactory<PersistentBinFileWriterBufferedInternal>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<PersistentBinFileWriterBufferedInternal>);

  container_->RegisterClass<IPersistentBinaryFile, PersistentBinaryFileImpl, IIOThrottlingService>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<IPersistentBinaryFile>>(MakeShared<ClassFactory<IPersistentBinaryFile>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<IPersistentBinaryFile>);

  container_->RegisterClass<IPersistentBinaryFileBuffered, PersistentBinaryFileBufferedImpl, IPersistentBinaryFile>(ETypeContainerScope::Instance);
  container_->RegisterInstance<ClassFactory<IPersistentBinaryFileBuffered>>(MakeShared<ClassFactory<IPersistentBinaryFileBuffered>>(container_));
  CREATE_DUMMY_INSTANCE(ClassFactory<IPersistentBinaryFileBuffered>);

  container_->RegisterClass<IFileSystemService, FileSystemServiceImpl, IIOThrottlingService, IThreadPoolsService, ClassFactory<IPersistentLogFile>, ClassFactory<PersistentBinFileReaderInternal>, ClassFactory<PersistentBinFileWriterInternal>, ClassFactory<PersistentBinFileReaderBufferedInternal>, ClassFactory<PersistentBinFileWriterBufferedInternal>, ClassFactory<IPersistentBinaryFileBuffered>>(ETypeContainerScope::Process);

  RegisterStartupInfoLogging();

  container_->RegisterClass<ILoggerOutputService, LoggerOutputFileServiceImpl, ISettingsService, IFileSystemService, IDirectoriesService>(ETypeContainerScope::Process);

  container_->RegisterClass<IHttpService, HttpServiceImpl>(ETypeContainerScope::Process);
  container_->RegisterClass<IHashingService, HashingServiceImpl, IThreadPoolsService, IFileSystemService>(ETypeContainerScope::Process);
  container_->RegisterClass<IHttpFileService, HttpFileServiceImpl, ISettingsService>(ETypeContainerScope::Process);

  container_->RegisterClass<PersistentModel<FSharedContextInfo>, PersistentModel<FSharedContextInfo>, ILifetimeService, IFileSystemService>(ETypeContainerScope::Instance);
  container_->RegisterClass<ISharedContextService, SharedContextServiceImpl, ISettingsService, IDirectoriesService, PersistentModel<FSharedContextInfo>>(ETypeContainerScope::Process);
  container_->RegisterClass<PersistentModel<FUserContextInfo>, PersistentModel<FUserContextInfo>, ILifetimeService, IFileSystemService>(ETypeContainerScope::Instance);
  container_->RegisterClass<IUserContextService, UserContextServiceImpl, ISettingsService, IDirectoriesService, ISharedContextService, PersistentModel<FUserContextInfo>>(ETypeContainerScope::Process);

  container_->RegisterClass<IApiHelperService, ApiHelperServiceImpl, ISettingsService, IHttpService, IHttpFileService, IUserContextService>(ETypeContainerScope::Process);
  container_->RegisterClass<ICompressionService, CompressionServiceImpl, IThreadPoolsService, IFileSystemService>(ETypeContainerScope::Process);
  container_->RegisterClass<ICryptoService, CryptoServiceImpl>(ETypeContainerScope::Process);

  container_->RegisterClass<ILoggerService, LoggerServiceImpl, ILifetimeService, IThreadPoolsService, ILoggerOutputService, IStartupInfoLogging, ICompressionService>(ETypeContainerScope::Process);

  RegisterBinaryDiffServices();

  container_->RegisterClass<IChunkedDownloadPrepService, ChunkedDownloadPrepServiceImpl, IApiHelperService, IHttpService, IFileSystemService>(ETypeContainerScope::Process);
  container_->RegisterClass<IChunkedFileDownloadService, ChunkedFileDownloadServiceImpl, IApiHelperService, IHttpFileService, IFileSystemService, ITimerService, ILifetimeService, ISettingsService, IChunkedDownloadPrepService>(ETypeContainerScope::Process);

  REGISTER_DELEGATE_DISPATCHER(IInstalledModsInfoServiceDelegate);
  REGISTER_DELEGATE_DISPATCHER(IModsCleanupServiceDelegate);
  REGISTER_DELEGATE_DISPATCHER(IModsValidationServiceDelegate);
  REGISTER_DELEGATE_DISPATCHER(ICFCoreLibraryDelegate);

  container_->RegisterClass<IUserPhasingPercentService, UserPhasingPercentServiceImpl, ISharedContextService>(ETypeContainerScope::Process);
  container_->RegisterClass<CFCoreApiDelegatesDispatcher<CFCoreApiDelegates>, CFCoreApiDelegatesDispatcher<CFCoreApiDelegates>>(ETypeContainerScope::Process);
  container_->RegisterClass<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>, CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>(ETypeContainerScope::Process);
  container_->RegisterClass<IFeaturePhasingService, FeaturePhasingServiceImpl, IUserPhasingPercentService, CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>(ETypeContainerScope::Process);

  RegisterCFCoreAnalyticsImpl();
  RegisterLanguageTestAnalyticsService();
  RegisterCFCoreApiImpl();

  container_->RegisterClass<IDiagnosticsService, DiagnosticsServiceImpl, ILoggerService, ICFCoreApiAuthorized,  IDirectoriesService, IFileSystemService, IHttpFileService, CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>(ETypeContainerScope::Process);

  RegisterCFCoreAuthenticationImpl();

  container_->RegisterClass<ICreateModFileChunkedService, CreateModFileChunkedServiceImpl, ILifetimeService, ICFCoreApiCreation, IFileSystemService>(ETypeContainerScope::Process);
  container_->RegisterClass<ICFCoreCreation, CFCoreCreationImpl, ICFCoreApi, ICFCoreAuthentication, ICreateModFileChunkedService, IFileSystemService>(ETypeContainerScope::Process);

  container_->RegisterClass<IModsInstallationQueueService, ModsInstallationQueueServiceImpl, ISettingsService>(ETypeContainerScope::Process);
  container_->RegisterClass<IInstallationCommandUtilsService, InstallationCommandUtilsServiceImpl, IDirectoriesService, IFileSystemService, IFeaturePhasingService>(ETypeContainerScope::Process);
  container_->RegisterClass<IFingerprintsService, FingerprintsServiceImpl, IDirectoriesService, IFileSystemService, IHashingService, ISettingsService>(ETypeContainerScope::Process);

  RegisterInstalledModInfoServices();
  RegisterUnmanagedModsService();
  RegisterInstalledModPendingStatusService();

  container_->RegisterClass<IInstalledModsSynchronizeWithServerService, InstalledModsSynchronizeWithServerServiceImpl, ISettingsService, IInstalledModsInfoService, ICFCoreApi, IModsInstallationQueueService>(ETypeContainerScope::Process);

  container_->RegisterClass<IPremiumModPurchasePollingService, PremiumModPurchasePollingServiceImpl, ITimerService>(ETypeContainerScope::Process);
  container_->RegisterClass<ICFCorePremiumMods, CFCorePremiumModsImpl, ISettingsService, ICryptoService, ICFCoreApi, IPremiumModPurchasePollingService>(ETypeContainerScope::Process);
  container_->RegisterClass<IModsValidationService, ModsValidationServiceImpl, IDirectoriesService, IFileSystemService, IInstalledModsInfoService, IFingerprintsService, ICFCoreApi, IFeaturePhasingService, ISettingsService, IDelegateBroadcaster<IModsValidationServiceDelegate>>(ETypeContainerScope::Process);
  container_->RegisterClass<IDynamicContentService, DynamicContentServiceImpl, ISettingsService, IInstalledModsInfoService, IInstalledModsSynchronizeWithServerService, ICFCorePremiumMods>(ETypeContainerScope::Process);

  RegisterInstallCommands();

  container_->RegisterClass<IInstallationCommandsRunnerService, InstallationCommandsRunnerServiceImpl, InstallationCommandsMap>(ETypeContainerScope::Process);

  RegisterInstallStrategies();

  container_->RegisterClass<IModsCleanupService, ModsCleanupServiceImpl, IFeaturePhasingService, IInstallationCommandUtilsService, IGetInstalledModsService, IDirectoriesService, IFileSystemService, IDelegateBroadcaster<IModsCleanupServiceDelegate>>(ETypeContainerScope::Process);
  container_->RegisterClass<IModsInstallationService, ModsInstallationServiceImpl, ILifetimeService, IDirectoriesService, IFileSystemService, IModsInstallationQueueService, InstallationStrategyArray>(ETypeContainerScope::Process);

  RegisterCFCoreLibrary();
  RegisterCFCoreClientServerLibrary();
  RegisterCFCoreSubscription();

  container_->RegisterClass<ICFCoreUtils, CFCoreUtilsImpl, ICompressionService>(ETypeContainerScope::Process);
  container_->RegisterClass<ICFCore, CFCoreImpl, ISettingsService, ILifetimeInternalService, ILifetimeService, ICFCoreApi, ICFCoreLibraryInternal, ICFCoreAuthenticationInternal, ICFCoreCreation, ICFCorePremiumMods, ICFCoreAnalytics, IUserContextService, ICFCoreSubscriptionInternal, ICFCoreUtils>(ETypeContainerScope::Process);

  Bootstrap::logger_service_ = container_->GetInstance<ILoggerService>();
  Bootstrap::cfcore_ = container_->GetInstance<ICFCore>();
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterBinaryDiffServices() {
  container_->RegisterClass<BinaryDiffServiceBSDiffImpl, BinaryDiffServiceBSDiffImpl, IIOThrottlingService, IThreadPoolsService, IHashingService, IFileSystemService>(ETypeContainerScope::Process);
  container_->RegisterClass<BinaryDiffServiceHDiffImpl, BinaryDiffServiceHDiffImpl, IIOThrottlingService, IThreadPoolsService, IHashingService, IFileSystemService>(ETypeContainerScope::Process);

  auto bsdiff = container_->GetInstance<BinaryDiffServiceBSDiffImpl>();
  auto hdiff = container_->GetInstance<BinaryDiffServiceHDiffImpl>();

  auto diff_method_to_service_map = MakeShared<DiffMethodToServiceMap>(
    DiffMethodToServiceMapBuilder()
    .Add(ECFCoreDiffMethod::GBSDIF42, bsdiff)
    .Add(ECFCoreDiffMethod::HDIFF, hdiff)
  );

  container_->RegisterInstance<DiffMethodToServiceMap>(diff_method_to_service_map);
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterCFCoreAnalyticsImpl() {
  container_->RegisterClass<ICFCoreAnalyticsInternal, CFCoreAnalyticsImpl, ISettingsService, IUserContextService, ICryptoService, IHttpService, IFeaturePhasingService>(ETypeContainerScope::Process);

  container_->RegisterFactory<ICFCoreAnalytics>(
    []() -> TSharedRef<ICFCoreAnalytics> {
      auto analytics = container_->GetInstance<ICFCoreAnalyticsInternal>();
      return analytics->PublicInterface();
    }
  );
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterLanguageTestAnalyticsService() {
  container_->RegisterClass<LanguageTestAnalyticsServiceImpl,
    LanguageTestAnalyticsServiceImpl,
    ICFCoreAnalyticsInternal,
    IFeaturePhasingService,
    ISettingsService,
    CFCoreApiDelegatesDispatcher<CFCoreApiDelegates>,
    CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>(ETypeContainerScope::Process);

  container_->GetInstance<LanguageTestAnalyticsServiceImpl>();
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterCFCoreApiImpl() {
  container_->RegisterClass<CFCoreApiAuthorizedImpl, CFCoreApiAuthorizedImpl, ISettingsService, IApiHelperService, CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>(ETypeContainerScope::Process);

  container_->RegisterFactory<ICFCoreApiAuthorized>(
    []() -> TSharedRef<ICFCoreApiAuthorized> {
      auto impl = container_->GetInstance<CFCoreApiAuthorizedImpl>();
      return impl;
    }
  );


  container_->RegisterClass<ICFCoreApi, CFCoreApiImpl, ISettingsService, IApiHelperService, ICFCoreApiAuthorized, CFCoreApiDelegatesDispatcher<CFCoreApiDelegates>, IFeaturePhasingService>(ETypeContainerScope::Process);

  // ICFCoreApiAuthentication
  container_->RegisterFactory<ICFCoreApiAuthentication>(
    []() -> TSharedRef<ICFCoreApiAuthentication> {
      auto api = container_->GetInstance<ICFCoreApi>();
      return api->Authentication().ToSharedRef();
    }
  );

  // ICFCoreApiCreation
  container_->RegisterFactory<ICFCoreApiCreation>(
    []() -> TSharedRef<ICFCoreApiCreation> {
      auto api = container_->GetInstance<ICFCoreApi>();
      return api->Creation().ToSharedRef();
    }
  );
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterCFCoreAuthenticationImpl() {
  container_->RegisterClass<CFCoreAuthenticationImpl, CFCoreAuthenticationImpl, ICFCoreApi, IUserContextService>(ETypeContainerScope::Process);

  // ICFCoreAuthentication
  container_->RegisterFactory<ICFCoreAuthentication>(
    []() -> TSharedRef<ICFCoreAuthentication> {
      auto impl = container_->GetInstance<CFCoreAuthenticationImpl>();
      return StaticCastSharedRef<ICFCoreAuthentication>(impl);
    }
  );

  // ICFCoreAuthenticationInternal
  container_->RegisterFactory<ICFCoreAuthenticationInternal>(
    []() -> TSharedRef<ICFCoreAuthenticationInternal> {
      auto impl = container_->GetInstance<CFCoreAuthenticationImpl>();
      return StaticCastSharedRef<ICFCoreAuthenticationInternal>(impl);
    }
  );
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterInstalledModInfoServices() {
  container_->RegisterClass<InstalledModsInfoSystemServiceImpl, InstalledModsInfoSystemServiceImpl, ISettingsService, IDirectoriesService, IFileSystemService, ICFCoreApi, IDelegateBroadcaster<IInstalledModsInfoServiceDelegate>>(ETypeContainerScope::Process);
  container_->RegisterClass<InstalledModsInfoUserServiceImpl, InstalledModsInfoUserServiceImpl, InstalledModsInfoSystemServiceImpl, IUserContextService, ISettingsService>(ETypeContainerScope::Process);

  container_->RegisterFactory<IInstalledModsInfoService>(
    []() -> TSharedRef<IInstalledModsInfoService> {
      auto impl = container_->GetInstance<InstalledModsInfoUserServiceImpl>();
      return StaticCastSharedRef<IInstalledModsInfoService>(impl);
    }
  );

  container_->RegisterClass<IGetInstalledModsService, GetInstalledModsServiceImpl, InstalledModsInfoSystemServiceImpl, InstalledModsInfoUserServiceImpl>(ETypeContainerScope::Process);
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterUnmanagedModsService() {
  container_->RegisterClass<UnmanagedModsServiceImpl, UnmanagedModsServiceImpl, InstalledModsInfoSystemServiceImpl, InstalledModsInfoUserServiceImpl, ISettingsService, IDirectoriesService, IInstallationCommandUtilsService>(ETypeContainerScope::Process);
  auto unmanaged = container_->GetInstance<UnmanagedModsServiceImpl>();
  auto registrar = container_->GetInstance<IDelegateRegistrar<IModsValidationServiceDelegate>>();

  registrar->Register(FGuid::NewGuid(), unmanaged);

  container_->RegisterFactory<IUnmanagedModsService>(
    []() -> TSharedRef<IUnmanagedModsService> {
      auto impl = container_->GetInstance<UnmanagedModsServiceImpl>();
      return StaticCastSharedRef<IUnmanagedModsService>(impl);
    }
  );
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterInstalledModPendingStatusService() {
  container_->RegisterClass<InstalledModPendingStatusServiceImpl, InstalledModPendingStatusServiceImpl, IModsInstallationQueueService, IInstalledModsInfoService, IInstallationCommandUtilsService>(ETypeContainerScope::Process);

  const auto guid = FGuid::NewGuid();

  auto impl = container_->GetInstance<InstalledModPendingStatusServiceImpl>();
  container_->GetInstance<IDelegateRegistrar<IModsValidationServiceDelegate>>()->Register(guid, impl);
  container_->GetInstance<IDelegateRegistrar<IModsCleanupServiceDelegate>>()->Register(guid, impl);
  container_->GetInstance<IDelegateRegistrar<ICFCoreLibraryDelegate>>()->Register(guid, impl);
  container_->GetInstance<IDelegateRegistrar<IInstalledModsInfoServiceDelegate>>()->Register(guid, impl);
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterInstallCommands() {
  container_->RegisterClass<IChunkCompletionPersistencyService, ChunkCompletionPersistencyServiceImp, IInstalledModsInfoService>(ETypeContainerScope::Process);

  container_->RegisterClass<InstallationCommandDownloadImpl, InstallationCommandDownloadImpl, IInstallationCommandUtilsService, ISettingsService, ICFCoreApi, ICFCorePremiumMods, IHttpService, IChunkedFileDownloadService, IChunkCompletionPersistencyService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandVerifyHashImpl, InstallationCommandVerifyHashImpl, IInstallationCommandUtilsService, IHashingService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandUnzipImpl, InstallationCommandUnzipImpl, IInstallationCommandUtilsService, IFileSystemService, ICompressionService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandCleanupImpl, InstallationCommandCleanupImpl, IInstallationCommandUtilsService, IFileSystemService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandMoveTempOutputDirImpl, InstallationCommandMoveTempOutputDirImpl, IInstallationCommandUtilsService, ISettingsService, IFileSystemService, IDirectoriesService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandDeleteOutputDirImpl, InstallationCommandDeleteOutputDirImpl, IInstallationCommandUtilsService, ISettingsService, IFileSystemService, IDirectoriesService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandDownloadDeltaImpl, InstallationCommandDownloadDeltaImpl, IInstallationCommandUtilsService, ISettingsService, ICFCoreApi, IHttpService, IChunkedFileDownloadService, ISharedContextService, IChunkCompletionPersistencyService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandApplyDeltaPatchImpl, InstallationCommandApplyDeltaPatchImpl, IInstallationCommandUtilsService, IFileSystemService, DiffMethodToServiceMap>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandValidateInstalledModImpl, InstallationCommandValidateInstalledModImpl, IInstallationCommandUtilsService, IModsValidationService, IInstalledModsInfoService>(ETypeContainerScope::Process);
  container_->RegisterClass<InstallationCommandDeltaDiffSupportedImpl, InstallationCommandDeltaDiffSupportedImpl, IInstallationCommandUtilsService, IFeaturePhasingService, ICFCoreApi, DiffMethodToServiceMap>(ETypeContainerScope::Process);

  auto inst_cmd_download = container_->GetInstance<InstallationCommandDownloadImpl>();
  auto inst_cmd_verify_hash = container_->GetInstance<InstallationCommandVerifyHashImpl>();
  auto inst_cmd_unzip = container_->GetInstance<InstallationCommandUnzipImpl>();
  auto inst_cmd_cleanup = container_->GetInstance<InstallationCommandCleanupImpl>();
  auto inst_cmd_mov_tmp_output_dir = container_->GetInstance<InstallationCommandMoveTempOutputDirImpl>();
  auto inst_cmd_del_output_dir = container_->GetInstance<InstallationCommandDeleteOutputDirImpl>();
  auto inst_cmd_download_delta = container_->GetInstance<InstallationCommandDownloadDeltaImpl>();
  auto inst_cmd_apply_delta_patch = container_->GetInstance<InstallationCommandApplyDeltaPatchImpl>();
  auto inst_cmd_validate_installed = container_->GetInstance<InstallationCommandValidateInstalledModImpl>();
  auto inst_cmd_delta_diff_supported = container_->GetInstance<InstallationCommandDeltaDiffSupportedImpl>();

  auto installation_commands = MakeShared<InstallationCommandsMap>(
    InstallationCommandsMapBuilder()
    .Add(inst_cmd_download->GetCommandId(), inst_cmd_download)
    .Add(inst_cmd_verify_hash->GetCommandId(), inst_cmd_verify_hash)
    .Add(inst_cmd_unzip->GetCommandId(), inst_cmd_unzip)
    .Add(inst_cmd_cleanup->GetCommandId(), inst_cmd_cleanup)
    .Add(inst_cmd_mov_tmp_output_dir->GetCommandId(), inst_cmd_mov_tmp_output_dir)
    .Add(inst_cmd_del_output_dir->GetCommandId(), inst_cmd_del_output_dir)
    .Add(inst_cmd_download_delta->GetCommandId(), inst_cmd_download_delta)
    .Add(inst_cmd_apply_delta_patch->GetCommandId(), inst_cmd_apply_delta_patch)
    .Add(inst_cmd_validate_installed->GetCommandId(), inst_cmd_validate_installed)
    .Add(inst_cmd_delta_diff_supported->GetCommandId(), inst_cmd_delta_diff_supported)
  );

  IInstallationCommandVisitorInitialize visitor_initialize;
  for (const auto& pair : *installation_commands) {
    IInstallationCommandPtr installation_command = pair.Value;
    installation_command->Visit(visitor_initialize);
  }

  container_->RegisterInstance<InstallationCommandsMap>(installation_commands);
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterInstallStrategies() {
  container_->RegisterClass<IInstallationStrategy, InstallationStrategyImpl, IInstallationCommandsRunnerService>(ETypeContainerScope::Instance);

  // Delta diff
  auto installtion_strategy_delta_diff =
    container_->GetInstance<IInstallationStrategy>();

  installtion_strategy_delta_diff->SetCommandSequences(
    // install
    TArray<ECFCoreInstallationCommands>{
      ECFCoreInstallationCommands::Cleanup,
      ECFCoreInstallationCommands::DeltaDiffSupported,
      ECFCoreInstallationCommands::ValidateInstalledMod,
      ECFCoreInstallationCommands::DownloadDelta,
      ECFCoreInstallationCommands::VerifyHash,
      ECFCoreInstallationCommands::Unzip,
      ECFCoreInstallationCommands::ApplyDeltaPatch,
      ECFCoreInstallationCommands::DeleteOutputDir,
      ECFCoreInstallationCommands::MoveToOutputDir,
    },
    // uninstall
    TArray<ECFCoreInstallationCommands>{
      ECFCoreInstallationCommands::DeleteOutputDir,
    },
    // cleanup
    TArray<ECFCoreInstallationCommands>{
      ECFCoreInstallationCommands::Cleanup,
  });

  // Full download
  auto installtion_strategy_full =
    container_->GetInstance<IInstallationStrategy>();

  installtion_strategy_full->SetCommandSequences(
    // install
    TArray<ECFCoreInstallationCommands>{
      ECFCoreInstallationCommands::Cleanup,
      ECFCoreInstallationCommands::Download,
      ECFCoreInstallationCommands::VerifyHash,
      ECFCoreInstallationCommands::Unzip,
      ECFCoreInstallationCommands::DeleteOutputDir,
      ECFCoreInstallationCommands::MoveToOutputDir,
    },
    // uninstall
    TArray<ECFCoreInstallationCommands>{
      ECFCoreInstallationCommands::DeleteOutputDir,
    },
    // cleanup
    TArray<ECFCoreInstallationCommands>{
      ECFCoreInstallationCommands::Cleanup,
  });

  // Add all strategies
  auto strategies = MakeShared<InstallationStrategyArray>(
    InstallationStrategyArray({
      installtion_strategy_delta_diff,
      installtion_strategy_full
    }));

  container_->RegisterInstance<InstallationStrategyArray>(strategies);
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterCFCoreLibrary() {
  container_->RegisterClass<InstallAnalyticsTaggerPremiumModsImpl,
                            InstallAnalyticsTaggerPremiumModsImpl,
                            CFCoreApiDelegatesDispatcher<
                              CFCoreApiAuthorizedDelegates>>(
    ETypeContainerScope::Process);

  auto premium_tagger =
    StaticCastSharedRef<IInstallAnalyticsTagger>(
      container_->GetInstance<InstallAnalyticsTaggerPremiumModsImpl>());
  auto taggers = MakeShared<InstallAnalyticsTaggerArray>(
    InstallAnalyticsTaggerArray({
      premium_tagger
    }));
  container_->RegisterInstance<InstallAnalyticsTaggerArray>(taggers);

  container_->RegisterClass<CFCoreLibraryImpl, CFCoreLibraryImpl, IInstalledModsInfoService, IGetInstalledModsService, IModsInstallationService, IModsValidationService, IDynamicContentService, IModsCleanupService, IInstalledModsSynchronizeWithServerService, IUnmanagedModsService, ICFCoreAnalyticsInternal, IDelegateBroadcaster<ICFCoreLibraryDelegate>, InstallAnalyticsTaggerArray>(ETypeContainerScope::Process);
  container_->RegisterClass<CFCoreProgressLibraryImpl, CFCoreProgressLibraryImpl, CFCoreLibraryImpl, CFCoreLibraryImpl>(ETypeContainerScope::Process);
  container_->RegisterClass<CFCoreOuterLibraryImpl, CFCoreOuterLibraryImpl, CFCoreProgressLibraryImpl, CFCoreProgressLibraryImpl, IInstalledModsInfoService>(ETypeContainerScope::Process);

  // ICFCoreLibrary
  container_->RegisterFactory<ICFCoreLibrary>(
    []() -> TSharedRef<ICFCoreLibrary> {
      auto impl = container_->GetInstance<CFCoreOuterLibraryImpl>();
      return StaticCastSharedRef<ICFCoreLibrary>(impl);
    }
  );

  // ICFCoreLibraryInternal
  container_->RegisterFactory<ICFCoreLibraryInternal>(
    []() -> TSharedRef<ICFCoreLibraryInternal> {
      auto impl = container_->GetInstance<CFCoreOuterLibraryImpl>();
      return StaticCastSharedRef<ICFCoreLibraryInternal>(impl);
    }
  );
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterCFCoreClientServerLibrary() {
  container_->RegisterClass<IClientServerLibraryShared, ClientServerLibrarySharedImpl, ISettingsService, ICFCoreLibrary, IInstalledModsSynchronizeWithServerService, IModsValidationService>(ETypeContainerScope::Process);
  container_->RegisterClass<IClientServerLibraryServer, ClientServerLibraryServerImpl, IClientServerLibraryShared, ICFCoreLibrary, ICFCoreApi>(ETypeContainerScope::Process);
  container_->RegisterClass<IClientServerLibraryClient, ClientServerLibraryClientImpl, IClientServerLibraryShared, ICFCoreLibrary, ICFCoreApi, ICFCorePremiumMods, IDynamicContentService>(ETypeContainerScope::Process);
  container_->RegisterClass<ICFCoreClientServerLibrary, CFCoreClientServerLibraryImpl, IClientServerLibraryShared, IClientServerLibraryServer, IClientServerLibraryClient>(ETypeContainerScope::Process);

  auto clnt_srv_lib = container_->GetInstance<ICFCoreClientServerLibrary>();
  auto cfcore_lib_impl = container_->GetInstance<ICFCoreLibraryInternal>();
  cfcore_lib_impl->SetClientServerLibrary(clnt_srv_lib);
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterStartupInfoLogging() {
#if PLATFORM_WINDOWS
  container_->RegisterClass<IStartupInfoLogging, StartupInfoLoggingImplWin>(ETypeContainerScope::Process);
  return;
#else
  container_->RegisterClass<IStartupInfoLogging, StartupInfoLoggingImplNull>(ETypeContainerScope::Process);
#endif
}

// -----------------------------------------------------------------------------
void Bootstrap::RegisterCFCoreSubscription() {
  container_->RegisterClass<ISubscribeInstalledModsService, SubscribeInstalledModsServiceImpl, IUserContextService, ICFCoreApiAuthorized, CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>, IInstalledModsInfoService>(ETypeContainerScope::Process);
  container_->RegisterClass<ISubscriptionSyncService, SubscriptionSyncServiceImpl, ICFCoreLibrary, ICFCoreApiAuthorized, ICFCoreAuthentication, IInstalledModsInfoService, ISettingsService>(ETypeContainerScope::Process);
  container_->RegisterClass<CFCoreSubscriptionImpl, CFCoreSubscriptionImpl, ICFCoreLibrary, ICFCoreApiAuthorized, ITimerService, ICFCoreAuthentication, ISettingsService, ISubscriptionSyncService, ISubscribeInstalledModsService>(ETypeContainerScope::Process);

  // ICFCoreSubscription
  container_->RegisterFactory<ICFCoreSubscription>(
    []() -> TSharedRef<ICFCoreSubscription> {
      auto impl = container_->GetInstance<CFCoreSubscriptionImpl>();
      return StaticCastSharedRef<ICFCoreSubscription>(impl);
    }
  );

  // ICFCoreSubscriptionInternal
  container_->RegisterFactory<ICFCoreSubscriptionInternal>(
    []() -> TSharedRef<ICFCoreSubscriptionInternal> {
      auto impl = container_->GetInstance<CFCoreSubscriptionImpl>();
      return StaticCastSharedRef<ICFCoreSubscriptionInternal>(impl);
    }
  );
}

// -----------------------------------------------------------------------------
// static
FInternalSettings Bootstrap::CreateInternalSettings() {
  FInternalSettings internal_settings;
  internal_settings.chunkedUploadServerUrl = kChunkedUploadServerUrl;
  internal_settings.analyticsUrl = kAnalyticsUrl;

  FString plugin_version = Bootstrap::GetPluginVersion();
  internal_settings.pluginVersion = plugin_version;
  internal_settings.userAgent = FString::Format(TEXT("{0} {1}"), {
    kUserAgentPrefix,
    plugin_version
  });

  internal_settings.platform = GetPlatformName();

  internal_settings.httpFileDownloadTimeoutSecs =
    kDefaultHttpFileDownloadTimeInMinutes * 60;

  return internal_settings;
}

// -----------------------------------------------------------------------------
// static
FString Bootstrap::GetPluginVersion() {
  IPluginManager& plugin_manager = IPluginManager::Get();

  TSharedPtr<IPlugin> plugin = plugin_manager.FindPlugin(UE_PLUGIN_NAME);
  auto descriptor = plugin->GetDescriptor();

  return descriptor.VersionName;
}

// -----------------------------------------------------------------------------
// static
FString Bootstrap::GetPlatformName() {
// TODO(twolf): Instead of #ifdef use #if and reduce the extra == 1 test?
#ifdef PLATFORM_WINGDK
#if PLATFORM_WINGDK == 1
  return TEXT("windows_gdk");
#endif
#endif

#ifdef PLATFORM_XBOXONE
#if PLATFORM_XBOXONE == 1
  return TEXT("xbox_one");
#endif
#endif

#ifdef PLATFORM_XSX
#if PLATFORM_XSX == 1
   if (FPlatformMisc::GetConsoleType() == EXSXConsoleType::Lockhart) {
	 return TEXT("xbox_xss");
   }

  return TEXT("xbox_xsx");
#endif
#endif

#ifdef PLATFORM_MAC
#if PLATFORM_MAC == 1
  return TEXT("mac");
#endif
#endif

#ifdef PLATFORM_PS4
#if PLATFORM_PS4 == 1
  return TEXT("ps4");
#endif
#endif

#ifdef PLATFORM_PS5
#if PLATFORM_PS5 == 1
  return TEXT("ps5");
#endif
#endif

#ifdef PLATFORM_IOS
#if PLATFORM_IOS == 1
  return TEXT("ios");
#endif
#endif

#ifdef PLATFORM_TVOS
#if PLATFORM_TVOS == 1
  return TEXT("tvos");
#endif
#endif

#ifdef PLATFORM_ANDROID
#if PLATFORM_ANDROID == 1
  return TEXT("android");
#endif
#endif

#ifdef PLATFORM_SWITCH
#if PLATFORM_SWITCH == 1
  return TEXT("switch");
#endif
#endif

#ifdef PLATFORM_HOLOLENS
#if PLATFORM_HOLOLENS == 1
  return TEXT("hololens");
#endif
#endif

#ifdef PLATFORM_LINUX
#if PLATFORM_LINUX == 1
  return TEXT("linux");
#endif
#endif

#ifdef PLATFORM_WINDOWS
#if PLATFORM_WINDOWS == 1
  return TEXT("windows");
#endif
#endif
}

// -----------------------------------------------------------------------------
// static
TSharedPtr<FQueuedThreadPool> Bootstrap::CreateLoggerThreadPool() {
  TSharedPtr<FQueuedThreadPool> thread_pool =
    MakeShareable(FQueuedThreadPool::Allocate());

  thread_pool->Create(1,
                      32768U,
                      TPri_Normal,
                      TEXT("cfcore_logger_thread"));

  return thread_pool;
}

// -----------------------------------------------------------------------------
// static
TSharedPtr<FQueuedThreadPool> Bootstrap::CreateIOThreadPool() {
  TSharedPtr<FQueuedThreadPool> thread_pool =
    MakeShareable(FQueuedThreadPool::Allocate());
  thread_pool->Create(kIOThreadPoolSize,
                      32768U,
                      TPri_Normal,
                      TEXT("cfcore_io_threads"));

  return thread_pool;
}

// -----------------------------------------------------------------------------
// static
TSharedPtr<FQueuedThreadPool> Bootstrap::CreateCompressionThreadPool() {

  TSharedPtr<FQueuedThreadPool> thread_pool =
    MakeShareable(FQueuedThreadPool::Allocate());

  // NOTE(twolf): Two reasons for keeping the compression thread single:
  // - Compression is CPU intensive - so in-game, the trade off of parallelizing
  // is not worth it
  // - The current zip implementation is not thread-safe
  thread_pool->Create(1,
                      32768U,
                      TPri_Normal,
                      TEXT("cfcore_compression_threads"));

  return thread_pool;
}
