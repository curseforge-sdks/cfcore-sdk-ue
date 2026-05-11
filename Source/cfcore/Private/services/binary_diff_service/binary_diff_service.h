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
#include <common/cfcore_error.h>
#include <api/models/file_transfer_progress.h>
#include <Async/Future.h>
#include <Misc/QueuedThreadPool.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IBinaryDiffService {
public:
  DECLARE_DELEGATE_OneParam(FProgressDelegate, const FFileTransferProgress&);

public:
  virtual ~IBinaryDiffService() = default;

public:
  virtual TFuture<FCFCoreError> ApplyBinaryPatch(
    const FString& old_file,
    const FString& patch_file,
    const FString& output_file,
    FProgressDelegate progress_delegate) = 0;

  virtual TFuture<FCFCoreError> CalcOutputFileSize(
    const FString& patch_file,
    TSharedRef<int64> output_files_total_size) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IBinaryDiffService)