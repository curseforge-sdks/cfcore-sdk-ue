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
#include <creation/cfcore_creation.h>
#include <api/models/creation/uploaded_mod_file.h>
#include <api/models/creation/mod_file_chunk_metadata.h>
#include <creation/services/create_mod_file_chunked_service.h>

namespace cfcore {

// -----------------------------------------------------------------------------
struct FCreationInProgress {
  int64 id = 0;
  TSharedPtr<FString> localFilename;
  TSharedPtr<ICFCoreCreation::FFileTransferProgressDelegate> progressDelegate;

  FUploadedModFile uploadedModFileInfo;
  int64 fileSizeInBytes = 0;
  FModFileChunkMetadata chunksMetadata;
  int32 previousProgress = 0;
  TSharedPtr<TArray<uint8>> buffer;

  /**
   * Allows to cancel the upload operation
   */
  int64 cancelId;
  bool cancelRequest = false;

  int32 retries = 0;

  TPromise<ICreateModFileChunkedService::CreationResult> promise;
};

}