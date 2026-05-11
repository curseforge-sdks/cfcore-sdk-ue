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
#include "cfcore_creation_impl.h"
#include <services/logger/logger_macros.h>
#include "common/macros.h"

namespace cfcore {

#define ASSURE_USER_AUTHENTICATED() \
  if (!cfcore_authentication_->IsAuthenticated()) { \
    delegate.ExecuteIfBound({}, CreateUserNotAuthenticatedError()); \
    return; \
  }

}
using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreCreationImpl::CFCoreCreationImpl(
  TSharedPtr<ICFCoreApi> cfcore_api,
  TSharedPtr<ICFCoreAuthentication> cfcore_authentication,
  TSharedPtr<ICreateModFileChunkedService> create_mod_file_chunked_service,
  TSharedPtr<IFileSystemService> file_system_service) :
  cfcore_api_(cfcore_api),
  cfcore_authentication_(cfcore_authentication),
  create_mod_file_chunked_service_(create_mod_file_chunked_service),
  file_system_service_(file_system_service) {
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CreateMod(const FCreateModRequest& create_mod_request,
                                   TSharedPtr<TArray<uint8>> avatar_image_data,
                                   FCreateModDelegate delegate) {
  ASSURE_USER_AUTHENTICATED();

  cfcore_api_->Creation()->CreateMod(
    create_mod_request,
    avatar_image_data,
    ICFCoreApiCreation::FCreateModDelegate::CreateLambda(
      [=](const TOptional<FCFCoreMod>& opt_mod,
          const TOptional<FCFCoreApiResponseError>& opt_api_err) {
        TOptional<FCFCoreError> opt_err;

        if (opt_api_err.IsSet()) {
          FString desc = opt_api_err.GetValue().description;
          CFCORE_LOG(Error, TEXT("Failed to create mod (%s)"), *desc);
          opt_err = FCFCoreError(opt_api_err);
        }

        delegate.ExecuteIfBound(opt_mod, opt_err);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CreateMod(const FCreateModRequest& create_mod_request,
                                   const FString& avatar_image_filename,
                                   FCreateModDelegate delegate) {
  ASSURE_USER_AUTHENTICATED();

  auto f = file_system_service_->ReadBuffer(avatar_image_filename);
  f.Next([this, create_mod_request, delegate](ReadBufferRes result) {
    if (result.Key != EFileSystemError::None) {
      FCFCoreError err = FCFCoreError(ECFCoreErrorCodes::FileSystemError);
      err.description = TEXT("failed to read avatar from disk");
      delegate.ExecuteIfBound({}, err);
      return;
    }

    CreateMod(create_mod_request, result.Value, delegate);
  });
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::UpdateMod(int64 mod_id,
                                   const FUpdateModRequest& updated_mod_request,
                                   TSharedPtr<TArray<uint8>> avatar_image_data,
                                   FUpdateModDelegate delegate) {
  ASSURE_USER_AUTHENTICATED();

  cfcore_api_->Creation()->UpdateMod(
    mod_id,
    updated_mod_request,
    avatar_image_data,
    ICFCoreApiCreation::FUpdateModDelegate::CreateLambda(
      [=](const TOptional<FCFCoreMod>& opt_mod,
          const TOptional<FCFCoreApiResponseError>& opt_api_err) {
        TOptional<FCFCoreError> opt_err;

        if (opt_api_err.IsSet()) {
          FString desc = opt_api_err.GetValue().description;
          CFCORE_LOG(Error, TEXT("Failed to update mod (%s)"), *desc);
          opt_err = FCFCoreError(opt_api_err);
        }

        delegate.ExecuteIfBound(opt_mod, opt_err);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::UpdateMod(int64 mod_id,
                                   const FUpdateModRequest& update_mod_request,
                                   const FString& avatar_image_filename,
                                   FUpdateModDelegate delegate) {
  ASSURE_USER_AUTHENTICATED();

  auto f = file_system_service_->ReadBuffer(avatar_image_filename);
  f.Next([this, delegate, mod_id, update_mod_request](ReadBufferRes result) {
    if (result.Key != EFileSystemError::None) {
      FCFCoreError err = FCFCoreError(ECFCoreErrorCodes::FileSystemError);
      err.description = TEXT("failed to read avatar from disk");
      delegate.ExecuteIfBound({}, err);
      return;
    }

    UpdateMod(mod_id, update_mod_request, result.Value, delegate);
  });
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CreateModFile(
  int64 mod_id,
  const FCreateModFileRequest& create_mod_file_request,
  TSharedPtr<TArray<uint8>> file_data_to_upload,
  FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FCreateModFileDelegate delegate) {

  ASSURE_USER_AUTHENTICATED();

  if (create_mod_file_request.filename.IsEmpty()) {
    FCFCoreError error(ECFCoreErrorCodes::MissingParameter);
    error.description = TEXT("Missing filename parameter");
    delegate.ExecuteIfBound({}, error);
    return;
  }

  int64 file_id = cfcore_api_->Creation()->CreateModFile(
    mod_id,
    create_mod_file_request,
    file_data_to_upload,
    ICFCoreApiCreation::FFileTransferProgressDelegate::CreateLambda(
      [=](const FFileTransferProgress& progress) {
        file_transfer_progress_delegate.ExecuteIfBound(progress);
      }),
    ICFCoreApiCreation::FCreateModFileDelegate::CreateLambda(
      [=](const TOptional<FUploadedModFile>& opt_uploaded_file,
          const TOptional<FCFCoreApiResponseError>& opt_api_err) {
        TOptional<FCFCoreError> opt_err;

        if (opt_api_err.IsSet()) {
          FString desc = opt_api_err.GetValue().description;
          CFCORE_LOG(Error, TEXT("Failed to upload file (%s)"), *desc);
          opt_err = FCFCoreError(opt_api_err);
        }

        delegate.ExecuteIfBound(opt_uploaded_file, opt_err);
      })
  );

  create_mod_file_request_id_delegate.ExecuteIfBound(file_id);
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CreateModFile(
  int64 mod_id,
  const FCreateModFileRequest& create_mod_file_request,
  const FString& local_filename_to_upload,
  FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FCreateModFileDelegate delegate) {

  ASSURE_USER_AUTHENTICATED();

  FCreateModFileRequest request = create_mod_file_request;
  if (request.filename.IsEmpty()) {
    request.filename = FPaths::GetCleanFilename(local_filename_to_upload);
  }

  CFCORE_LOG(Log, TEXT("Starting to upload file: %s"), *request.filename);

  auto future = create_mod_file_chunked_service_->Create(
    mod_id,
    request,
    local_filename_to_upload,
    create_mod_file_request_id_delegate,
    file_transfer_progress_delegate);

  future.Next([delegate](ICreateModFileChunkedService::CreationResult res) {
    delegate.ExecuteIfBound(res.Value, res.Key);
  });
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CreateCookedModFile(
  int64 mod_id,
  int64 source_file_id,
  const FCreateCookedModFileRequest& create_cooked_mod_file_request,
  TSharedPtr<TArray<uint8>> file_data_to_upload,
  FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FCreateModFileDelegate delegate) {

  ASSURE_USER_AUTHENTICATED();

  int64 file_id = cfcore_api_->Creation()->CreateCookedModFile(
    mod_id,
    source_file_id,
    create_cooked_mod_file_request,
    file_data_to_upload,
    ICFCoreApiCreation::FFileTransferProgressDelegate::CreateLambda(
      [=](const FFileTransferProgress& progress) {
        file_transfer_progress_delegate.ExecuteIfBound(progress);
      }),
    ICFCoreApiCreation::FCreateModFileDelegate::CreateLambda(
      [=](const TOptional<FUploadedModFile>& opt_uploaded_file,
          const TOptional<FCFCoreApiResponseError>& opt_api_err) {
        TOptional<FCFCoreError> opt_err;

        if (opt_api_err.IsSet()) {
          FString desc = opt_api_err.GetValue().description;
          CFCORE_LOG(Error, TEXT("Failed to upload file (%s)"), *desc);
          opt_err = FCFCoreError(opt_api_err);
        }

        delegate.ExecuteIfBound(opt_uploaded_file, opt_err);
      })
  );

  create_mod_file_request_id_delegate.ExecuteIfBound(file_id);
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CreateCookedModFile(
  int64 mod_id,
  int64 source_file_id,
  const FCreateCookedModFileRequest& create_cooked_mod_file_request,
  const FString& local_filename_to_upload,
  FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FCreateModFileDelegate delegate) {

  ASSURE_USER_AUTHENTICATED();

  CFCORE_LOG(Log,
             TEXT("Starting to upload cooked file: %s"),
             *local_filename_to_upload);

  auto future = create_mod_file_chunked_service_->CreateCooked(
    mod_id,
    source_file_id,
    create_cooked_mod_file_request,
    local_filename_to_upload,
    create_mod_file_request_id_delegate,
    file_transfer_progress_delegate);

  future.Next([delegate](ICreateModFileChunkedService::CreationResult res) {
    delegate.ExecuteIfBound(res.Value, res.Key);
  });
}

// -----------------------------------------------------------------------------
void CFCoreCreationImpl::CancelCreation(int64 creation_id,
                                        FCancelCreationDelegate delegate) {
}

// -----------------------------------------------------------------------------
FCFCoreError CFCoreCreationImpl::CreateUserNotAuthenticatedError() {
  FCFCoreError error(ECFCoreErrorCodes::UserNotAuthenticated);
  error.description = TEXT("user is not authenticated or token is invalid");

  return error;
}
