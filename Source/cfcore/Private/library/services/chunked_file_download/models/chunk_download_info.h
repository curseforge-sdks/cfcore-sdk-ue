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

#include "chunked_file_download_bytes_range.h"

namespace cfcore {

// -----------------------------------------------------------------------------
struct FChunkDownloadInfo {
  FChunkedFileDownloadBytesRange range = {0, 0};

  // The identifier of the chunk within the download
  int64 chunk_id = 0;

  // The http request ID for this chunk download - can be used to cancel it
  int64 http_request_id = 0;

  // How many bytes were transferred so far for this chunk
  int64 transferred_bytes = 0;

  // Per-chunk retry counter - how many retries remaining for this specific chunk
  int32 retries = 0;

  FDateTime start_timestamp;
};

}