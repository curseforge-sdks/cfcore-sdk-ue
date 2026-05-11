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
#include "CoreMinimal.h"
#include "cfcore_library_abstract.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class CFCoreProgressLibraryImpl : public CFCoreLibraryAbstract {
public:
  CFCoreProgressLibraryImpl(
    TSharedRef<ICFCoreLibraryInternal> next_library_internal,
    TSharedRef<ICFCoreLibrary> next_library);

  virtual ~CFCoreProgressLibraryImpl() = default;

// ICFCoreLibrary
public:
  virtual ICFCoreLibrary::FModInstallProgressDelegate&
    OnModInstallProgress() override {
    return mod_install_progress_delegate_;
  }

  virtual ICFCoreLibrary::FModInstalledDelegate& OnModInstalled() override {
    return mod_installed_delegate_;
  }

  virtual void GetInstalledMods(
    FGetInstalledModsDelegate delegate) override;

  virtual void Install(const FCFCoreMod& mod,
    const FFile& file,
    const FInstallModAdditionalParams& additional_params,
    FInstallProgressDelegate on_progress,
    FInstalledDelegate on_installed) override;

private:
  TMap<int64, FLibraryProgress> file_id_to_install_progress;

  ICFCoreLibrary::FModInstallProgressDelegate mod_install_progress_delegate_;
  ICFCoreLibrary::FModInstalledDelegate mod_installed_delegate_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreProgressLibraryImpl)