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
#include "CoreMinimal.h"

#include <creation/cfcore_creation.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ICreateModFileChunkedService {
public:
  using CreationResult =
    TTuple<TOptional<FCFCoreError>, TOptional<FUploadedModFile>>;

  virtual ~ICreateModFileChunkedService() = default;

public:
  virtual TFuture<CreationResult> Create(
    int64 mod_id,
    const FCreateModFileRequest& create_mod_file_request,
    const FString& local_filename_to_upload,
    ICFCoreCreation::FCreateModFileRequestIdDelegate request_id_delegate,
    ICFCoreCreation::FFileTransferProgressDelegate progress_delegate) = 0;

  /**
   * Create and upload a cooked file (associated to a source file)
   */
  virtual TFuture<CreationResult> CreateCooked(
    int64 mod_id,
    int64 source_file_id,
    const FCreateCookedModFileRequest& create_cooked_mod_file_request,
    const FString& local_filename_to_upload,
    ICFCoreCreation::FCreateModFileRequestIdDelegate request_id_delegate,
    ICFCoreCreation::FFileTransferProgressDelegate progress_delegate) = 0;

  /**
   * Use the id from: ICFCoreCreation::FCreateModFileRequestIdDelegate
   */
  virtual void CancelCreation(int64 creation_id) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICreateModFileChunkedService)