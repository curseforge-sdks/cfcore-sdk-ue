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
#include <Async/Future.h>
#include <common/cfcore_error.h>
#include <library/models/installed_mod.h>
#include <library/models/mods_dir_info.h>
#include <library/models/library_progress.h>
#include <library/models/install_mod_additional_params.h>
#include <analytics/models/mod_install_params.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IModsInstallationService {
public:
  // TODO(twolf): Consider changing to TSharedPtr<FInstalledMod> to refrain
  // copying data
  struct InstallResult {
    FCFCoreError error;
    FInstalledMod installed_mod;
    FModInstallAnalyticsParams analytics_params;
  };

public:
  DECLARE_DELEGATE_OneParam(FModsInstallationProgressDelegate, const FLibraryProgress&);

public:
  virtual ~IModsInstallationService() = default;

public:
  virtual TFuture<FCFCoreError> Initialize() = 0;

  virtual TFuture<FCFCoreError> GetModsDirInfo(
    IN OUT TSharedRef<FModsDirInfo> mods_dir_info) = 0;

  virtual TFuture<InstallResult> Install(
    const FInstalledMod& installed_mod,
    const FInstallModAdditionalParams& additional_params,
    FModsInstallationProgressDelegate delegate) = 0;

  virtual bool CancelInstall(const FInstalledMod& installed_mod) = 0;

  virtual TFuture<FCFCoreError> Uninstall(
    const FInstalledMod& installed_mod) = 0;

  virtual bool IsModInstalling(const FInstalledMod& installed_mod) const = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IModsInstallationService)