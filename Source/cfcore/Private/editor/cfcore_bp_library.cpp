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
#include <editor/cfcore_bp_library.h>
#include <editor/cfcore_editor_settings.h>
#include <services/logger/logger_macros.h>

#include <JsonUtilities.h>

namespace cfcore {
const int64 kOneKB = 1024;
const int64 kOneMB = kOneKB * 1024;
const int64 kOneGB = kOneMB * 1024;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
// static
FCFCoreSettings UCFCoreBPLibrary::MakeSettingsFromProjectConfig() {
  const UCFCoreEditorSettings* project_config =
    GetDefault<UCFCoreEditorSettings>();

  FCFCoreSettings settings;
  settings.initOptions = project_config->initOptions;
  settings.defaultLanguage = project_config->defaultLanguage;
  settings.gameId = project_config->gameId;
  settings.apiKey = project_config->apiKey;
  settings.provider = project_config->provider;

  settings.modsDirectory = project_config->modsDirectory;
  settings.modsDirectoryMode = project_config->modsDirectoryMode;

  settings.userDataDirectory = project_config->userDataDirectory;

  settings.maxConcurrentInstallations =
    project_config->maxConcurrentInstallations;

  settings.isServer = project_config->isServer;

  settings.throttling = project_config->throttling;
  settings.premiumMods = project_config->premiumMods;
  settings.logger = project_config->logger;
  settings.dynamicContentCategoryIds =
    project_config->dynamicContentCategoryIds;
  settings.analytics = project_config->analytics;
  settings.ignoredDynamicModFiles = project_config->ignoredDynamicModFiles;
  settings.unmanagedMods = project_config->unmanagedMods;
  settings.subscriptions = project_config->subscriptions;
  settings.downloads = project_config->downloads;

  return settings;
}

// -----------------------------------------------------------------------------
// static
FCFCoreSettings UCFCoreBPLibrary::MakeSettings(
  const FString& default_language,
  int64 game_id,
  const FString& api_key,
  const FString& mods_directory,
  const FString& user_data_directory,
  int32 max_concurrent_installations/*= 3*/) {

  FCFCoreSettings settings;
  settings.defaultLanguage = default_language;
  settings.gameId = game_id;
  settings.apiKey = api_key;
  settings.modsDirectory = mods_directory;
  settings.userDataDirectory = user_data_directory;
  settings.maxConcurrentInstallations = max_concurrent_installations;
  return settings;
}

// -----------------------------------------------------------------------------
// static
FCFCoreApiRequestPagination UCFCoreBPLibrary::MakeApiRequestPagination(
  int32 index/*= 0*/,
  int32 page_size/*= 20*/) {
  FCFCoreApiRequestPagination pagination;
  pagination.index = index;
  pagination.pageSize = page_size;

  return pagination;
}

// -----------------------------------------------------------------------------
// static
FCFCoreGetCategoriesFilter UCFCoreBPLibrary::MakeGetCategoriesFilter(
  int32 class_id/*= 0*/) {

  FCFCoreGetCategoriesFilter filter;
  filter.classId = class_id;
  return filter;
}

// -----------------------------------------------------------------------------
// static
FExternalAuthAdditionalInfo UCFCoreBPLibrary::MakeExternalAuthAdditionalInfo(
  FDateTime eulaAcceptTime) {
  FExternalAuthAdditionalInfo additional_info;
  additional_info.eulaAcceptTime = eulaAcceptTime;

  return additional_info;
}

// -----------------------------------------------------------------------------
// static
FCreateModRequest UCFCoreBPLibrary::MakeCreateModRequest(
  int64 class_id,
  const FString& name,
  const FString& summary,
  ECFCoreMakrupType description_type,
  const FString& description,
  int64 primary_category_id,
  const TArray<int64>& game_category_ids,
  bool is_experimental) {

  FCreateModRequest create_mod_request;
  create_mod_request.classId = class_id;
  create_mod_request.name = name;
  create_mod_request.summary = summary;
  create_mod_request.descriptionType = description_type;
  create_mod_request.description = description;
  create_mod_request.primaryCategoryId = primary_category_id;

  for (int64 game_category_id : game_category_ids) {
    create_mod_request.gameCategoryIds.Add(game_category_id);
  }

  create_mod_request.isExperimental = is_experimental;

  return create_mod_request;
}

// -----------------------------------------------------------------------------
// static
FUpdateModRequest UCFCoreBPLibrary::MakeUpdateModRequest(
  int64 class_id,
  const FString& name,
  const FString& summary,
  ECFCoreMakrupType description_type,
  const FString& description,
  int64 primary_category_id,
  const TArray<int64>& game_category_ids,
  bool is_experimental) {

  FUpdateModRequest update_mod_request;
  update_mod_request.classId = class_id;
  update_mod_request.name = name;
  update_mod_request.summary = summary;
  update_mod_request.descriptionType = description_type;
  update_mod_request.description = description;
  update_mod_request.primaryCategoryId = primary_category_id;

  for (int64 game_category_id : game_category_ids) {
    update_mod_request.gameCategoryIds.Add(game_category_id);
  }

  update_mod_request.isExperimental = is_experimental;

  return update_mod_request;
}

// -----------------------------------------------------------------------------
// static
FCreateModFileRequest UCFCoreBPLibrary::MakeCreateModFileRequest(
  ECFCoreChangelogMarkupType changelogType,
  const FString& changelog,
  const FString& filename,
  const FString& displayName,
  const TArray<int64>& gameVersionIds,
  ECFCoreFileReleaseType releaseType,
  const FModFileCookingOptions& cookingOptions,
  bool isMarkedForManualRelease) {

  FCreateModFileRequest create_mod_file_request;
  create_mod_file_request.changelogType = changelogType;
  create_mod_file_request.changelog = changelog;
  create_mod_file_request.filename = filename;
  create_mod_file_request.displayName = displayName;
  create_mod_file_request.releaseType = releaseType;
  create_mod_file_request.isMarkedForManualRelease = isMarkedForManualRelease;

  for (int64 game_version_id : gameVersionIds) {
    create_mod_file_request.gameVersionIds.Add(game_version_id);
  }

  create_mod_file_request.cookingOptions = cookingOptions;

  return create_mod_file_request;
}

// -----------------------------------------------------------------------------
// static
FAssureServerModsUpdatedParams
UCFCoreBPLibrary::MakeAssureServerModsUpdatedParams(
  const TArray<int64>& ModIds,
  const TArray<int64>& DevModIds) {

  FAssureServerModsUpdatedParams params;
  params.modIds = ModIds;
  params.devModIds = DevModIds;

  return MoveTemp(params);
}


// -----------------------------------------------------------------------------
// static
FString UCFCoreBPLibrary::FormatFileSize(int64 Bytes) {
  FString Result;
  if (Bytes < kOneKB) {
    Result = FString::Printf(TEXT("%lld B"), Bytes);
  } else if (Bytes < kOneMB) {
    Result =
      FString::Printf(TEXT("%.2lf KB"), static_cast<double>(Bytes) / kOneKB);
  } else if (Bytes < kOneGB) {
    Result =
      FString::Printf(TEXT("%.2lf MB"), static_cast<double>(Bytes) / kOneMB);
  } else {
    Result =
      FString::Printf(TEXT("%.2lf GB"), static_cast<double>(Bytes) / kOneGB);
  }

  return Result;
}

// -----------------------------------------------------------------------------
// static
FCFCoreFileSize UCFCoreBPLibrary::BreakFileSize(int64 Bytes) {
  FCFCoreFileSize cfcore_file_size;
  cfcore_file_size.kb = static_cast<double>(Bytes) / kOneKB;
  cfcore_file_size.mb = static_cast<double>(Bytes) / kOneMB;
  cfcore_file_size.gb = static_cast<double>(Bytes) / kOneGB;

  return cfcore_file_size;
}

// -----------------------------------------------------------------------------
// static
int64 UCFCoreBPLibrary::ExtractIdFromExtraData(const FString& InExtraData,
                                               const FString& InFieldName) {

  TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(InExtraData);

  TSharedPtr<FJsonObject> json_obj;
  if (FJsonSerializer::Deserialize(reader, json_obj) == false) {
    return 0;
  }

  int64 id;
  if (!json_obj->TryGetNumberField(InFieldName, id)) {
    return 0;
  }

  return id;
}

// -----------------------------------------------------------------------------
// static
void UCFCoreBPLibrary::SplitInstalledMods(
  const TArray<FInstalledMod>& InInstalledMods,
  const FSplitInstalledModsOptions& InOptions,
  TArray<FInstalledMod>& OutFirstInstalledMods,
  TArray<FInstalledMod>& OutSecondInstalledMods) {

  OutFirstInstalledMods.Empty();
  OutSecondInstalledMods.Empty();

  for (const FInstalledMod& Mod : InInstalledMods) {
    if (!InOptions.byEnabled) {
      OutFirstInstalledMods.Add(Mod);
    } else if (Mod.enabled) {
      OutFirstInstalledMods.Add(Mod);
    } else {
      OutSecondInstalledMods.Add(Mod);
    }
  }

  if (!InOptions.sortByLoadOrder) {
    return;
  }

  OutFirstInstalledMods.Sort(
    [](const FInstalledMod& A, const FInstalledMod& B) {
      return A.loadOrder < B.loadOrder;
    });

  OutSecondInstalledMods.Sort(
    [](const FInstalledMod& A, const FInstalledMod& B) {
      return A.loadOrder < B.loadOrder;
    });
}

// -----------------------------------------------------------------------------
// static
void UCFCoreBPLibrary::UpdateInstalledModsLoadOrder(
  const TArray<FInstalledMod>& InInstalledMods,
  int32 InModIndexToUpdate,
  int32 InNewLoadOrder,
  TArray<FInstalledMod>& OutOrderedInstalledMods,
  TArray<FInstalledModProperties>& OutOrderedInstalledModsProperties) {

  OutOrderedInstalledMods.Empty();
  OutOrderedInstalledModsProperties.Empty();

  if (InModIndexToUpdate >= InInstalledMods.Num()) {
    CFCORE_LOG(Error, TEXT("Invalid index passed"));
    return;
  }

  const FInstalledMod& mod_to_update = InInstalledMods[InModIndexToUpdate];
  int32 load_order = 0;

  for (int32 i = 0; i < InInstalledMods.Num(); ++i) {
    const FInstalledMod& mod = InInstalledMods[i];
    if (mod.details.id == mod_to_update.details.id) {
      continue;
    } else if (load_order == InNewLoadOrder) {
      OutOrderedInstalledMods.Add(mod_to_update);
      OutOrderedInstalledModsProperties.Add(
        MakeInstalledModProperties(mod_to_update, InNewLoadOrder));
      ++load_order;
    }

    OutOrderedInstalledMods.Add(mod);
    OutOrderedInstalledModsProperties.Add(
      MakeInstalledModProperties(mod, load_order));
    ++load_order;
  }

  // Handle case where the mod to update is the last one in the list
  if (load_order == InNewLoadOrder) {
    OutOrderedInstalledMods.Add(mod_to_update);
    OutOrderedInstalledModsProperties.Add(
      MakeInstalledModProperties(mod_to_update, InNewLoadOrder));
  }
}

// -----------------------------------------------------------------------------
// static
FInstalledModProperties UCFCoreBPLibrary::MakeInstalledModProperties(
  const FInstalledMod& InInstalledMod,
  int32 InLoadOrder) {

  FInstalledModProperties props;
  props.modId = InInstalledMod.details.id;
  props.enabled = InInstalledMod.enabled;
  props.loadOrder = InLoadOrder;
  return props;
}
