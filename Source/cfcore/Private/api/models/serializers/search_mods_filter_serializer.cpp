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
#include "search_mods_filter_serializer.h"
#include <common/constants.h>
#include "GenericPlatform/GenericPlatformHttp.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
// static
void SearchModsFilterSerializer::Serialize(FCFCoreSearchModsFilter filter,
                                           UrlSearchParams& search_params) {
  if (filter.classId > 0) {
    search_params.Append(kApiUrlParamsClassId, filter.classId);
  }

  if (filter.categoryId > 0) {
    search_params.Append(kApiUrlParamsCategoryId, filter.categoryId);
  }

  if (filter.categoryIds.Num() > 0) {
    FString category_ids_as_string = FString::Printf(
      TEXT("[%s]"),
      *FString::JoinBy(filter.categoryIds, TEXT(","), [](int32 num) {
        return LexToString(num);
      })
    );

    search_params.Append(kApiUrlParamsCategoryIds, category_ids_as_string);
  }

  if (!filter.gameVersion.IsEmpty()) {
    search_params.Append(kApiUrlParamsGameVersion, filter.gameVersion);
  }

  if (!filter.searchFilter.IsEmpty()) {
    // TODO(twolf): Fix this on the server to accept %20 and not %2520
    FString encoded = filter.searchFilter.Replace(TEXT(" "), TEXT("%20"));
    encoded = FGenericPlatformHttp::UrlEncode(encoded);
    search_params.Append(kApiUrlParamsSearchFilter, encoded);
  }

  if (filter.sortField != ECFCoreModsSearchSortField::None) {
    search_params.Append(kApiUrlParamsSortField, (int32)filter.sortField);
  }

  if (filter.sortOrder != ECFCoreSortOrder::None) {
    ENUM_TO_STR(str_sort_order, filter.sortOrder);
    search_params.Append(kApiUrlParamsSortOrder, str_sort_order);
  }

  if (filter.modLoaderType != ECFCoreModLoaderType::Any) {
    search_params.Append(kApiUrlParamsModLoaderType,
                         (int32)filter.modLoaderType);
  }

  if (filter.gameVersionTypeId > 0) {
    search_params.Append(kApiUrlParamsGameVersionTypeId,
                         filter.gameVersionTypeId);
  }

  if (filter.authorId > 0) {
    search_params.Append(kApiUrlParamsAuthorId, filter.authorId);
  }

  if (filter.primaryAuthorId > 0) {
    search_params.Append(kApiUrlParamsPrimaryAuthorId, filter.primaryAuthorId);
  }

  if (filter.premiumFilterType != ECFCorePremiumFilterType::FreeAndPremium) {
    int32 int_val = static_cast<int32>(filter.premiumFilterType);
    search_params.Append(kApiUrlParamsPremiumType, int_val);
  }

  if (filter.modsSearchEnhancedFeatures > 0) {
    search_params.Append(kApiUrlParamsModsSearchEnhancedFeatures,
                         filter.modsSearchEnhancedFeatures);
  }
}
