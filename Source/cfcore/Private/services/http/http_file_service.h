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

#include "models/http_response.h"
#include "models/http_progress.h"

#include <services/buffer/buffer.h>
#include "www_form.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class IHttpFileService {
public:
  DECLARE_DELEGATE_OneParam(FProgressDelegate, const HttpProgress&);
  DECLARE_DELEGATE_TwoParams(FCompleteDelegate, const HttpResponse&, TSharedPtr<IBuffer>);

public:
  virtual ~IHttpFileService() = default;

public:
  // Returns the request id with which you can call |Abort|
  virtual int64 DownloadFile(const FString& url,
                             const TMap<FString, FString>& headers,
                             FProgressDelegate on_progress,
                             FCompleteDelegate on_complete) = 0;

  virtual int64 PostWebForm(const FString& url,
                            const TMap<FString, FString>& headers,
                            const WWWForm& form,
                            FProgressDelegate on_progress,
                            FCompleteDelegate on_complete) = 0;

  virtual int64 PutWebForm(const FString& url,
                           const TMap<FString, FString>& headers,
                           const WWWForm& form,
                           FProgressDelegate on_progress,
                           FCompleteDelegate on_complete) = 0;

  virtual int64 PostBuffer(const FString& url,
                           const TMap<FString, FString>& headers,
                           const TSharedPtr<TArray<uint8>> buffer,
                           FProgressDelegate on_progress,
                           FCompleteDelegate on_complete) = 0;

  virtual int64 PutBuffer(const FString& url,
                          const TMap<FString, FString>& headers,
                          const TSharedPtr<TArray<uint8>> buffer,
                          FProgressDelegate on_progress,
                          FCompleteDelegate on_complete) = 0;

  virtual void Abort(int64 file_id) = 0;
};

}; // namespace cfcore

Expose_TNameOf(cfcore::IHttpFileService)