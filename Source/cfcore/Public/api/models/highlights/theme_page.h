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
#include "carousel_item_destination.h"

#include "theme_page.generated.h"

// -----------------------------------------------------------------------------
/**
  */
USTRUCT(BlueprintType)
struct CFCORE_API FThemePage {
  GENERATED_BODY()

  /**
   * The unique id of the Theme Page - which can be used with
   * FCarouselItemDestination.value where applicable
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 id = 0;

  /**
   * Image url
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString bannerUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString mainTitle;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString subTitle;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<int64> modIds;
};
