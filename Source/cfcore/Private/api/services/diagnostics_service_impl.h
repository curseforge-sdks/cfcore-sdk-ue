/*MIT License

Copyright (c) 2025 Overwolf Ltd.

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

#include <common/cfcore_error.h>

#include <api/cfcore_api_delegates_dispatcher.h>
#include <api/cfcore_api_authorized_delegates.h>

#include "diagnostics_service.h"

namespace cfcore {
class ILoggerService;
class IDirectoriesService;
class IFileSystemService;
class IHttpFileService;

// -----------------------------------------------------------------------------
class DiagnosticsServiceImpl : public IDiagnosticsService {
public:
  DiagnosticsServiceImpl(
    TSharedRef<ILoggerService> logger_service,
    TSharedRef<ICFCoreApiAuthorized> api_authorized,
    TSharedRef<IDirectoriesService> directories_service,
    TSharedRef<IFileSystemService> file_system_service,
    TSharedRef<IHttpFileService> http_file_service,
    TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
      auth_delegates_dispatcher);

private:
  void RegisterCFCoreApiDispatcherDelegates(
    TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
      auth_delegates_dispatcher);

  void SendUserLogs();

  TFuture<FCFCoreError> ZipLogs(const FString& zip_file);

  TFuture<FCFCoreError> GetLogsCollectionUrl(
    OUT TSharedRef<FString> out_upload_url);

  TFuture<FCFCoreError> ReadZipFileToBuffer(
    const TSharedRef<FString> zip_file,
    OUT TSharedRef<TArray<uint8>> out_buffer);

  TFuture<FCFCoreError> UploadBuffer(TSharedRef<FString> upload_url,
                                     TSharedRef<TArray<uint8>> buffer);

  TFuture<FCFCoreError> DeleteZip(const TSharedRef<FString> zip_file);

  FString CreateZipFilename();

private:
  TSharedRef<ILoggerService> logger_service_;
  TSharedRef<ICFCoreApiAuthorized> api_authorized_;
  TSharedRef<IDirectoriesService> directories_service_;
  TSharedRef<IFileSystemService> file_system_service_;
  TSharedRef<IHttpFileService> http_file_service_;
  TSharedRef<CFCoreApiAuthorizedDelegates> delegates_;

  bool is_diagnostics_in_progress_;

};

} // namespace cfcore