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
#include "installation_command_validate_installed_mod_impl.h"

#include <common/macros.h>

#include "installation_command_utils_service.h"

#include <services/logger/logger_macros.h>
#include <library/services/mods_validation/mods_validation_service.h>
#include <library/services/installed_mods_info/installed_mods_info_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandValidateInstalledModImpl::
InstallationCommandValidateInstalledModImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<IModsValidationService> mods_validation_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service) :
  utils_service_(utils_service),
  mods_validation_service_(mods_validation_service),
  installed_mods_info_service_(installed_mods_info_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandValidateInstalledModImpl::Run(
  FInstallModRequestPtr request) {

  if (!utils_service_->IsRequestForUpdate(request)) {
    ECFCoreErrorCodes code = ECFCoreErrorCodes::UnsupportedStrategy;
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
  }

  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::Validating);

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TArray<FInstalledMod> installed_mods { request->installedMod };
  mods_validation_service_->PerformValidation(installed_mods)
    .Next([this, request, promise](FCFCoreError err) {

    TOptional<FInstalledMod> installed_mod =
      installed_mods_info_service_->GetInstalledMod(request->modId);
    if (!installed_mod.IsSet()) {
      CFCORE_LOG(Error, TEXT("Missing installed mod info (?)"));
    } else {
      request->installedMod = *installed_mod;
    }


    promise->SetValue(err);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandValidateInstalledModImpl::Cancel(
  FInstallModRequestPtr request) {

  return true;
}
