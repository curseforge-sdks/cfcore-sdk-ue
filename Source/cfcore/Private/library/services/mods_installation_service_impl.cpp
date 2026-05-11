/* MIT License

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
SOFTWARE. */
#include "mods_installation_service_impl.h"
#include <library/models/library_progress.h>

#include <common/future_utils.h>
#include <common/macros.h>
#include <common/constants.h>

#include <services/logger/logger_macros.h>
#include <analytics/cfcore_analytics_internal.h>
#include <library/services/mods_installation_queue_service.h>
#include <Async/Async.h>
#include <Misc/Paths.h>

namespace cfcore {
};

using namespace cfcore;

// -----------------------------------------------------------------------------
ModsInstallationServiceImpl::ModsInstallationServiceImpl(
  TSharedPtr<ILifetimeService> lifetime_service,
  TSharedPtr<IDirectoriesService> directories_service,
  TSharedPtr<IFileSystemService> file_sys_srv,
  TSharedRef<IModsInstallationQueueService> installations_queue_service,
  TSharedRef<InstallationStrategyArray> installation_strategies) :
    lifetime_service_(lifetime_service),
    directories_service_(directories_service),
    file_system_service_(file_sys_srv),
    installations_queue_service_(installations_queue_service),
    installation_strategies_(installation_strategies) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsInstallationServiceImpl::Initialize() {
  on_shutdown_handle_ = lifetime_service_->OnShutdown().AddLambda([this]() {
    HandleShutdown();
  });

  return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsInstallationServiceImpl::GetModsDirInfo(
  IN OUT TSharedRef<FModsDirInfo> mods_dir_info) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto path = MakeShared<FString>(directories_service_->GetModsDirectory());
  auto disk_space_info = MakeShared<DiskSpaceInfo>();
  auto future = file_system_service_->GetDiskSpaceInfo(path, disk_space_info);

  future.Next(
    [mods_dir_info, path, disk_space_info, promise](EFileSystemError err) {

    if (err != EFileSystemError::None) {
      promise->SetValue(FCFCoreError(ECFCoreErrorCodes::FileSystemError));
      return;
    }

    mods_dir_info->pathOnDisk = *path;
    mods_dir_info->totalDiskSizeInBytes = disk_space_info->total_size_in_bytes;
    mods_dir_info->freeDiskSizeInBytes = disk_space_info->free_size_int_bytes;

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<IModsInstallationService::InstallResult>
ModsInstallationServiceImpl::Install(
  const FInstalledMod& installed_mod,
  const FInstallModAdditionalParams& additional_params,
  FModsInstallationProgressDelegate delegate) {

  check(IsInGameThread());

  if (IsAlreadyBeingInstalled(installed_mod)) {
    uint64 id = installed_mod.details.id;
    CFCORE_LOG(Error, TEXT("Mod (%lld) is already being installed"), id);

    auto err_code = ECFCoreErrorCodes::ModAlreadyBeingInstalled;
    InstallResult result{ FCFCoreError(err_code), installed_mod };
    return MakeFulfilledPromise<InstallResult>(result).GetFuture();
  }

  auto req = MakeShared<FInstallModRequest>();
  req->modId = installed_mod.details.id;
  req->installedMod = installed_mod;
  req->on_progress = delegate;
  req->additional_params = additional_params;
  req->promise = MakeUnique<TPromise<InstallResult>>();

  AddTrackingEvents(req);
  AddAnalyticsParams(req);

  installations_queue_service_->Enqueue(req);

  TriggerProgressEvent(delegate, installed_mod, ELibraryProgressState::Pending);
  TriggerNextInstallation();

  return req->promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool ModsInstallationServiceImpl::CancelInstall(const FInstalledMod& mod) {
  return CancelInstallByModId(mod.details.id);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsInstallationServiceImpl::Uninstall(
  const FInstalledMod& installed_mod) {

  auto request = MakeShared<FInstallModRequest>();
  request->modId = installed_mod.details.id;
  request->installedMod = installed_mod;
  request->on_progress =
    IModsInstallationService::FModsInstallationProgressDelegate::CreateLambda(
      [](const FLibraryProgress&) {});

  return RunUninstallStrategies(request);
}

// -----------------------------------------------------------------------------
bool ModsInstallationServiceImpl::IsModInstalling(
  const FInstalledMod& installed_mod) const {

  return IsAlreadyBeingInstalled(installed_mod);
}

// -----------------------------------------------------------------------------
void ModsInstallationServiceImpl::HandleShutdown() {
  TArray<FInstallModRequestPtr> requests;
  installations_queue_service_->GetAllRequests(requests);

  for (const auto& request : requests) {
    CancelInstallByRequest(request);
  }
}

// -----------------------------------------------------------------------------
bool ModsInstallationServiceImpl::IsAlreadyBeingInstalled(
  const FInstalledMod& installed_mod) const {

  int64 mod_id = installed_mod.details.id;
  FInstallModRequestPtr request;
  if (!installations_queue_service_->PeekByModId(mod_id, request)) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
void ModsInstallationServiceImpl::RemoveRequestAndTriggerNext(
  FInstallModRequestPtr request) {

  installations_queue_service_->Remove(request);
  TriggerNextInstallation();
}

// -----------------------------------------------------------------------------
void ModsInstallationServiceImpl::TriggerNextInstallation() {
  check(IsInGameThread());

  FInstallModRequestPtr request;
  if (!installations_queue_service_->MoveNextToInProgress(request)) {
    return;
  }

  if (request->cancelRequest) {
    auto err_code = ECFCoreErrorCodes::InstallCancelled;
    InstallResult result = CreateInstallResult(FCFCoreError(err_code), request);
    request->promise->SetValue(result);
    RemoveRequestAndTriggerNext(request);
    return;
  }

  RunInstallStrategies(request).Next([this, request](FCFCoreError err) {
    RunCleanupStrategies(request).Next([this, err, request](FCFCoreError) {
      FCFCoreError fixed_err = err;
      ConvertToInstallErrorCode(fixed_err);

      InstallResult result = CreateInstallResult(fixed_err, request);
      request->promise->SetValue(result);
      RemoveRequestAndTriggerNext(request);
    });
  });
}

// -----------------------------------------------------------------------------
IModsInstallationService::InstallResult
ModsInstallationServiceImpl::CreateInstallResult(
  const FCFCoreError& err,
  FInstallModRequestPtr request) {

  InstallResult result{ err, request->installedMod, request->analytics_params };
  return result;
}

// -----------------------------------------------------------------------------
// NOTE(twolf): Some error results, in the context of a mod installation, should
// be changed so that they are more convenient to handle (e.g. not to handle
// DownloadCancelled, UnzipCancelled etc...)
void ModsInstallationServiceImpl::ConvertToInstallErrorCode(FCFCoreError& err) {
  switch (err.code) {
  case ECFCoreErrorCodes::DownloadCancelled:
    err.code = ECFCoreErrorCodes::InstallCancelled;
  }
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsInstallationServiceImpl::RunInstallStrategies(
  FInstallModRequestPtr request) {

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();

  for (auto strategy : *installation_strategies_) {
    functions->Add([request, strategy]() {
      request->strategy = strategy;
      return strategy->Install(request.ToSharedRef());
    });
  }

  return FutureUtils::SequenceAny(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsInstallationServiceImpl::RunUninstallStrategies(
  FInstallModRequestPtr request) {

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();

  for (auto strategy : *installation_strategies_) {
    functions->Add([request, strategy]() {
      request->strategy = strategy;
      return strategy->Uninstall(request.ToSharedRef());
    });
  }

  return FutureUtils::SequenceAny(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsInstallationServiceImpl::RunCleanupStrategies(
  FInstallModRequestPtr request) {

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();

  for (auto strategy : *installation_strategies_) {
    functions->Add([request, strategy]() {
      request->strategy = strategy;
      return strategy->Cleanup(request.ToSharedRef());
    });
  }

  return FutureUtils::SequenceAny(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
void ModsInstallationServiceImpl::AddTrackingEvents(
  FInstallModRequestPtr request) {

  const FInstalledMod& mod = request->installedMod;
  if (IsNewInstallation(mod)) {
    return;
  }

  TMap<FString, FString>& tracking = request->additional_params.tracking;
  if (mod.installedFile.id != mod.latestUpdatedFile.id) {
    tracking.Add(kTrackingType, kTrackingTypeUpdate);
    return;
  }

  if (mod.preInstallStatus == EInstalledModStatus::Invalid) {
    tracking.Add(kTrackingType, kTrackingTypeFix);
    return;
  }

  tracking.Add(kTrackingType, kTrackingTypeOther);
  return;
}

// -----------------------------------------------------------------------------
void ModsInstallationServiceImpl::AddAnalyticsParams(
  FInstallModRequestPtr request) {

  const FInstalledMod& installed_mod = request->installedMod;
  request->analytics_params.isUpdate = !IsNewInstallation(installed_mod);
}

// -----------------------------------------------------------------------------
bool ModsInstallationServiceImpl::IsNewInstallation(const FInstalledMod& mod) {
  if (mod.installedFile.id == 0) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
void ModsInstallationServiceImpl::TriggerProgressEvent(
  FModsInstallationProgressDelegate delegate,
  const FInstalledMod& installed_mod,
  ELibraryProgressState state) {

  FLibraryProgress lib_progress;
  lib_progress.modId = installed_mod.details.id;
  lib_progress.fileId = installed_mod.latestUpdatedFile.id;
  lib_progress.state = state;

  delegate.ExecuteIfBound(lib_progress);
}

// -----------------------------------------------------------------------------
bool ModsInstallationServiceImpl::CancelInstallByModId(int64 mod_id) {
  FInstallModRequestPtr request;
  if (!installations_queue_service_->PeekByModId(mod_id, request)) {
    return false;
  }

  return CancelInstallByRequest(request);
}

// -----------------------------------------------------------------------------
bool ModsInstallationServiceImpl::CancelInstallByRequest(
  const FInstallModRequestPtr& request) {

  request->cancelRequest = true;

  if (!request->inProgerss) {
    return true;
  }

  if (request->strategy.IsValid()) {
    request->strategy->Cancel(request.ToSharedRef());
  }

  return true;
}

// -----------------------------------------------------------------------------