/* MIT License

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
SOFTWARE. */
#include "installation_command_unzip_impl.h"

#include <common/macros.h>

#include "installation_command_utils_service.h"
#include <services/logger/logger_macros.h>
#include <services/filesystem/file_system_service.h>
#include <utils/compression/compression_service.h>

#include <services/buffer/buffer_impl.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandUnzipImpl::InstallationCommandUnzipImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<IFileSystemService> filesys_service,
  TSharedRef<ICompressionService> zip_service) :
  utils_service_(utils_service),
  file_system_service_(filesys_service),
  zip_service_(zip_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandUnzipImpl::Run(
  FInstallModRequestPtr request) {

  int64 id = request->modId;
  const FFile& file = request->installedMod.latestUpdatedFile;

  FString out_dir = utils_service_->GetTempOutputDir(file);

  CFCORE_LOG(Log, TEXT("Unzipping mod=%lld to %s ..."), id, *out_dir);

  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::PendingUnzipping);

  const FString zip_filename = request->state.downloaded_mod_file;
  return PerformUnzip(request, zip_filename, out_dir);
}

// -----------------------------------------------------------------------------
bool InstallationCommandUnzipImpl::Cancel(FInstallModRequestPtr request) {
  return true; // TODO(twolf): zip_service_->Cancel(...
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandUnzipImpl::PerformUnzip(
  FInstallModRequestPtr request,
  const FString& filename,
  const FString& output_dir) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = zip_service_->Unzip(
    filename,
    output_dir,
    ICompressionService::FProgressDelegate::CreateLambda(
      [this, request](const FCompressionProgress& progress) {
        HandleCompressionProgress(progress, request);
      }));

  future.Next([this, request, filename, promise](ECompressionError err) {
    if (err != ECompressionError::None) {
      HandleCompressionError(err, request, promise);
      return;
    }

    TryDeletingZipFile(filename, promise);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
void InstallationCommandUnzipImpl::TryDeletingZipFile(
  const FString& zip_file,
  TSharedRef<TPromise<FCFCoreError>> promise) {

  auto file = MakeShared<FString>(zip_file);
  file_system_service_->DeleteFile(file).Next([file, promise](bool success) {
    if (!success) {
      CFCORE_LOG(Warning, TEXT("Failed deleting zip %s"), **file);
    } else {
      CFCORE_LOG(Log, TEXT("Successfully deleted zip %s"), **file);
    }

    promise->SetValue(FCFCoreError());
  });
}

// -----------------------------------------------------------------------------
void InstallationCommandUnzipImpl::HandleFileSystemError(
  EFileSystemError filesys_err,
  FInstallModRequestPtr request,
  TSharedRef<TPromise<FCFCoreError>> promise) {

  FCFCoreError err(ECFCoreErrorCodes::FailedToUnzip);
  ENUM_TO_STR(err_str, filesys_err);

  err.description = FString::Format(TEXT("Failed unzip mod {0} ({1})"), {
    request->modId,
    *err_str
  });

  CFCORE_LOG(Error, TEXT("%s"), *err.description);

  promise->SetValue(err);
}

// -----------------------------------------------------------------------------
void InstallationCommandUnzipImpl::HandleCompressionError(
  ECompressionError compression_error,
  FInstallModRequestPtr request,
  TSharedRef<TPromise<FCFCoreError>> promise) {

  FCFCoreError err(ECFCoreErrorCodes::FailedToUnzip);

  ENUM_TO_STR(err_str, compression_error);

  err.description = FString::Format(TEXT("Failed unzip mod {0} ({1})"), {
    request->modId,
    *err_str
  });

  CFCORE_LOG(Error, TEXT("%s"), *err.description);

  promise->SetValue(err);
}

// -----------------------------------------------------------------------------
void InstallationCommandUnzipImpl::HandleCompressionProgress(
  const FCompressionProgress& progress,
  const FInstallModRequestPtr& request) {

  FLibraryProgressDataTransfer transfer = { 0 };
  transfer.progress = progress.progress;
  transfer.filename = progress.file;

  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::Unzipping,
                                       &transfer);
}
