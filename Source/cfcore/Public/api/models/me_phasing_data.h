/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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

#include "me_phasing_data.generated.h"

// -----------------------------------------------------------------------------
// Allows for feature flags and ab-testing features.
// NOTE: All properties should be of type int32 and contain a value between 0
// and 100 - where 0 means disabled for all.
USTRUCT(BlueprintType)
struct CFCORE_API FMePhasingData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 deltaDiffs = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 analyticsUserEngagement = 100;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 analyticsPerfAndStability = 100;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 modFileSizeValidation = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 deleteStaleModDirs = 100;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 resumableDownloads = 100;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 useTranslations = 0;

  // The following are used for unit testing and shouldn't be removed
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 testFieldZero = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 testFieldFifty = 50;
};
