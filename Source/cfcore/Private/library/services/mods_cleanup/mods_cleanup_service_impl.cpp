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
#include "mods_cleanup_service_impl.h"

#include <common/future_utils.h>
#include <common/macros.h>
#include <common/constants.h>

#include <services/logger/logger_macros.h>
#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>

#include <api/services/feature_phasing_service.h>

#include <library/services/installation_commands/installation_command_utils_service.h>
#include <library/services/installed_mods_info/get_installed_mods_service.h>

namespace cfcore {

};

using namespace cfcore;

// -----------------------------------------------------------------------------
ModsCleanupServiceImpl::ModsCleanupServiceImpl(
  TSharedRef<IFeaturePhasingService> feature_phasing_service,
  TSharedRef<IInstallationCommandUtilsService> command_utils_service,
  TSharedRef<IGetInstalledModsService> get_installed_mods_service,
  TSharedRef<IDirectoriesService> directories_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<IDelegateBroadcaster<IModsCleanupServiceDelegate>> delegates) :
  feature_phasing_service_(feature_phasing_service),
  command_utils_service_(command_utils_service),
  get_installed_mods_service_(get_installed_mods_service),
  directories_service_(directories_service),
  file_system_service_(file_system_service),
  delegates_(delegates) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsCleanupServiceImpl::Run(
  const FModsCleanupOptions& options) {

  if (!feature_phasing_service_->IsFeatureEnabledForUser(
    &FMePhasingData::deleteStaleModDirs)) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto scanned_local_mod_items = MakeShared<TArray<FScannedLocalModItem>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, scanned_local_mod_items]() {
        return command_utils_service_->ScanLocalMods(scanned_local_mod_items);
      },
      [this, scanned_local_mod_items]() {
        return FilterStaleMods(scanned_local_mod_items);
      },
      [this, scanned_local_mod_items]() {
        return DeleteStaleMods(scanned_local_mod_items);
      },
      [this, options]() {
        return DeleteTempDir(options);
      }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsCleanupServiceImpl::FilterStaleMods(
  TSharedRef<TArray<FScannedLocalModItem>> scanned_local_mod_items) {

  TSet<FString> managed_mod_dirs = GetInstalledModsRootDirectories();

  // Remove non-stale mods
  scanned_local_mod_items->RemoveAll(
    [this, &managed_mod_dirs](const FScannedLocalModItem& item) {
      // Only remove CFCore types, other types are hard to determine if they are
      // stale or not.
      if (item.directory_mode != EModsDirectoryMode::CFCore) {
        return true;
      }

      return managed_mod_dirs.Contains(item.path);
    });

  return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
}

