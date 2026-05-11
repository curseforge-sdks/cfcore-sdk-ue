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

#include "get_mods_highlights_filters.generated.h"

// -----------------------------------------------------------------------------
/**
  * Each boolean property, when set to true, will return a new list of mods, as
  * described below. Each list is limited to 10 items most.
  */
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreGetModsHighlightsFilters {
  GENERATED_BODY()

  /**
   * This is manually configured on console.curseforge.com and allows promoting
   * mods.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool carousel = true;

  /**
   * This is manually configured on console.curseforge.com and allows promoting
   * mods.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool banners = true;

  /**
   * Manually configured game categories for pomoting categories.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool categories = true;

  /**
   * List of the latest uploaded mods (i.e. a new file was uploaded).
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool latest = false;

  /**
   * List of the mods ordered by their all-time download count.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool mostDownloaded = false;

  /**
   * List of the premium mods ordered by latest uploaded.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool premium = false;

  /**
   * List of the mods ordered by trending popularity.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool trending = false;

  /**
   * Return a list of Theme Page objects.
   *
   * NOTE: This is only applicable for the |GetModsHighlightsV2| and above
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool themePages = false;
};
