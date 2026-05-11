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

#include "poll_purchase_params.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FPollPurchaseParams {
  GENERATED_BODY()

  /**
   * The mod ids to check ownership for. The delegate fires as soon as all
   * of these mods appear in the CheckMods response.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Premium")
  TArray<int64> modIds;

  /**
   * How often (in seconds) to call CheckMods while waiting for the purchase
   * to complete. Defaults to 5 seconds.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Premium")
  float intervalSeconds = 5.0f;

  /**
   * Maximum total time (in seconds) to keep polling before giving up with a
   * PurchasePollingTimeout error. Defaults to 10 minutes.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Premium")
  float maxDurationSeconds = 600.0f;
};
