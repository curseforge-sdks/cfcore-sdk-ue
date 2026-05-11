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
#include <CoreMinimal.h>
#include "http_file_service.h"
#include <services/settings_service.h>
#include <services/filesystem/file_system_service.h>
#include <Http.h>
#include <services/http/models/file_transfer_tracking.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class HttpFileServiceImpl : public IHttpFileService {
public:
  HttpFileServiceImpl(TSharedPtr<ISettingsService> settings_service);
  virtual ~HttpFileServiceImpl() = default;

public:
  virtual int64 DownloadFile(const FString& url,
                             const TMap<FString, FString>& headers,
                             FProgressDelegate on_progress,
                             FCompleteDelegate on_complete) override;

  virtual int64 PostWebForm(const FString& url,
                            const TMap<FString, FString>& headers,
                            const WWWForm& form,
                            FProgressDelegate on_progress,
                            FCompleteDelegate on_complete) override;

  virtual int64 PutWebForm(const FString& url,
                           const TMap<FString, FString>& headers,
                           const WWWForm& form,
                           FProgressDelegate on_progress,
                           FCompleteDelegate on_complete) override;

  virtual int64 PostBuffer(const FString& url,
                           const TMap<FString, FString>& headers,
                           const TSharedPtr<TArray<uint8>> buffer,
                           FProgressDelegate on_progress,
                           FCompleteDelegate on_complete) override;

  virtual int64 PutBuffer(const FString& url,
                          const TMap<FString, FString>& headers,
                          const TSharedPtr<TArray<uint8>> buffer,
                          FProgressDelegate on_progress,
                          FCompleteDelegate on_complete) override;

  virtual void Abort(int64 file_id) override;

private:
  FHttpRequestRef CreateBaseHttpRequest(const FString& url,
                                        const TMap<FString, FString>& headers);

  int64 SubmitWebForm(const FString& url,
                      const FString& verb, // POST/PUT
                      const TMap<FString, FString>& headers,
                      const WWWForm& form,
                      FProgressDelegate on_progress,
                      FCompleteDelegate on_complete);

  int64 SendBuffer(const FString& url,
                   const FString& verb,
                   const TMap<FString, FString>& headers,
                   const TSharedPtr<TArray<uint8>> buffer,
                   FProgressDelegate on_progress,
                   FCompleteDelegate on_complete);

  int64 GenerateFileId(FHttpRequestRef request,
                       FProgressDelegate on_progress,
                       FCompleteDelegate on_complete);

  // NOTE(twolf): |fill_body| when true, will fill the HttpResponse.Body field,
  // which is an FString. This is only relevant for string/json results - i.e.
  // for uploading content and not for downloading content
  struct HttpResponse CreateHttpResponse(FHttpRequestPtr request,
                                         FHttpResponsePtr response,
                                         const FileTransferTracking& tracker,
                                         bool fill_body);

  void HandleDownloadFileProgress(int64 file_id,
                                  FHttpRequestPtr request,
                                  int64 bytes_received);
  void HandleUploadFileProgress(int64 file_id,
                                FHttpRequestPtr request,
                                int64 bytes_sent);
  void TrackTransferProgress(int64 file_id,
                             FileTransferTracking tracker,
                             int64 bytes_total,
                             int64 bytes_transferred);

  void HandleFileTransferComplete(int64 file_id,
                                  FHttpRequestPtr request,
                                  FHttpResponsePtr response,
                                  bool fill_http_response_body);

  bool GetTracker(int64 file_id, FileTransferTracking& out, bool remove);
  void UpdateTracker(int64 file_id, const FileTransferTracking& update);

private:
  int64 request_sequence_;
  TSharedPtr<ISettingsService> settings_service_;

  FCriticalSection transfer_track_cs_;
  TMap<int32, FileTransferTracking> transfer_tracking_;
};

} // namespace cfcore
