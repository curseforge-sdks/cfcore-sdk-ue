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

namespace cfcore {
class IInstallationCommandUtilsService;
class ISettingsService;
class IFileSystemService;
class IDirectoriesService;

// -----------------------------------------------------------------------------
// Should be called after completing the download and unzipping of the new mod
// file - this will move the output from the temp directory to the install dir
class InstallationCommandMoveTempOutputDirImpl : public IInstallationCommand {
public:
  InstallationCommandMoveTempOutputDirImpl(
    TSharedRef<IInstallationCommandUtilsService> utils_service,
    TSharedRef<ISettingsService> settings_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<IDirectoriesService> directories_service);
  virtual ~InstallationCommandMoveTempOutputDirImpl() = default;

public:
  virtual ECFCoreInstallationCommands GetCommandId() const {
    return ECFCoreInstallationCommands::MoveToOutputDir;
  }

  virtual TFuture<FCFCoreError> Run(FInstallModRequestPtr request) override;

  virtual bool Cancel(FInstallModRequestPtr request) override;

  virtual void Visit(IInstallationCommandVisitor& visitor) override {};

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(InstallationCommandMoveTempOutputDirImpl);

  TFuture<FCFCoreError> MoveTempOutputDir(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> MoveModuleDirs(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> MoveTempOutputDirModules(
    FInstallModRequestPtr request);

  TFuture<FCFCoreError> MoveDirectoryOrFile(
    const FString& src_path,
    const FString& dest_path,
    FInstallModRequestPtr request);

private:
  TSharedRef<IInstallationCommandUtilsService> utils_service_;
  TSharedRef<ISettingsService> settings_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IDirectoriesService> directories_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::InstallationCommandMoveTempOutputDirImpl)