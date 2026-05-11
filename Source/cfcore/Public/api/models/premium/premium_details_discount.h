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

#include "premium_details_platform_data.h"

#include "premium_details_discount.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FPremiumDetailsDiscount {
  GENERATED_BODY()

  /**
   * The discounted price - if the value is 0, there is no discount
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  float discountPrice = 0;

  /**
  * The discount percent (discountPrice / tierPrice) * 100
  */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  float percent = 0;

  /**
   * The utc end date of the discount - after this date, purchasing with the
   * discounted product id will fail
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime endDate = FDateTime::MinValue();

  /**
   * Non-windows platform information (e.g. the product id on the relevant
   * platform)
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FPremiumDetailsPlatformData platformData;
};
