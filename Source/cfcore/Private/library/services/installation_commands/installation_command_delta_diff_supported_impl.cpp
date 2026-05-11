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
#include "installation_command_delta_diff_supported_impl.h"

#include <common/macros.h>
#include <common/constants.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>
#include <api/services/feature_phasing_service.h>
#include <api/cfcore_api.h>

#include "installation_command_utils_service.h"
#include "common/delta_diff_utils.h"
#include "models/file_delta_diff_extra_data.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandDeltaDiffSupportedImpl::InstallationCommandDeltaDiffSupportedImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<IFeaturePhasingService> feature_phasing_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map) :
  utils_service_(utils_service),
  feature_phasing_service_(feature_phasing_service),
  cfcore_api_(cfcore_api),
  diff_method_to_service_map_(diff_method_to_service_map) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandDeltaDiffSupportedImpl::Run(
  FInstallModRequestPtr request) {

  if (!utils_service_->IsRequestForUpdate(request)) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedStrategy;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  auto feature = &FMePhasingData::deltaDiffs;
  bool enabled = feature_phasing_service_->IsFeatureEnabledForUser(feature);
  if (!enabled) {
    CFCORE_LOG(Log,
               TEXT("Diff feature disabled for user (modId=%lld)"),
               request->modId);

    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedStrategy;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  return GetAndValidateDeltaDiffInfo(request);
}

// -----------------------------------------------------------------------------
bool InstallationCommandDeltaDiffSupportedImpl::Cancel(
  FInstallModRequestPtr request) {

  return true;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDeltaDiffSupportedImpl::GetAndValidateDeltaDiffInfo(
  FInstallModRequestPtr request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto file_delta_diff = MakeShared<FFileDeltaDiff>();
  GetModFileDeltaDiff(request->installedMod, file_delta_diff).Next(
    [this, request, promise, file_delta_diff](FCFCoreError err) {

    if (err.isError) {
      promise->SetValue(err);
      return;
    }

    request->state.file_delta_diff_info = file_delta_diff;

    FFileDeltaDiffExtraData extra_data;
    if (!DeltaDiffUtils::TryParsingExtraJsonData(request, extra_data)) {
      ECFCoreErrorCodes code = ECFCoreErrorCodes::MissingFileInformation;
      promise->SetValue(FCFCoreError(code));
      return;
    }

    if (!DeltaDiffUtils::IsDiffMethodSupported(extra_data,
                                               diff_method_to_service_map_)) {

      ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedSchemaVersion;
      promise->SetValue(code);
      return;
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstallationCommandDeltaDiffSupportedImpl::GetModFileDeltaDiff(
  const FInstalledMod& installed_mod,
  OUT TSharedRef<FFileDeltaDiff> out_file_delta_diff) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  int64 mod_id = installed_mod.details.id;
  int64 new_file_id = installed_mod.latestUpdatedFile.id;
  int64 old_file_id = installed_mod.installedFile.id;

  cfcore_api_->GetModFileDeltaDiff(
    mod_id,
    new_file_id,
    old_file_id,
    ICFCoreApi::FGetModFileDeltaDiffDelegate::CreateLambda(
      [promise, out_file_delta_diff](
        const TOptional<FFileDeltaDiff>& opt_diff,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

    if (opt_err.IsSet()) {
      CFCORE_LOG(Log, TEXT("Missing delta diff file"));
      promise->SetValue(opt_err);
      return;
    }

    if (opt_diff->extraJsonData.IsEmpty()) {
      CFCORE_LOG(Error, TEXT("Delta diff missing extraJsonData"));
      auto code = ECFCoreErrorCodes::MissingFileInformation;
      promise->SetValue(FCFCoreError(code));
      return;
    }

    *out_file_delta_diff = *opt_diff;
    promise->SetValue(FCFCoreError());
  }));

  return promise->GetFuture();
}
