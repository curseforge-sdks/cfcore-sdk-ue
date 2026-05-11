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

#include "models/cfcore_file_size.h"
#include "models/split_installed_mods_options.h"

#include <models/cfcore_settings.h>
#include <api/common/cfcore_api_request_pagination.h>
#include <api/models/filters/search_mods_filter.h>
#include <api/models/filters/get_categories_filter.h>
#include <api/models/external_auth_additional_info.h>

#include <api/models/enums/mods_search_sort_field.h>
#include <api/models/enums/sort_order.h>
#include <api/models/enums/mod_loader_type.h>
#include <api/models/enums/markup_type.h>
#include <api/models/enums/changelog_markup_type.h>
#include <api/models/enums/file_release_type.h>

#include <api/models/creation/create_mod_request.h>
#include <api/models/creation/update_mod_request.h>
#include <api/models/creation/create_mod_file_request.h>

#include <library/models/installed_mod.h>
#include <library/models/installed_mod_properties.h>
#include <library/models/assure_server_mods_updated_params.h>

#include <Kismet/BlueprintFunctionLibrary.h>

#include "cfcore_bp_library.generated.h"

UCLASS()
class CFCORE_API UCFCoreBPLibrary : public UBlueprintFunctionLibrary {
  GENERATED_BODY()
public:
  // Creates an FCFCoreSettings from the global project settings
  UFUNCTION(BlueprintPure, Category="cfcore")
  static FCFCoreSettings MakeSettingsFromProjectConfig();

  UFUNCTION(BlueprintPure, category="cfcore", meta=(NativeMakeFunc))
  static FCFCoreSettings MakeSettings(const FString& default_language,
                                      int64 game_id,
                                      const FString& api_key,
                                      const FString& mods_directory,
                                      const FString& user_data_directory,
                                      int32 max_concurrent_installations = 3);

  UFUNCTION(BlueprintPure, category="cfcore|Api", meta=(NativeMakeFunc))
  static FCFCoreApiRequestPagination MakeApiRequestPagination(
    int32 index = 0,
    int32 page_size = 20);

  UFUNCTION(BlueprintPure, category="cfcore|Api", meta=(NativeMakeFunc))
  static FCFCoreGetCategoriesFilter MakeGetCategoriesFilter(int32 class_id = 0);

  UFUNCTION(BlueprintPure, category="cfcore|Auth", meta=(NativeMakeFunc))
  static FExternalAuthAdditionalInfo MakeExternalAuthAdditionalInfo(
    FDateTime eulaAcceptTime);

  UFUNCTION(BlueprintPure, category = "cfcore|Creation", meta = (NativeMakeFunc, AutoCreateRefTerm="game_category_ids"))
  static FCreateModRequest MakeCreateModRequest(
    int64 class_id,
    const FString& name,
    const FString& summary,
    ECFCoreMakrupType description_type,
    const FString& description,
    int64 primary_category_id,
    const TArray<int64>& game_category_ids,
    bool is_experimental);

  UFUNCTION(BlueprintPure, category = "cfcore|Creation", meta = (NativeMakeFunc))
  static FUpdateModRequest MakeUpdateModRequest(
    int64 class_id,
    const FString& name,
    const FString& summary,
    ECFCoreMakrupType description_type,
    const FString& description,
    int64 primary_category_id,
    const TArray<int64>& game_category_ids,
    bool is_experimental);

  UFUNCTION(BlueprintPure, category = "cfcore|Creation", meta = (NativeMakeFunc))
  static FCreateModFileRequest MakeCreateModFileRequest(
    ECFCoreChangelogMarkupType changelog_type,
    const FString& changelog,
    const FString& filename,
    const FString& displayName,
    const TArray<int64>& gameVersionIds,
    ECFCoreFileReleaseType releaseType,
    const FModFileCookingOptions& cookingOptions,
    bool isMarkedForManualRelease);

  UFUNCTION(BlueprintPure, category = "cfcore|Library", meta = (NativeMakeFunc))
  static FAssureServerModsUpdatedParams MakeAssureServerModsUpdatedParams(
    const TArray<int64>& ModIds,
    /**
     * Get the latest unpublished or published files of these mod ids
     */
    const TArray<int64>& DevModIds);

  /**
   * Converts a file size from bytes to a friendly size (B/KB/MB/GB).
   */
  UFUNCTION(BlueprintPure, category = "cfcore|Utility")
  static FString FormatFileSize(int64 Bytes);

  /**
   * Breaks a file size (in bytes) into different representations.
   */
  UFUNCTION(BlueprintPure, category = "cfcore|Utility")
  static FCFCoreFileSize BreakFileSize(int64 Bytes);

  /**
  * Extract the modid field from the entitlement ExtraData string
  */
  UFUNCTION(BlueprintPure, category = "cfcore|Utility")
  static int64 ExtractIdFromExtraData(const FString& InExtraData,
                                      const FString& InFieldName);

  UFUNCTION(BlueprintPure, category = "cfcore|Utility")
  static void SplitInstalledMods(const TArray<FInstalledMod>& InInstalledMods,
                                 const FSplitInstalledModsOptions& InOptions,
                                 TArray<FInstalledMod>& OutFirstInstalledMods,
                                 TArray<FInstalledMod>& OutSecondInstalledMods);

  /**
   * A convinience function to update the load order of installed mods. You can
   * then pass it on to the UpdateInstalledModsProperties function.
   */
  UFUNCTION(BlueprintPure, category = "cfcore|Utility")
  static void UpdateInstalledModsLoadOrder(
    const TArray<FInstalledMod>& InInstalledMods,
    int32 InModIndexToUpdate,
    int32 InNewLoadOrder,
    TArray<FInstalledMod>& OutOrderedInstalledMods,
    TArray<FInstalledModProperties>& OutOrderedInstalledModsProperties);

private:
  static FInstalledModProperties MakeInstalledModProperties(
    const FInstalledMod& InInstalledMod,
    int32 InLoadOrder);
};
