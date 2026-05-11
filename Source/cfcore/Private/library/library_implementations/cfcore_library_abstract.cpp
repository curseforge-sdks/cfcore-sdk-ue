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
#include "cfcore_library_abstract.h"
#include <library/common/cfcore_library_macros.h>

namespace cfcore {

using FModInstallProgressDelegate = ICFCoreLibrary::FModInstallProgressDelegate;

}
using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreLibraryAbstract::CFCoreLibraryAbstract(
  TSharedRef<ICFCoreLibraryInternal> next_library_internal,
  TSharedRef<ICFCoreLibrary> next_library) :
  next_library_internal_(next_library_internal),
  next_library_(next_library) {
}

// -----------------------------------------------------------------------------
CFCoreLibraryAbstract::~CFCoreLibraryAbstract() {}

// ICFCoreLibraryInternal
// -----------------------------------------------------------------------------
TSharedPtr<ICFCoreLibrary> CFCoreLibraryAbstract::GetLibrary() {
  return next_library_internal_->GetLibrary();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreLibraryAbstract::Initialize() {
  return next_library_internal_->Initialize();
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::SetClientServerLibrary(
  TSharedPtr<ICFCoreClientServerLibrary> client_server_library) {

  next_library_internal_->SetClientServerLibrary(client_server_library);
}

// ICFCoreLibrary
// -----------------------------------------------------------------------------
ICFCoreLibrary::FModInstallProgressDelegate&
CFCoreLibraryAbstract::OnModInstallProgress() {
  return next_library_->OnModInstallProgress();
}

// -----------------------------------------------------------------------------
ICFCoreLibrary::FModInstalledDelegate& CFCoreLibraryAbstract::OnModInstalled() {
  return next_library_->OnModInstalled();
}

// -----------------------------------------------------------------------------
ICFCoreClientServerLibrary*
CFCoreLibraryAbstract::ClientServerLibrary() {

  return next_library_->ClientServerLibrary();
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::SyncWithServer(
  FSyncWithServerDelegate delegate) {

  next_library_->SyncWithServer(delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::GetSystemInstalledMods(
  FGetInstalledModsDelegate delegate) {

  next_library_->GetSystemInstalledMods(delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::GetInstalledMods(
  FGetInstalledModsDelegate delegate) {

  next_library_->GetInstalledMods(delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::GetModsDirInfo(
  FGetModsDirInfoDelegate delegate) {

  next_library_->GetModsDirInfo(delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::Install(
  FCFCoreMod mod,
  FInstallProgressDelegate on_progress,
  FInstalledDelegate on_installed) {

  CFCORE_LIBRARY_MARK_UNSUPPORTED_INSTALL;
  next_library_->Install(mod, on_progress, on_installed);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::Install(
  const FCFCoreMod& mod,
  const FFile& file,
  FInstallProgressDelegate on_progress,
  FInstalledDelegate on_installed) {

  CFCORE_LIBRARY_MARK_UNSUPPORTED_INSTALL;
  next_library_->Install(mod, file, on_progress, on_installed);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::Install(
  const FCFCoreMod& mod,
  const FFile& file,
  const FInstallModAdditionalParams& additional_params,
  FInstallProgressDelegate on_progress,
  FInstalledDelegate on_inst) {

  next_library_->Install(mod, file, additional_params, on_progress, on_inst);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::CancelInstallation(
  int64 mod_id,
  FCancelInstallDelegate delegate) {

  next_library_->CancelInstallation(mod_id, delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::Uninstall(int64 mod_id,
  FUninstallDelegate delegate) {

  next_library_->Uninstall(mod_id, delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::PerformModsValidation(
  const TArray<FInstalledMod>& installed_mods_to_validate,
  FPerformModsValidationDelegate delegate) {

  next_library_->PerformModsValidation(installed_mods_to_validate, delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::UpdateInstalledModsProperties(
  const TArray<FInstalledModProperties>& installed_mod_properties,
  FUpdateInstalledModsPropertiesDelegate delegate) {

  next_library_->UpdateInstalledModsProperties(
    installed_mod_properties,
    delegate);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryAbstract::CleanTempDir(
  FCleanTempDirDelegate delegate) {

  next_library_->CleanTempDir(delegate);
}

