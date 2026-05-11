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
#include "mod_links.h"
#include "category.h"
#include "mod_author.h"
#include "mod_asset.h"
#include "file.h"
#include "file_index.h"
#include "premium/premium_details.h"
#include "rating_details.h"

#include "enums/mod_status.h"

#include "mod.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreMod {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 id = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 gameId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 gamePopularityRank = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString name;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString slug;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FModLinks links;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString summary;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreModStatus status = ECFCoreModStatus::None;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 downloadCount = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isFeatured = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 classId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 primaryCategoryId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FCategory> categories;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FModAuthor> authors;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FModAsset logo;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FModAsset> screenshots;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FModAsset> videos;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 mainFileId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FFile> latestFiles;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FFileIndex> latestFilesIndexes;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime dateCreated = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime dateModified = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime dateReleased = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool allowModDistribution = true;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isAvailable = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FRatingDetails ratingDetails;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FPremiumDetails premiumDetails;
};
