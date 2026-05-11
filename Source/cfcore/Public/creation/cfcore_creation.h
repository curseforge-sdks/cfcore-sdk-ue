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

#include <common/cfcore_error.h>

#include <api/models/mod.h>
#include <api/models/file_transfer_progress.h>
#include <api/models/creation/uploaded_mod_file.h>
#include <api/models/creation/create_mod_request.h>
#include <api/models/creation/update_mod_request.h>
#include <api/models/creation/create_mod_file_request.h>
#include <api/models/creation/create_cooked_mod_file_request.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ICFCoreCreation {
public:
  DECLARE_DELEGATE_TwoParams(FCreateModDelegate, const TOptional<FCFCoreMod>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_TwoParams(FUpdateModDelegate, const TOptional<FCFCoreMod>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FCreateModFileRequestIdDelegate, int64);
  DECLARE_DELEGATE_TwoParams(FCreateModFileDelegate, const TOptional<FUploadedModFile>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FFileTransferProgressDelegate, const FFileTransferProgress&);
  DECLARE_DELEGATE_OneParam(FCancelCreationDelegate, const TOptional<FCFCoreError>&);

public:
  virtual ~ICFCoreCreation() = default;

public:
  virtual void CreateMod(const FCreateModRequest& create_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FCreateModDelegate delegate) = 0;
  virtual void CreateMod(const FCreateModRequest& create_mod_request,
                         const FString& avatar_image_filename,
                         FCreateModDelegate delegate) = 0;

  virtual void UpdateMod(int64 mod_id,
                         const FUpdateModRequest& update_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FUpdateModDelegate delegate) = 0;
  virtual void UpdateMod(int64 mod_id,
                         const FUpdateModRequest& update_mod_request,
                         const FString& avatar_image_filename,
                         FUpdateModDelegate delegate) = 0;

  /**
   * Use only for smaller files - the limit is 2^31 - 1 bytes ~2GB
   */
  virtual void CreateModFile(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) = 0;

  /**
   * Returns an upload id with which you can cancel the upload
   */
  virtual void CreateModFile(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    const FString& local_filename_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) = 0;

  virtual void CreateCookedModFile(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) = 0;

  virtual void CreateCookedModFile(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    const FString& local_filename_to_upload,
    FCreateModFileRequestIdDelegate create_mod_file_request_id_delegate,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) = 0;

  /**
   * Abort mod file creation (e.g. CreateModFile, CreateCookedModFile)
   */
  virtual void CancelCreation(int64 creation_id,
                              FCancelCreationDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreCreation)