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

#include <api/models/enums/mods_search_sort_field.h>
#include <api/models/enums/sort_order.h>
#include <api/models/enums/mod_loader_type.h>
#include <api/models/enums/premium_filter_type.h>

#include "search_mods_filter.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreSearchModsFilter {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int32 classId = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int32 categoryId = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TArray<int32> categoryIds;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString gameVersion;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString searchFilter;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreModsSearchSortField sortField = ECFCoreModsSearchSortField::None;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreSortOrder sortOrder = ECFCoreSortOrder::Desc;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreModLoaderType modLoaderType = ECFCoreModLoaderType::Any;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int32 gameVersionTypeId = 0;

  /**
   * Filter only mods that the given authorId is a *member* of
   * (unlike primaryAuthorId)
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int32 authorId = 0;

  /**
   * Filter only mods that the given primaryAuthorId is the owner of.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int32 primaryAuthorId = 0;

  /**
   * Allows filtering Free/Premium mods from the search results
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCorePremiumFilterType premiumFilterType = ECFCorePremiumFilterType::FreeAndPremium;

  /**
   * Bitwise number with values from ECFCoreModsSearchEnhancedFeatures.
   *
   * For example:
   * ECFCoreModsSearchEnhancedFeatures::ExtractModId |
   * ECFCoreModsSearchEnhancedFeatures::AnotherFutureFeature
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 modsSearchEnhancedFeatures = 0;
};
