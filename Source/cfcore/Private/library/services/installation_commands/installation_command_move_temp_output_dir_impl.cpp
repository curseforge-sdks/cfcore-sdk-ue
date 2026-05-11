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
#include "installation_command_move_temp_output_dir_impl.h"

#include <common/macros.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>
#include <services/settings_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/directories_service.h>

#include "installation_command_utils_service.h"

#include <Misc/Paths.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandMoveTempOutputDirImpl::
InstallationCommandMoveTempOutputDirImpl(
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
TFuture<FCFCoreError> InstallationCommandMoveTempOutputDirImpl::Run(
  FInstallModRequestPtr request) {

  auto dir_mode = settings_service_->GetSettings().modsDirectoryMode;

  if (dir_mode == EModsDirectoryMode::CFCore) {
    return MoveTempOutputDir(request);
  } else {
    return MoveTempOutputDirModules(request);
  }
}

// -----------------------------------------------------------------------------
bool InstallationCommandMoveTempOutputDirImpl::Cancel(FInstallModRequestPtr) {
  return true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandMoveTempOutputDirImpl::MoveTempOutputDir(
  FInstallModRequestPtr request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  const FInstalledMod& mod = request->installedMod;
  const FFile& updated_file = mod.latestUpdatedFile;

  FString src_path = utils_service_->GetTempOutputDir(updated_file);
  FString dest_path = utils_service_->GetOutputDir(updated_file);
  FString path_on_disk = utils_service_->GetRelativePathOnDisk(updated_file);

  auto future = MoveDirectoryOrFile(src_path, dest_path, request);
  future.Next([request, path_on_disk, promise](FCFCoreError err) {
    if (!err.isError) {
      request->installedMod.pathOnDisk = *path_on_disk;
      promise->SetValue(FCFCoreError());
      return;
    }

    promise->SetValue(err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandMoveTempOutputDirImpl::MoveTempOutputDirModules(
  FInstallModRequestPtr request) {

  const FInstalledMod& mod = request->installedMod;
  const FFile& updated_file = mod.latestUpdatedFile;

  if (updated_file.modules.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  FString root_temp_dir = utils_service_->GetTempOutputDir(updated_file);

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  for (FFileModule module : updated_file.modules) {
    FString src_path = FPaths::Combine(root_temp_dir, module.name);
    FString dest_path = utils_service_->GetUnderModsDir(module.name);

    futures->Add(MoveDirectoryOrFile(src_path, dest_path, request));
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<FCFCoreError>(futures);
  future.Next([this, request, promise](TArray<FCFCoreError> errors) {
    for (FCFCoreError& err : errors) {
      if (err.isError) {
        promise->SetValue(err);
        return;
      }
    }

    request->installedMod.pathOnDisk =
      *directories_service_->GetRelativeModsDirectory();
    promise->SetValue(FCFCoreError());
  });

  CFCORE_LOG(Log,
             TEXT("Moving %d mod dirs ..."),
             mod.latestUpdatedFile.modules.Num());

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandMoveTempOutputDirImpl::MoveDirectoryOrFile(
  const FString& src_path,
  const FString& dest_path,
  FInstallModRequestPtr request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  CFCORE_LOG(Log, TEXT("Moving %s to %s ..."), *src_path, *dest_path);

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  futures->Add(file_system_service_->MoveDirectory(src_path, dest_path));
  futures->Add(file_system_service_->MoveFile(src_path, dest_path));

  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([promise, src_path, dest_path](TArray<bool> deleted_results) {
    for (bool deleted : deleted_results) {
      if (deleted) {
        promise->SetValue(FCFCoreError());
        return;
      }
    }

    CFCORE_LOG(Error, TEXT("Failed to move path %s"), *src_path);
    auto code = ECFCoreErrorCodes::FailedToMoveModDirectory;
    FCFCoreError err(code);
    err.description = FString::Format(TEXT("{0} to {1}"), {
      src_path,
      dest_path
    });

    promise->SetValue(err);
  });

  auto state = ELibraryProgressState::Copying;
  FLibraryProgressDataTransfer transfer = { 0 };
  transfer.filename = src_path;
  utils_service_->TriggerProgressEvent(request,
                                       state,
                                       &transfer);

  return promise->GetFuture();
}
