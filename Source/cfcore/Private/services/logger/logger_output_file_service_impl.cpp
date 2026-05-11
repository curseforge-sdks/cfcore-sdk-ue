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
#include "logger_output_file_service_impl.h"
#include <common/cfcore_log.h>
#include <common/future_utils.h>

#include <services/settings_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/directories_service.h>
#include <services/filesystem/persistent_file/persistent_log_file.h>

#include <Misc/Paths.h>

namespace cfcore {
const TCHAR kLogFilePrefix[] = TEXT("game_");
const TCHAR kLogMessageExtendsPrevious[] =
  TEXT("---------- This log extends the previous one ----------");
const int32 kMaxHistory = 30;
const int32 kMinLogFileSizeInMB = 2;
const int32 kMaxLogFileSizeInMB = 10;

const int32 kMaxRolloutFailures = 10;

#define WAIT_GET(future, res_variable) \
  future.Wait(); \
  auto res_variable = future.Get();
}

using namespace cfcore;

// -----------------------------------------------------------------------------
LoggerOutputFileServiceImpl::LoggerOutputFileServiceImpl(
  TSharedPtr<ISettingsService> settings_service,
  TSharedPtr<IFileSystemService> filesystem_service,
  TSharedPtr<IDirectoriesService> directories_service) :
  filesystem_service_(filesystem_service),
  directories_service_(directories_service),
  rollout_failures_count_(0) {

  settings_service->OnSettingsUpdated().AddLambda([this, settings_service]() {
    InitializeSettings(settings_service);
  });

  InitializeSettings(settings_service);
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::WriteLogLine(const FString& message) {
  if (!settings_.enabled) {
    return true;
  }

  // Must be called on the logger thread
  // TODO(twolf): Consider actually verifying it is the logger thread and not
  // just the non-game thread
  check(!IsInGameThread());

  if (DidReachMaxRolloutFailures() || !CheckForRolloutAndPerform()) {
    rollout_failures_count_++;
    return false;
  }

  if (!current_output_->IsValid()) {
    return false;
  }

  current_output_->WriteLine(message);
  return true;
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::GetClosedLogFiles(
  OUT TSharedRef<TArray<FString>> out_closed_log_files) {

  if (!GetExistingLogFiles(out_closed_log_files)) {
    return false;
  }

  out_closed_log_files->Sort(
    [this](const FString& first, const FString& second) {

    return IsNewerLogFilename(first, second);
  });

  RemoveOpenLogFileIfExists(out_closed_log_files);
  return true;
}

// -----------------------------------------------------------------------------
void LoggerOutputFileServiceImpl::InitializeSettings(
  const TSharedPtr<ISettingsService>& settings) {

  game_id_ = settings->GetSettings().gameId;
  settings_ = settings->GetSettings().logger;

  AdjustValidSettings();
}

// -----------------------------------------------------------------------------
void LoggerOutputFileServiceImpl::AdjustValidSettings() {
  settings_.history = FMath::Min(kMaxHistory, settings_.history);
  settings_.history = FMath::Max(0, settings_.history);

  settings_.maxSizeInMB =
    FMath::Min(kMaxLogFileSizeInMB, settings_.maxSizeInMB);
  settings_.maxSizeInMB =
    FMath::Max(kMinLogFileSizeInMB, settings_.maxSizeInMB);
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::DidReachMaxRolloutFailures() {
  if (rollout_failures_count_ >= kMaxRolloutFailures) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::CheckForRolloutAndPerform() {
  if (!current_output_.IsValid()) {
    if (!RolloutNewLogFile()) {
      return false;
    }
  }

  if (DidFilePassMaxSize()) {
    if (!RolloutNewLogFile()) {
      return false;
    }

    current_output_->WriteLine(kLogMessageExtendsPrevious);
  }

  return true;
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::IsNewerLogFilename(
  const FString& first,
  const FString& second) const {

  return first > second;
}

// -----------------------------------------------------------------------------
void LoggerOutputFileServiceImpl::RemoveOpenLogFileIfExists(
  IN OUT TSharedRef<TArray<FString>> log_files) {

  if (!current_output_.IsValid()) {
    return;
  }

  log_files->RemoveAt(0);
  return;
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::DidFilePassMaxSize() {
  int64 total_size_mb = current_output_->TotalSizeInBytes() / (1024 * 1024);

  return (total_size_mb >= settings_.maxSizeInMB);
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::RolloutNewLogFile() {
  current_output_.Reset();

  CleanOldLogFiles();

  FString filename = FString::Format(TEXT("{0}{1}.log"), {
    *GetLogFilePrefix(),
    *FDateTime::UtcNow().ToString(TEXT("%Y.%m.%d-%H.%M.%S.%s"))
  });

  const FString& logs_dir = directories_service_->GetUserLogsDirectory();
  filename = FPaths::Combine(logs_dir, filename);

  current_output_ = filesystem_service_->CreatePersistentLogFile(filename);
  return current_output_->IsValid();
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::CleanOldLogFiles() {
  auto log_files = MakeShared<TArray<FString>>();

  if (!GetExistingLogFiles(log_files)) {
    return false;
  }

  int32 max_files = settings_.history;
  int32 delta = log_files->Num() - max_files;

  if (delta <= 0) {
    return true;
  }

  log_files->Sort([](const FString& first, const FString& second) {
    return first > second;
  });

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  while (delta > 0) {
    int32 oldest_index = log_files->Num() - 1;
    auto file_to_delete = MakeShared<FString>((*log_files)[oldest_index]);
    futures->Add(filesystem_service_->DeleteFile(file_to_delete));

    log_files->RemoveAt(oldest_index);
    delta = log_files->Num() - max_files;
  }

  auto future = FutureUtils::Parallel<bool>(futures);
  WAIT_GET(future, errors);

  for (const bool& err : errors) {
    if (!err) {
      return false;
    }
  }

  return true;
}

// -----------------------------------------------------------------------------
bool LoggerOutputFileServiceImpl::GetExistingLogFiles(
  OUT TSharedRef<TArray<FString>> log_files) {

  auto logs_dir =
    MakeShared<FString>(directories_service_->GetUserLogsDirectory());
  auto file_prefix = MakeShared<FString>(GetLogFilePrefix());

  auto future = filesystem_service_->GetFilesInDirectory(logs_dir,
                                                         file_prefix,
                                                         log_files);

  WAIT_GET(future, res);
  return res;
}

// -----------------------------------------------------------------------------
FString LoggerOutputFileServiceImpl::GetLogFilePrefix() {
  FString file_prefix = FString::Format(TEXT("{0}{1}_"), {
    kLogFilePrefix,
    game_id_
  });

  return file_prefix;
}
