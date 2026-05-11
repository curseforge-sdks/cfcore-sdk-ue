/*MIT License

Copyright (c) 2022 Overwolf Ltd.

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

#include <analytics/models/mod_install_params.h>
#include <library/models/installed_mod.h>
#include <library/models/install_mod_additional_params.h>

#include <library/services/mods_installation_service.h>

#include "install_mod_request_state.h"

namespace cfcore {
class IInstallationCommand;
class IInstallationStrategy;

// -----------------------------------------------------------------------------
struct FInstallModRequest {
  int64 modId;
  FInstalledMod installedMod;

  /**
   * Is the request being processed right now
   */
  bool inProgerss = false;

  int64 cancelId = 0;
  bool cancelRequest = false;

  TSharedPtr<IInstallationCommand> current_command;

  // The current installation strategy that is installing/uninstalling the mod
  TSharedPtr<IInstallationStrategy> strategy;

  IModsInstallationService::FModsInstallationProgressDelegate on_progress;

  // NOTE: This is a Ptr type because we don't want to have to remember to call
  // SetValue on it during unit tests (forgetting to call it will cause a crash
  // that is usually hard to track down).
  TUniquePtr<TPromise<IModsInstallationService::InstallResult>> promise;

  /**
   * Key-Value pairs for sending tracking parameters to server
   */
  FInstallModAdditionalParams additional_params;

  FInstallModRequestState state;

  FModInstallAnalyticsParams analytics_params;
};

typedef TSharedPtr<FInstallModRequest> FInstallModRequestPtr;
typedef TSharedRef<FInstallModRequest> FInstallModRequestRef;
}