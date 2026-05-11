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

#include "enums/session_type.h"

#include <common/defines.h>

#include "mod_browsing_funnel_params.generated.h"

/**
 * Parameters for sending a ModBrowsingFunnel analytic.
 *
 * NOTE: Numeric fields with value CF_INVALID_NUMBER will be ignored.
 */
USTRUCT(BlueprintType)
struct CFCORE_API FModBrowsingFunnelParams {
  GENERATED_BODY()

  /**
   * The name of the page in which the analytic was triggered from.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString pageName;

  /**
   * If the page has multiple components (e.g. a carousel or shelf).
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString pageComponent;

  /**
   * If from a shelf - what is the identifier of the shelf.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString shelfName;

  /**
   * If a mod page what is the mod id.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 modId = CF_INVALID_NUMBER;

  /**
   * If coming from a themed page - the page id.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 pageId = CF_INVALID_NUMBER;

  /**
   * If a carousel item - what is the item's position.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 itemPosition = CF_INVALID_NUMBER;

  /**
   * For actions (e.g. install/purchase/uninstall/update)
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString actionType;
};
