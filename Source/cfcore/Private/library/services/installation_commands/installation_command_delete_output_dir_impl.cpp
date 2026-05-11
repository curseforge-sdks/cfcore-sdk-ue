/* MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include "installation_command_delete_output_dir_impl.h"

#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/settings_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/directories_service.h>

#include "installation_command_utils_service.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandDeleteOutputDirImpl::
InstallationCommandDeleteOutputDirImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<IDirectoriesService> directories_service) :
  utils_service_(utils_service),
  settings_service_(settings_service),
  file_system_service_(file_system_service),
  directories_service_(directories_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDeleteOutputDirImpl::Run(
  FInstallModRequestPtr request) {

  if (utils_service_->IsNewInstallation(request)) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  auto settings = settings_service_->GetSettings();
  auto dir_mode = settings.modsDirectoryMode;

  if (dir_mode == EModsDirectoryMode::CFCore) {
    return DeleteExistingOutputDir(request);
  }

#define __DELEGATE_FUNC__(func) \
    [this, request]() { return func(request); }

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  functions->Add(__DELEGATE_FUNC__(DeleteExistingOutputDirModules));
  functions->Add(__DELEGATE_FUNC__(DeleteRequiredOutputDirModules));

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
bool InstallationCommandDeleteOutputDirImpl::Cancel(FInstallModRequestPtr) {
  return true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDeleteOutputDirImpl::DeleteExistingOutputDir(
  FInstallModRequestPtr request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  const FInstalledMod& mod = request->installedMod;

  auto local_mod_dir =
    MakeShared<FString>(utils_service_->GetOutputDir(mod.installedFile));

  auto future = file_system_service_->DeleteDirectory(local_mod_dir);
  future.Next([promise](bool success) {
    if (!success) {
      auto code = ECFCoreErrorCodes::FailedDeletingOutputDirectory;
      promise->SetValue(FCFCoreError(code));
    } else {
      promise->SetValue(FCFCoreError());
    }
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
// When the dir mode is Flat, we need to delete the old version of the mod's
// folders one by one based on the info we have from the server (Modules)
TFuture<FCFCoreError>
InstallationCommandDeleteOutputDirImpl::DeleteExistingOutputDirModules(
  FInstallModRequestPtr request) {

  const FInstalledMod& mod = request->installedMod;
  const TArray<FFileModule>& modules = mod.installedFile.modules;

  if (modules.Num() > 0) {
    // TODO(twolf): Actually show the progress of each folder (once we move to
    // command classes)
    FLibraryProgressDataTransfer transfer = { 0 };
    transfer.filename = modules[0].name;
    utils_service_->TriggerProgressEvent(request,
                                         ELibraryProgressState::CleaningUp,
                                         &transfer);
  }

  return CleanupExistingOutputDirModules(modules);
}

// -----------------------------------------------------------------------------
// We call this to make sure that the newly unzipped folders/files will not fail
// to move due to an existing folder/file on disk (e.g. there might be a mod
// on the local disk that was not installed via CFCore)
TFuture<FCFCoreError>
InstallationCommandDeleteOutputDirImpl::DeleteRequiredOutputDirModules(
  FInstallModRequestPtr request) {

  const FInstalledMod& mod = request->installedMod;
  const TArray<FFileModule>& modules = mod.latestUpdatedFile.modules;

  if (modules.Num() > 0) {
    FLibraryProgressDataTransfer transfer = { 0 };
    transfer.filename = modules[0].name;
    utils_service_->TriggerProgressEvent(request,
                                         ELibraryProgressState::CleaningUp,
                                         &transfer);
  }

  return CleanupExistingOutputDirModules(modules);
}


// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDeleteOutputDirImpl::CleanupExistingOutputDirModules(
  const TArray<FFileModule>& modules) {

  if (modules.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  for (FFileModule module : modules) {
    auto dir =
      MakeShared<FString>(utils_service_->GetUnderModsDir(module.name));

    futures->Add(CleanupDirectoryOrFile(MoveTemp(dir)));
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<FCFCoreError>(futures);
  future.Next([promise](TArray<FCFCoreError> errors) {
    for (FCFCoreError& err : errors) {
      if (err.isError) {
        promise->SetValue(err);
        return;
      }
    }

    promise->SetValue(FCFCoreError());
  });

  CFCORE_LOG(Log, TEXT("Cleanup possible %d mod dirs ..."), modules.Num());

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDeleteOutputDirImpl::CleanupDirectoryOrFile(
  TSharedRef<FString> path) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  CFCORE_LOG(Log, TEXT("Cleanup possible module %s ..."), *(path.Get()));

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  futures->Add(file_system_service_->DeleteDirectory(path));
  futures->Add(file_system_service_->DeleteFile(path));

  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([promise, path](TArray<bool> deleted_results) {
    for (bool deleted : deleted_results) {
      if (deleted) {
        promise->SetValue(FCFCoreError());
        return;
      }
    }

    CFCORE_LOG(Error, TEXT("Failed cleaning module %s"), *(path.Get()));
    auto code = ECFCoreErrorCodes::FailedDeletingOutputDirectory;
    FCFCoreError err(code);
    err.description = *(path.Get());
    promise->SetValue(err);
    });

  return promise->GetFuture();
}
