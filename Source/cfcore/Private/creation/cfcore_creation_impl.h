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
#include "CoreMinimal.h"

#include <common/macros.h>

#include <creation/cfcore_creation.h>
#include <authentication/cfcore_authentication.h>

#include <api/cfcore_api.h>
#include <api/services/api_helper_service.h>
#include <services/user_context/user_context_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/http/http_file_service.h>

#include "services/create_mod_file_chunked_service.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class CFCoreCreationImpl : public ICFCoreCreation {
public:
  CFCoreCreationImpl(
    TSharedPtr<ICFCoreApi> cfcore_api,
    TSharedPtr<ICFCoreAuthentication> cfcore_authentication,
    TSharedPtr<ICreateModFileChunkedService> create_mod_file_chunked_service,
    TSharedPtr<IFileSystemService> file_system_service);
  virtual ~CFCoreCreationImpl() = default;

// ICFCoreCreation
public:
  virtual void CreateMod(const FCreateModRequest& create_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FCreateModDelegate delegate) override;
  virtual void CreateMod(const FCreateModRequest& create_mod_request,
                         const FString& avatar_image_filename,
                         FCreateModDelegate delegate) override;

  virtual void UpdateMod(int64 mod_id,
                         const FUpdateModRequest& update_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FUpdateModDelegate delegate) override;
  virtual void UpdateMod(int64 mod_id,
                         const FUpdateModRequest& update_mod_request,
                         const FString& avatar_image_filename,
                         FUpdateModDelegate delegate) override;

  virtual void CreateModFile(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) override;

  virtual void CreateModFile(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    const FString& local_filename_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) override;

  virtual void CreateCookedModFile(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) override;

  virtual void CreateCookedModFile(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    const FString& local_filename_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) override;

  virtual void CancelCreation(int64 creation_id,
                              FCancelCreationDelegate delegate) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CFCoreCreationImpl);

  FCFCoreError CreateUserNotAuthenticatedError();

private:
  TSharedPtr<ICFCoreApi> cfcore_api_;
  TSharedPtr<ICFCoreAuthentication> cfcore_authentication_;
  TSharedPtr<ICreateModFileChunkedService> create_mod_file_chunked_service_;
  TSharedPtr<IFileSystemService> file_system_service_;
};

} // namespace cfcore