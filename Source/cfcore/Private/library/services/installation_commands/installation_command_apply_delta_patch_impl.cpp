/* MIT License

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
SOFTWARE. */
#include "installation_command_apply_delta_patch_impl.h"

#include <common/macros.h>
#include <common/future_utils.h>
#include <common/path_utils.h>
#include <common/scoped_lambda.h>
#include "common/delta_diff_utils.h"

#include <services/logger/logger_macros.h>

#include "installation_command_utils_service.h"

#include <services/filesystem/file_system_service.h>
#include <services/binary_diff_service/binary_diff_service.h>

#include <Json.h>
#include <JsonUtilities.h>

namespace cfcore {

#define __PATCH_DELEGATE_FUNC__(func) \
    [this, request, extra_data]() { return func(request, extra_data); }

}

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandApplyDeltaPatchImpl::InstallationCommandApplyDeltaPatchImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map) :
  utils_service_(utils_service),
  file_system_service_(file_system_service),
  diff_method_to_service_map_(diff_method_to_service_map) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandApplyDeltaPatchImpl::Run(
  FInstallModRequestPtr request) {

  auto scoped_analytics = MakeShared<FScopedLambda<bool>>([request](bool ctx) {
    bool is_error = ctx;
    request->analytics_params.postDiffFailed = is_error;
    request->analytics_params.isDiff = !is_error;
  },
  false);

  FFileDeltaDiffExtraData extra_data;
  if (!DeltaDiffUtils::TryParsingExtraJsonData(request, extra_data)) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::MissingFileInformation;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  FCFCoreError error = ValidateExtraData(extra_data, request);
  if (error.isError) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(error)).GetFuture();
  }

  auto progress_state = ELibraryProgressState::Patching;
  utils_service_->TriggerProgressEvent(request, progress_state);

  InitializeRequestPatchState(request);

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  functions->Add(__PATCH_DELEGATE_FUNC__(CalcSumOfOutputFiles));
  functions->Add(__PATCH_DELEGATE_FUNC__(PatchAllFiles));

  SHAREABLE_PROMISE(FCFCoreError, promise);
  FutureUtils::SequenceAll(functions, &FCFCoreError::isError)
    .Next([this, request, scoped_analytics, promise](FCFCoreError err) {

    UninitializeRequestPatchState(request);

    scoped_analytics->SetContext(err.isError);
    promise->SetValue(err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandApplyDeltaPatchImpl::Cancel(
  FInstallModRequestPtr request) {

  request->cancelRequest = true;
  return true;
}

// -----------------------------------------------------------------------------
FCFCoreError InstallationCommandApplyDeltaPatchImpl::ValidateExtraData(
  const FFileDeltaDiffExtraData& extra_data,
  FInstallModRequestPtr request) {

  if (!DeltaDiffUtils::IsDiffMethodSupported(extra_data,
                                             diff_method_to_service_map_)) {

    CFCORE_LOG(Error,
               TEXT("Unsupported delta diff method (modId=%lld,ver=%d)"),
               request->modId,
               extra_data.version);

    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedSchemaVersion;
    return FCFCoreError(code);
  }

  if (extra_data.files.Num() == 0) {
    CFCORE_LOG(Warning,
               TEXT("Zero diff files (modId=%lld)"),
               extra_data.version,
               request->modId);
  }

  return FCFCoreError();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandApplyDeltaPatchImpl::CalcSumOfOutputFiles(
  FInstallModRequestPtr request,
  const FFileDeltaDiffExtraData& extra_data) {

  const TArray<FFileDeltaDiffExtraDataFileInfo>& patch_files = extra_data.files;

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  for (const FFileDeltaDiffExtraDataFileInfo& patch : patch_files) {
    functions->Add([this, request, patch]() {
      return AddOutputFileSize(request, patch);
    });
  }

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
void InstallationCommandApplyDeltaPatchImpl::InitializeRequestPatchState(
  FInstallModRequestPtr request) {

  check(!mod_id_to_patch_state_map_.Contains(request->modId));
  mod_id_to_patch_state_map_.Add(request->modId, { 0 });
}

// -----------------------------------------------------------------------------
void InstallationCommandApplyDeltaPatchImpl::UninitializeRequestPatchState(\
  FInstallModRequestPtr request) {

  check(mod_id_to_patch_state_map_.Contains(request->modId));
  mod_id_to_patch_state_map_.Remove(request->modId);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandApplyDeltaPatchImpl::PatchAllFiles(
  FInstallModRequestPtr request,
  const FFileDeltaDiffExtraData& extra_data) {

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  for (const FFileDeltaDiffExtraDataFileInfo& patch : extra_data.files) {
    functions->Add([this, request, extra_data, patch]() {
      return ApplyPatch(request, extra_data, patch);
    });
  }

  // NOTE(twolf): We run this in sequence for performance reasons, if one
  // patch fails - it would be a costly loss to run other patches (because
  // we can't proceed if even 1 patch fails)
  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandApplyDeltaPatchImpl::ApplyPatch(
  FInstallModRequestPtr request,
  const FFileDeltaDiffExtraData& extra_data,
  const FFileDeltaDiffExtraDataFileInfo& patch) {

  IBinaryDiffServiceRef* diff_service =
    diff_method_to_service_map_->Find(patch.method);
  if (!diff_service) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedDiffMethod;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  FString old_file = GetOldFilePath(patch, request);
  FString patch_file = GetPatchFilePath(patch, request);
  FString output_file = GetOutputFilePath(extra_data, patch, request);

  SHAREABLE_PROMISE(FCFCoreError, promise);

  FApplyDeltaPatchState& state = mod_id_to_patch_state_map_[request->modId];
  auto future = (*diff_service)->ApplyBinaryPatch(
    old_file,
    patch_file,
    output_file,
    IBinaryDiffService::FProgressDelegate::CreateLambda(
      [this, request](const FFileTransferProgress& progress) {

        HandlePatchingProgress(request, progress);
      })
    );

  future.Next([this, promise, patch_file](FCFCoreError err) {
    if (err.isError) {
      promise->SetValue(err);
      return;
    }

    file_system_service_->DeleteFile(MakeShared<FString>(patch_file))
      .Next([promise, patch_file](bool success) {

      if (!success) {
        CFCORE_LOG(Warning,
                    TEXT("Failed erasing delta diff patch file (%s)"),
                    *patch_file);
      }

      return promise->SetValue(FCFCoreError());
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void InstallationCommandApplyDeltaPatchImpl::HandlePatchingProgress(
  FInstallModRequestPtr request,
  const FFileTransferProgress& progress) {

  FApplyDeltaPatchState& state = mod_id_to_patch_state_map_[request->modId];

  int64 current_processed_bytes =
    state.total_processed_output_files_bytes + progress.transferredBytes;

  int32 progress_percent =
    (static_cast<float>(current_processed_bytes) / state.total_output_files_bytes) * 100;

  // NOTE(apliscov): we don't send a progress event if progress hasn't changed
  if (progress_percent <= state.previous_progress) {
    return;
  }

  state.previous_progress = progress_percent;

  SumCurrentFileSize(state, progress_percent, progress.transferredBytes);

  FLibraryProgressDataTransfer data_transfer;
  data_transfer.progress = progress_percent;
  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::Patching,
                                       &data_transfer);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandApplyDeltaPatchImpl::AddOutputFileSize(
  FInstallModRequestPtr request,
  const FFileDeltaDiffExtraDataFileInfo& patch) {

  IBinaryDiffServiceRef* diff_service =
    diff_method_to_service_map_->Find(patch.method);
  if (!diff_service) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedDiffMethod;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  FString patch_file = GetPatchFile(request, patch);

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto output_file_size = MakeShared<int64>(0);
  (*diff_service)->CalcOutputFileSize(patch_file, output_file_size)
    .Next([this, request, promise, output_file_size](FCFCoreError err) {

    check(mod_id_to_patch_state_map_.Contains(request->modId));
    FApplyDeltaPatchState& state = mod_id_to_patch_state_map_[request->modId];
    state.total_output_files_bytes += *output_file_size;

    promise->SetValue(err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
FString InstallationCommandApplyDeltaPatchImpl::GetPatchFile(
  FInstallModRequestPtr request,
  const FFileDeltaDiffExtraDataFileInfo& patch) {

  FString patchFilename = patch.patchFilename;
  const FFile& latestUpdatedFile = request->installedMod.latestUpdatedFile;
  FString new_dir = utils_service_->GetTempOutputDir(latestUpdatedFile);
  FString patch_file = FPaths::Combine(new_dir, patch.path, patchFilename);
  return patch_file;
}

// -----------------------------------------------------------------------------
FString InstallationCommandApplyDeltaPatchImpl::GetOldFilePath(
  const FFileDeltaDiffExtraDataFileInfo& patch,
  FInstallModRequestPtr request) {

  FString origFilename = patch.originalFilename;
  FString patchFilename = patch.patchFilename;
  const FFile& installedFile = request->installedMod.installedFile;
  FString existing_dir = utils_service_->GetOutputDir(installedFile);
  FString old_file = FPaths::Combine(existing_dir, patch.path, origFilename);
  old_file = PathUtils::NormalizePath(old_file);
  return old_file;
}

// -----------------------------------------------------------------------------
FString InstallationCommandApplyDeltaPatchImpl::GetPatchFilePath(
  const FFileDeltaDiffExtraDataFileInfo& patch,
  FInstallModRequestPtr request) {

  FString patchFilename = patch.patchFilename;
  const FFile& latestUpdatedFile = request->installedMod.latestUpdatedFile;
  FString new_dir = utils_service_->GetTempOutputDir(latestUpdatedFile);
  FString patch_file = FPaths::Combine(new_dir, patch.path, patchFilename);
  patch_file = PathUtils::NormalizePath(patch_file);
  return patch_file;
}

// -----------------------------------------------------------------------------
FString InstallationCommandApplyDeltaPatchImpl::GetOutputFilePath(
  const FFileDeltaDiffExtraData& extra_data,
  const FFileDeltaDiffExtraDataFileInfo& patch,
  FInstallModRequestPtr request) {

  FString filename = patch.originalFilename;
  if (SupportsNewFilename(extra_data, patch)) {
    filename = patch.newFilename;
  }

  FString path = patch.path;
  if (SupportsNewPath(extra_data, patch)) {
    path = patch.newPath;
  }

  const FFile& latestUpdatedFile = request->installedMod.latestUpdatedFile;
  FString new_dir = utils_service_->GetTempOutputDir(latestUpdatedFile);
  FString output_file = FPaths::Combine(new_dir, path, filename);
  output_file = PathUtils::NormalizePath(output_file);
  return output_file;
}


// -----------------------------------------------------------------------------
bool InstallationCommandApplyDeltaPatchImpl::SupportsNewFilename(
  const FFileDeltaDiffExtraData& extra_data,
  const FFileDeltaDiffExtraDataFileInfo& patch) {

  if (extra_data.version < 3) {
    return false;
  }

  if (patch.newFilename.IsEmpty()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool InstallationCommandApplyDeltaPatchImpl::SupportsNewPath(
  const FFileDeltaDiffExtraData& extra_data,
  const FFileDeltaDiffExtraDataFileInfo& patch) {

  if (extra_data.version < 3) {
    return false;
  }

  if (patch.newPath.IsEmpty()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
void InstallationCommandApplyDeltaPatchImpl::SumCurrentFileSize(
  FApplyDeltaPatchState& state,
  int32 progress_percent,
  int64 transferred_bytes ) {

  if (progress_percent == 100) {
    state.total_processed_output_files_bytes += transferred_bytes;
  }
}
