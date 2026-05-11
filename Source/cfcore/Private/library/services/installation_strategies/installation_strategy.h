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
#include "CoreMinimal.h"

#include <common/cfcore_error.h>
#include <common/future_utils.h>
#include <library/services/models/install_mod_request.h>
#include <library/services/installation_commands/models/enums/installation_commands.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// An interface that represents a method for installing a mod
class IInstallationStrategy {
public:
  virtual ~IInstallationStrategy() = default;

public:
  /**
   * Set commands for installing/uninstalling and cleaning up
   */
  virtual void SetCommandSequences(
    TArray<ECFCoreInstallationCommands> install_commands,
    TArray<ECFCoreInstallationCommands> uninstall_commands,
    TArray<ECFCoreInstallationCommands> cleanup_commands) = 0;

  /**
   * Tries to run insallation strategy
   */
  virtual TFuture<FCFCoreError> Install(
    TSharedRef<FInstallModRequest> request) = 0;

  /**
   */
  virtual TFuture<FCFCoreError> Uninstall(
    TSharedRef<FInstallModRequest> request) = 0;

  /**
   */
  virtual TFuture<FCFCoreError> Cleanup(
    TSharedRef<FInstallModRequest> request) = 0;

  /**
   */
  virtual bool Cancel(TSharedRef<FInstallModRequest> request) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IInstallationStrategy)