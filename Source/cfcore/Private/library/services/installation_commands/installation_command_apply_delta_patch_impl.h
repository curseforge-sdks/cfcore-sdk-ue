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

#include <api/models/file_transfer_progress.h>
#include <api/models/file_delta_diff.h>
#include <library/services/installation_commands/models/file_delta_diff_extra_data.h>
#include <library/services/installation_commands/models/diff_method_to_service_map.h>
#include "models/apply_delta_patch_state.h"

namespace cfcore {
class IInstallationCommandUtilsService;
class IFileSystemService;

// -----------------------------------------------------------------------------
class InstallationCommandApplyDeltaPatchImpl : public IInstallationCommand {
public:
  InstallationCommandApplyDeltaPatchImpl(
    TSharedRef<IInstallationCommandUtilsService> utils_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map);
  virtual ~InstallationCommandApplyDeltaPatchImpl() = default;

public:
  virtual ECFCoreInstallationCommands GetCommandId() const {
    return ECFCoreInstallationCommands::ApplyDeltaPatch;
  }

  virtual TFuture<FCFCoreError> Run(FInstallModRequestPtr request) override;

  virtual bool Cancel(FInstallModRequestPtr request) override;

  virtual void Visit(IInstallationCommandVisitor& visitor) override {};

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(InstallationCommandApplyDeltaPatchImpl);

  void InitializeRequestPatchState(FInstallModRequestPtr request);
  void UninitializeRequestPatchState(FInstallModRequestPtr request);

  TFuture<FCFCoreError> ApplyPatch(
    FInstallModRequestPtr request,
    const FFileDeltaDiffExtraData& extra_data,
    const FFileDeltaDiffExtraDataFileInfo& patch);

  void HandlePatchingProgress(FInstallModRequestPtr request,
                              const FFileTransferProgress& progress);

  TFuture<FCFCoreError> AddOutputFileSize(
    FInstallModRequestPtr request,
    const FFileDeltaDiffExtraDataFileInfo& patch);

  FString GetPatchFile(
    FInstallModRequestPtr request,
    const FFileDeltaDiffExtraDataFileInfo& patch);

  TFuture<FCFCoreError> CalcSumOfOutputFiles(
    FInstallModRequestPtr request,
    const FFileDeltaDiffExtraData& extra_data);

  FCFCoreError ValidateExtraData(const FFileDeltaDiffExtraData& extra_data,
                                 FInstallModRequestPtr request);

  FString GetOldFilePath(const FFileDeltaDiffExtraDataFileInfo& patch,
                         FInstallModRequestPtr request);

  FString GetPatchFilePath(const FFileDeltaDiffExtraDataFileInfo& patch,
                           FInstallModRequestPtr request);

  FString GetOutputFilePath(const FFileDeltaDiffExtraData& extra_data,
                            const FFileDeltaDiffExtraDataFileInfo& patch,
                            FInstallModRequestPtr request);

  bool SupportsNewFilename(const FFileDeltaDiffExtraData& extra_data,
                           const FFileDeltaDiffExtraDataFileInfo& patch);

  bool SupportsNewPath(const FFileDeltaDiffExtraData& extra_data,
                       const FFileDeltaDiffExtraDataFileInfo& patch);

  void SumCurrentFileSize(FApplyDeltaPatchState& state,
    int32 progress_percent,
    int64 transferred_bytes);

  TFuture<FCFCoreError> PatchAllFiles(
    FInstallModRequestPtr request,
    const FFileDeltaDiffExtraData& extra_data);

private:
  TSharedRef<IInstallationCommandUtilsService> utils_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map_;

  TMap<int64, FApplyDeltaPatchState> mod_id_to_patch_state_map_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallationCommandApplyDeltaPatchImpl)