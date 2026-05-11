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
#include <api/common/cfcore_api_response_error.h>

#include <api/models/mod.h>
#include <api/models/creation/create_mod_request.h>
#include <api/models/creation/create_cooked_mod_file_request.h>
#include <api/models/creation/update_mod_request.h>
#include <api/models/creation/create_mod_file_request.h>
#include <api/models/creation/uploaded_mod_file.h>
#include <api/models/creation/mod_file_chunked_info.h>
#include <api/models/creation/mod_file_chunk_metadata.h>
#include <api/models/file_transfer_progress.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// These APIs also require being authorized
class ICFCoreApiCreation {
public:
  DECLARE_DELEGATE_TwoParams(FCreateModDelegate, const TOptional<FCFCoreMod>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FUpdateModDelegate, const TOptional<FCFCoreMod>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FCreateModFileDelegate, const TOptional<FUploadedModFile>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FCreateModFileChunkedDelegate, const TOptional<FModFileChunkedInfo>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_OneParam(FFileTransferProgressDelegate, const FFileTransferProgress&);
  DECLARE_DELEGATE_TwoParams(FUploadModFileChunkDelegate, const bool&, const TOptional<FCFCoreApiResponseError>&);

 public:
  virtual ~ICFCoreApiCreation() = default;

public:
  virtual void CreateMod(FCreateModRequest create_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FCreateModDelegate delegate) = 0;

  virtual void UpdateMod(int64 mod_id,
                         FUpdateModRequest update_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FUpdateModDelegate delegate) = 0;

  /**
   * Returns an id which you can pass to abort the upload
   */
  virtual int64 CreateModFile(
    int64 mod_id,
    FCreateModFileRequest create_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) = 0;

  /**
   * Create a mod file with chunked uploading - use |UploadModFileChunk| with
   * the parameters you recieve from this function.
   *
   * Allows uploading large files from disk (e.g. larger than 1GB)
   */
  virtual void CreateModFileChunked(
    int64 mod_id,
    FCreateModFileRequest create_mod_file_request,
    FCreateModFileChunkedDelegate delegate) = 0;

  /**
   * Used in correspondance to |CreateModFileChunked| and
   * |CreateCookedModFileChunked|.
   * We recommend using chunks no larger than 100MB in size.
   *
   * Returns an id which you can pass to abort the upload
   */
  virtual int64 UploadModFileChunk(
    const FModFileChunkMetadata& chunk_metadata,
    TSharedPtr<TArray<uint8>> chunk_data,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FUploadModFileChunkDelegate delegate) = 0;

  /**
   * Used for manual cooking - you first use |CreateModFile| to upload the
   * source file, then you can associate cooked files with the sourceFileId
   */
  virtual int64 CreateCookedModFile(
    int64 mod_id,
    int64 source_file_id,
    FCreateCookedModFileRequest create_cooked_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) = 0;

  /**
   * Create a cooked mod file with chunked uploading - use |UploadModFileChunk|
   * with the parameters you recieve from this function.
   *
   * Allows uploading large files from disk (e.g. larger than 1GB)
   */
  virtual void CreateCookedModFileChunked(
    int64 mod_id,
    int64 source_file_id,
    FCreateCookedModFileRequest create_cooked_mod_file_request,
    FCreateModFileChunkedDelegate delegate) = 0;

  /**
   * Cancels a call to CreateModFile, CreateCoookedModFile, UploadModFileChunk
   * etc...
   */
  virtual void CancelCreation(int64 cancel_id) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreApiCreation)