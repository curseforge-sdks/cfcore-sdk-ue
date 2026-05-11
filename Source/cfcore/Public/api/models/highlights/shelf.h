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

#include <api/models/enums/shelf_type.h>
#include <api/models/highlights/carousel_item_destination.h>
#include <api/models/highlights/query_params.h>
#include <api/models/highlights/shelf_content.h>

#include "shelf.generated.h"

/**
 * A shelf is an item (e.g. banner) or a collection of items (mods / categories)
 */
USTRUCT(BlueprintType)
struct CFCORE_API FShelf {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 displayIndex = MAX_int32;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreShelfType type = ECFCoreShelfType::Category;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString name;

  /**
   * For Generated Shelves, the queryParams object will contain the exact query
   * that returned these results.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FQueryParams queryParams;

  /**
   * Shelf content will have only one populated property out of three,
   * depending on the shelf type.
   * For Category type shelves this property is a list of categories.
   * For Generated or Custom shelves, this property is a list of mods.
   * For Banner shelves, this property is a banner.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FShelfContent content;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FCarouselItemDestination destination;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool seeAllButton = false;
};