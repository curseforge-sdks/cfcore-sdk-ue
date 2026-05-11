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
#include "cfcore_progress_library_impl.h"

namespace cfcore {

using FModInstallProgressDelegate = ICFCoreLibrary::FModInstallProgressDelegate;

}

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreProgressLibraryImpl::CFCoreProgressLibraryImpl(
  TSharedRef<ICFCoreLibraryInternal> next_library_internal,
  TSharedRef<ICFCoreLibrary> next_library) :
  CFCoreLibraryAbstract(next_library_internal, next_library) {
}

// -----------------------------------------------------------------------------
void CFCoreProgressLibraryImpl::GetInstalledMods(
  FGetInstalledModsDelegate delegate) {

  next_library_->GetInstalledMods(FGetInstalledModsDelegate::CreateLambda(
    [this, delegate](const TArray<FInstalledMod>& installed_mods) {

    TArray<FInstalledMod> installed_mods_copy(installed_mods);

    for (FInstalledMod& installed_mod : installed_mods_copy) {
      int64 file_id = installed_mod.installedFile.id;
      if (file_id_to_install_progress.Contains(file_id)) {
        installed_mod.installProgress = file_id_to_install_progress[file_id];
      }
    }

    delegate.ExecuteIfBound(installed_mods_copy);
  }));
}

// -----------------------------------------------------------------------------
void CFCoreProgressLibraryImpl::Install(
  const FCFCoreMod& mod,
  const FFile& file,
  const FInstallModAdditionalParams& additional_params,
  FInstallProgressDelegate on_progress,
  FInstalledDelegate on_inst) {

  next_library_->Install(
    mod,
    file,
    additional_params,
    FInstallProgressDelegate::CreateLambda(
      [this, on_progress](const FLibraryProgress& progress) {

      file_id_to_install_progress.Add(progress.fileId, progress);
      mod_install_progress_delegate_.Broadcast(progress);
      on_progress.ExecuteIfBound(progress);
    }),
    FInstalledDelegate::CreateLambda(
      [this, on_inst, file](const TOptional<FInstalledMod>& opt_inst_mod,
                            const TOptional<FCFCoreError>& opt_err) {

      file_id_to_install_progress.Remove(file.id);
      mod_installed_delegate_.Broadcast(opt_inst_mod, opt_err);
      on_inst.ExecuteIfBound(opt_inst_mod, opt_err);
    }));
}

