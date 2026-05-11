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
#include "CoreMinimal.h"
#include "api_response_handler.h"

#include <common/macros.h>

#include <Json.h>
#include <JsonUtilities.h>
#include <api/cfcore_api_creation.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// NOTE(twolf): This is a decorator class
template<typename ResponseModel,
         typename DelegateTwoParamsWithError>
class ApiResponseHandlerWithProgressImpl : public IApiResponseHandler {
public:
  // ---------------------------------------------------------------------------
 ApiResponseHandlerWithProgressImpl(
    TSharedPtr<IApiResponseHandler> inner_handler,
    ICFCoreApiCreation::FFileTransferProgressDelegate progress_delegate) :
    inner_handler_(inner_handler),
    progress_delegate_(progress_delegate) {
  }

// IApiResponseHandler
public:
  // ---------------------------------------------------------------------------
  virtual void HandleProgress(const HttpProgress& http_progress) {
    inner_handler_->HandleProgress(http_progress);

    FFileTransferProgress progress;
    progress.progress = http_progress.progress;
    progress.transferredBytes = http_progress.transferred_bytes;
    progress.bytesPerSecond = http_progress.bytes_per_second;

    progress_delegate_.ExecuteIfBound(progress);
  }

  // ---------------------------------------------------------------------------
  virtual void HandleResponse(const HttpResponse& http_response) override {
    inner_handler_->HandleResponse(http_response);
  }

  // ---------------------------------------------------------------------------
  virtual void HandleApiResponseError(
    const FCFCoreApiResponseError& error) override {
    inner_handler_->HandleApiResponseError(error);
  }

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ApiResponseHandlerWithProgressImpl);

private:
  TSharedPtr<IApiResponseHandler> inner_handler_;
  ICFCoreApiCreation::FFileTransferProgressDelegate progress_delegate_;
};

} // namespace cfcore