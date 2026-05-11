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
#include "installation_command_utils_service_impl.h"

#include <common/macros.h>
#include <common/future_utils.h>
#include <common/constants.h>

#include <services/logger/logger_macros.h>
#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>
#include <api/services/feature_phasing_service.h>

#include <Misc/Paths.h>
#include <Misc/DefaultValueHelper.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandUtilsServiceImpl::InstallationCommandUtilsServiceImpl(
  TSharedRef<IDirectoriesService> directories_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<IFeaturePhasingService> feature_phasing_service) :
  directories_service_(directories_service),
  file_system_service_(file_system_service),
  feature_phasing_service_(feature_phasing_service) {
}

// -----------------------------------------------------------------------------
void InstallationCommandUtilsServiceImpl::TriggerProgressEvent(
  FInstallModRequestPtr request,
  ELibraryProgressState state,
  FLibraryProgressDataTransfer* data_transfer/*= nullptr*/) {

  FLibraryProgress lib_progress;
  lib_progress.modId = request->installedMod.details.id;
  lib_progress.fileId = request->installedMod.latestUpdatedFile.id;
  lib_progress.state = state;

  if (data_transfer) {
    lib_progress.dataTransfer = *data_transfer;
  }

  request->on_progress.ExecuteIfBound(lib_progress);
}

// -----------------------------------------------------------------------------
FString InstallationCommandUtilsServiceImpl::GetTempOutputDir(
  const FFile& file) {

  FString mod_file_path = GetModFileDirPattern(file);
  return GetUnderTempDir(mod_file_path);
}

// -----------------------------------------------------------------------------
FString InstallationCommandUtilsServiceImpl::GetUnderTempDir(
  const FString& path) {

  FString local_mods_temp_dir = FPaths::Combine(
    directories_service_->GetModsDirectory(),
    kTempDir);

  return FPaths::Combine(local_mods_temp_dir, path);
}

// -----------------------------------------------------------------------------
FString InstallationCommandUtilsServiceImpl::GetOutputDir(
  const FFile& file) {

  FString mod_file_path = GetModFileDirPattern(file);
  return GetUnderModsDir(mod_file_path);
}

// -----------------------------------------------------------------------------
FString InstallationCommandUtilsServiceImpl::GetUnderModsDir(
  const FString& path) {

  FString local_mods_inst_dir = directories_service_->GetModsDirectory();
  FString dir = FPaths::Combine(local_mods_inst_dir, path);
  return dir;
}


// -----------------------------------------------------------------------------
FString InstallationCommandUtilsServiceImpl::GetRelativePathOnDisk(
  const FFile& file) {

  FString mod_file_path = GetModFileDirPattern(file);
  FString dir = FPaths::Combine(
    directories_service_->GetRelativeModsDirectory(),
    mod_file_path);

  return dir;
}

// -----------------------------------------------------------------------------
// [modId]_[fileId]
FString InstallationCommandUtilsServiceImpl::GetModFileDirPattern(
  const FFile& file) {

  return FString::Format(TEXT("{0}_{1}"), { file.modId, file.id });
}

