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
#include "unmanaged_mods_service_impl.h"
#include <common/constants.h>
#include <common/macros.h>
#include <common/future_utils.h>
#include <common/path_utils.h>

#include <services/logger/logger_macros.h>

#include <services/settings_service.h>
#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>

#include <library/services/installed_mods_info/installed_mods_info_service.h>
#include <library/services/installation_commands/installation_command_utils_service.h>

#include <Json.h>
#include <JsonUtilities.h>

namespace cfcore {

using InstallModsInfoResult = IInstalledModsInfoService::InstallModsInfoResult;

}

namespace cfcore {
};

using namespace cfcore;

// -----------------------------------------------------------------------------
UnmanagedModsServiceImpl::UnmanagedModsServiceImpl(
  TSharedRef<IInstalledModsInfoService> installed_mods_info_system_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_user_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IDirectoriesService> directories_service,
  TSharedRef<IInstallationCommandUtilsService> command_utils_service) :
  installed_mods_info_system_service_(installed_mods_info_system_service),
  installed_mods_info_user_service_(installed_mods_info_user_service),
  settings_service_(settings_service),
  directories_service_(directories_service),
  command_utils_service_(command_utils_service) {
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> UnmanagedModsServiceImpl::Initialize() {
  if (!IsUnamangedModsEnabled()) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto local_mod_items = MakeShared<TArray<FScannedLocalModItem>>();
  auto invalid_unmanaged_mods = MakeShared<TMap<FString, FInstalledMod>>();
  auto removed_unmanaged_mods = MakeShared<TArray<FInstalledMod>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, local_mod_items]() {
        return RetrieveLocalRootModDirectories(local_mod_items);
      },
      [this, local_mod_items, invalid_unmanaged_mods, removed_unmanaged_mods]() {
        return FilterExistingMods(local_mod_items,
                                  invalid_unmanaged_mods,
                                  removed_unmanaged_mods);
      },
      [this, removed_unmanaged_mods]() {
        return UninstallRemovedUnmanagedMods(removed_unmanaged_mods);
      },
      [this, local_mod_items, invalid_unmanaged_mods]() {
        return AddNewUnamanagedMods(local_mod_items, invalid_unmanaged_mods);
      }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<bool> UnmanagedModsServiceImpl::OnPerformQuickValidation(
  const FInstalledMod& installed_mod,
  bool& out_skip_other_validations) {

  out_skip_other_validations = false;

  if (installed_mod.unmanaged) {
    out_skip_other_validations = true;
  }

  return MakeFulfilledPromise<bool>(true).GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
UnmanagedModsServiceImpl::RetrieveLocalRootModDirectories(
  TSharedRef<TArray<FScannedLocalModItem>> out_local_mod_items) {

  out_local_mod_items->Empty();

  auto local_mod_relative_items = MakeShared<TArray<FScannedLocalModItem>>();
  auto local_mod_items = MakeShared<TArray<FScannedLocalModItem>>();

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
    [this, local_mod_relative_items]() {
      return command_utils_service_->ScanLocalMods(local_mod_relative_items);
    },
    [this, local_mod_items]() {
      if (!settings_service_->GetSettings().unmanagedMods.scanOneLevelUp) {
        return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
      }

      return command_utils_service_->ScanLocalModsLeveUpRelativeDir(
        local_mod_items);
    },
    [this, local_mod_relative_items, local_mod_items, out_local_mod_items]() {
      out_local_mod_items->Append(*local_mod_relative_items);
      out_local_mod_items->Append(*local_mod_items);

      out_local_mod_items->RemoveAll(
        [this](const FScannedLocalModItem& item) {
          return ShouldExcludeFromScan(item);
        });

      return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
    },
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
// This function will remove existing mods (managed and unmanaged) from
// local_mod_items and will add unmanaged mods that are not found in
// local_mod_items out_missing_unmanaged_mods.
TFuture<FCFCoreError>
UnmanagedModsServiceImpl::FilterExistingMods(
  const TSharedRef<TArray<FScannedLocalModItem>> local_mod_items,
  TSharedRef<TMap<FString, FInstalledMod>> out_invalid_unmanaged_mods,
  TSharedRef<TArray<FInstalledMod>> out_removed_unmanaged_mods) {

  out_removed_unmanaged_mods->Empty();
  out_invalid_unmanaged_mods->Empty();

  TArray<FInstalledMod> installed_mods;
  installed_mods_info_system_service_->GetInstalledMods(installed_mods);

  for (const FInstalledMod& installed_mod : installed_mods) {
    if (IsUnamangedModInvalid(installed_mod)) {
      out_invalid_unmanaged_mods->Add(installed_mod.pathOnDisk, installed_mod);
      continue;
    }

    int32 index = local_mod_items->IndexOfByPredicate(
      [installed_mod](const FScannedLocalModItem& item) {
        FString path_on_disk = FPaths::Combine(
          item.relative_path_part,
          item.path);

        path_on_disk = PathUtils::NormalizePath(path_on_disk);

        return path_on_disk.Equals(installed_mod.pathOnDisk,
                                   ESearchCase::IgnoreCase);
      });

    if (index != INDEX_NONE) {
      // NOTE: Keep managed mods in the list so that we add them to a potential
      // new user. If it already exists for the current user, it won't be
      // duplicated
      if (!installed_mod.unmanaged) {
        local_mod_items->RemoveAt(index);
      }
    } else if (installed_mod.unmanaged) {
      out_removed_unmanaged_mods->Add(installed_mod);
    }
  }

  return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
}

// -----------------------------------------------------------------------------
bool UnmanagedModsServiceImpl::IsUnamangedModInvalid(
  const FInstalledMod& installed_mod) {

  return (installed_mod.unmanaged &&
         installed_mod.status != EInstalledModStatus::Normal);
}


// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
UnmanagedModsServiceImpl::UninstallRemovedUnmanagedMods(
  const TSharedRef<TArray<FInstalledMod>> missing_unmanaged_mods) {

  if (missing_unmanaged_mods->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<InstallModsInfoResult>>>();

  TArray<FInternalInstalledModInfoProperties> mod_updates;
  for (const FInstalledMod& mod : *missing_unmanaged_mods) {
    futures->Add(installed_mods_info_system_service_->Uninstall(mod.id));
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<InstallModsInfoResult>(futures);
  future.Next([this, promise](TArray<InstallModsInfoResult> results) {
    for (const InstallModsInfoResult& res : results) {
      FString id = res.Value.id.ToString();
      if (res.Key.isError) {
        CFCORE_LOG(Error, TEXT("Couldn't uninstall mod (%s)"), *id);
      }
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
UnmanagedModsServiceImpl::AddNewUnamanagedMods(
  const TSharedRef<TArray<FScannedLocalModItem>> local_mod_items,
  TSharedRef<TMap<FString, FInstalledMod>> invalid_unmanaged_mods) {

  TArray<FInstalledMod> installed_mods;
  for (const FScannedLocalModItem& local_mod_item : *local_mod_items) {
    FInstalledMod installed_mod = CreateUnmanagedInstalledMod(local_mod_item);

    OverrideExistingMod(invalid_unmanaged_mods, local_mod_item, installed_mod);

    installed_mods.Add(installed_mod);
  }

  return installed_mods_info_user_service_->SetUnmanagedInstalledMods(
    installed_mods);
}

// -----------------------------------------------------------------------------
void UnmanagedModsServiceImpl::OverrideExistingMod(
  TSharedRef<TMap<FString, FInstalledMod>>& invalid_unmanaged_mods,
  const FScannedLocalModItem& local_mod_item,
  FInstalledMod& installed_mod) {

  FString path_on_disk = FPaths::Combine(
    local_mod_item.relative_path_part,
    local_mod_item.path);

  path_on_disk = PathUtils::NormalizePath(path_on_disk);

  if (!invalid_unmanaged_mods->Contains(path_on_disk)) {
    return;
  }

  installed_mod = (*invalid_unmanaged_mods)[path_on_disk];
  installed_mod.status = EInstalledModStatus::Normal;
}

// -----------------------------------------------------------------------------
FInstalledMod
UnmanagedModsServiceImpl::CreateUnmanagedInstalledMod(
  const FScannedLocalModItem& local_mod_item) {

  FString path_on_disk = FPaths::Combine(local_mod_item.relative_path_part,
                                       local_mod_item.path);

  TArray<FInstalledMod> installed_mods;
  installed_mods_info_system_service_->GetInstalledMods(installed_mods);

  // This will assure we don't create duplicates of the same unmanaged mod.
  FInstalledMod* existing = installed_mods.FindByPredicate(
    [&path_on_disk](const FInstalledMod& mod) {

    return mod.pathOnDisk == path_on_disk;
  });

  if (existing != nullptr) {
    existing->unmanaged = true;
    return *existing;
  }

  FInstalledMod new_installed_mod = FInstalledMod::CreateNew();
  new_installed_mod.dateInstalled = FDateTime::UtcNow();
  new_installed_mod.dateUpdated = FDateTime::UtcNow();
  new_installed_mod.pathOnDisk = path_on_disk;
  new_installed_mod.unmanaged = true;
  new_installed_mod.enabled = true;
  new_installed_mod.details.gameId = settings_service_->GetSettings().gameId;
  new_installed_mod.details.name = local_mod_item.path;
  new_installed_mod.details.isAvailable = true;

  return new_installed_mod;
}

// -----------------------------------------------------------------------------
bool UnmanagedModsServiceImpl::IsUnamangedModsEnabled() {
  return settings_service_->GetSettings().unmanagedMods.enabled;
}

// -----------------------------------------------------------------------------
bool UnmanagedModsServiceImpl::IsHidden(
  const cfcore::FScannedLocalModItem& item) const {

  return item.path.StartsWith(".");
}

// -----------------------------------------------------------------------------
bool UnmanagedModsServiceImpl::ShouldExcludeFromScan(
  const cfcore::FScannedLocalModItem& item) const {

  return !item.is_directory || IsHidden(item);
}
