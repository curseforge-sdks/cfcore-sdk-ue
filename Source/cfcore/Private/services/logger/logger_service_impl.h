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
#include <Containers/Queue.h>
#include <Misc/QueuedThreadPool.h>

#include "models/logger_task.h"
#include "common/cfcore_error.h"

#include "logger_service.h"

namespace cfcore {
class ILifetimeService;
class IThreadPoolsService;
class ILoggerOutputService;
class IStartupInfoLogging;
class ICompressionService;

// -----------------------------------------------------------------------------
class LoggerServiceImpl : public ILoggerService {
public:
  LoggerServiceImpl(
    TSharedPtr<ILifetimeService> lifetime_service,
    TSharedPtr<IThreadPoolsService> thread_pools_service,
    TSharedPtr<ILoggerOutputService> logger_output_service,
    TSharedPtr<IStartupInfoLogging> startup_info_logging,
    TSharedPtr<ICompressionService> zip_service);

// ILoggerService
public:
  virtual TFuture<bool> Initialize() override;
  virtual void Log(const ANSICHAR* file,
                   int32 line,
                   ELogVerbosity::Type verbosity,
                   const TCHAR* Fmt,
                   ...) override;

  virtual void ZipUserLogs(const FLogsCollectionParamsRef params) override;

private:
  void FormatLogMessage(const ANSICHAR* file,
                        int32 line,
                        ELogVerbosity::Type verbosity,
                        TCHAR* buffer,
                        OUT FString& msg);

  FLoggerTask CreateLogTask(const FString& message);
  FLoggerTask CreateZipUserLogsTask(FLogsCollectionParamsRef params);
  void LoggerThreadLoop();
  void ProcessQueue();
  void PerformZipUserLogs(FLogsCollectionParamsRef params);

  TFuture<FCFCoreError> GetClosedLogFiles(
    OUT TSharedRef<TArray<FString>> out_log_files);

  TFuture<FCFCoreError> ZipLogFiles(const FLogsCollectionParamsRef params);

  FString CreateZipFilename();

private:
  TSharedPtr<ILifetimeService> lifetime_service_;
  TSharedPtr<FQueuedThreadPool> thread_pool_;
  TSharedPtr<ILoggerOutputService> logger_output_service_;
  TSharedPtr<IStartupInfoLogging> startup_info_logging_;
  TSharedPtr<ICompressionService> zip_service_;

  TQueue<FLoggerTask, EQueueMode::Mpsc> logs_queue_;
  FDelegateHandle on_shutdown_handle_;
  FEventRef event_shutdown_;
  bool is_thread_running_;
  bool collected_logs_;
};

} // namespace cfcore