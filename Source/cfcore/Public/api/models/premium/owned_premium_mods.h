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

#include "owned_premium_mods.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FUserTrialData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 modId = 0;

  /**
   * Set server-side. Use this to determine whether the trial has expired -
   * do NOT compare expiresAt against the local clock, as users can manipulate
   * it. expiresAt is provided solely for displaying a countdown in the UI.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isExpired = false;

  /** ISO 8601 UTC date string (e.g. "2025-01-01T00:00:00Z"). For UI display
   * only - use isExpired for access gating. */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString expiresAt;
};

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FOwnedPremiumMods {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<int64> ownedMods;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FUserTrialData> trialMods;
};
