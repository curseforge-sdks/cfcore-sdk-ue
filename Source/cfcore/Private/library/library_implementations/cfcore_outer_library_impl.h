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

class IInstalledModsInfoService;

// -----------------------------------------------------------------------------
class CFCoreOuterLibraryImpl :
  public CFCoreLibraryAbstract,
  public TSharedFromThis<CFCoreOuterLibraryImpl> {
public:
  CFCoreOuterLibraryImpl(
    TSharedRef<ICFCoreLibraryInternal> next_library_internal,
    TSharedRef<ICFCoreLibrary> next_library,
    TSharedRef<IInstalledModsInfoService> installed_mods_info_service);

  virtual ~CFCoreOuterLibraryImpl() = default;

// ICFCoreLibraryInternal
public:
  virtual TSharedPtr<ICFCoreLibrary> GetLibrary() override;

// ICFCoreLibrary
public:
  virtual void Install(FCFCoreMod mod,
    FInstallProgressDelegate on_progress,
    FInstalledDelegate on_installed) override;

  virtual void Install(const FCFCoreMod& mod,
    const FFile& file,
    FInstallProgressDelegate on_progress,
    FInstalledDelegate on_installed) override;

  virtual void Install(const FCFCoreMod& mod,
    const FFile& file,
    const FInstallModAdditionalParams& additional_params,
    FInstallProgressDelegate on_progress,
    FInstalledDelegate on_installed) override;

private:
  void Install(const FCFCoreMod& mod,
               const TOptional<FFile> opt_file,
               const FInstallModAdditionalParams& params,
               FInstallProgressDelegate on_prgrs,
               FInstalledDelegate on_inst);

  FCFCoreError ValidateInstallRequest(
    const FCFCoreMod& mod,
    const FFile* file_to_install,
    const FInstallModAdditionalParams& additional_params);

private:
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreOuterLibraryImpl)