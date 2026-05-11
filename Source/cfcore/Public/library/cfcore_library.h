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
#include "CoreMinimal.h"
#include <common/cfcore_error.h>
#include <api/models/mod.h>
#include <library/models/installed_mod.h>
#include <library/models/mods_dir_info.h>
#include <library/models/library_progress.h>
#include <library/models/install_mod_additional_params.h>
#include <library/cfcore_client_server_library.h>
#include <library/models/installed_mod_properties.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ICFCoreLibrary {
public:
  DECLARE_DELEGATE_OneParam(FSyncWithServerDelegate, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FGetInstalledModsDelegate, const TArray<FInstalledMod>&);
  DECLARE_DELEGATE_TwoParams(FGetModsDirInfoDelegate, const TOptional<FModsDirInfo>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FInstalledDelegate, const TOptional<FInstalledMod>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FInstallProgressDelegate, const FLibraryProgress&);
  DECLARE_DELEGATE_OneParam(FCancelInstallDelegate, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FUninstallDelegate, TOptional<FInstalledMod>, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FPerformModsValidationDelegate, const TOptional<TArray<FInstalledMod>>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FUpdateInstalledModsPropertiesDelegate, const TOptional<FCFCoreError>&);
  DECLARE_MULTICAST_DELEGATE_OneParam(FModInstallProgressDelegate, const FLibraryProgress&);
  DECLARE_MULTICAST_DELEGATE_TwoParams(FModInstalledDelegate, const TOptional<FInstalledMod>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FCleanTempDirDelegate, const TOptional<FCFCoreError>&);

public:
  virtual ~ICFCoreLibrary() = default;

public:
  /**
   * Allows to listen to library related events such as OnModInstallProgress
   */
  virtual FModInstallProgressDelegate& OnModInstallProgress() = 0;

  /**
   * Allows to listen to library related events such as OnModInstallProgress
   */
  virtual FModInstalledDelegate& OnModInstalled() = 0;

  // See cfcore_client_server_library.h
  virtual ICFCoreClientServerLibrary* ClientServerLibrary() = 0;

  // Perform a sync of all installed mods with the server - this will update
  // each InstalledMod's state to reflect updates
  virtual void SyncWithServer(FSyncWithServerDelegate delegate) = 0;

  // Get user's locally installed mods (including those pending to be installed)
  virtual void GetInstalledMods(FGetInstalledModsDelegate delegate) = 0;

  // Get all mods that are installed on the machine (including those pending
  // to be installed)
  virtual void GetSystemInstalledMods(FGetInstalledModsDelegate delegate) = 0;

  virtual void GetModsDirInfo(FGetModsDirInfoDelegate delegate) = 0;

  virtual void Install(FCFCoreMod mod,
                       FInstallProgressDelegate on_progress,
                       FInstalledDelegate on_installed) = 0;

  /**
   * Install a specific file of a mod
   */
  virtual void Install(const FCFCoreMod& mod,
                       const FFile& file,
                       FInstallProgressDelegate on_progress,
                       FInstalledDelegate on_installed) = 0;

  /**
   * This is (hopefully) the last version of Install, that supports more params
   */
  virtual void Install(const FCFCoreMod& mod,
                       const FFile& file,
                       const FInstallModAdditionalParams& additional_params,
                       FInstallProgressDelegate on_progress,
                       FInstalledDelegate on_installed) = 0;

  virtual void CancelInstallation(int64 mod_id,
                                  FCancelInstallDelegate delegate) = 0;

  virtual void Uninstall(int64 mod_id, FUninstallDelegate delegate) = 0;

  /**
   * Performs local validation (for corruption/replacement/deletion) of
   * installed mods.
   * Any mod that is invalid will be returned via the delegate and the local
   * status of an invalid mod will be set to Invalid and saved to the state of
   * the installed mods on disk - meaning, you may also check for invalid mods
   * by calling |GetInstalledMods| and checking each mod's status.
   */
  virtual void PerformModsValidation(
    const TArray<FInstalledMod>& installed_mods_to_validate,
    FPerformModsValidationDelegate delegate) = 0;

  /**
   * Allow the user to update specific properties of an installed mod, such as
   * if the mod is enabled or not, and the load order of the mod.
   */
  virtual void UpdateInstalledModsProperties(
    const TArray<FInstalledModProperties>& installed_mod_properties,
    FUpdateInstalledModsPropertiesDelegate delegate) = 0;

  /**
   * This will clean all stale files/folders from the temp directory to free up
   * some diskspace
   */
  virtual void CleanTempDir(FCleanTempDirDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreLibrary)