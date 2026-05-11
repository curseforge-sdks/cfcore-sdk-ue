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
#include <Async/Async.h>

#include <common/cfcore_error.h>

#include <library/services/mods_installation_service.h>
#include <library/services/models/install_mod_request.h>
#include <library/models/library_progress_data_transfer.h>
#include <library/models/enums/library_progress_state.h>

#include <api/models/file.h>

#include <library/models/installed_mod.h>
#include "models/scanned_local_mod_item.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class IInstallationCommandUtilsService {
public:
  virtual ~IInstallationCommandUtilsService() = default;

public:
  virtual void TriggerProgressEvent(
    FInstallModRequestPtr request,
    ELibraryProgressState state,
    FLibraryProgressDataTransfer* data_transfer = nullptr) = 0;

  /**
   * Given a file, returns the absolute temporary directory on local disk
   */
  virtual FString GetTempOutputDir(const FFile& file) = 0;

  /**
   * Returns the full directory, of the given path, under the temporary dir.
   */
  virtual FString GetUnderTempDir(const FString& path) = 0;

  /**
   * Returns the installation directory for a mod file
   */
  virtual FString GetOutputDir(const FFile& file) = 0;

  /**
   * Returns the full directory, of the given path, under the mods installation
   * directory.
   */
  virtual FString GetUnderModsDir(const FString& path) = 0;

  virtual FString GetRelativePathOnDisk(const FFile& file) = 0;

  virtual FString GetModFileDirPattern(const FFile& file) = 0;

  virtual bool IsNewInstallation(FInstallModRequestPtr request) = 0;
  virtual bool IsRequestForUpdate(FInstallModRequestPtr request) = 0;

  /**
   * Scans the local mods directory and returns a list of scanned items.
   * This is can be used to find local mods that are not managed by the plugin.
   */
  virtual TFuture<FCFCoreError> ScanLocalMods(
    TSharedRef<TArray<FScannedLocalModItem>> scanned_local_mod_items) = 0;

  /**
   * Scans the local mods dir and returns a list of canned items. Unlike the
   * |ScanLocalMods|, this function will only scan one level up the relative
   * directory, which is only applicable for CFCore mod structure.
   */
  virtual TFuture<FCFCoreError> ScanLocalModsLeveUpRelativeDir(
    TSharedRef<TArray<FScannedLocalModItem>> scanned_local_mod_items) = 0;

  virtual bool IsResumableDownloadSupported(
    const FInstalledMod& installed_mod) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IInstallationCommandUtilsService)