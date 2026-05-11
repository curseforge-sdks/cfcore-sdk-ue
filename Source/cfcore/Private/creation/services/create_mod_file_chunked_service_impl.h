/*MIT License

Copyright (c) 2023 Overwolf Ltd.

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

#include <api/cfcore_api_creation.h>
#include <services/filesystem/file_system_service.h>
#include <services/lifetime/lifetime_service.h>
#include <api/models/creation/mod_file_chunked_info.h>

#include "create_mod_file_chunked_service.h"
#include "models/creation_in_progress.h"


namespace cfcore {

// -----------------------------------------------------------------------------
class CreateModFileChunkedServiceImpl : public ICreateModFileChunkedService {
public:
  CreateModFileChunkedServiceImpl(
    TSharedPtr<ILifetimeService> lifetime_service,
    TSharedPtr<ICFCoreApiCreation> api_creation,
    TSharedPtr<IFileSystemService> file_system_service);

// ICreateModFileChunkedService
public:
  virtual TFuture<CreationResult> Create(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    const FString& local_filename_to_upload,
    ICFCoreCreation::FCreateModFileRequestIdDelegate request_id_delegate,
    ICFCoreCreation::FFileTransferProgressDelegate progress_delegate) override;

  virtual TFuture<CreationResult> CreateCooked(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    const FString& local_filename_to_upload,
    ICFCoreCreation::FCreateModFileRequestIdDelegate request_id_delegate,
    ICFCoreCreation::FFileTransferProgressDelegate progress_delegate) override;

  virtual void CancelCreation(int64 creation_id) override;

private:
  void HandleShutdown();

  TSharedRef<FCreationInProgress> NewCreation(
    const FString& local_filename_to_upload,
    ICFCoreCreation::FFileTransferProgressDelegate& progress_delegate);

  TFuture<TOptional<FModFileChunkedInfo>> CreateModFileChunked(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    TSharedRef<FCreationInProgress> creation);

  TFuture<TOptional<FModFileChunkedInfo>> CreateCookedModFileChunked(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    TSharedRef<FCreationInProgress> creation);

  void ValidateModFileChunkedInfo(
    TSharedRef<FCreationInProgress> creation,
    TOptional<FModFileChunkedInfo> opt_info,
    const TOptional<FCFCoreApiResponseError>& opt_err,
    TSharedRef<TPromise<TOptional<FModFileChunkedInfo>>> promise);

  void StartUploadingChunks(
    TOptional<FModFileChunkedInfo> opt_info,
    TSharedRef<FCreationInProgress> creation);
  void CalcTotalChunks(TSharedRef<FCreationInProgress> creation);
  void UploadNextChunk(TSharedRef<FCreationInProgress> creation);

  void HandleFileSystemError(
    const TSharedRef<FCreationInProgress>& creation,
    EFileSystemError filesys_error);

  bool IsLastChunk(int32 current_chunk, int32 total_chunks);
  void HandleChunkProgress(const TSharedRef<FCreationInProgress>& creation,
                           const FFileTransferProgress& chunk_progress);
  void HandleUploadChunkError(
    TSharedRef<FCreationInProgress> creation,
    const FCFCoreApiResponseError& err);

  void HandleUploadChunkCompletion(TSharedRef<FCreationInProgress> creation);

  void FinalizeCreation(
    const TSharedRef<FCreationInProgress>& creation,
    TOptional<FCFCoreError> opt_err);

private:
  TSharedPtr<ILifetimeService> lifetime_service_;
  TSharedPtr<ICFCoreApiCreation> api_creation_;
  TSharedPtr<IFileSystemService> file_system_service_;

  FDelegateHandle on_shutdown_handle_;

  TMap<int64, TSharedRef<FCreationInProgress>> creations_in_progress_;
};

} // namespace cfcore