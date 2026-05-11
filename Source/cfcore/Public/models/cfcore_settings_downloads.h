/*MIT License

Copyright (c) 2026 Overwolf Ltd.

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

#include "cfcore_settings_downloads.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreSettingsDownloads {
  GENERATED_BODY()

  /**
   * The maximum amount of chunks that can be downloaded in parallel.
   * Valid range: 1-32
   * Note - if |FInstallModAdditionalParams::throttleDownloadKbps| > 0, then
   * this setting is ignored and only one chunk is downloaded at a time.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore", meta = (ClampMin = "1", ClampMax = "32", UIMin = "1", UIMax = "32"))
  int32 maxParallelChunks = 1;
};
