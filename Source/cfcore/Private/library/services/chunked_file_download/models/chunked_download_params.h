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
#include <library/models/downloaded_chunk.h>
#include <library/models/installed_mod.h>

namespace cfcore {

// -----------------------------------------------------------------------------
struct FChunkedDownloadParams {
  // Url used for the head request - this must be the edge domain
  FString head_request_url;

  // Url for the chunked (Range) download - this can NOT be the edge domain
  FString download_url;

  FString destination_file_path;

  TMap<FString, FString> tracking;
  int32 throttle_download_kbps = 0;

  // Allows resuming a download
  bool allow_resumable_download = false;
  FInstalledModDownloadInfo existing_download_info;
};

}