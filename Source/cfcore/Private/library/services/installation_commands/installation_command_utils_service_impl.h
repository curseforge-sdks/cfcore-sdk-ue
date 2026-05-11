/*MIT License

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
SOFTWARE.*/
#pragma once
#include <CoreMinimal.h>
#include "installation_command_utils_service.h"

namespace cfcore {
class IDirectoriesService;
class IFileSystemService;
class IFeaturePhasingService;

// -----------------------------------------------------------------------------
class InstallationCommandUtilsServiceImpl :
  public IInstallationCommandUtilsService {
public:
  InstallationCommandUtilsServiceImpl(
    TSharedRef<IDirectoriesService> directories_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<IFeaturePhasingService> feature_phasing_service);
  virtual ~InstallationCommandUtilsServiceImpl() = default;

public:
  virtual void TriggerProgressEvent(
    FInstallModRequestPtr request,
    ELibraryProgressState state,
    FLibraryProgressDataTransfer* data_transfer = nullptr) override;

  virtual FString GetTempOutputDir(const FFile& file) override;

  virtual FString GetUnderTempDir(const FString& path) override;

  virtual FString GetOutputDir(const FFile& file) override;

  virtual FString GetUnderModsDir(const FString& path) override;

  virtual FString GetRelativePathOnDisk(const FFile& file) override;

  virtual FString GetModFileDirPattern(const FFile& file) override;

  virtual bool IsNewInstallation(FInstallModRequestPtr request) override;
  virtual bool IsRequestForUpdate(FInstallModRequestPtr request) override;

  virtual TFuture<FCFCoreError> ScanLocalMods(
    TSharedRef<TArray<FScannedLocalModItem>> scanned_local_mod_items) override;
  virtual TFuture<FCFCoreError> ScanLocalModsLeveUpRelativeDir(
    TSharedRef<TArray<FScannedLocalModItem>> scanned_local_mod_items) override;

  virtual bool IsResumableDownloadSupported(
    const FInstalledMod& installed_mod) override;

private:
  struct RootModItem {
    FString path;
    bool is_directory = true;
  };

  struct ScanLocalModsOptions {
    // the root path to search in
    FString root_path;
    // fill in the relative path part (e.g. CF/flat mode)
    FString relative_path;
    FString ignore_path;
  };

  TFuture<FCFCoreError> ScanLocalModsInternal(
    struct ScanLocalModsOptions& scan_options,
    TSharedRef<TArray<FScannedLocalModItem>> out_scanned_local_mod_items);

  TFuture<FCFCoreError> RetrieveLocalRootModItems(
    const FString& root_path,
    TSharedRef<TArray<struct RootModItem>> out_root_mod_items);

  bool IsPathCFCoreDirectoryPattern(FString path);

private:
  TSharedRef<IDirectoriesService> directories_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IFeaturePhasingService> feature_phasing_service_;
};

} // namespace cfcore