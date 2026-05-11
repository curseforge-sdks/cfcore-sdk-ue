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
#include <library/cfcore_library.h>
#include "library/cfcore_library_internal.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class CFCoreLibraryAbstract : public ICFCoreLibraryInternal,
                              public ICFCoreLibrary {
public:
  CFCoreLibraryAbstract(
    TSharedRef<ICFCoreLibraryInternal> next_library_internal,
    TSharedRef<ICFCoreLibrary> next_library);

  virtual ~CFCoreLibraryAbstract() = 0;

// ICFCoreLibraryInternal
public:
  virtual TSharedPtr<ICFCoreLibrary> GetLibrary() override;

  virtual TFuture<FCFCoreError> Initialize() override;
  virtual void SetClientServerLibrary(
    TSharedPtr<ICFCoreClientServerLibrary> client_server_library) override;

// ICFCoreLibrary
public:
  virtual ICFCoreLibrary::FModInstallProgressDelegate& OnModInstallProgress()
    override;

  virtual ICFCoreLibrary::FModInstalledDelegate& OnModInstalled() override;

  virtual ICFCoreClientServerLibrary* ClientServerLibrary() override;

  virtual void SyncWithServer(FSyncWithServerDelegate delegate) override;

  virtual void GetSystemInstalledMods(
    FGetInstalledModsDelegate delegate) override;

  virtual void GetInstalledMods(
    FGetInstalledModsDelegate delegate) override;

  virtual void GetModsDirInfo(FGetModsDirInfoDelegate delegate) override;

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

  virtual void CancelInstallation(int64 mod_id,
    FCancelInstallDelegate delegate) override;

  virtual void Uninstall(int64 mod_id, FUninstallDelegate delegate) override;

  virtual void PerformModsValidation(
    const TArray<FInstalledMod>& installed_mods_to_validate,
    FPerformModsValidationDelegate delegate) override;

  virtual void UpdateInstalledModsProperties(
    const TArray<FInstalledModProperties>& installed_mod_properties,
    FUpdateInstalledModsPropertiesDelegate delegate) override;

  virtual void CleanTempDir(FCleanTempDirDelegate delegate) override;

protected:
  TSharedRef<ICFCoreLibraryInternal> next_library_internal_;
  TSharedRef<ICFCoreLibrary> next_library_;
};

} // namespace cfcore