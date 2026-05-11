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

#include <services/logger/logger_macros.h>
#include <services/lifetime/lifetime_service.h>
#include <services/filesystem/file_system_service.h>

#include <Json.h>
#include <JsonUtilities.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// Helper class for reading/writing a model to the file system.
//
// auto persistent_model = MakeShared<PersistentModel<FUserContextInfo>>(...);
// persistent_model->Initialize(local_filename);
// persistent_model->LoadModelFromDisk().Next([]() {
//   auto model = persistent_model->GetModel();
// });
template <typename InStructType>
class PersistentModel
  : public TSharedFromThis<PersistentModel<InStructType>> {
public:
  // ---------------------------------------------------------------------------
  PersistentModel(
    TSharedPtr<ILifetimeService> lifetime_service,
    TSharedPtr<IFileSystemService> file_system_service) :
    lifetime_service_(lifetime_service),
    file_system_service_(file_system_service) {
  }

public:
  // ---------------------------------------------------------------------------
  InStructType& GetModel() {
    return model_;
  }

public:
  // ---------------------------------------------------------------------------
  void Initialize(FString local_model_file) {
    local_model_file_ = local_model_file;
  }

  // ---------------------------------------------------------------------------
  TFuture<bool> LoadModelFromDisk() {
    TSharedRef<PersistentModel<InStructType>> self = this->AsShared();
    auto future =
      self->file_system_service_->ReadText(self->local_model_file_);
    return future.Next([self](TTuple<EFileSystemError, FString> result) {
      // NOTE(twolf): This is to protect us from garbage collection (of model_)
      // in case we return while shutting down
      if (self->lifetime_service_->IsShuttingDown()) {
        return false;
      }

      self->model_ = {};

      FString json_data = result.Value;

      if ((result.Key != EFileSystemError::None) || (json_data.IsEmpty())) {
        FString model_name = InStructType::StaticStruct()->GetName();
        CFCORE_LOG(Log, TEXT("%s not loaded from disk"), *model_name);
        return false;
      }

      bool success = FJsonObjectConverter::JsonObjectStringToUStruct(
        json_data,
        &self->model_,
        0,
        0);

      if (!success) {
        FString model_name = InStructType::StaticStruct()->GetName();
        CFCORE_LOG(Warning, TEXT("Failed to deserialize %s"), *model_name);
      }

      return true;
    });
  }

  // ---------------------------------------------------------------------------
  TFuture<bool> SaveModelToDisk() {
    TSharedRef<PersistentModel<InStructType>> self = this->AsShared();
    FString json;
    bool result = FJsonObjectConverter::UStructToJsonObjectString(
      self->model_,
      json,
      0/*CheckFlags*/,
      0/*SkipFlags*/,
      0/*Indent*/,
      nullptr/*ExportCb*/,
      false/*bPrettyPrint*/);

    if (!result) {
      TPromise<bool> promise(nullptr);

      FString model_name = InStructType::StaticStruct()->GetName();
      CFCORE_LOG(Error, TEXT("Failed to serialize %s"), *model_name);
      promise.SetValue(false);
      return promise.GetFuture();
    }

    auto future = self->file_system_service_->WriteText(self->local_model_file_,
                                                        json);
    return future.Next([self](EFileSystemError result) {
      if (result != EFileSystemError::None) {
        FString model_name = InStructType::StaticStruct()->GetName();
        CFCORE_LOG(Warning, TEXT("Failed to save %s to disk"), *model_name);
        return false;
      }

      return true;
    });
  }

private:
  TSharedPtr<ILifetimeService> lifetime_service_;
  TSharedPtr<IFileSystemService> file_system_service_;

  InStructType model_;
  FString local_model_file_;
};

} // namespace cfcore
