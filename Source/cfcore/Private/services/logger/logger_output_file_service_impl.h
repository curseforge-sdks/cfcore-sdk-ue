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
#pragma once
#include <CoreMinimal.h>
#include "logger_output_service.h"
#include <models/cfcore_settings_logger.h>

namespace cfcore {
class ISettingsService;
class IFileSystemService;
class IDirectoriesService;
class IPersistentLogFile;

// -----------------------------------------------------------------------------
class LoggerOutputFileServiceImpl : public ILoggerOutputService {
public:
  LoggerOutputFileServiceImpl(
    TSharedPtr<ISettingsService> settings_service,
    TSharedPtr<IFileSystemService> filesystem_service,
    TSharedPtr<IDirectoriesService> directories_service);

// ILoggerOutputRolloutService
public:
  virtual bool WriteLogLine(const FString& message) override;
  virtual bool GetClosedLogFiles(
    OUT TSharedRef<TArray<FString>> out_closed_log_files) override;

private:
  void InitializeSettings(const TSharedPtr<ISettingsService>& settings);
  void AdjustValidSettings();

  bool DidReachMaxRolloutFailures();
  bool CheckForRolloutAndPerform();
  bool IsNewerLogFilename(const FString& first,const FString& second) const;
  void RemoveOpenLogFileIfExists(IN OUT TSharedRef<TArray<FString>> log_files);
  bool DidFilePassMaxSize();
  bool RolloutNewLogFile();
  bool CleanOldLogFiles();
  bool GetExistingLogFiles(OUT TSharedRef<TArray<FString>> log_files);

  FString GetLogFilePrefix();

private:
  TSharedPtr<IFileSystemService> filesystem_service_;
  TSharedPtr<IDirectoriesService> directories_service_;

  int64 game_id_;
  FCFCoreSettingsLogger settings_;

  TSharedPtr<IPersistentLogFile> current_output_;

  // We want to stop trying after many failures
  int32 rollout_failures_count_;
};

} // namespace cfcore