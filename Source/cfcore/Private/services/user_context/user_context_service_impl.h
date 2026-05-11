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
#include "user_context_service.h"
#include <services/persistent_model.h>
#include <services/user_context/shared_context_service.h>

namespace cfcore {
class ISettingsService;
class IDirectoriesService;

// -----------------------------------------------------------------------------
class UserContextServiceImpl : public IUserContextService {
public:
  UserContextServiceImpl(
    TSharedPtr<ISettingsService> settings_service,
    TSharedPtr<IDirectoriesService> dirs_service,
    TSharedPtr<ISharedContextService> shared_context_service,
    TSharedPtr<PersistentModel<FUserContextInfo>> persistent_model);

// IUserContextService
public:
  // ---------------------------------------------------------------------------
  virtual FUserContextInfo UserContextInfo() override {
    return persistent_model_->GetModel();
  }

  // ---------------------------------------------------------------------------
  virtual FSharedContextInfo SharedContextInfo() override {
    return shared_context_service_->SharedContextInfo();
  }

public:
  virtual TFuture<FCFCoreError> Initialize() override;

  virtual TFuture<bool> SetAuthToken(const FString& auth_token) override;

  virtual TFuture<FCFCoreError> SetInstalledMods(
    const TArray<FUserModState>& user_mods) override;

  virtual TFuture<FCFCoreError> SetLibraryMigrationPerformed() override;

  virtual TFuture<FCFCoreError> SetSubscribedToInstalledMods() override;

private:
  void SetUserContextFile(const FString& user_context_id);
  TFuture<bool> AssureUserContextFileExists(const FString& user_context_id);
  FString GetUserContextFileName(const FString& user_context_id);

private:
  TSharedPtr<ISettingsService> settings_service_;
  TSharedPtr<IDirectoriesService> directories_service_;
  TSharedPtr<ISharedContextService> shared_context_service_;
  TSharedPtr<PersistentModel<FUserContextInfo>> persistent_model_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::PersistentModel<FUserContextInfo>)