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
#include <api/models/category.h>
#include <api/models/mod.h>

#include "mods_highlights.generated.h"

// -----------------------------------------------------------------------------
/**
  * See FCFCoreGetModsHighlightsFilters
  */
USTRUCT(BlueprintType)
struct CFCORE_API FModsHighlights {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCategory> categories;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCFCoreMod> carousel;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCFCoreMod> premium;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCFCoreMod> trending;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCFCoreMod> mostDownloaded;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCFCoreMod> latest;
};
