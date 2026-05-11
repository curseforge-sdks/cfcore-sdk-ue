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
#include "logger_service_impl.h"
#include "logger_macros.h"

#include <common/future_utils.h>

//#include <services/settings_service.h>
#include <services/lifetime/lifetime_service.h>
#include <services/thread_pools_service.h>
#include <services/logger/logger_output_service.h>
#include <services/logger/startup_info_logging/startup_info_logging.h>
#include <utils/compression/compression_service.h>

#include <Async/Async.h>
#include <Misc/Paths.h>
#include <Misc/FeedbackContext.h>

namespace cfcore {
const uint32 kThreadIntervalInMilliseconds = 500;
const uint32 kCollectLogsTimeoutInSeconds = 60;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
LoggerServiceImpl::LoggerServiceImpl(
  TSharedPtr<ILifetimeService> lifetime_service,
  TSharedPtr<IThreadPoolsService> thread_pools_service,
  TSharedPtr<ILoggerOutputService> logger_output_service,
  TSharedPtr<IStartupInfoLogging> startup_info_logging,
  TSharedPtr<ICompressionService> zip_service) :
  lifetime_service_(lifetime_service),
  thread_pool_(thread_pools_service->Logger()),
  logger_output_service_(logger_output_service),
  startup_info_logging_(startup_info_logging),
  zip_service_(zip_service),
  event_shutdown_(EEventMode::ManualReset),
  is_thread_running_(false),
  collected_logs_(false) {

  on_shutdown_handle_ = lifetime_service_->OnShutdown().AddLambda([this]() {
    event_shutdown_->Trigger();
  });
}

// -----------------------------------------------------------------------------
TFuture<bool> LoggerServiceImpl::Initialize() {
  if (!is_thread_running_) {
    AsyncPool(*thread_pool_, [this]() {
      is_thread_running_ = true;
      LoggerThreadLoop();
      is_thread_running_ = false;
    });
  }

  return MakeFulfilledPromise<bool>(true).GetFuture();
}

// -----------------------------------------------------------------------------
void LoggerServiceImpl::Log(const ANSICHAR* file,
                            int32 line,
                            ELogVerbosity::Type verbosity,
                            const TCHAR* Fmt,
                            ...) {
  if (!Fmt) {
    return;
  }

  // This will use Fmt and ... to fill in Buffer
  CFCORE_GROWABLE_LOGF({
    // Log to console output - we use a literal string here because without it,
    // we get exceptions when trying to build on Linux
    if (GWarn){
      GWarn->Log(TEXT("LogCFCore"), verbosity, Buffer);
    }

    FString message;
    FormatLogMessage(file, line, verbosity, Buffer, message);

    FLoggerTask logger_task = CreateLogTask(message);
    logs_queue_.Enqueue(MoveTemp(logger_task));
  });
}

// -----------------------------------------------------------------------------
void LoggerServiceImpl::ZipUserLogs(const FLogsCollectionParamsRef params) {
  if (collected_logs_) {
    auto code = ECFCoreErrorCodes::AlreadySentUserLogs;
    params->promise->SetValue(FCFCoreError(code));
    return;
  }

  collected_logs_ = true;
  FLoggerTask logger_task = CreateZipUserLogsTask(params);
  logs_queue_.Enqueue(MoveTemp(logger_task));
}

// -----------------------------------------------------------------------------
void LoggerServiceImpl::FormatLogMessage(const ANSICHAR* file,
                                         int32 line,
                                         ELogVerbosity::Type verbosity,
                                         TCHAR* buffer,
                                         OUT FString& message) {
  FString datetime_str = FDateTime::UtcNow().ToString(TEXT("%Y.%m.%d-%H.%M.%S:%s"));
  FString timestamp = FString::Printf(
    TEXT("[%s][%3llu]"),
    *datetime_str,
    GFrameCounter % 1000);

  FString filename = FPaths::GetCleanFilename(file);
  FString location = FString::Printf(TEXT("%s:%d"),
                                     *filename,
                                     line);

  message = FString::Printf(TEXT("%s[%s]\t\t%s\t[%s]"),
                            *timestamp,
                            ToString(verbosity),
                            buffer,
                            *location);
}

// -----------------------------------------------------------------------------
FLoggerTask LoggerServiceImpl::CreateLogTask(const FString& message) {
  FLoggerTask logger_task;
  logger_task.task_type = ELoggerTaskType::Log;
  logger_task.msg = message;
  return logger_task;
}

// -----------------------------------------------------------------------------
FLoggerTask LoggerServiceImpl::CreateZipUserLogsTask(
  FLogsCollectionParamsRef params) {

  FLoggerTask logger_task;
  logger_task.task_type = ELoggerTaskType::ZipUserLogs;
  logger_task.logs_collection_params = params;
  return logger_task;
}

// -----------------------------------------------------------------------------
void LoggerServiceImpl::LoggerThreadLoop() {
  startup_info_logging_->Run();

  while (!event_shutdown_->Wait(kThreadIntervalInMilliseconds)) {
    ProcessQueue();
  }
}

// -----------------------------------------------------------------------------
void LoggerServiceImpl::ProcessQueue() {
  FLoggerTask current_task;

  while (logs_queue_.Dequeue(current_task)) {
    switch(current_task.task_type) {
      case ELoggerTaskType::Log:
        logger_output_service_->WriteLogLine(MoveTemp(current_task.msg));
        break;
      case ELoggerTaskType::ZipUserLogs:
        if (!current_task.logs_collection_params.IsValid()) {
          break;
        }

        PerformZipUserLogs(current_task.logs_collection_params.ToSharedRef());
        break;
    }
  }
}

// -----------------------------------------------------------------------------
void LoggerServiceImpl::PerformZipUserLogs(FLogsCollectionParamsRef params) {

  auto logs = MakeShared<TArray<FString>>();
  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
    [this, logs]() { return GetClosedLogFiles(logs);},
    [this, logs, params]() {
      params->log_files = *logs;
      return ZipLogFiles(params);
    }
  });

  auto future = FutureUtils::SequenceAll(functions, &FCFCoreError::isError);

  future.WaitFor(FTimespan::FromSeconds(kCollectLogsTimeoutInSeconds));

  FCFCoreError err;
  if (future.IsReady()) {
    err = future.Get();
  }

  if (err.isError) {
    CFCORE_LOG(Warning, TEXT("Failed to collect logs"));
  }

  // TODO(apliscov): Consider setting value on main thread, but beware deadlocks
  params->promise->SetValue(err);
  return;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> LoggerServiceImpl::GetClosedLogFiles(
  OUT TSharedRef<TArray<FString>> out_log_files) {

  bool success = logger_output_service_->GetClosedLogFiles(out_log_files);

  FCFCoreError error;
  if (!success) {
    error = FCFCoreError(ECFCoreErrorCodes::FileSystemError);
  }

  return MakeFulfilledPromise<FCFCoreError>(error).GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> LoggerServiceImpl::ZipLogFiles(
  const FLogsCollectionParamsRef params) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  TSharedRef<TArray<FString>> files_to_zip =
    MakeShared<TArray<FString>>(params->log_files);
  const FString& zip_file = params->zip_file;

  zip_service_->Zip(files_to_zip, zip_file, ICompressionService::FProgressDelegate())
    .Next([this, zip_file, promise](ECompressionError err) {

    FCFCoreError cfcore_err;
    if (err != ECompressionError::None) {
      CFCORE_LOG(Warning,
        TEXT("Failed to compress user logs (error code %d)"),
        static_cast<int>(err));
      cfcore_err = FCFCoreError(ECFCoreErrorCodes::FailedToZip);
    }

    promise->SetValue(cfcore_err);
  });

  return promise->GetFuture();
}
