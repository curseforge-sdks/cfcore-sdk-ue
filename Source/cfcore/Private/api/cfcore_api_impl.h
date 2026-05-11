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
#include <common/macros.h>

#include <api/cfcore_api.h>
#include <api/cfcore_api_authentication.h>
#include <api/cfcore_api_authorized.h>
#include <api/cfcore_api_creation.h>

#include <api/services/api_helper_service.h>
#include <api/services/feature_phasing_service.h>
#include <services/settings_service.h>
#include "cfcore_api_delegates_dispatcher.h"
#include "cfcore_api_delegates.h"

namespace cfcore {

using ApiDelegatesDispatcher = CFCoreApiDelegatesDispatcher<CFCoreApiDelegates>;

class UrlSearchParams;

// -----------------------------------------------------------------------------
class CFCoreApiImpl : public ICFCoreApi,
                      public ICFCoreApiAuthentication,
                      public ICFCoreApiCreation,
                      public TSharedFromThis<CFCoreApiImpl> {
public:
CFCoreApiImpl(TSharedPtr<ISettingsService> settings_service,
              TSharedPtr<IApiHelperService> api_helper_service,
              TSharedPtr<ICFCoreApiAuthorized> cfcore_api_authorized,
              TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher,
              TSharedPtr<IFeaturePhasingService> feature_phasing_service);

// -----------------------------------------------------------------------------
// ICFCoreApi
public:
  virtual TSharedPtr<ICFCoreApiAuthentication> Authentication() override {
    return this->AsShared();
  }

  virtual TSharedPtr<ICFCoreApiAuthorized> Authorized() override {
    return cfcore_api_authorized_;
  }

  virtual TSharedPtr<ICFCoreApiCreation> Creation() override {
    return this->AsShared();
  }

public:
  virtual void GetGame(FGetGameDelegate delegate) override;

  virtual void GetVersionTypes(FGetVersionTypesDelegate delegate) override;

  virtual void GetVersions(FGetVersionsDelegate delegate) override;

  virtual void GetVersionsDetailed(
    FGetVersionsDetailedDelegate delegate) override;

  virtual void GetCategories(FCFCoreGetCategoriesFilter filter,
                             FGetCategoriesDelegate delegate) override;

  virtual void SearchMods(FCFCoreSearchModsFilter filter,
                          FCFCoreApiRequestPagination pagination,
                          FSearchModsDelegate delegate) override;

  virtual void GetModsHighlights(FCFCoreGetModsHighlightsFilter filter,
                                 FGetModsHighlightsDelegate delegate) override;

  virtual void GetModsHighlightsV2(
    FCFCoreGetModsHighlightsFilter filter,
    FGetModsHighlightsV2Delegate delegate) override;

  virtual void GetModsHighlightsV3(
    FCFCoreGetModsHighlightsFilter filter,
    FGetModsHighlightsV3Delegate delegate) override;

  virtual void GetMods(const TArray<int64>& mod_ids,
                       FGetModsDelegate delegate) override;

  virtual void GetMods(const FCFCoreGetModsFilter& filter,
                       FGetModsDelegate delegate) override;

  virtual void GetMod(int64 mod_id, FGetModDelegate delegate) override;

  virtual void GetFiles(const FCFCoreGetFilesFilter& filter,
                        FGetFilesDelegate delegate) override;

  virtual void GetDirectDownloadDomain(
    FGetDirectDownloadDomainDelegate delegate) override;

  virtual void GetDirectDownloadDomainEx(
    FGetDirectDownloadDomainExDelegate delegate) override;

  virtual void MatchPlatformFiles(const FMatchPlatformFilesFilter& filter,
                                  FMatchPlatformFilesDelegate delegate) override;

  virtual void GetModDescription(int64 mod_id,
                                 FGetModDescDelegate delegate) override;

  virtual void GetActiveCookingVersion(FGetActiveCookingVersionDelegate delegate) override;
  virtual void GetModFileDownloadUrl(
    int64 mod_id,
    int64 file_id,
    FGetModFileDownloadUrlDelegate delegate) override;

  virtual void GetModFileDeltaDiff(
    int64 mod_id,
    int64 new_file_id,
    int64 old_file_id,
    FGetModFileDeltaDiffDelegate delegate) override;

// -----------------------------------------------------------------------------
// ICFCoreApiAuthentication
public:
  virtual void GetTerms(FGetTermsDelegate delegate) override;

  virtual void SendSecurityCode(const FString& email,
                                FSendSecurityCodeDelegate delegate) override;

  virtual void GetAuthToken(const FString& email,
                            int32 otp,
                            FGetAuthTokenDelegate delegate) override;

  virtual void GetAuthTokenByExternalProvider(
    ECFCoreExternalAuthProvider provider,
    const FString& external_token,
    const FExternalAuthAdditionalInfo& additional_info,
    FGetAuthTokenDelegate delegate) override;

// -----------------------------------------------------------------------------
// ICFCoreApiCreation
  virtual void CreateMod(FCreateModRequest create_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FCreateModDelegate delegate) override;

  virtual void UpdateMod(int64 mod_id,
                         FUpdateModRequest update_mod_request,
                         TSharedPtr<TArray<uint8>> avatar_image_data,
                         FUpdateModDelegate delegate) override;

  virtual int64 CreateModFile(
    int64 mod_id,
    FCreateModFileRequest create_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) override;

  virtual void CreateModFileChunked(
    int64 mod_id,
    FCreateModFileRequest create_mod_file_request,
    FCreateModFileChunkedDelegate delegate) override;

  virtual int64 UploadModFileChunk(
    const FModFileChunkMetadata& chunk_metadata,
    TSharedPtr<TArray<uint8>> chunk_data,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FUploadModFileChunkDelegate delegate) override;

  virtual int64 CreateCookedModFile(
    int64 mod_id,
    int64 source_file_id,
    FCreateCookedModFileRequest create_cooked_mod_file_request,
    TSharedPtr<TArray<uint8>> file_data,
    FFileTransferProgressDelegate file_transfer_progress_delegate,
    FCreateModFileDelegate delegate) override;

  virtual void CreateCookedModFileChunked(
    int64 mod_id,
    int64 source_file_id,
    FCreateCookedModFileRequest create_cooked_mod_file_request,
    FCreateModFileChunkedDelegate delegate) override;

  virtual void CancelCreation(int64 cancel_id) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CFCoreApiImpl);

  void InitializeSettings();
  void AppendLanguageParamIfEnabled(UrlSearchParams& search_params);

private:
  TSharedPtr<ISettingsService> settings_service_;
  TSharedPtr<IApiHelperService> api_helper_service_;
  TSharedPtr<ICFCoreApiAuthorized> cfcore_api_authorized_;
  TSharedRef<ApiDelegatesDispatcher> api_delegates_dispatcher_;
  TSharedPtr<IFeaturePhasingService> feature_phasing_service_;

  FString api_base_url_;
  FString chunked_upload_server_url_;
};

} // namespace cfcore