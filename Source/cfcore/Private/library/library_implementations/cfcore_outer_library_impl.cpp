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
#include "cfcore_outer_library_impl.h"

#include <services/logger/logger_macros.h>

#include "library/services/installed_mods_info/installed_mods_info_service.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreOuterLibraryImpl::CFCoreOuterLibraryImpl(
  TSharedRef<ICFCoreLibraryInternal> next_library_internal,
  TSharedRef<ICFCoreLibrary> next_library,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service) :
  CFCoreLibraryAbstract(next_library_internal, next_library),
  installed_mods_info_service_(installed_mods_info_service) {
}

// ICFCoreLibraryInternal
// -----------------------------------------------------------------------------
TSharedPtr<ICFCoreLibrary> CFCoreOuterLibraryImpl::GetLibrary() {
  return this->AsShared();
}

// ICFCoreLibrary
// -----------------------------------------------------------------------------
void CFCoreOuterLibraryImpl::Install(
  FCFCoreMod mod,
  FInstallProgressDelegate on_prgrs,
  FInstalledDelegate on_installed) {

  Install(mod,
          TOptional<FFile>(),
          FInstallModAdditionalParams(),
          on_prgrs,
          on_installed);
}

// -----------------------------------------------------------------------------
void CFCoreOuterLibraryImpl::Install(
  const FCFCoreMod& mod,
  const FFile& file,
  FInstallProgressDelegate on_progress,
  FInstalledDelegate on_installed) {

  Install(mod, file, FInstallModAdditionalParams(), on_progress, on_installed);
}

// -----------------------------------------------------------------------------
void CFCoreOuterLibraryImpl::Install(
  const FCFCoreMod& mod,
  const FFile& file,
  const FInstallModAdditionalParams& additional_params,
  FInstallProgressDelegate on_prgrs,
  FInstalledDelegate on_inst) {

  TOptional<FFile> opt_file;
  if (file.id != 0) {
    opt_file = file;
  }

  Install(mod, opt_file, additional_params, on_prgrs, on_inst);
}

// -----------------------------------------------------------------------------
void CFCoreOuterLibraryImpl::Install(
  const FCFCoreMod& mod,
  const TOptional<FFile> opt_file,
  const FInstallModAdditionalParams& params,
  FInstallProgressDelegate on_prgrs,
  FInstalledDelegate on_inst) {

  check(IsInGameThread());

  const FFile* file_to_inst = installed_mods_info_service_->GetLatestFile(mod);
  if (opt_file) {
    file_to_inst = &opt_file.GetValue();
  }

  FCFCoreError err = ValidateInstallRequest(mod, file_to_inst, params);

  if (err.isError) {
    on_inst.ExecuteIfBound({}, err);
    return;
  }

  CFCORE_LOG(Log,
    TEXT("Request to Install mod '%s' (modId=%lld, fileId=%lld)"),
    *mod.name,
    mod.id,
    file_to_inst != nullptr ? file_to_inst->id : 0);

  next_library_->Install(mod, *file_to_inst, params, on_prgrs, on_inst);
}

// -----------------------------------------------------------------------------
FCFCoreError CFCoreOuterLibraryImpl::ValidateInstallRequest(
  const FCFCoreMod& mod,
  const FFile* file_to_install,
  const FInstallModAdditionalParams& additional_params) {

  if (!file_to_install) {
    CFCORE_LOG(Error, TEXT("Mod (%lld) has no latest file"), mod.id);
    auto err_code = ECFCoreErrorCodes::MissingLatestFileInformation;
    return FCFCoreError(err_code);
  }

  if (file_to_install->downloadUrl.IsEmpty()) {
    CFCORE_LOG(Error,
      TEXT("File (%lld) for Mod (%lld) has no url"),
      file_to_install->id,
      mod.id);

    auto err_code = ECFCoreErrorCodes::MissingFileInformation;
    return FCFCoreError(err_code);
  }

  if (file_to_install->modId != mod.id) {
    CFCORE_LOG(Error,
      TEXT("File (%lld) doesn't belong to Mod (%lld)"),
      file_to_install->id,
      mod.id);

    auto err_code = ECFCoreErrorCodes::FileNotBelongingToMod;
    return FCFCoreError(err_code);
  }

  return FCFCoreError();
}