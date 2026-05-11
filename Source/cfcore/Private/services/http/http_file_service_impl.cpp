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
#include "http_file_service_impl.h"
#include "http_response_buffer_impl.h"
#include <common/constants.h>

#include <services/logger/logger_macros.h>
namespace cfcore {

#ifdef PLATFORM_XSX
  #if PLATFORM_XSX == 1
    const bool kFixXboxSXContentLengthHeaderBug = true;
  #else
    const bool kFixXboxSXContentLengthHeaderBug = false;
  #endif
#else
  const bool kFixXboxSXContentLengthHeaderBug = false;
#endif

};

using namespace cfcore;

// -----------------------------------------------------------------------------
HttpFileServiceImpl::HttpFileServiceImpl(
  TSharedPtr<ISettingsService> settings_service) :
  request_sequence_(0),
  settings_service_(settings_service) {
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::DownloadFile(const FString& url,
                                        const TMap<FString, FString>& headers,
                                        FProgressDelegate on_progress,
                                        FCompleteDelegate on_complete) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers);
  request->SetVerb("GET");

  int64 file_id = GenerateFileId(request, on_progress, on_complete);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
  request->OnRequestProgress64().BindLambda(
    [this, file_id](FHttpRequestPtr request, int32, int64 bytes_received) {
#else
  request->OnRequestProgress().BindLambda(
    [this, file_id](FHttpRequestPtr request, int32, int32 bytes_received) {
#endif
      HandleDownloadFileProgress(file_id, request, bytes_received);
    }
  );

  request->OnProcessRequestComplete().BindLambda(
    [this, file_id](FHttpRequestPtr request, FHttpResponsePtr response, bool) {
      HandleFileTransferComplete(file_id, request, response, false);
    });

  CFCORE_LOG(Log,
             TEXT("Starting to download: %s, file id: %d"),
             *url,
             file_id);

  request->ProcessRequest();

  return file_id;
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::PostWebForm(const FString& url,
                                       const TMap<FString, FString>& headers,
                                       const WWWForm& form,
                                       FProgressDelegate on_progress,
                                       FCompleteDelegate on_complete) {
  return SubmitWebForm(url, "POST", headers, form, on_progress, on_complete);
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::PutWebForm(const FString& url,
                                      const TMap<FString, FString>& headers,
                                      const WWWForm& form,
                                      FProgressDelegate on_progress,
                                      FCompleteDelegate on_complete) {
  return SubmitWebForm(url, "PUT", headers, form, on_progress, on_complete);
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::PostBuffer(const FString& url,
                                      const TMap<FString, FString>& headers,
                                      const TSharedPtr<TArray<uint8>> buffer,
                                      FProgressDelegate on_progress,
                                      FCompleteDelegate on_complete) {
  return SendBuffer(url, "POST", headers, buffer, on_progress, on_complete);
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::PutBuffer(const FString& url,
                                     const TMap<FString, FString>& headers,
                                     const TSharedPtr<TArray<uint8>> buffer,
                                     FProgressDelegate on_progress,
                                     FCompleteDelegate on_complete) {
  return SendBuffer(url, "PUT", headers, buffer, on_progress, on_complete);
}

// -----------------------------------------------------------------------------
void HttpFileServiceImpl::Abort(int64 file_id) {
  FileTransferTracking tracker;
  if (!GetTracker(file_id, tracker, false)) {
    return;
  }

  if (!tracker.request.IsValid()) {
    return;
  }

  tracker.cancel_requested = true;
  UpdateTracker(file_id, tracker);

  tracker.request->CancelRequest();
}

// -----------------------------------------------------------------------------
void HttpFileServiceImpl::HandleDownloadFileProgress(int64 file_id,
                                                     FHttpRequestPtr request,
                                                     int64 bytes_received) {
  FileTransferTracking tracker;
  if (!GetTracker(file_id, tracker, false)) {
    return;
  }

  const FHttpResponsePtr response { request->GetResponse() };
  if (!response.IsValid()) {
    return;
  }

  int64 bytes_total = response->GetContentLength();

  if (bytes_total <= 0 || kFixXboxSXContentLengthHeaderBug) {
    FString length_str = response->GetHeader(kHttpHeaderContentLength);
    bytes_total = FCString::Strtoi64(*length_str, nullptr, 10);
  }

  TrackTransferProgress(file_id, tracker, bytes_total, bytes_received);
}

// -----------------------------------------------------------------------------
void HttpFileServiceImpl::HandleUploadFileProgress(int64 file_id,
                                                   FHttpRequestPtr request,
                                                   int64 bytes_sent) {
  FileTransferTracking tracker;
  if (!GetTracker(file_id, tracker, false)) {
    return;
  }

  int64 bytes_total = request->GetContentLength();

  if (bytes_total <= 0 || kFixXboxSXContentLengthHeaderBug) {
    FString length_str = request->GetHeader(kHttpHeaderContentLength);
    bytes_total = FCString::Strtoi64(*length_str, nullptr, 10);
  }

  TrackTransferProgress(file_id, tracker, bytes_total, bytes_sent);
}

// -----------------------------------------------------------------------------
void HttpFileServiceImpl::TrackTransferProgress(int64 file_id,
                                                FileTransferTracking tracker,
                                                int64 bytes_total,
                                                int64 bytes_transferred) {
  int32 progress_percent = (bytes_transferred / (double)bytes_total) * 100;

  // NOTE(twolf): We don't want to filter progress events so that the transfer
  // rate can be reflected.
  // if (tracker.progress_percent == progress_percent) {
  //   return;
  // }

  FDateTime current_timestamp = FDateTime::Now();

  int64 delta_bytes = bytes_transferred - tracker.transferred_bytes;
  FTimespan delta_time = current_timestamp - tracker.timestamp;

  tracker.progress_percent = progress_percent;
  tracker.transferred_bytes = bytes_transferred;
  tracker.timestamp = current_timestamp;

  UpdateTracker(file_id, tracker);

  HttpProgress progress;
  progress.progress = progress_percent;
  progress.transferred_bytes = tracker.transferred_bytes;
  progress.bytes_per_second = delta_bytes / delta_time.GetTotalSeconds();

  tracker.on_progress.ExecuteIfBound(progress);
}

// -----------------------------------------------------------------------------
void HttpFileServiceImpl::HandleFileTransferComplete(
  int64 file_id,
  FHttpRequestPtr req,
  FHttpResponsePtr res,
  bool fill_http_response_body) {

  FileTransferTracking tracker;
  if (!GetTracker(file_id, tracker, true)) {
    CFCORE_LOG(Warning, TEXT("Empty tracker for file id: %d"), file_id);
    return;
  }

  auto http_response = CreateHttpResponse(req,
                                          res,
                                          tracker,
                                          fill_http_response_body);

  TSharedPtr<IBuffer> buffer = MakeShareable(new HttpResponseBuffer(res));

  tracker.on_complete.ExecuteIfBound(http_response, buffer);
}

// -----------------------------------------------------------------------------
bool HttpFileServiceImpl::GetTracker(int64 file_id,
                                     FileTransferTracking& out,
                                     bool remove) {
  FScopeLock lock(&transfer_track_cs_);
  FileTransferTracking* tracker = nullptr;

  tracker = transfer_tracking_.Find(file_id);
  if (!tracker) {
    return false;
  }

  // NOTE(twolf): This might be problematic if deep copy is required - consider
  // converting |FileTransferTracking| to a UStruct for automatic deep copying
  out = *tracker;

  if (remove) {
    transfer_tracking_.Remove(file_id);
  }

  return true;
}

// -----------------------------------------------------------------------------
void HttpFileServiceImpl::UpdateTracker(int64 file_id,
                                        const FileTransferTracking& update) {
  FScopeLock lock(&transfer_track_cs_);
  transfer_tracking_[file_id] = update;
}


// -----------------------------------------------------------------------------
FHttpRequestRef HttpFileServiceImpl::CreateBaseHttpRequest(
  const FString& url,
  const TMap<FString, FString>& headers) {

  FHttpRequestRef request = FHttpModule::Get().CreateRequest();
  // TODO(twolf): Refactor - we got failures when downloading files with spaces
  // but this might only be a problem with fiddler.
  FString encoded_url = url.Replace(TEXT(" "), TEXT("%20"));
  request->SetURL(encoded_url);

  for (const TPair<FString, FString>& pair : headers) {
    request->SetHeader(pair.Key, pair.Value);
  }

#if ENGINE_MAJOR_VERSION >= 5 || ENGINE_MINOR_VERSION >= 26
  float timeout =
    settings_service_->GetInternalSettings().httpFileDownloadTimeoutSecs;
  request->SetTimeout(timeout);
#endif

  return request;
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::SubmitWebForm(const FString& url,
                                         const FString& verb,
                                         const TMap<FString, FString>& headers,
                                         const WWWForm& form,
                                         FProgressDelegate on_progress,
                                         FCompleteDelegate on_complete) {
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers);
  request->SetHeader(kHttpHeaderContentType, form.GetContentTypeHeader());
  request->SetVerb(verb);

  auto contents = form.GetContents();
  request->SetContent(*(contents.Get()));

  int64 file_id = GenerateFileId(request, on_progress, on_complete);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
  request->OnRequestProgress64().BindLambda(
    [this, file_id](FHttpRequestPtr request, int64 bytes_sent, int64) {
#else
  request->OnRequestProgress().BindLambda(
    [this, file_id](FHttpRequestPtr request, int32 bytes_sent, int32) {
#endif
      HandleUploadFileProgress(file_id, request, bytes_sent);
    });

  request->OnProcessRequestComplete().BindLambda(
    [this, file_id](FHttpRequestPtr request, FHttpResponsePtr response, bool) {
      HandleFileTransferComplete(file_id, request, response, true);
    });

  request->ProcessRequest();

  return file_id;
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::SendBuffer(const FString& url,
                                      const FString& verb,
                                      const TMap<FString, FString>& headers,
                                      const TSharedPtr<TArray<uint8>> buffer,
                                      FProgressDelegate on_progress,
                                      FCompleteDelegate on_complete) {

  // NOTE(apliscov): This method does not add a content type header.
  FHttpRequestRef request = CreateBaseHttpRequest(url, headers);
  request->SetVerb(verb);

  request->SetContent(*buffer);

  int64 file_id = GenerateFileId(request, on_progress, on_complete);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
  request->OnRequestProgress64().BindLambda(
    [this, file_id](FHttpRequestPtr request, int64 bytes_sent, int64) {
#else
  request->OnRequestProgress().BindLambda(
    [this, file_id](FHttpRequestPtr request, int32 bytes_sent, int32) {
#endif
      HandleUploadFileProgress(file_id, request, bytes_sent);
    });

  request->OnProcessRequestComplete().BindLambda(
    [this, file_id](FHttpRequestPtr request, FHttpResponsePtr response, bool) {
      HandleFileTransferComplete(file_id, request, response, true);
    });

  request->ProcessRequest();

  return file_id;
}

// -----------------------------------------------------------------------------
int64 HttpFileServiceImpl::GenerateFileId(FHttpRequestRef request,
                                          FProgressDelegate on_progress,
                                          FCompleteDelegate on_complete) {
  int64 file_id = ++request_sequence_;

  FileTransferTracking tracker;
  tracker.request = request;
  tracker.timestamp = FDateTime::Now();
  tracker.on_progress = on_progress;
  tracker.on_complete = on_complete;

  FScopeLock lock(&transfer_track_cs_);
  transfer_tracking_.Add(file_id, tracker);

  return file_id;
}

// -----------------------------------------------------------------------------
struct HttpResponse HttpFileServiceImpl::CreateHttpResponse(
  FHttpRequestPtr request,
  FHttpResponsePtr response,
  const FileTransferTracking& tracker,
  bool fill_body) {

  struct HttpResponse http_response;
  EHttpRequestStatus::Type status = request->GetStatus();

  http_response.cancelled = tracker.cancel_requested;

  if (response.IsValid()) {
    http_response.statusCode = response->GetResponseCode();
    http_response.contentType = response->GetContentType();
  }

  if (status != EHttpRequestStatus::Type::Succeeded) {
    http_response.nonHttpError = true;
    http_response.extraInfo = EHttpRequestStatus::ToString(status);
  } else if (fill_body) {
    http_response.body = response->GetContentAsString();
  }

  if (EHttpResponseCodes::IsOk(http_response.statusCode)) {
    return http_response;
  }

  if (tracker.cancel_requested) {
    CFCORE_LOG(Error, TEXT("Cancelled request: %s"), *request->GetURL());
  } else {
    CFCORE_LOG(Error,
               TEXT("Failed to perform request: %s with status code (%d): %s"),
               *request->GetURL(),
               http_response.statusCode,
               *http_response.body);
  }

  return http_response;
}
