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
#include <api/models/mod_author.h>
#include <api/models/mod_asset.h>
#include <api/models/premium/premium_details.h>
#include <api/models/rating_details.h>

#include "highlights_mod.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreHighlightsMod {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 id = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FModAsset logo;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FPremiumDetails premiumDetails;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString name;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FModAuthor> authors;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FRatingDetails ratingDetails;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 downloadCount = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 fileSize = 0;
};