// -----------------------------------------------------------------------------
TSet<FString> ModsCleanupServiceImpl::GetInstalledModsRootDirectories() {
  FString relative_mods_dir = directories_service_->GetRelativeModsDirectory();
  int32 rel_mods_dir_len = relative_mods_dir.Len();

  TArray<FInstalledMod> installed_mods;
  get_installed_mods_service_->GetSystemInstalledMods(installed_mods);

  TSet<FString> managed_mod_dirs;
  for (const FInstalledMod& installed_mod : installed_mods) {
    // Skip unmanaged mods, since we might be detecting stale mods as unmanaged
    if (installed_mod.unmanaged) {
      continue;
    }

    FString mod_root_dir = installed_mod.pathOnDisk;
    if (rel_mods_dir_len > 0) {
      mod_root_dir = installed_mod.pathOnDisk.Mid(rel_mods_dir_len + 1);
    }

    managed_mod_dirs.Add(mod_root_dir);
  }

  return MoveTemp(managed_mod_dirs);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsCleanupServiceImpl::DeleteStaleMods(
  TSharedRef<TArray<FScannedLocalModItem>> stale_local_mods) {

  if (stale_local_mods->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  for (const FScannedLocalModItem& item : *stale_local_mods) {
    FString full_path = command_utils_service_->GetUnderModsDir(item.path);
    CFCORE_LOG(Log, TEXT("Deleting mods dir %s ..."), *full_path);

    auto shared_full_path = MakeShared<FString>(full_path);
    futures->Add(file_system_service_->DeleteDirectory(shared_full_path));
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([promise](TArray<bool> results) {
    for (bool& success : results) {
      if (!success) {
        CFCORE_LOG(Log, TEXT("Failed deleting some dir"));
      }
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
// Perform cleanups so that failed installations won't take up storage space
TFuture<FCFCoreError> ModsCleanupServiceImpl::DeleteTempDir(
  const FModsCleanupOptions& options) {

  auto temp_files_and_dirs = MakeShared<TSet<FString>>();
  auto temp_items_to_delete = MakeShared<TSet<FString>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
    [this, temp_files_and_dirs]() {
      return GetTempFilesAndDirs(temp_files_and_dirs);
    },
    [this, options, temp_files_and_dirs, temp_items_to_delete]() {
      return GetTempItemsToDelete(options,
                                  temp_files_and_dirs,
                                  temp_items_to_delete);
    },
    [this, temp_items_to_delete]() {
      return DeleteTempItems(temp_items_to_delete);
    }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}


// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsCleanupServiceImpl::GetTempFilesAndDirs(
  TSharedRef<TSet<FString>> out_relative_files_and_dirs) {

  out_relative_files_and_dirs->Empty();

  FString temp_dir = command_utils_service_->GetUnderTempDir("");
  auto tmp = MakeShared<FString>(temp_dir);
  auto prefix = MakeShared<FString>("");
  auto files = MakeShared<TArray<FString>>();

  SHAREABLE_PROMISE(FCFCoreError, promise);

  file_system_service_->GetFilesInDirectoryRecursive(tmp, prefix, files)
    .Next([this, out_relative_files_and_dirs, temp_dir, files, promise](bool) {

    int32 tmp_dir_length = temp_dir.Len();

    for (const FString& file : *files) {
      FString file_or_dir = file.Mid(tmp_dir_length);
      int32 first_slash = file_or_dir.Find(TEXT("/"));

      if (first_slash == INDEX_NONE) {
        out_relative_files_and_dirs->Add(file_or_dir);
      }
      else {
        FString dir = file_or_dir.Left(first_slash);
        out_relative_files_and_dirs->Add(dir);
      }
    }

    promise->SetValue(FCFCoreError());
      });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsCleanupServiceImpl::GetTempItemsToDelete(
  const FModsCleanupOptions& options,
  TSharedRef<TSet<FString>> relative_files_and_dirs,
  TSharedRef<TSet<FString>> out_temp_relative_items_to_delete) {

  out_temp_relative_items_to_delete->Empty();

  TArray<FInstalledMod> installed_mods;
  get_installed_mods_service_->GetSystemInstalledMods(installed_mods);

  FString temp_dir = command_utils_service_->GetUnderTempDir("");

  TSet<FString> temp_items_to_keep;
  for (const FInstalledMod& installed_mod : installed_mods) {
    delegates_->ForEach([&options, &installed_mod, &temp_items_to_keep](
      const TSharedRef<IModsCleanupServiceDelegate> delegate) {

      TSet<FString> keep = delegate->OnGetKeepTempFilesAndDirs(
        installed_mod,
        options.force_cleanup);

      temp_items_to_keep.Append(keep);
    });
  }

  for (const FString& item : *relative_files_and_dirs) {
    if (!temp_items_to_keep.Contains(item.ToLower())) {
      out_temp_relative_items_to_delete->Add(item);
    }
  }

  return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsCleanupServiceImpl::DeleteTempItems(
  TSharedRef<TSet<FString>> temp_relative_items_to_delete) {

  if (temp_relative_items_to_delete->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<bool>>>();

  for (const FString& item : *temp_relative_items_to_delete) {
    FString full_path = command_utils_service_->GetUnderTempDir(item);
    auto shared_full_path = MakeShared<FString>(full_path);

    if (item.Contains(TEXT("."))) {
      futures->Add(file_system_service_->DeleteFile(shared_full_path));
    } else {
      futures->Add(file_system_service_->DeleteDirectory(shared_full_path));
    }
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([promise](TArray<bool> results) {
    for (bool& success : results) {
      if (!success) {
        CFCORE_LOG(Log, TEXT("Failed deleting some temp file/dir"));
      }
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}
