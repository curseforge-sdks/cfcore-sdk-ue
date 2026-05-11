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
#include "shared_context_service_impl.h"
#include <common/constants.h>
#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/settings_service.h>
#include <services/directories_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
SharedContextServiceImpl::SharedContextServiceImpl(
  TSharedPtr<ISettingsService> settings_service,
  TSharedPtr<IDirectoriesService> directories_service,
  TSharedPtr<PersistentModel<FSharedContextInfo>> persistent_model) :
  settings_service_(settings_service),
  directories_service_(directories_service),
  persistent_model_(persistent_model) {
}

// -----------------------------------------------------------------------------
TFuture<bool> SharedContextServiceImpl::Initialize() {
  SetLocalSharedContextFile();

  SHAREABLE_PROMISE(bool, promise);

  TFuture<bool> future;
  FString local_file = GetLocalSharedContextFileName();

  if (FPaths::FileExists(local_file)) {
    future = persistent_model_->LoadModelFromDisk();
  } else {
    future = persistent_model_->SaveModelToDisk();
  }

  future.Next([this, promise](bool) {
    return InnerInitialize(promise);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> SharedContextServiceImpl::SetLibraryMigrationPerformed() {
  FSharedContextInfo& shared_context_info = persistent_model_->GetModel();

  if (shared_context_info.performedLibraryMigration) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  shared_context_info.performedLibraryMigration = true;

  SHAREABLE_PROMISE(FCFCoreError, promise);
  persistent_model_->SaveModelToDisk().Next([promise](bool success) {
    if (success) {
      promise->SetValue(FCFCoreError());
      return;
    }

    CFCORE_LOG(Warning, TEXT("Failed to save shared context info to disk"));
    auto code = ECFCoreErrorCodes::FailedToSaveModsStateToDisk;
    promise->SetValue(FCFCoreError(code));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void SharedContextServiceImpl::SetLocalSharedContextFile() {
  FString local_file = GetLocalSharedContextFileName();

  persistent_model_->Initialize(local_file);
}

// -----------------------------------------------------------------------------
void SharedContextServiceImpl::InnerInitialize(
  TSharedRef<TPromise<bool>> promise) {

  FSharedContextInfo& shared_context = persistent_model_->GetModel();
  if (AssureMachineUniqueId(shared_context)) {
    promise->SetValue(true);
    return;
  }

  persistent_model_->SaveModelToDisk().Next([promise](bool success) {
    promise->SetValue(success);
  });
}

// -----------------------------------------------------------------------------
// By saving this value - we handle devices where FPlatformMisc::GetLoginId is
// not persistent accross game sessions
bool SharedContextServiceImpl::AssureMachineUniqueId(
  FSharedContextInfo& shared_context) {

  if (!shared_context.machineId.IsEmpty()) {
    return true;
  }

  shared_context.machineId = FPlatformMisc::GetLoginId();

  return false;
}

// -----------------------------------------------------------------------------
FString SharedContextServiceImpl::GetLocalSharedContextFileName() {
  FString user_data_dir = directories_service_->GetUserDataDirectory();
  int64 game_id = settings_service_->GetSettings().gameId;

  FString local_file = FPaths::Combine(user_data_dir,
    FString::FromInt(game_id),
    kSharedInfoFile);

  return local_file;
}