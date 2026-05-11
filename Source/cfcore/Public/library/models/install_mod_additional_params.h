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

#include "enums/mod_install_origin.h"

#include "install_mod_additional_params.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FInstallModAdditionalParams {
  GENERATED_BODY()

  /**
   * Tracking key-value params to be sent to the server upon installing a new
   * mod.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TMap<FString, FString> tracking;

  /**
   * 0 (default) = no throttling
   * 100 - 102,400 bytes per second
   * 200 - 204,800 bytes per second
   * etc...
   *
   * NOTE(twolf): Use this for smaller files being downloaded - as it is very
   * inefficient for large downloaded files
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int32 throttleDownloadKbps = 0;

  /**
   * A dynamic content install means the mod's assets exist on disk but are not
   * owned/installed by the user - as an example, this allows viewing other
   * players, of a multi-player session, with their unique cosmetics.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool dynamicContent = false;

  /**
   * The origin of the mod installation, e.g. mod page, subscription module,
   * joining a server.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  EModInstallOrigin origin = EModInstallOrigin::ModPage;
};
