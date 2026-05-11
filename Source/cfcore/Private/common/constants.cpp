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
#include "constants.h"

namespace cfcore {

const TCHAR kApiBaseUrl[] = TEXT("https://{0}.api.curseforge.com");
const TCHAR kChunkedUploadServerUrl[] = TEXT("https://chunk-uploads.curseforge.com");
const TCHAR kAnalyticsUrl[] = TEXT("https://analyticsnew.overwolf.com/analytics/Counter");
const TCHAR kUserAgentPrefix[] = TEXT("cfcore.ue");

const TCHAR kPlatformUnknown[] = TEXT("unknown");

const TCHAR kTrue[] = TEXT("true");
const TCHAR kFalse[] = TEXT("false");

const TCHAR kApiUrlGetGame[] = TEXT("{0}/v1/games/{1}");
const TCHAR kApiUrlGetVersionTypes[] = TEXT("{0}/v1/games/{1}/version-types");
const TCHAR kApiUrlGetVersions[] = TEXT("{0}/v1/games/{1}/versions");
const TCHAR kApiUrlGetVersionsDetailed[] = TEXT("{0}/v2/games/{1}/versions");
const TCHAR kApiUrlGetCategories[] = TEXT("{0}/v1/categories?{1}");
const TCHAR kApiUrlSearchMods[] = TEXT("{0}/v1/mods/search?{1}");
const TCHAR kApiUrlPostModsHighlights[] = TEXT("{0}/v1/mods/highlights");
const TCHAR kApiUrlPostModsHighlightsV2[] = TEXT("{0}/v2/mods/highlights");
const TCHAR kApiUrlPostModsHighlightsV3[] = TEXT("{0}/v3/mods/highlights");
const TCHAR kApiUrlPostMods[] = TEXT("{0}/v1/mods");
const TCHAR kApiUrlGetMod[] = TEXT("{0}/v1/mods/{1}");
const TCHAR kApiUrlPostModFiles[] = TEXT("{0}/v1/mods/files");
const TCHAR kApiUrlGetDirectDownloadDomain[] = TEXT("{0}/v2/mod-files/direct-download-domain");
const TCHAR kApiUrlGetDirectDownloadDomainV3[] = TEXT("{0}/v3/mod-files/direct-download-domain");
const TCHAR kApiUrlPostMatchPlatformFiles[] = TEXT("{0}/v1/mods/files/match-platform");

// TODO(apliscov): remove raw and markup params, use UrlSearchParams instead
const TCHAR kApiUrlGetModDescription[] = TEXT("{0}/v1/mods/{1}/description?{2}");
const TCHAR kApiUrlGetTerms[] = TEXT("{0}/v1/auth/terms");
const TCHAR kApiUrlPostSendSecurityCode[] = TEXT("{0}/v1/auth/passwordless/init");
const TCHAR kApiUrlPostGenAuthToken[] = TEXT("{0}/v1/auth/passwordless/token");
const TCHAR kApiUrlPostAuthTokenByExternalProvider[] = TEXT("{0}/v1/auth/external");
const TCHAR kApiUrlGetModFileDownloadUrl[] = TEXT("{0}/v2/mods/{1}/files/{2}/download-url");
const TCHAR kApiUrlGetModFileDeltaDiff[] = TEXT("{0}/v1/mods/{1}/files/{2}/deltas/{3}");
const TCHAR kApiUrlPostCreateMod[] = TEXT("{0}/v2/mods");
const TCHAR kApiUrlPutUpdateMod[] = TEXT("{0}/v2/mods/{1}");
const TCHAR kApiUrlPostCreateModFile[] = TEXT("{0}/v2/mods/{1}/files");
const TCHAR kApiUrlPostCreateCookedModFileChunked[] = TEXT("{0}/v2/mods/{1}/files/{2}/cooked-chunked-upload");
const TCHAR kApiUrlPostCreateModFileChunked[] = TEXT("{0}/v2/mods/{1}/files/chunked-upload");
const TCHAR kApiUrlPostUploadModFileChunk[] = TEXT("{0}/v1/uploads/chunked-upload");
const TCHAR kApiUrlPostCreateCookedModFile[] = TEXT("{0}/v2/mods/{1}/files/{2}/cooked");
const TCHAR kApiUrlGetMe[] = TEXT("{0}/v1/users/me");
const TCHAR kApiUrlGetMyMods[] = TEXT("{0}/v1/users/me/mods");
const TCHAR kApiUrlGetMyRatings[] = TEXT("{0}/v1/users/me/ratings");
const TCHAR kApiUrlPutUpdateRating[] = TEXT("{0}/v1/mods/{1}/rating/{2}");
const TCHAR kApiUrlDeleteRemoveRating[] = TEXT("{0}/v1/mods/{1}/rating");
const TCHAR kApiUrlGetReportingReasons[] = TEXT("{0}/v1/reporting/reasons");
const TCHAR kApiUrlPostReportMod[] = TEXT("{0}/v2/mods/{1}/report");
const TCHAR kApiUrlGetGenerateTempToken[] = TEXT("{0}/v1/users/me/generate-temp-token");
const TCHAR kApiUrlGetBlockedDetails[] = TEXT("{0}/v1/users/me/blocked-details");
const TCHAR kApiUrlPostUnblockMods[] = TEXT("{0}/v1/users/me/unblock");
const TCHAR kApiUrlGetMyPremiumMods[] = TEXT("{0}/v1/users/me/premium-mods");
const TCHAR kApiUrlGetMyPremiumModsV2[] = TEXT("{0}/v2/users/me/premium-mods");
const TCHAR kApiUrlPostPremiumModsCheck[] = TEXT("{0}/v1/users/me/premium-mods/check");
const TCHAR kApiUrlPostGeneratePremiumCheckoutUrl[] = TEXT("{0}/v1/premium-mods/checkout");
const TCHAR kApiUrlPostInitiatePurchase[] = TEXT("{0}/v1/premium-mods/consoles/initiate-purchase");
const TCHAR kApiUrlPostFinalizePurchase[] = TEXT("{0}/v1/premium-mods/consoles/finalize-purchase");
const TCHAR kApiUrlPostEntitlementsRedeemCode[] = TEXT("{0}/v1/users/me/redeem-code");
const TCHAR kApiUrlPostDlcCheck[] = TEXT("{0}/v1/users/me/premium-mods/dlc-check");
const TCHAR kApiUrlActiveCookingVersion[] = TEXT("{0}/v1/cooking/sdk-version");
const TCHAR kApiUrlGetModsSubscriptions[] = TEXT("{0}/v1/users/me/mods/subscriptions");
const TCHAR kApiUrlGetModsSubscriptionsIds[] = TEXT("{0}/v1/users/me/mods/subscriptions/ids");
const TCHAR kApiUrlPostSubscribe[] = TEXT("{0}/v1/users/me/mods/subscriptions/subscribe/{1}");
const TCHAR kApiUrlPostSubscribeMods[] = TEXT("{0}/v1/users/me/mods/subscriptions/subscribe");
const TCHAR kApiUrlPostUnsubscribe[] = TEXT("{0}/v1/users/me/mods/subscriptions/unsubscribe/{1}");
const TCHAR kApiUrlPostDiagnosticsInit[] = TEXT("{0}/v1/diagnostics/init");

const TCHAR kApiActionUpVote[] = TEXT("up-vote");
const TCHAR kApiActionDownVote[] = TEXT("down-vote");

const TCHAR kApiUrlParamsIndex[] = TEXT("index");
const TCHAR kApiUrlParamsPageSize[] = TEXT("pageSize");
const TCHAR kApiUrlParamsGameId[] = TEXT("gameId");
const TCHAR kApiUrlParamsClassId[] = TEXT("classId");
const TCHAR kApiUrlParamsClassesOnly[] = TEXT("classesOnly");
const TCHAR kApiUrlParamsCategoryId[] = TEXT("categoryId");
const TCHAR kApiUrlParamsCategoryIds[] = TEXT("categoryIds");
const TCHAR kApiUrlParamsGameVersion[] = TEXT("gameVersion");
const TCHAR kApiUrlParamsSearchFilter[] = TEXT("searchFilter");
const TCHAR kApiUrlParamsSortField[] = TEXT("sortField");
const TCHAR kApiUrlParamsSortOrder[] = TEXT("sortOrder");
const TCHAR kApiUrlParamsModLoaderType[] = TEXT("modLoaderType");
const TCHAR kApiUrlParamsGameVersionTypeId[] = TEXT("gameVersionTypeId");
const TCHAR kApiUrlParamsAuthorId[] = TEXT("authorId");
const TCHAR kApiUrlParamsPrimaryAuthorId[] = TEXT("primaryAuthorId");
const TCHAR kApiUrlParamsPremiumType[] = TEXT("premiumType");
const TCHAR kApiUrlParamsModsSearchEnhancedFeatures[] =
  TEXT("modsSearchEnhancedFeatures");
const TCHAR kApiUrlParamsLanguage[] = TEXT("lang");
const TCHAR kApiUrlParamsRaw[] = TEXT("raw");
const TCHAR kApiUrlParamsMarkup[] = TEXT("markup");

const TCHAR kHttpHeaderAuthorization[] = TEXT("Authorization");
const TCHAR kHttpHeaderUserAgent[] = TEXT("User-Agent");
const TCHAR kHttpHeaderPlatform[] = TEXT("x-platform");
const TCHAR kHttpHeaderProvider[] = TEXT("x-provider");
const TCHAR kHttpHeaderModServerRequest[] = TEXT("x-mod-server-request");
const TCHAR kHttpHeaderApiKey[] = TEXT("x-api-key");
const TCHAR kHttpHeaderContentType[] = TEXT("Content-Type");
const TCHAR kHttpHeaderContentLength[] = TEXT("Content-Length");
const TCHAR kHttpHeaderRange[] = TEXT("Range");

const TCHAR kHttpContentTypeApplicationJson[] = TEXT("application/json");
const TCHAR kHttpContentTypeApplicationZip[] = TEXT("application/zip");

const TCHAR kApiResponseErrorRequestCancelled[] = TEXT("Request Cancelled");
const TCHAR kApiResponseErrorFailedToSendRequest[] =
  TEXT("Failed to send request to server with error: {0}");
const TCHAR kApiResponseErrorUserTokenExpired[] =
  TEXT("User token has expired");
const TCHAR kApiResponseErrorMissingPriviliges[] =
  TEXT("Missing user token or api key missing priviliges to access end point");
const TCHAR kApiResponseErrorResourceExpired[] = TEXT("The resource expired");
const TCHAR kApiResponseErrorFailedToParseJsonResponse[] =
  TEXT("Failed to parse JSON response");

const TCHAR kLibraryFilename[] = TEXT("library.json");
const TCHAR kUserInfoFile[] = TEXT("user_info.json");
const TCHAR kSharedInfoFile[] = TEXT("shared_info.json");
const TCHAR kTempDir[] = TEXT(".temp");

const float kDefaultHttpFileDownloadTimeInMinutes = 20;
const float kDeltaDiffTimeoutInSeconds = 300;
const float kMaxSubscriptionsPerBatch = 50;

const TCHAR kServerSuffix[] = TEXT("server");

const TCHAR kTrackingOrigin[] = TEXT("origin");
const TCHAR kTrackingOriginJoinServer[] = TEXT("join_srv");

const TCHAR kTrackingType[] = TEXT("type");
const TCHAR kTrackingTypeUpdate[] = TEXT("update");
const TCHAR kTrackingTypeFix[] = TEXT("fix");
const TCHAR kTrackingTypeOther[] = TEXT("other");

const TCHAR kTrackingDynamicContent[] = TEXT("dc");
const TCHAR kTrackingResumableDownload[] = TEXT("resume");

const TCHAR kPhasingFeatureDeltaDiffs[] = TEXT("deltaDiffs");

const TCHAR kControlGroup[] = TEXT("control");
const TCHAR kTestGroup[] = TEXT("test");

const TCHAR kPageViewAction[] = TEXT("pageView");
const TCHAR kPurchaseAction[] = TEXT("purchase");

}; // namespace cfcore