// -----------------------------------------------------------------------------
bool InstallationCommandUtilsServiceImpl::IsNewInstallation(
  FInstallModRequestPtr request) {

  if (request->installedMod.installedFile.id == 0) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
bool InstallationCommandUtilsServiceImpl::IsRequestForUpdate(
  FInstallModRequestPtr request) {

  int64 installed_file_id = request->installedMod.installedFile.id;
  int64 latest_file_id = request->installedMod.latestUpdatedFile.id;

  // is a new installation
  if (installed_file_id == 0) {
    return false;
  }

  // is a reinstallation/fix
  if (installed_file_id == latest_file_id) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandUtilsServiceImpl::ScanLocalMods(
  TSharedRef<TArray<FScannedLocalModItem>> out_scanned_local_mod_items) {

  out_scanned_local_mod_items->Empty();

  ScanLocalModsOptions options;
  options.root_path = directories_service_->GetModsDirectory();
  options.relative_path = directories_service_->GetRelativeModsDirectory();

  return ScanLocalModsInternal(options, out_scanned_local_mod_items);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandUtilsServiceImpl::ScanLocalModsLeveUpRelativeDir(
  TSharedRef<TArray<FScannedLocalModItem>> out_scanned_local_mod_items) {

  out_scanned_local_mod_items->Empty();

  ScanLocalModsOptions options;
  options.root_path = directories_service_->GetModsRootDirectory();
  options.relative_path = "";
  options.ignore_path = directories_service_->GetRelativeModsDirectory();

  return ScanLocalModsInternal(options, out_scanned_local_mod_items);
}

// -----------------------------------------------------------------------------
bool InstallationCommandUtilsServiceImpl::IsResumableDownloadSupported(
  const FInstalledMod& installed_mod) {

  bool feature_enabled = feature_phasing_service_->IsFeatureEnabledForUser(
    &FMePhasingData::resumableDownloads);
  if (!feature_enabled){
    return false;
  }

  if (installed_mod.downloadInfo.fileId == 0) {
    return true;
  }

  if (installed_mod.latestUpdatedFile.id != installed_mod.downloadInfo.fileId) {
    return false;
  }

  bool can_resume = installed_mod.downloadInfo.canResume;
  if (!can_resume) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandUtilsServiceImpl::ScanLocalModsInternal(
  struct ScanLocalModsOptions& scan_options,
  TSharedRef<TArray<FScannedLocalModItem>> out_scanned_local_mod_items) {

  out_scanned_local_mod_items->Empty();
  auto root_mod_items = MakeShared<TArray<struct RootModItem>>();

  SHAREABLE_PROMISE(FCFCoreError, promise);

  RetrieveLocalRootModItems(scan_options.root_path, root_mod_items)
    .Next([this,
           promise,
           scan_options,
           root_mod_items,
           out_scanned_local_mod_items](FCFCoreError) {

    for (const struct RootModItem& item : *root_mod_items) {
      // This can happen when in CFCore dir mode yet wanting to scan one level
      // up
      if (item.path.Equals(scan_options.ignore_path, ESearchCase::IgnoreCase)) {
        continue;
      }

      FScannedLocalModItem scanned_item;
      scanned_item.path = item.path;
      scanned_item.relative_path_part = scan_options.relative_path;
      scanned_item.is_directory = item.is_directory;
      scanned_item.directory_mode = EModsDirectoryMode::Flat;

      if (!item.is_directory) {
        out_scanned_local_mod_items->Add(scanned_item);
        continue;
      }

      if (IsPathCFCoreDirectoryPattern(item.path)) {
        scanned_item.directory_mode = EModsDirectoryMode::CFCore;
      }

      out_scanned_local_mod_items->Add(scanned_item);
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandUtilsServiceImpl::RetrieveLocalRootModItems(
  const FString& root_path,
  TSharedRef<TArray<struct RootModItem>> out_root_mod_items) {

  out_root_mod_items->Empty();

  auto path = MakeShared<FString>(root_path);
  auto prefix = MakeShared<FString>("");
  auto files = MakeShared<TArray<FString>>();

  SHAREABLE_PROMISE(FCFCoreError, promise);

  // NOTE: This will NOT return directories that do not have files in them.
  file_system_service_->GetFilesInDirectoryRecursive(path, prefix, files)
    .Next([this, root_path, files, out_root_mod_items, promise](bool) {

    TSet<FString> unique_mod_dirs;

    for (const FString& file : *files) {
      FString relative_dir_file = file.Mid(root_path.Len() + 1);
      int32 first_slash = relative_dir_file.Find(TEXT("/"));

      struct RootModItem mod_root_item;

      // This could mean a root-level file
      if (first_slash == -1) {
        mod_root_item.is_directory = false;
        mod_root_item.path = relative_dir_file;
      } else {
        mod_root_item.is_directory = true;
        FString mod_root_dir = relative_dir_file.Left(first_slash);
        mod_root_item.path = mod_root_dir;
      }

      // Skip duplicates
      FString key = mod_root_item.path.ToLower();
      if (unique_mod_dirs.Contains(key)) {
        continue;
      } else {
        unique_mod_dirs.Add(key);
      }

      out_root_mod_items->Add(mod_root_item);
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
// Return true if the path is of type: {modId}_{fileId} where modId and fileId
// are integers.
bool InstallationCommandUtilsServiceImpl::IsPathCFCoreDirectoryPattern(
  FString path) {

  TArray<FString> parts;
  path.ParseIntoArray(parts, TEXT("_"), true);

  if (parts.Num() != 2) {
    return false;
  }

  for (const FString& part : parts) {
    int64 value;
    if (!FDefaultValueHelper::ParseInt64(part, value)) {
      return false;
    }
  }

  return true;
}
