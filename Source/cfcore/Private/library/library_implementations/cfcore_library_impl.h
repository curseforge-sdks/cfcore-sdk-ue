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
#include <common/delegate_broadcaster.h>
#include <library/models/handle_install_context.h>
#include "library/cfcore_library_internal.h"
#include <library/cfcore_library.h>

#include <library/models/install_mod_params.h>
#include "library/services/mods_installation_service.h"
#include "library/services/installed_mods_info/installed_mods_info_service.h"

#include "library/cfcore_library_delegate.h"
#include "library/services/install_analytics_tagger/install_analytics_tagger.h"

namespace cfcore {
class IGetInstalledModsService;
class IModsInstallationService;
class IModsValidationService;
class IDynamicContentService;
class IModsCleanupService;
class IInstalledModsSynchronizeWithServerService;
class IUnmanagedModsService;
class ICFCoreAnalyticsInternal;

// -----------------------------------------------------------------------------
class CFCoreLibraryImpl : public ICFCoreLibraryInternal,
                          public ICFCoreLibrary,
                          public TSharedFromThis<CFCoreLibraryImpl> {
public:
  CFCoreLibraryImpl(
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
    TSharedRef<IGetInstalledModsService> get_installed_mods_service,
    TSharedRef<IModsInstallationService> mods_inst_srv,
    TSharedRef<IModsValidationService> mods_validation_service,
    TSharedRef<IDynamicContentService> dynamic_content_service,
    TSharedRef<IModsCleanupService> mods_cleanup_service,
    TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service,
    TSharedRef<IUnmanagedModsService> unmanaged_mods_service,
    TSharedRef<ICFCoreAnalyticsInternal> cfcore_analytics_internal,
    TSharedRef<IDelegateBroadcaster<ICFCoreLibraryDelegate>> delegates,
    TSharedRef<InstallAnalyticsTaggerArray> install_analytics_taggers);
  virtual ~CFCoreLibraryImpl() = default;

// ICFCoreLibraryInternal
public:
  virtual TSharedPtr<ICFCoreLibrary> GetLibrary() override;

  virtual TFuture<FCFCoreError> Initialize() override;
  virtual void SetClientServerLibrary(
    TSharedPtr<ICFCoreClientServerLibrary> client_server_library) override;

// ICFCoreLibrary
public:
  virtual FModInstallProgressDelegate& OnModInstallProgress() override;
  virtual FModInstalledDelegate& OnModInstalled() override;

  virtual ICFCoreClientServerLibrary* ClientServerLibrary() override {
    return cfcore_client_server_library_.Get();
  }

  virtual void SyncWithServer(FSyncWithServerDelegate delegate) override;

  virtual void GetInstalledMods(FGetInstalledModsDelegate delegate) override;
  virtual void GetSystemInstalledMods(
    FGetInstalledModsDelegate delegate) override;

  virtual void GetModsDirInfo(FGetModsDirInfoDelegate delegate) override;

  virtual void Install(FCFCoreMod mod,
                       FInstallProgressDelegate on_progress,
                       FInstalledDelegate on_installed) override;

  virtual void Install(const FCFCoreMod& mod,
                       const FFile& file,
                       FInstallProgressDelegate on_progress,
                       FInstalledDelegate on_installed) override;

  virtual void Install(const FCFCoreMod& mod,
                       const FFile& file,
                       const FInstallModAdditionalParams& additional_params,
                       FInstallProgressDelegate on_progress,
                       FInstalledDelegate on_installed) override;

  // TODO(twolf): Not only download
  virtual void CancelInstallation(int64 mod_id,
                                  FCancelInstallDelegate delegate) override;

  virtual void Uninstall(int64 mod_id, FUninstallDelegate delegate) override;

  virtual void PerformModsValidation(
    const TArray<FInstalledMod>& installed_mods_to_validate,
    FPerformModsValidationDelegate delegate) override;

  virtual void UpdateInstalledModsProperties(
    const TArray<FInstalledModProperties>& installed_mod_properties,
    FUpdateInstalledModsPropertiesDelegate delegate) override;

  virtual void CleanTempDir(FCleanTempDirDelegate delegate) override;

private:
  FCFCoreError ValidateInstallRequest(
    const FCFCoreMod& mod,
    const FFile* file_to_install,
    const FInstallModAdditionalParams& additional_params);

  void HandleInstall(TSharedRef<FInstallModParams> install_params);

  TFuture<FCFCoreError> PerformInstalledModsValidations();

  TFuture<FHandleInstallContext> HandleInstallDynamicContent(
    TSharedRef<FInstallModParams> install_params);

  TFuture<FHandleInstallContext> SetInstalling(
    TSharedRef<FInstallModParams> install_params);

  TFuture<FHandleInstallContext> StartInstall(
    TSharedRef<FInstallModParams> install_params);

  TFuture<FCFCoreError> SetUninstalled(FInstalledMod& installed_mod);
  void StartUninstall(const FInstalledMod& installed_mod,
                      FUninstallDelegate delegate);

  TArray<FInstalledMod> GetInvalidInstalledMods(
    const TArray<FInstalledMod>& filter);

  TArray<FInstalledMod> FilterOutHiddenMods(
    const TArray<FInstalledMod>& installed_mods) const;
  bool ShouldHideInstalledMod(const FInstalledMod& installed_mod) const;

  void SendModInstallAnalytics(const FHandleInstallContext& context,
                               TSharedRef<FInstallModParams> inst_params);

  TFuture<IInstalledModsInfoService::InstallModsInfoResult>
  UpdateInstalledModState(const FCFCoreError& err,
                          const FInstalledMod& installed_mod,
                          TSharedRef<FInstallModParams> inst_params);

  void FillModInstallOrigin(FModInstallAnalyticsParams& analytics_params,
                            const FInstallModAdditionalParams& extra_params);

  bool WasInstallationCancelled(const FCFCoreError& err) const;

private:
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
  TSharedRef<IGetInstalledModsService> get_installed_mods_service_;
  TSharedRef<IModsInstallationService> mods_installation_service_;
  TSharedRef<IModsValidationService> mods_validation_service_;
  TSharedRef<IDynamicContentService> dynamic_content_service_;
  TSharedPtr<ICFCoreClientServerLibrary> cfcore_client_server_library_;
  TSharedRef<IModsCleanupService> mods_cleanup_service_;
  TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service_;
  TSharedRef<IUnmanagedModsService> unmanaged_mods_service_;
  TSharedRef<ICFCoreAnalyticsInternal> cfcore_analytics_internal_;
  TSharedRef<InstallAnalyticsTaggerArray> install_analytics_taggers_;

  TSharedRef<IDelegateBroadcaster<ICFCoreLibraryDelegate>> delegates_;

  FModInstallProgressDelegate dummy_mod_install_progress_delegate_;
  FModInstalledDelegate dummy_mod_installed_delegate_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreLibraryImpl)
