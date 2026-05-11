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
#include "user_context_service_impl.h"
#include <common/constants.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include <services/settings_service.h>
#include <services/directories_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
UserContextServiceImpl::UserContextServiceImpl(
  TSharedPtr<ISettingsService> settings_service,
  TSharedPtr<IDirectoriesService> directories_service,
  TSharedPtr<ISharedContextService> shared_context_service,
  TSharedPtr<PersistentModel<FUserContextInfo>> persistent_model) :
  settings_service_(settings_service),
  directories_service_(directories_service),
  shared_context_service_(shared_context_service),
  persistent_model_(persistent_model) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> UserContextServiceImpl::Initialize() {

  const FCFCoreSettings& settings = settings_service_->GetSettings();
  FString user_context_id = settings.initOptions.userContextId;

  if (user_context_id.IsEmpty()) {
    user_context_id = "local";
  }

  SetUserContextFile(user_context_id);

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  futures->Add(AssureUserContextFileExists(user_context_id));
  futures->Add(shared_context_service_->Initialize());

  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([this, promise](TArray<bool> successes) {
    bool result = true;

    for (bool& success : successes) {
      result &= success;
    }

    ECFCoreErrorCodes code = ECFCoreErrorCodes::None;
    if (!result) {
      code = ECFCoreErrorCodes::FileSystemError;
    }

    promise->SetValue(FCFCoreError(code));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> UserContextServiceImpl::SetAuthToken(const FString& auth_token) {
  check(IsInGameThread());
  persistent_model_->GetModel().authToken = auth_token;
  return persistent_model_->SaveModelToDisk();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> UserContextServiceImpl::SetInstalledMods(
  const TArray<FUserModState>& user_mods) {

  check(IsInGameThread());
  FUserContextInfo& user_context_info = persistent_model_->GetModel();

  user_context_info.installedMods.Empty();

  for (const FUserModState& user_mod : user_mods) {
    user_context_info.installedMods.Add(user_mod);
  }

  return persistent_model_->SaveModelToDisk().Next([](bool success) {
    if (success) {
      return FCFCoreError();
    } else {
      return FCFCoreError(ECFCoreErrorCodes::FileSystemError);
    }
  });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> UserContextServiceImpl::SetLibraryMigrationPerformed() {
  check(IsInGameThread());
  return shared_context_service_->SetLibraryMigrationPerformed();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> UserContextServiceImpl::SetSubscribedToInstalledMods() {
  check(IsInGameThread());

  persistent_model_->GetModel().subscribedToInstalledMods = true;

  return persistent_model_->SaveModelToDisk().Next([](bool success) {
    if (success) {
      return FCFCoreError();
    } else {
      return FCFCoreError(ECFCoreErrorCodes::FileSystemError);
    }
  });
}

// -----------------------------------------------------------------------------
void UserContextServiceImpl::SetUserContextFile(
  const FString& user_context_id) {

  FString local_file = GetUserContextFileName(user_context_id);

  persistent_model_->Initialize(local_file);
}

// -----------------------------------------------------------------------------
TFuture<bool> UserContextServiceImpl::AssureUserContextFileExists(
  const FString& user_context_id) {

  FString local_file = GetUserContextFileName(user_context_id);

  TFuture<bool> future;
  if (FPaths::FileExists(local_file)) {
    future = persistent_model_->LoadModelFromDisk();
  } else {
    persistent_model_->GetModel() = FUserContextInfo();
    future = persistent_model_->SaveModelToDisk();
  }

  return future;
}

// -----------------------------------------------------------------------------
FString UserContextServiceImpl::GetUserContextFileName(
  const FString& user_context_id) {

  FString user_data_dir = directories_service_->GetUserDataDirectory();
  int64 game_id = settings_service_->GetSettings().gameId;

  FString local_file = FPaths::Combine(user_data_dir,
                                       FString::FromInt(game_id),
                                       user_context_id,
                                       kUserInfoFile);

  return local_file;
}