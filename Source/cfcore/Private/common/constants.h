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

namespace cfcore {

extern const TCHAR kApiBaseUrl[];
extern const TCHAR kChunkedUploadServerUrl[];
extern const TCHAR kAnalyticsUrl[];
extern const TCHAR kUserAgentPrefix[];

extern const TCHAR kPlatformUnknown[];

extern const TCHAR kTrue[];
extern const TCHAR kFalse[];

/**
 * /v1/games/{gameId}
 */
extern const TCHAR kApiUrlGetGame[];

/**
 * /v1/games/{gameId}/version-types
 */
extern const TCHAR kApiUrlGetVersionTypes[];

/**
 * /v1/games/{gameId}/versions
 */
extern const TCHAR kApiUrlGetVersions[];

/**
 * /v2/games/{gameId}/versions
 */
extern const TCHAR kApiUrlGetVersionsDetailed[];

/**
 * /v1/categories?{searchParams}
 */
extern const TCHAR kApiUrlGetCategories[];

/**
 * /v1/mods/search?{searchParams}
 */
extern const TCHAR kApiUrlSearchMods[];

/**
 * /v1/mods/highlights
 */
extern const TCHAR kApiUrlPostModsHighlights[];

/**
 * /v2/mods/highlights
 */
extern const TCHAR kApiUrlPostModsHighlightsV2[];

/**
 * /v3/mods/highlights
 */
extern const TCHAR kApiUrlPostModsHighlightsV3[];

/**
 * /v1/mods
 */
extern const TCHAR kApiUrlPostMods[];

/**
 * /v1/mods/{modId}
 */
extern const TCHAR kApiUrlGetMod[];

/**
 * /v1/mods/files
 */
extern const TCHAR kApiUrlPostModFiles[];

/**
 * /v2/mod-files/direct-download-domain
 */
extern const TCHAR kApiUrlGetDirectDownloadDomain[];

/**
 * /v3/mod-files/direct-download-domain
 */
extern const TCHAR kApiUrlGetDirectDownloadDomainV3[];

/**
 * /v1/mods/files/match-platform
 */
extern const TCHAR kApiUrlPostMatchPlatformFiles[];

/**
 * /v1/mods/{modId}/description?raw=true&markup=true
 */
extern const TCHAR kApiUrlGetModDescription[];

/**
 * /v1/auth/terms
 */
extern const TCHAR kApiUrlGetTerms[];

/**
 * /v1/auth/passwordless/init
 */
extern const TCHAR kApiUrlPostSendSecurityCode[];

/**
 * /v1/auth/passwordless/token
 */
extern const TCHAR kApiUrlPostGenAuthToken[];

/**
 * /v1/auth/external
 */
extern const TCHAR kApiUrlPostAuthTokenByExternalProvider[];

/**
 * /v2/mods/{modId}/files/{fileId}/download-url
 */
extern const TCHAR kApiUrlGetModFileDownloadUrl[];

/**
 * /v1/mods/{modId}/files/{newFileId}/deltas/{oldFileId}
 */
extern const TCHAR kApiUrlGetModFileDeltaDiff[];

/**
 * /v2/mods
 */
extern const TCHAR kApiUrlPostCreateMod[];

/**
 * /v2/mods/{modId}
 */
extern const TCHAR kApiUrlPutUpdateMod[];

/**
 * /v2/mods/{modId}/files
 */
extern const TCHAR kApiUrlPostCreateModFile[];

/**
 * /v2/mods/{modId}/files/{fileId}/cooked-chunked-upload
 */
extern const TCHAR kApiUrlPostCreateCookedModFileChunked[];

/**
 * /v2/mods/{modId}/files/chunked-upload
 */
extern const TCHAR kApiUrlPostCreateModFileChunked[];

/**
 * /v1/uploads/chunked-upload
 */
extern const TCHAR kApiUrlPostUploadModFileChunk[];

/**
 * /v2/mods/{modId}/files/{sourceFileId}/cooked
 */
extern const TCHAR kApiUrlPostCreateCookedModFile[];

/**
 * /v1/users/me
 */
extern const TCHAR kApiUrlGetMe[];

/**
 * /v1/users/me/mods
 */
extern const TCHAR kApiUrlGetMyMods[];

/**
 * /v1/users/me/ratings
 */
extern const TCHAR kApiUrlGetMyRatings[];

/**
 * /v1/mods/{modId}/rating/{voteAction}
 */
extern const TCHAR kApiUrlPutUpdateRating[];

/**
 * /v1/mods/{modId}/rating
 */
extern const TCHAR kApiUrlDeleteRemoveRating[];

/**
 * /v1/reporting/reasons
 */
extern const TCHAR kApiUrlGetReportingReasons[];

/**
 * /v2/mods/{modId}/report
 */
extern const TCHAR kApiUrlPostReportMod[];

/**
 * /v1/users/me/generate-temp-token
 */
extern const TCHAR kApiUrlGetGenerateTempToken[];

/**
 * /v1/users/me/blocked-details
 */
extern const TCHAR kApiUrlGetBlockedDetails[];

/**
 * /v1/users/me/unblock
 */
extern const TCHAR kApiUrlPostUnblockMods[];

/**
 * /v1/users/me/premium-mods
 */
extern const TCHAR kApiUrlGetMyPremiumMods[];

/**
 * /v2/users/me/premium-mods
 */
extern const TCHAR kApiUrlGetMyPremiumModsV2[];

/**
 * /v1/users/me/premium-mods/check
 */
extern const TCHAR kApiUrlPostPremiumModsCheck[];

/**
 * /v1/premium-mods/checkout
 */
extern const TCHAR kApiUrlPostGeneratePremiumCheckoutUrl[];

/**
 * /v1/premium-mods/consoles/initiate-purchase
 */
extern const TCHAR kApiUrlPostInitiatePurchase[];

/**
 * /v1/premium-mods/consoles/finalize-purchase
 */
extern const TCHAR kApiUrlPostFinalizePurchase[];

/**
 * /v1/users/me/redeem-code
 */
extern const TCHAR kApiUrlPostEntitlementsRedeemCode[];

/**
 * /v1/users/me/premium-mods/dlc-check
 */
extern const TCHAR kApiUrlPostDlcCheck[];

/**
 * /v1/cooking/sdk-version
 */
extern const TCHAR kApiUrlActiveCookingVersion[];

/**
 * /v1/me/mods/subscriptions
 */
extern const TCHAR kApiUrlGetModsSubscriptions[];

/**
 * /v1/me/mods/subscriptions/ids
 */
extern const TCHAR kApiUrlGetModsSubscriptionsIds[];

/**
 * /v1/me/mods/subscriptions/subscribe/{modId}
 */
extern const TCHAR kApiUrlPostSubscribe[];

/**
 * /v1/me/mods/subscriptions/subscribe
 */
extern const TCHAR kApiUrlPostSubscribeMods[];

/**
 * /v1/me/mods/subscriptions/unsubscribe/{modId}
 */
extern const TCHAR kApiUrlPostUnsubscribe[];

/**
 * /v1/diagnostics/init
 */
extern const TCHAR kApiUrlPostDiagnosticsInit[];

extern const TCHAR kApiActionUpVote[];
extern const TCHAR kApiActionDownVote[];

extern const TCHAR kApiUrlParamsIndex[];
extern const TCHAR kApiUrlParamsPageSize[];
extern const TCHAR kApiUrlParamsGameId[];
extern const TCHAR kApiUrlParamsClassId[];
extern const TCHAR kApiUrlParamsClassesOnly[];
extern const TCHAR kApiUrlParamsCategoryId[];
extern const TCHAR kApiUrlParamsCategoryIds[];
extern const TCHAR kApiUrlParamsGameVersion[];
extern const TCHAR kApiUrlParamsSearchFilter[];
extern const TCHAR kApiUrlParamsSortField[];
extern const TCHAR kApiUrlParamsSortOrder[];
extern const TCHAR kApiUrlParamsModLoaderType[];
extern const TCHAR kApiUrlParamsGameVersionTypeId[];
extern const TCHAR kApiUrlParamsAuthorId[];
extern const TCHAR kApiUrlParamsPrimaryAuthorId[];
extern const TCHAR kApiUrlParamsPremiumType[];
extern const TCHAR kApiUrlParamsModsSearchEnhancedFeatures[];
extern const TCHAR kApiUrlParamsLanguage[];
extern const TCHAR kApiUrlParamsRaw[];
extern const TCHAR kApiUrlParamsMarkup[];

extern const TCHAR kHttpHeaderAuthorization[];
extern const TCHAR kHttpHeaderUserAgent[];
extern const TCHAR kHttpHeaderPlatform[];
extern const TCHAR kHttpHeaderProvider[];
extern const TCHAR kHttpHeaderModServerRequest[];
extern const TCHAR kHttpHeaderApiKey[];
extern const TCHAR kHttpHeaderContentType[];
extern const TCHAR kHttpHeaderContentLength[];
extern const TCHAR kHttpHeaderRange[];

extern const TCHAR kHttpContentTypeApplicationJson[];
extern const TCHAR kHttpContentTypeApplicationZip[];

extern const TCHAR kApiResponseErrorRequestCancelled[];
extern const TCHAR kApiResponseErrorFailedToSendRequest[];
extern const TCHAR kApiResponseErrorUserTokenExpired[];
extern const TCHAR kApiResponseErrorMissingPriviliges[];
extern const TCHAR kApiResponseErrorResourceExpired[];
extern const TCHAR kApiResponseErrorFailedToParseJsonResponse[];

extern const TCHAR kLibraryFilename[];
extern const TCHAR kUserInfoFile[];
extern const TCHAR kSharedInfoFile[];
extern const TCHAR kTempDir[];

extern const float kDefaultHttpFileDownloadTimeInMinutes;
extern const float kDeltaDiffTimeoutInSeconds;

/**
 * Maximum number of mods to subscribe to in one API call.
 * See ICFCoreApiAuthorized::SubscribeMods.
 */
extern const float kMaxSubscriptionsPerBatch;

extern const TCHAR kServerSuffix[];

extern const TCHAR kTrackingOrigin[];
extern const TCHAR kTrackingOriginJoinServer[];

extern const TCHAR kTrackingType[];
extern const TCHAR kTrackingTypeUpdate[];
extern const TCHAR kTrackingTypeFix[];
extern const TCHAR kTrackingTypeOther[];

extern const TCHAR kTrackingDynamicContent[];
extern const TCHAR kTrackingResumableDownload[];

extern const TCHAR kPhasingFeatureDeltaDiffs[];

extern const TCHAR kControlGroup[];
extern const TCHAR kTestGroup[];

extern const TCHAR kPageViewAction[];
extern const TCHAR kPurchaseAction[];

}; // namespace cfcore