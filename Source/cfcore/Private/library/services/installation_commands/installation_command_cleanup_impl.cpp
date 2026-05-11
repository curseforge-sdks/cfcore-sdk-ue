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
#include "installation_command_cleanup_impl.h"

#include <common/macros.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>
#include <services/filesystem/file_system_service.h>

#include "installation_command_utils_service.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
InstallationCommandCleanupImpl::InstallationCommandCleanupImpl(
  TSharedRef<IInstallationCommandUtilsService> utils_service,
  TSharedRef<IFileSystemService> file_system_service) :
  utils_service_(utils_service),
  file_sys_service_(file_system_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstallationCommandCleanupImpl::Run(
  FInstallModRequestPtr request) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  const FFile& latest_update_file = request->installedMod.latestUpdatedFile;

  CFCORE_LOG(Log,
             TEXT("Cleaning up (modId=%lld, fileId=%lld) ..."),
             request->installedMod.details.id,
             latest_update_file.id);

  utils_service_->TriggerProgressEvent(request,
                                       ELibraryProgressState::CleaningUp);

  auto out_dir = MakeShared<FString>(
    utils_service_->GetTempOutputDir(latest_update_file));

  auto zip_filename = MakeShared<FString>(
    utils_service_->GetUnderModsDir(latest_update_file.fileName));

  if (!request->state.downloaded_mod_file.IsEmpty()) {
    *zip_filename = request->state.downloaded_mod_file;
  }

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  futures->Add(file_sys_service_->DeleteDirectory(out_dir));
  futures->Add(file_sys_service_->DeleteFile(zip_filename));
  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([promise](TArray<bool> results) {
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool InstallationCommandCleanupImpl::Cancel(FInstallModRequestPtr request) {
  return true;
}
