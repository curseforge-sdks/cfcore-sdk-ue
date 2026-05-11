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
#include <Async/Future.h>

#include <common/cfcore_error.h>

#include <services/http/models/http_progress.h>

#include <api/models/file_transfer_progress.h>

#include "models/chunked_download_initiated.h"
#include "models/chunked_download_params.h"
#include "models/chunk_completion_info.h"
namespace cfcore {

// -----------------------------------------------------------------------------
// TODO(twolf): Consider moving this to services/http and making it more general
// purpose (without being award of library models - mainly under
// FChunkedDownloadParams)
class IChunkedFileDownloadService {
public:
  /**
   * Invoked as soon as there is a cancellable file id
   */
  DECLARE_DELEGATE_OneParam(FDownloadInitiatedDelegate, const FDownloadInitiated&);
  DECLARE_DELEGATE_OneParam(FFileTransferProgressDelegate, const FFileTransferProgress&);
  DECLARE_DELEGATE_OneParam(FChunkCompletionDelegate, const FChunkCompletionInfo&);

public:
  virtual ~IChunkedFileDownloadService() = default;

public:
  virtual TFuture<FCFCoreError> Download(
    const FChunkedDownloadParams& params,
    FDownloadInitiatedDelegate download_init_delegate,
    FFileTransferProgressDelegate progress_delegate,
    TOptional<FChunkCompletionDelegate> opt_chunk_completion_delegate) = 0;

  /**
   * Use the id from: FRequestIdDelegate
   */
  virtual bool CancelDownload(int64 request_id) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IChunkedFileDownloadService)