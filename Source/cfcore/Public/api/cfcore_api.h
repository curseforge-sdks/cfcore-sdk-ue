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
#include "CoreMinimal.h"

#include "cfcore_api_authentication.h"
#include "cfcore_api_authorized.h"
#include "cfcore_api_creation.h"

#include <api/common/cfcore_api_response_error.h>
#include <api/common/cfcore_api_request_pagination.h>
#include <api/common/cfcore_api_response_pagination.h>

#include "models/filters/search_mods_filter.h"
#include "models/filters/get_categories_filter.h"
#include "models/filters/get_mods_filter.h"
#include "models/filters/get_files_filter.h"
#include "models/filters/match_platform_files_filter.h"
#include "models/filters/get_mods_highlights_filter.h"

#include "models/game.h"
#include "models/game_version_type.h"
#include "models/game_version_by_type.h"
#include "models/game_versions_detailed_by_type.h"
#include "models/category.h"
#include "models/mod.h"
#include "models/file.h"
#include "models/file_delta_diff.h"
#include "models/auth_token.h"
#include "models/sdk_version.h"
#include "models/direct_download_domain_ex.h"
#include "models/mod_file_download_url.h"
#include "models/highlights/mods_highlights.h"
#include "models/highlights/mods_highlights_v2.h"
#include "models/highlights/mods_highlights_v3.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class ICFCoreApi {
public:
  DECLARE_DELEGATE_TwoParams(FGetGameDelegate, TOptional<FGame>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetVersionTypesDelegate, TOptional<TArray<FGameVersionType>>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetVersionsDelegate, TOptional<TArray<FGameVersionsByType>>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetVersionsDetailedDelegate, const TOptional<TArray<FGameVersionsDetailedByType>>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetCategoriesDelegate, TOptional<TArray<FCategory>>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_ThreeParams(FSearchModsDelegate, TOptional<TArray<FCFCoreMod>>, TOptional<FCFCoreApiResponsePagination>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModsHighlightsDelegate, TOptional<FModsHighlights>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModsHighlightsV2Delegate, TOptional<FModsHighlightsV2>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModsHighlightsV3Delegate, TOptional<FModsHighlightsV3>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModsDelegate, TOptional<TArray<FCFCoreMod>>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModDelegate, TOptional<FCFCoreMod>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModDescDelegate, TOptional<FString>, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetFilesDelegate, const TOptional<TArray<FFile>>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetDirectDownloadDomainDelegate, const TOptional<FString>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetDirectDownloadDomainExDelegate, const TOptional<FDirectDownloadDomainEx>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FMatchPlatformFilesDelegate, TOptional<TArray<FFile>>, TOptional<FCFCoreApiResponseError>);
  DECLARE_DELEGATE_ThreeParams(FGetModFileDownloadUrlDelegate, const TOptional<FModFileDownloadUrl>&, const FString&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetModFileDeltaDiffDelegate, const TOptional<FFileDeltaDiff>&, const TOptional<FCFCoreApiResponseError>&);
  DECLARE_DELEGATE_TwoParams(FGetActiveCookingVersionDelegate, const TOptional<FSDKVersion>, const TOptional<FCFCoreApiResponseError>&);

public:
  virtual ~ICFCoreApi() = default;

public:
  virtual TSharedPtr<ICFCoreApiAuthentication> Authentication() = 0;

  // APIs that require an auth token
  virtual TSharedPtr<ICFCoreApiAuthorized> Authorized() = 0;

  // Also, APIs that require an auth token
  virtual TSharedPtr<ICFCoreApiCreation> Creation() = 0;

public:
  // ---------------------------------------------------------------------------
  // https://docs.curseforge.com/#get-game
  virtual void GetGame(FGetGameDelegate delegate) = 0;

  // https://docs.curseforge.com/#get-version-types
  virtual void GetVersionTypes(FGetVersionTypesDelegate delegate) = 0;

  // https://docs.curseforge.com/#get-versions
  virtual void GetVersions(FGetVersionsDelegate delegate) = 0;

  // link to come
  virtual void GetVersionsDetailed(FGetVersionsDetailedDelegate delegate) = 0;

  // https://docs.curseforge.com/#get-categories
  virtual void GetCategories(FCFCoreGetCategoriesFilter filter,
                             FGetCategoriesDelegate delegate) = 0;

  // https://docs.curseforge.com/#search-mods
  virtual void SearchMods(FCFCoreSearchModsFilter filter,
                          FCFCoreApiRequestPagination pagination,
                          FSearchModsDelegate delegate) = 0;

  UE_DEPRECATED(4.xx, "This function is deprecated. Use the GetModsHighlightsV3 function instead.")
  virtual void GetModsHighlights(FCFCoreGetModsHighlightsFilter filter,
                                 FGetModsHighlightsDelegate delegate) = 0;

  UE_DEPRECATED(4.xx, "This function is deprecated. Use the GetModsHighlightsV3 function instead.")
  virtual void GetModsHighlightsV2(FCFCoreGetModsHighlightsFilter filter,
                                   FGetModsHighlightsV2Delegate delegate) = 0;

  virtual void GetModsHighlightsV3(FCFCoreGetModsHighlightsFilter filter,
                                   FGetModsHighlightsV3Delegate delegate) = 0;

  // https://docs.curseforge.com/#get-mods
  virtual void GetMods(const TArray<int64>& modIds,
                       FGetModsDelegate delegate) = 0;

  virtual void GetMods(const FCFCoreGetModsFilter& filter,
                       FGetModsDelegate delegate) = 0;

  // https://docs.curseforge.com/#get-mod
  virtual void GetMod(int64 modId, FGetModDelegate delegate) = 0;

  /**
   * Given an array of mod file ids - returns an array of FFile objects.
   *
   * https://docs.curseforge.com/#get-files
   */
  virtual void GetFiles(const FCFCoreGetFilesFilter& filter,
                        FGetFilesDelegate delegate) = 0;

  /**
   * This is used for chunked downloading - and should probably not be used
   * outside of the SDK itself - we expose it because it's part of the API.
   */
  virtual void GetDirectDownloadDomain(
    FGetDirectDownloadDomainDelegate delegate) = 0;

  /**
   * This is used for chunked downloading - and should probably not be used
   * outside of the SDK itself - we expose it because it's part of the API.
   */
  virtual void GetDirectDownloadDomainEx(
    FGetDirectDownloadDomainExDelegate delegate) = 0;

  /**
   * Takes an array of file ids and returns an array of FFile objects that match
   * each of the file ids for your current platform.
   * This is mainly used for client-server game sessions where the server
   * returns a list of file ids and the client has to download the matching
   * files for it's platform
   *
   * @param filter - the filter object that contains the input file ids.
   * @param delegate - the delegate that is triggered when the call ends
   */
  virtual void MatchPlatformFiles(const FMatchPlatformFilesFilter& filter,
                                  FMatchPlatformFilesDelegate delegate) = 0;

  // https://docs.curseforge.com/#get-mod-description
  virtual void GetModDescription(int64 modId,
                                 FGetModDescDelegate delegate) = 0;

  /**
   * For games that support cloud cooking, this will return information about
   * the current version of the game's cloud cooker
   */
  virtual void GetActiveCookingVersion(
    FGetActiveCookingVersionDelegate delegate) = 0;

  /**
   * Get the download url of a mod file - for premium mods, this requires the
   * user to be authenticated.
   *
   * TODO(twolf): Move to ICFCoreApiAuthorized
   */
  virtual void GetModFileDownloadUrl(
    int64 mod_id,
    int64 file_id,
    FGetModFileDownloadUrlDelegate delegate) = 0;

  /**
   * When updating a mod file, there may exist a delta diff file for the update.
   * Use this API to check.
   */
  virtual void GetModFileDeltaDiff(
    int64 mod_id,
    int64 new_file_id,
    int64 old_file_id,
    FGetModFileDeltaDiffDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreApi)