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
#include "installation_command.h"

#include <common/macros.h>

#include <api/models/file_delta_diff.h>
#include <library/services/installation_commands/models/diff_method_to_service_map.h>

namespace cfcore {
class IInstallationCommandUtilsService;
class IFeaturePhasingService;
class ICFCoreApi;

// -----------------------------------------------------------------------------
// Test the prerequists for running a delta-diff installation
class InstallationCommandDeltaDiffSupportedImpl : public IInstallationCommand {
public:
  InstallationCommandDeltaDiffSupportedImpl(
    TSharedRef<IInstallationCommandUtilsService> utils_service,
    TSharedRef<IFeaturePhasingService> feature_phasing_service,
    TSharedRef<ICFCoreApi> cfcore_api,
    TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map);
  virtual ~InstallationCommandDeltaDiffSupportedImpl() = default;

public:
  virtual ECFCoreInstallationCommands GetCommandId() const {
    return ECFCoreInstallationCommands::DeltaDiffSupported;
  }

  virtual TFuture<FCFCoreError> Run(FInstallModRequestPtr request) override;

  virtual bool Cancel(FInstallModRequestPtr request) override;

  virtual void Visit(IInstallationCommandVisitor& visitor) override {};

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(InstallationCommandDeltaDiffSupportedImpl);

  TFuture<FCFCoreError> GetAndValidateDeltaDiffInfo(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> GetModFileDeltaDiff(
    const FInstalledMod& installed_mod,
    OUT TSharedRef<FFileDeltaDiff> out_file_delta_diff);

private:
  TSharedRef<IInstallationCommandUtilsService> utils_service_;
  TSharedRef<IFeaturePhasingService> feature_phasing_service_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallationCommandDeltaDiffSupportedImpl)