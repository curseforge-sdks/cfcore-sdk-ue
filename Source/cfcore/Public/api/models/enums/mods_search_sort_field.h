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

#include "mods_search_sort_field.generated.h"

// -----------------------------------------------------------------------------
UENUM(BlueprintType)
enum class ECFCoreModsSearchSortField : uint8 {
  None = 0 UMETA(DisplayName = "None"),
  /**
   * Sorts by Featured + Popularity
   */
  Featured = 1 UMETA(DisplayName = "Featured"),
  Popularity = 2 UMETA(DisplayName = "Popularity"),
  /**
   * Sorts by last file uploaded for the mod (newest to oldest)
   */
  LastUpdated = 3 UMETA(DisplayName = "Last Updated"),
  Name = 4 UMETA(DisplayName = "Name"),
  Author = 5 UMETA(DisplayName = "Author"),
  TotalDownloads = 6 UMETA(DisplayName = "Total Installs"),
  Category = 7 UMETA(DisplayName = "Category"),
  GameVersion = 8 UMETA(DisplayName = "Game Version"),
  /**
   * Sorts by mods that have early-access files first
   */
  EarlyAccess = 9 UMETA(DisplayName = "Early Access"),
  /**
   * Sorts by Featured + Released date of mod (newest to oldest)
   */
  FeaturedReleased = 10 UMETA(DisplayName = "Featured Released"),
  ReleasedDate = 11 UMETA(DisplayName = "Released Date"),
  Rating = 12 UMETA(DisplayName = "Rating"),
};
