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
#include <Containers/Map.h>

#include "mods_installation_queue_service.h"

#include <library/cfcore_library_delegate.h>
#include <library/services/mods_validation/mods_validation_service_delegate.h>
#include <library/services/mods_cleanup/mods_cleanup_service_delegate.h>
#include <library/services/installed_mods_info/installed_mods_info_service_delegate.h>

namespace cfcore {

class IModsInstallationQueueService;
class IInstalledModsInfoService;
class IInstallationCommandUtilsService;

// -----------------------------------------------------------------------------
class InstalledModPendingStatusServiceImpl :
  public IModsValidationServiceDelegate,
  public IModsCleanupServiceDelegate,
  public ICFCoreLibraryDelegate,
  public IInstalledModsInfoServiceDelegate {

public:
  InstalledModPendingStatusServiceImpl(
    TSharedRef<IModsInstallationQueueService> installation_queue_service,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
    TSharedRef<IInstallationCommandUtilsService> command_utils_service);

// IModsValidationServiceDelegate
public:
  virtual TFuture<bool> OnPerformQuickValidation(
    const FInstalledMod& installed_mod,
    bool& out_skip_other_validations) override;

// IModsCleanupServiceDelegate
public:
  virtual TSet<FString> OnGetKeepTempFilesAndDirs(
    const FInstalledMod& installed_mod,
    bool force_cleanup) override;

// ICFCoreLibraryDelegate
public:
  virtual bool OnHideInstalledMod(const FInstalledMod& installed_mod) override;

// IInstalledModsInfoServiceDelegate
  virtual void OnSetInstallFailed(const FInstalledMod& installed_mod,
                                  bool& out_keep_installed_mod) override;

private:
  void FillModInInstallingQueueFilesAndDirs(const FInstalledMod& installed_mod,
                                            TSet<FString>& keep_files_and_dirs);

  bool IsModInInstallingQueue(const FInstalledMod& installed_mod);
  bool IsModInUpdatePending(const FInstalledMod& installed_mod);
  TFuture<bool> SetModOutOfDate(const FInstalledMod& installed_mod);
  TFuture<bool> UninstallMod(const FInstalledMod& installed_mod);

private:
  TSharedRef<IModsInstallationQueueService> installation_queue_service_;
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
  TSharedRef<IInstallationCommandUtilsService> command_utils_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstalledModPendingStatusServiceImpl)