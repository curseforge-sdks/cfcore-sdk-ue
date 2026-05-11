/* MIT License

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
SOFTWARE. */
#include "cfcore_api_impl.h"

#include <common/constants.h>
#include "common/api_macros.h"
#include "cfcore_api_delegates.h"

#include <api/models/me_phasing_data.h>

#include <api/common/url_search_params.h>
#include <api/models/serializers/get_categories_filter_serializer.h>
#include <api/models/serializers/search_mods_filter_serializer.h>
#include <api/models/serializers/api_request_pagination_serializer.h>
#include <api/models/serializers/create_mod_request_serializer.h>
#include <api/models/serializers/create_mod_file_request_serializer.h>
#include <api/models/serializers/mod_file_chunk_metadata_serializer.h>
#include <api/models/serializers/create_cooked_mod_file_request_serializer.h>
#include <api/models/serializers/get_auth_token_by_external_provider_request_serializer.h>

#include <api/models/model_responses/game_response.h>
#include <api/models/model_responses/game_version_type_array_response.h>
#include <api/models/model_responses/game_version_array_response.h>
#include <api/models/model_responses/game_versions_detailed_array_response.h>
#include <api/models/model_responses/category_array_response.h>
#include <api/models/model_responses/mod_array_response.h>
#include <api/models/model_responses/file_array_response.h>
#include <api/models/model_responses/mod_id_array_response.h>
#include <api/models/model_responses/mod_response.h>
#include <api/models/model_responses/string_response.h>
#include <api/models/model_responses/auth_token_response.h>
#include <api/models/model_responses/reporting_reasons_response.h>
#include <api/models/model_responses/me_response.h>
#include <api/models/model_responses/my_ratings_response.h>
#include <api/models/model_responses/terms_response.h>
#include <api/models/model_responses/create_mod_file_response.h>
#include <api/models/model_responses/create_mod_file_chunked_response.h>
#include <api/models/model_responses/upload_mod_file_chunk_response.h>
#include <api/models/model_responses/generate_temp_token_response.h>
#include <api/models/model_responses/get_blocked_details_response.h>
#include <api/models/model_responses/sdk_version_response.h>
#include <api/models/model_responses/premium_mods_check_request_response.h>
#include <api/models/model_responses/generate_premium_checkout_url_response.h>
#include <api/models/model_responses/get_mod_file_download_url_response.h>
#include <api/models/model_responses/get_mod_file_delta_diff_response.h>
#include <api/models/model_responses/direct_download_domain_ex_response.h>
#include <api/models/model_responses/get_mods_highlights_response.h>
#include <api/models/model_responses/get_mods_highlights_v2_response.h>
#include <api/models/model_responses/get_mods_highlights_v3_response.h>
#include <api/models/model_responses/initiate_purchase_response.h>

#include "models/dtos/get_mods_by_ids_request_dto.h"
#include "models/dtos/get_files_by_ids_request_dto.h"
#include "models/dtos/send_security_code_request_dto.h"
#include "models/dtos/get_auth_token_request_dto.h"
#include "models/dtos/get_auth_token_by_external_provider_request_dto.h"
#include "models/dtos/report_mod_request_dto.h"
#include "models/dtos/premium_mods_check_request_dto.h"
#include "models/dtos/get_mods_highlights_request_dto.h"

#include <JsonUtilities.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
// static
CFCoreApiImpl::CFCoreApiImpl(
  TSharedPtr<ISettingsService> settings_service,
  TSharedPtr<IApiHelperService> api_helper_service,
  TSharedPtr<ICFCoreApiAuthorized> cfcore_api_authorized,
  TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher,
  TSharedPtr<IFeaturePhasingService> feature_phasing_service) :
  settings_service_(settings_service),
  api_helper_service_(api_helper_service),
  cfcore_api_authorized_(cfcore_api_authorized),
  api_delegates_dispatcher_(api_delegates_dispatcher),
  feature_phasing_service_(feature_phasing_service) {

  settings_service->OnSettingsUpdated().AddLambda([this]() {
    InitializeSettings();
  });

  InitializeSettings();
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetGame(FGetGameDelegate delegate) {
  FString url = FString::Format(kApiUrlGetGame, {
    api_base_url_,
    settings_service_->GetSettings().gameId
  });

  auto handler = CREATE_RESPONSE_HANDLER(FGameResponse,
                                         FGetGameDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetVersionTypes(FGetVersionTypesDelegate delegate) {
  FString url = FString::Format(kApiUrlGetVersionTypes, {
    api_base_url_,
    settings_service_->GetSettings().gameId
  });

  auto handler = CREATE_RESPONSE_HANDLER(FGameVersionTypeArrayResponse,
                                         FGetVersionTypesDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetVersions(FGetVersionsDelegate delegate) {
  FString url = FString::Format(kApiUrlGetVersions, {
    api_base_url_,
    settings_service_->GetSettings().gameId,
  });

  auto handler = CREATE_RESPONSE_HANDLER(FGameVersionByTypeArrayResponse,
                                         FGetVersionsDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetVersionsDetailed(FGetVersionsDetailedDelegate delegate) {
  FString url = FString::Format(kApiUrlGetVersionsDetailed, {
    api_base_url_,
    settings_service_->GetSettings().gameId
  });

  auto handler = CREATE_RESPONSE_HANDLER(
    FGameVersionsDetailedByTypeArrayResponse,
    FGetVersionsDetailedDelegate,
    delegate);
  api_helper_service_->PerformGet(url, handler);
}


// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetCategories(FCFCoreGetCategoriesFilter filter,
                                  FGetCategoriesDelegate delegate) {
  UrlSearchParams search_params;
  GetCategoriesFilterSerializer::Serialize(filter, search_params);

  search_params.Append(kApiUrlParamsGameId,
                       settings_service_->GetSettings().gameId);

  FString url = FString::Format(kApiUrlGetCategories, {
    api_base_url_,
    search_params.ToString()
  });

  auto handler = CREATE_RESPONSE_HANDLER(FCategoryArrayResponse,
                                         FGetCategoriesDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::SearchMods(FCFCoreSearchModsFilter filter,
                               FCFCoreApiRequestPagination pagination,
                               FSearchModsDelegate delegate) {
  UrlSearchParams search_params;
  SearchModsFilterSerializer::Serialize(filter, search_params);
  ApiRequestPaginationSerializer::Serialize(pagination, search_params);

  search_params.Append(kApiUrlParamsGameId,
                       settings_service_->GetSettings().gameId);

  FString url = FString::Format(kApiUrlSearchMods, {
    api_base_url_,
    search_params.ToString()
  });

  auto handler = CREATE_RESPONSE_HANDLER_WITH_PAGINATION(
    FModArrayResponse,
    FSearchModsDelegate,
    delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetModsHighlights(FCFCoreGetModsHighlightsFilter filter,
                                      FGetModsHighlightsDelegate delegate) {

  FString url = FString::Format(kApiUrlPostModsHighlights, { api_base_url_ });

  FGetModsHighlightsRequestDto dto;
  dto.gameId = settings_service_->GetSettings().gameId;
  dto.excludedModIds = filter.excludedModIds;
  dto.filters = filter.filters;

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FGetModsHighlightsResponse,
                                         FGetModsHighlightsDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetModsHighlightsV2(FCFCoreGetModsHighlightsFilter filter,
                                        FGetModsHighlightsV2Delegate delegate) {

  FString url = FString::Format(kApiUrlPostModsHighlightsV2, { api_base_url_ });

  FGetModsHighlightsRequestDto dto;
  dto.gameId = settings_service_->GetSettings().gameId;
  dto.excludedModIds = filter.excludedModIds;
  dto.filters = filter.filters;

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FGetModsHighlightsV2Response,
                                         FGetModsHighlightsV2Delegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetModsHighlightsV3(FCFCoreGetModsHighlightsFilter filter,
                                        FGetModsHighlightsV3Delegate delegate) {

  FString url = FString::Format(kApiUrlPostModsHighlightsV3, { api_base_url_ });

  FGetModsHighlightsRequestDto dto;
  dto.gameId = settings_service_->GetSettings().gameId;
  dto.excludedModIds = filter.excludedModIds;
  dto.filters = filter.filters;

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FGetModsHighlightsV3Response,
                                         FGetModsHighlightsV3Delegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetMods(const TArray<int64>& mod_ids,
                            FGetModsDelegate delegate) {

  FString url = FString::Format(kApiUrlPostMods, { api_base_url_ });

  FGetModsByIdsRequestDto dto{ mod_ids };
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FModArrayResponse,
                                         FGetModsDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetMods(const FCFCoreGetModsFilter& filter,
                            FGetModsDelegate delegate) {

  FString url = FString::Format(kApiUrlPostMods, { api_base_url_ });

  FGetModsByIdsRequestDto dto;
  dto.modIds = filter.modIds;
  dto.devModIds = filter.devModIds;
  dto.filterPcOnly = filter.filterPcOnly;
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FModArrayResponse,
                                         FGetModsDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);

}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetMod(int64 mod_id, FGetModDelegate delegate) {
  FString url = FString::Format(kApiUrlGetMod, { api_base_url_, mod_id });

  auto handler = CREATE_RESPONSE_HANDLER(
    FModResponse,
    FGetModDelegate,
    FGetModDelegate::CreateLambda([this, delegate](
      const TOptional<FCFCoreMod>& mod,
      const TOptional<FCFCoreApiResponseError>& error) {

      for (auto api_delegate : api_delegates_dispatcher_->Delegates()) {
        api_delegate->get_mod.ExecuteIfBound(mod, error);
      }

      delegate.ExecuteIfBound(mod, error);
    })
  );
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetFiles(const FCFCoreGetFilesFilter& filter,
                             FGetFilesDelegate delegate) {

  FString url = FString::Format(kApiUrlPostModFiles, { api_base_url_ });

  FGetFilesByIdsRequestDto dto{ filter.fileIds };
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FFileArrayResponse,
                                         FGetFilesDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetDirectDownloadDomain(
  FGetDirectDownloadDomainDelegate delegate) {

  FString url = FString::Format(kApiUrlGetDirectDownloadDomain, {
    api_base_url_,
  });

  auto handler = CREATE_RESPONSE_HANDLER(FStringResponse,
                                         FGetDirectDownloadDomainDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetDirectDownloadDomainEx(
  FGetDirectDownloadDomainExDelegate delegate) {

  FString url = FString::Format(kApiUrlGetDirectDownloadDomainV3, {
    api_base_url_,
  });

  auto handler = CREATE_RESPONSE_HANDLER(FDirectDownloadDomainExResponse,
                                         FGetDirectDownloadDomainExDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::MatchPlatformFiles(const FMatchPlatformFilesFilter& filter,
                                       FMatchPlatformFilesDelegate delegate) {

  FString url = FString::Format(kApiUrlPostMatchPlatformFiles, {
    api_base_url_,
  });

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(filter, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FFileArrayResponse,
                                         FMatchPlatformFilesDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetModDescription(int64 mod_id,
                                      FGetModDescDelegate delegate) {

  UrlSearchParams search_params;

  search_params.Append(kApiUrlParamsRaw, kTrue);
  search_params.Append(kApiUrlParamsMarkup, kTrue);

  AppendLanguageParamIfEnabled(search_params);

  FString url = FString::Format(kApiUrlGetModDescription, {
    api_base_url_,
    mod_id,
    search_params.ToString()
  });

  auto handler = CREATE_RESPONSE_HANDLER(
    FStringResponse,
    FGetModDescDelegate,
    FGetModDescDelegate::CreateLambda([this, delegate](
      const TOptional<FString>& description,
      const TOptional<FCFCoreApiResponseError>& error) {

      for (auto api_delegate : api_delegates_dispatcher_->Delegates()) {
        api_delegate->get_mod_description.ExecuteIfBound(description, error);
      }

      delegate.ExecuteIfBound(description, error);
    })
  );
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetActiveCookingVersion(
  FGetActiveCookingVersionDelegate delegate) {

  FString url = FString::Format(kApiUrlActiveCookingVersion, {
    api_base_url_
  });

  auto handler = CREATE_RESPONSE_HANDLER(FSDKVersionResponse,
                                         FGetActiveCookingVersionDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetTerms(FGetTermsDelegate delegate) {
  FString url = FString::Format(kApiUrlGetTerms, { api_base_url_ });

  FString lang = settings_service_->GetSettings().defaultLanguage;
  if (lang.Len() > 0) {
    url = FString::Format(TEXT("{0}?lang={1}"), {
      url,
      lang
    });
  }

  auto handler = CREATE_RESPONSE_HANDLER(FTermsResponse,
                                         FGetTermsDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::SendSecurityCode(const FString& email,
                                     FSendSecurityCodeDelegate delegate) {

  FString url = FString::Format(kApiUrlPostSendSecurityCode, {
    api_base_url_
  });

  FSendSecurityCodeRequestDto dto;
  dto.email = email;
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_EMPTY_RESPONSE_HANDLER(FSendSecurityCodeDelegate,
                                               delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetAuthToken(const FString& email,
                                 int32 otp,
                                 FGetAuthTokenDelegate delegate) {

  FString url = FString::Format(kApiUrlPostGenAuthToken, { api_base_url_ });

  FGetAuthTokenRequestDto dto;
  dto.email = email;
  dto.otp = otp;
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, json_data);

  auto handler = CREATE_RESPONSE_HANDLER(FAuthTokenResponse,
                                         FGetAuthTokenDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, json_data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetAuthTokenByExternalProvider(
  ECFCoreExternalAuthProvider provider,
  const FString& external_token,
  const FExternalAuthAdditionalInfo& additional_info,
  FGetAuthTokenDelegate delegate) {

  FString url = FString::Format(kApiUrlPostAuthTokenByExternalProvider, {
    api_base_url_
  });

  FGetAuthTokenByExternalProviderRequestDto dto;
  GetAuthTokenByExternalProviderRequestSerializer::Serialize(provider,
                                                             external_token,
                                                             additional_info,
                                                             dto);
  FString data;
  FJsonObjectConverter::UStructToJsonObjectString(dto, data);

  auto handler = CREATE_RESPONSE_HANDLER(FAuthTokenResponse,
                                         FGetAuthTokenDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, data, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetModFileDownloadUrl(
  int64 mod_id,
  int64 file_id,
  FGetModFileDownloadUrlDelegate delegate) {

  FString url = FString::Format(kApiUrlGetModFileDownloadUrl, {
    api_base_url_,
    mod_id,
    file_id,
  });

  auto handler = CREATE_RESPONSE_HANDLER_WITH_SIGNATURE(
    FGetModFileDownloadUrlResponse,
    FGetModFileDownloadUrlDelegate,
    delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::GetModFileDeltaDiff(
  int64 mod_id,
  int64 new_file_id,
  int64 old_file_id,
  FGetModFileDeltaDiffDelegate delegate) {

  FString url = FString::Format(kApiUrlGetModFileDeltaDiff, {
    api_base_url_,
    mod_id,
    new_file_id,
    old_file_id
  });

  auto handler = CREATE_RESPONSE_HANDLER(FGetModFileDeltaDiffResponse,
                                         FGetModFileDeltaDiffDelegate,
                                         delegate);
  api_helper_service_->PerformGet(url, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::CreateMod(FCreateModRequest create_mod_request,
                              TSharedPtr<TArray<uint8>> avatar_image_data,
                              FCreateModDelegate delegate) {

  FString url = FString::Format(kApiUrlPostCreateMod, { api_base_url_ });

  WWWForm www_form;
  CreateModRequestSerializer::Serialize(create_mod_request, www_form);

  if (avatar_image_data.IsValid() && avatar_image_data->Num() > 0) {
    www_form.AddBinaryData("file", MoveTemp(avatar_image_data));
  }

  auto handler = CREATE_RESPONSE_HANDLER(FModResponse,
                                         FCreateModDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, www_form, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::UpdateMod(int64 mod_id,
                              FUpdateModRequest update_mod_request,
                              TSharedPtr<TArray<uint8>> avatar_image_data,
                              FUpdateModDelegate delegate) {

  FString url = FString::Format(kApiUrlPutUpdateMod, { api_base_url_, mod_id });

  WWWForm www_form;
  CreateModRequestSerializer::Serialize(update_mod_request, www_form);

  if (avatar_image_data.IsValid() && avatar_image_data->Num() > 0) {
    www_form.AddBinaryData("file", MoveTemp(avatar_image_data));
  }

  auto handler = CREATE_RESPONSE_HANDLER(FModResponse,
                                         FUpdateModDelegate,
                                         delegate);
  api_helper_service_->PerformPut(url, www_form, handler);
}

// -----------------------------------------------------------------------------
int64 CFCoreApiImpl::CreateModFile(
  int64 mod_id,
  FCreateModFileRequest create_mod_file_request,
  TSharedPtr<TArray<uint8>> file_data,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FCreateModFileDelegate delegate) {

  FString url = FString::Format(kApiUrlPostCreateModFile, {
    api_base_url_,
    mod_id
  });

  WWWForm www_form;
  CreateModFileRequestSerializer::Serialize(create_mod_file_request, www_form);
  www_form.AddBinaryData("file", file_data);

  auto handler = CREATE_RESPONSE_HANDLER_WITH_PROGRESS(
    FCreateModFileResponse,
    FCreateModFileDelegate,
    delegate,
    file_transfer_progress_delegate);

  return api_helper_service_->PerformPost(url, www_form, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::CreateCookedModFileChunked(
  int64 mod_id,
  int64 source_file_id,
  FCreateCookedModFileRequest create_cooked_mod_file_request,
  FCreateModFileChunkedDelegate delegate) {

  FString url = FString::Format(kApiUrlPostCreateCookedModFileChunked, {
      api_base_url_,
      mod_id,
      source_file_id,
    });

  WWWForm www_form;
  CreateCookedModFileRequestSerializer::Serialize(
    create_cooked_mod_file_request,
    www_form);

  auto handler = CREATE_RESPONSE_HANDLER(FCreateModFileChunkedResponse,
                                         FCreateModFileChunkedDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, www_form, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::CreateModFileChunked(
  int64 mod_id,
  FCreateModFileRequest create_mod_file_request,
  FCreateModFileChunkedDelegate delegate) {

  FString url = FString::Format(kApiUrlPostCreateModFileChunked, {
    api_base_url_,
    mod_id
  });

  WWWForm www_form;
  CreateModFileRequestSerializer::Serialize(create_mod_file_request, www_form);

  auto handler = CREATE_RESPONSE_HANDLER(FCreateModFileChunkedResponse,
                                         FCreateModFileChunkedDelegate,
                                         delegate);
  api_helper_service_->PerformPost(url, www_form, handler);
}

// -----------------------------------------------------------------------------
int64 CFCoreApiImpl::UploadModFileChunk(
  const FModFileChunkMetadata& chunk_metadata,
  TSharedPtr<TArray<uint8>> chunk_data,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FUploadModFileChunkDelegate delegate) {

  FString url = FString::Format(kApiUrlPostUploadModFileChunk, {
    chunked_upload_server_url_
  });

  WWWForm www_form;
  ModFileChunkMetadataSerializer::Serialize(chunk_metadata, www_form);
  www_form.AddBinaryData("file", chunk_data);

  auto handler = CREATE_RESPONSE_HANDLER_WITH_PROGRESS(
    FUploadModFileChunkResponse,
    FUploadModFileChunkDelegate,
    delegate,
    file_transfer_progress_delegate);

  return api_helper_service_->PerformPost(url, www_form, handler);
}

// -----------------------------------------------------------------------------
int64 CFCoreApiImpl::CreateCookedModFile(
  int64 mod_id,
  int64 source_file_id,
  FCreateCookedModFileRequest create_cooked_mod_file_request,
  TSharedPtr<TArray<uint8>> file_data,
  FFileTransferProgressDelegate file_transfer_progress_delegate,
  FCreateModFileDelegate delegate) {

  FString url = FString::Format(kApiUrlPostCreateCookedModFile, {
    api_base_url_,
    mod_id,
    source_file_id
  });

  WWWForm www_form;
  CreateCookedModFileRequestSerializer::Serialize(
    create_cooked_mod_file_request,
    www_form);
  www_form.AddBinaryData("file", MoveTemp(file_data));

  auto handler = CREATE_RESPONSE_HANDLER_WITH_PROGRESS(
    FCreateModFileResponse,
    FCreateModFileDelegate,
    delegate,
    file_transfer_progress_delegate);

  return api_helper_service_->PerformPost(url, www_form, handler);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::CancelCreation(int64 cancel_id) {
  api_helper_service_->CancelFileOperation(cancel_id);
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::InitializeSettings() {
  api_base_url_ = settings_service_->GetInternalSettings().apiBaseUrl;
  chunked_upload_server_url_ =
    settings_service_->GetInternalSettings().chunkedUploadServerUrl;
}

// -----------------------------------------------------------------------------
void CFCoreApiImpl::AppendLanguageParamIfEnabled(
  UrlSearchParams& search_params) {

  const FString& language = settings_service_->GetSettings().defaultLanguage;
  bool use_translations = feature_phasing_service_->IsFeatureEnabledForUser(
    &FMePhasingData::useTranslations);

  if (language.Len() > 0 && use_translations) {
    search_params.Append(kApiUrlParamsLanguage, language);
  }
}
