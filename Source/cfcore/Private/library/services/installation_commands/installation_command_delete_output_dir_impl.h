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
#include <common/cfcore_error.h>
#include <common/future_utils.h>

namespace cfcore {
class IInstallationCommandUtilsService;
class ISettingsService;
class IFileSystemService;
class IDirectoriesService;

// -----------------------------------------------------------------------------
// Deletes the existing installed mod - either for uninstalling or for
// reinstalling (e.g. invalid/corrupted mods)
class InstallationCommandDeleteOutputDirImpl : public IInstallationCommand {
public:
  InstallationCommandDeleteOutputDirImpl(
    TSharedRef<IInstallationCommandUtilsService> utils_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<IDirectoriesService> directories_service);
  virtual ~InstallationCommandDeleteOutputDirImpl() = default;

public:
  virtual ECFCoreInstallationCommands GetCommandId() const {
    return ECFCoreInstallationCommands::DeleteOutputDir;
  }

  virtual TFuture<FCFCoreError> Run(FInstallModRequestPtr request) override;

  virtual bool Cancel(FInstallModRequestPtr request) override;

  virtual void Visit(IInstallationCommandVisitor& visitor) override {};

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(InstallationCommandDeleteOutputDirImpl);

  TFuture<FCFCoreError> DeleteExistingOutputDir(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> DeleteExistingOutputDirModules(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> DeleteRequiredOutputDirModules(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> CleanupExistingOutputDirModules(
    const TArray<FFileModule>& modules);

  TFuture<FCFCoreError> CleanupDirectoryOrFile(TSharedRef<FString> path);

private:
  TSharedRef<IInstallationCommandUtilsService> utils_service_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IDirectoriesService> directories_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallationCommandDeleteOutputDirImpl)