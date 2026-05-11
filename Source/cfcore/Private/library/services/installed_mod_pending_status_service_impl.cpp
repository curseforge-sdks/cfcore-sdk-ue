/* MIT License

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
SOFTWARE. */
#include "installed_mod_pending_status_service_impl.h"

#include <common/macros.h>
#include <services/logger/logger_macros.h>

#include <library/services/mods_installation_queue_service.h>
#include <library/services/installed_mods_info/installed_mods_info_service.h>
#include <library/services/installation_commands/installation_command_utils_service.h>

namespace cfcore {
const double kResumableTempFilesMaxAgeInDays = 4;
};

using namespace cfcore;

// -----------------------------------------------------------------------------
InstalledModPendingStatusServiceImpl::InstalledModPendingStatusServiceImpl(
  TSharedRef<IModsInstallationQueueService> installation_queue_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
  TSharedRef<IInstallationCommandUtilsService> command_utils_service) :
  installation_queue_service_(installation_queue_service),
  installed_mods_info_service_(installed_mods_info_service),
  command_utils_service_(command_utils_service) {
}

// -----------------------------------------------------------------------------
TFuture<bool> InstalledModPendingStatusServiceImpl::OnPerformQuickValidation(
  const FInstalledMod& installed_mod,
  bool& out_skip_other_validations) {

  out_skip_other_validations = false;

  if (IsModInInstallingQueue(installed_mod)) {
    out_skip_other_validations = true;
    return MakeFulfilledPromise<bool>(true).GetFuture();
  }

  if (installed_mod.status != EInstalledModStatus::Pending) {
    return MakeFulfilledPromise<bool>(true).GetFuture();
  }

  if (IsModInUpdatePending(installed_mod)) {
    return SetModOutOfDate(installed_mod);
  }

  if (!installed_mod.downloadInfo.canResume) {
    out_skip_other_validations = true;
    return UninstallMod(installed_mod);
  }

  // Mod is pending but not an update and isn't in the installation queue,
  // meaning it is probably in a status from the last run of the game (e.g. shut
  // down while installing).
  out_skip_other_validations = true;

  return MakeFulfilledPromise<bool>(true).GetFuture();
}

// -----------------------------------------------------------------------------
TSet<FString> InstalledModPendingStatusServiceImpl::OnGetKeepTempFilesAndDirs(
  const FInstalledMod& installed_mod,
  bool force_cleanup) {

  TSet<FString> keep_files_and_dirs;
  if (installed_mod.status != EInstalledModStatus::Pending) {
    return keep_files_and_dirs;
  }

  FillModInInstallingQueueFilesAndDirs(installed_mod, keep_files_and_dirs);

  if (!installed_mod.downloadInfo.canResume) {
    return keep_files_and_dirs;
  }

  if (installed_mod.downloadInfo.tempFile.IsEmpty()) {
    return keep_files_and_dirs;
  }

  FString temp_dir = command_utils_service_->GetUnderTempDir("");
  if (installed_mod.downloadInfo.tempFile.Len() <= temp_dir.Len()) {
    return keep_files_and_dirs;
  }

  FDateTime now = FDateTime::UtcNow();
  FTimespan delta = now - installed_mod.downloadInfo.lastModified;
  bool expired = delta.GetTotalDays() > kResumableTempFilesMaxAgeInDays;
  if (force_cleanup || expired) {
    return keep_files_and_dirs;
  }

  FString relative = installed_mod.downloadInfo.tempFile.Mid(temp_dir.Len());
  keep_files_and_dirs.Add(relative.ToLower());

  return keep_files_and_dirs;
}

// -----------------------------------------------------------------------------
bool InstalledModPendingStatusServiceImpl::OnHideInstalledMod(
  const FInstalledMod& installed_mod) {

  if (installed_mod.status != EInstalledModStatus::Pending) {
    return false;
  }

  if (IsModInInstallingQueue(installed_mod)) {
    return false;
  }

  if (installed_mod.downloadInfo.canResume) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
void InstalledModPendingStatusServiceImpl::OnSetInstallFailed(
  const FInstalledMod& installed_mod,
  bool& out_keep_installed_mod) {

  out_keep_installed_mod = false;

  if (!installed_mod.downloadInfo.canResume) {
    return;
  }

  out_keep_installed_mod = true;
}

// -----------------------------------------------------------------------------
void InstalledModPendingStatusServiceImpl::FillModInInstallingQueueFilesAndDirs(
  const FInstalledMod& installed_mod,
  TSet<FString>& keep_files_and_dirs) {

  if (!IsModInInstallingQueue(installed_mod)) {
    return;
  }

  // keep non-temp files and dirs, if we are installing them right now
  const FFile& file = installed_mod.latestUpdatedFile;
  keep_files_and_dirs.Add(file.fileName);

  FString temp_dir = command_utils_service_->GetUnderTempDir("");
  FString out_dir = command_utils_service_->GetTempOutputDir(file);
  out_dir = out_dir.Mid(temp_dir.Len());
  keep_files_and_dirs.Add(out_dir);
}

// -----------------------------------------------------------------------------
bool InstalledModPendingStatusServiceImpl::IsModInInstallingQueue(
  const FInstalledMod& installed_mod) {

  FInstallModRequestPtr req;
  int64 mod_id = installed_mod.details.id;
  bool is_in_queue = installation_queue_service_->PeekByModId(mod_id, req);

  return is_in_queue;
}

// -----------------------------------------------------------------------------
// Meaning the mod has an installed version and stopped while trying to update
bool InstalledModPendingStatusServiceImpl::IsModInUpdatePending(
  const FInstalledMod& installed_mod) {

  if (installed_mod.status != EInstalledModStatus::Pending) {
    return false;
  }

  return installed_mod.installedFile.id > 0;
}

// -----------------------------------------------------------------------------
TFuture<bool> InstalledModPendingStatusServiceImpl::SetModOutOfDate(
  const FInstalledMod& installed_mod) {

  int64 mod_id = installed_mod.details.id;
  CFCORE_LOG(Log, TEXT("Set mod status [Pending->OutOfDate (%lld)]"), mod_id);

  struct FInternalInstalledModInfoProperties props;
  props.modId = mod_id;
  props.status = EInstalledModStatus::OutOfDate;
  TArray<FInternalInstalledModInfoProperties> props_arr { props };

  SHAREABLE_PROMISE(bool, promise);

  installed_mods_info_service_->SetInstalledProperties(props_arr).Next(
    [promise](FCFCoreError err) {

    promise->SetValue(!err.isError);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> InstalledModPendingStatusServiceImpl::UninstallMod(
  const FInstalledMod& installed_mod) {

  SHAREABLE_PROMISE(bool, promise);

  int64 mod_id = installed_mod.details.id;
  installed_mods_info_service_->Uninstall(mod_id)
    .Next([promise] (IInstalledModsInfoService::InstallModsInfoResult res) {

    const FCFCoreError& err = res.Key;
    promise->SetValue(!err.isError);
  });

  return promise->GetFuture();
}
