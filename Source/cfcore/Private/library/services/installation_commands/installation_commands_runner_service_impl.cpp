/* MIT License

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
SOFTWARE. */
#include "installation_commands_runner_service_impl.h"
#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/directories_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandsRunnerServiceImpl::InstallationCommandsRunnerServiceImpl(
  TSharedRef<InstallationCommandsMap> installation_commands_map) :
  installation_commands_map_(installation_commands_map) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandsRunnerServiceImpl::Run(
  TSharedRef<FInstallModRequest> request,
  TSharedRef<ModInstallationCommands> commands) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  PerformNextPostAction(request, commands, promise);

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandsRunnerServiceImpl::Cancel(
  TSharedRef<FInstallModRequest> request) {

  if (request->current_command.IsValid()) {
    request->current_command->Cancel(request);
  }

  return true;
}

// -----------------------------------------------------------------------------
void InstallationCommandsRunnerServiceImpl::PerformNextPostAction(
  TSharedRef<FInstallModRequest> request,
  TSharedRef<ModInstallationCommands> commands,
  TSharedRef<TPromise<FCFCoreError>> promise) {

  ECFCoreInstallationCommands command;
  if (!commands->Dequeue(command)) {
    promise->SetValue(FCFCoreError(ECFCoreErrorCodes::None));
    return;
  }

  if (!installation_commands_map_->Contains(command)) {
    ENUM_TO_STR(command_str, command);
    CFCORE_LOG(Error, TEXT("Couldn't find install command %s"), *command_str);
    promise->SetValue(FCFCoreError(ECFCoreErrorCodes::NotImplemented));
    return;
  }

  request->current_command = (*installation_commands_map_)[command];
  request->current_command->Run(request)
    .Next([this, request, commands, promise](FCFCoreError err) {

    check(IsInGameThread());

    if (err.isError) {
      promise->SetValue(err);
      return;
    }

    PerformNextPostAction(request, commands, promise);
 });
}