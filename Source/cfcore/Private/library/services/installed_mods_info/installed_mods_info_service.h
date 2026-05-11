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
#include <Async/Async.h>
#include <common/cfcore_error.h>
#include <library/models/installed_mod.h>
#include <library/models/install_mod_additional_params.h>
#include <api/models/mod.h>
#include <api/models/file.h>
#include <api/models/filters/get_mods_filter.h>

#include <library/models/install_mod_params.h>

#include <library/services/models/internal_installed_mod_info_properties.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IInstalledModsInfoService {
public:
  using InstallModsInfoResult = TTuple<FCFCoreError, FInstalledMod>;

public:
  virtual ~IInstalledModsInfoService() = default;

public:
  virtual TFuture<FCFCoreError> Initialize() = 0;

  virtual TOptional<FInstalledMod> GetInstalledMod(int64 mod_id) = 0;
  virtual TOptional<FInstalledMod> GetInstalledMod(const FGuid& id) = 0;

  virtual void GetInstalledMods(TArray<FInstalledMod>& out_installed_mods) = 0;

  virtual TFuture<InstallModsInfoResult> SetInstalling(
    TSharedRef<FInstallModParams> install_params) = 0;
  virtual TFuture<InstallModsInfoResult> SetInstalled(
    const FInstalledMod& installed_mod,
    const FInstallModAdditionalParams& additional_params) = 0;
  virtual TFuture<InstallModsInfoResult> SetInstallFailed(
    const FInstalledMod& installed_mod) = 0;

  virtual TFuture<FCFCoreError> SetInstalledProperties(
    const TArray<FInternalInstalledModInfoProperties>& properties) = 0;

  /**
   * This was introduced with the unamanged mods feature to allow skipping the
   * standard tests taking place for managed mods.
   */
  virtual TFuture<FCFCoreError> SetUnmanagedInstalledMods(
    const TArray<FInstalledMod>& installed_mods) = 0;

  virtual TFuture<InstallModsInfoResult> Uninstall(int64 mod_id) = 0;
  virtual TFuture<InstallModsInfoResult> Uninstall(const FGuid& id) = 0;

  virtual const FFile* GetLatestFile(const FCFCoreMod& mod) = 0;

  virtual TFuture<FCFCoreError> UpdateModDetails(
    const TArray<FCFCoreMod>& mod_details) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IInstalledModsInfoService)