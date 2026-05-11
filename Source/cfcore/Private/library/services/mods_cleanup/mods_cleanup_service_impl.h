/*MIT License

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
SOFTWARE.*/
#pragma once
#include <CoreMinimal.h>

#include "mods_cleanup_service.h"
#include <library/services/installation_commands/models/scanned_local_mod_item.h>
#include <library/models/installed_mod.h>
#include <common/delegate_broadcaster.h>
#include "mods_cleanup_service_delegate.h"

namespace cfcore {
class IFeaturePhasingService;
class IInstallationCommandUtilsService;
class IDirectoriesService;
class IFileSystemService;
class IGetInstalledModsService;

// -----------------------------------------------------------------------------
class ModsCleanupServiceImpl : public IModsCleanupService {
public:
  ModsCleanupServiceImpl(
    TSharedRef<IFeaturePhasingService> feature_phasing_service,
    TSharedRef<IInstallationCommandUtilsService> command_utils_service,
    TSharedRef<IGetInstalledModsService> get_installed_mods_service,
    TSharedRef<IDirectoriesService> directories_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<IDelegateBroadcaster<IModsCleanupServiceDelegate>> delegates);

// IModsCleanupService
public:
  virtual TFuture<FCFCoreError> Run(
    const FModsCleanupOptions& options) override;

private:
  TFuture<FCFCoreError> FilterStaleMods(
    TSharedRef<TArray<FScannedLocalModItem>> scanned_local_mod_items);
  TSet<FString> GetInstalledModsRootDirectories();
  TFuture<FCFCoreError> DeleteStaleMods(
    TSharedRef<TArray<FScannedLocalModItem>> stale_local_mods);
  TFuture<FCFCoreError> DeleteTempDir(const FModsCleanupOptions& options);

  TFuture<FCFCoreError> GetTempFilesAndDirs(
    TSharedRef<TSet<FString>> out_relative_files_and_dirs);

  TFuture<FCFCoreError> GetTempItemsToDelete(
    const FModsCleanupOptions& options,
    TSharedRef<TSet<FString>> relative_files_and_dirs,
    TSharedRef<TSet<FString>> out_temp_relative_items_to_delete);

  TFuture<FCFCoreError> DeleteTempItems(
    TSharedRef<TSet<FString>> temp_relative_items_to_delete);

private:
  TSharedRef<IFeaturePhasingService> feature_phasing_service_;
  TSharedRef<IInstallationCommandUtilsService> command_utils_service_;
  TSharedRef<IGetInstalledModsService> get_installed_mods_service_;
  TSharedRef<IDirectoriesService> directories_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IDelegateBroadcaster<IModsCleanupServiceDelegate>> delegates_;
};

} // namespace cfcore