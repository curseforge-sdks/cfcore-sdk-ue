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
#include <Containers/Queue.h>

#include "mods_installation_service.h"
#include "models/install_mod_request.h"

#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/lifetime/lifetime_service.h>

#include <library/services/installation_strategies/installation_strategy.h>

namespace cfcore {
class IModsInstallationQueueService;

typedef TSharedRef<IInstallationStrategy> IInstallationStrategyRef;
typedef TArray<IInstallationStrategyRef> InstallationStrategyArray;

// -----------------------------------------------------------------------------
class ModsInstallationServiceImpl : public IModsInstallationService {
public:
  ModsInstallationServiceImpl(
    TSharedPtr<ILifetimeService> lifetime_service,
    TSharedPtr<IDirectoriesService> dirs_service,
    TSharedPtr<IFileSystemService> file_system_service,
    TSharedRef<IModsInstallationQueueService> installations_queue_service,
    TSharedRef<InstallationStrategyArray> installation_strategies);

// IModsInstallationService
public:
  virtual TFuture<FCFCoreError> Initialize() override;

  virtual TFuture<FCFCoreError> GetModsDirInfo(
    IN OUT TSharedRef<FModsDirInfo> mods_dir_info) override;

  virtual TFuture<InstallResult> Install(
    const FInstalledMod& installed_mod,
    const FInstallModAdditionalParams& additional_params,
    FModsInstallationProgressDelegate delegate) override;

  virtual bool CancelInstall(const FInstalledMod& installed_mod) override;

  virtual TFuture<FCFCoreError> Uninstall(
    const FInstalledMod& installed_mod) override;

  virtual bool IsModInstalling(
    const FInstalledMod& installed_mod) const override;

private:
  void HandleShutdown();

  bool IsAlreadyBeingInstalled(const FInstalledMod& installed_mod) const;
  InstallResult CreateInstallResult(const FCFCoreError& err,
                                    FInstallModRequestPtr request);
  void RemoveRequestAndTriggerNext(FInstallModRequestPtr request);
  void TriggerNextInstallation();

  void ConvertToInstallErrorCode(FCFCoreError& err);

  TFuture<FCFCoreError> RunInstallStrategies(FInstallModRequestPtr request);
  TFuture<FCFCoreError> RunUninstallStrategies(FInstallModRequestPtr request);
  TFuture<FCFCoreError> RunCleanupStrategies(FInstallModRequestPtr request);

  using ModInstallationAction = TFunction<TFuture<FCFCoreError>()>;
  using ModInstallationActionsQueue = TQueue<ModInstallationAction>;

  void AddTrackingEvents(FInstallModRequestPtr request);
  void AddAnalyticsParams(FInstallModRequestPtr request);
  bool IsNewInstallation(const FInstalledMod& installed_mod);

  void TriggerProgressEvent(FModsInstallationProgressDelegate delegate,
                            const FInstalledMod& installed_mod,
                            ELibraryProgressState state);

  bool CancelInstallByModId(int64 mod_id);
  bool CancelInstallByRequest(const FInstallModRequestPtr& request);

private:
  TSharedPtr<ILifetimeService> lifetime_service_;
  TSharedPtr<IDirectoriesService> directories_service_;
  TSharedPtr<IFileSystemService> file_system_service_;
  TSharedRef<IModsInstallationQueueService> installations_queue_service_;
  TSharedRef<InstallationStrategyArray> installation_strategies_;

  FDelegateHandle on_shutdown_handle_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallationStrategyArray)