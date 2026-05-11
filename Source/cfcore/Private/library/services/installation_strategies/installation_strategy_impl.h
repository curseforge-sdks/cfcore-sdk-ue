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
#include "installation_strategy.h"

namespace cfcore {
class IInstallationCommandsRunnerService;

// -----------------------------------------------------------------------------
class InstallationStrategyImpl : public IInstallationStrategy {
public:
  InstallationStrategyImpl(
    TSharedRef<IInstallationCommandsRunnerService> commands_runner_service);

// IInstallationStrategy
public:
  virtual void SetCommandSequences(
    TArray<ECFCoreInstallationCommands> install_commands,
    TArray<ECFCoreInstallationCommands> uninstall_commands,
    TArray<ECFCoreInstallationCommands> cleanup_commands) override {

    install_commands_ = install_commands;
    uninstall_commands_ = uninstall_commands;
    cleanup_commands_ = cleanup_commands;
  }

  virtual TFuture<FCFCoreError> Install(
    TSharedRef<FInstallModRequest> request) override;

  virtual TFuture<FCFCoreError> Uninstall(
    TSharedRef<FInstallModRequest> request) override;

  virtual TFuture<FCFCoreError> Cleanup(
    TSharedRef<FInstallModRequest> request) override;

  virtual bool Cancel(TSharedRef<FInstallModRequest> request) override;

private:
  TFuture<FCFCoreError> RunCommands(
    const TArray<ECFCoreInstallationCommands>& commands,
    TSharedRef<FInstallModRequest> request);

private:
  TSharedRef<IInstallationCommandsRunnerService> commands_runner_service_;
  TArray<ECFCoreInstallationCommands> install_commands_;
  TArray<ECFCoreInstallationCommands> uninstall_commands_;
  TArray<ECFCoreInstallationCommands> cleanup_commands_;
};

} // namespace cfcore