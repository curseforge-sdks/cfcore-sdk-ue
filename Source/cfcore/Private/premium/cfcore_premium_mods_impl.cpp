/*MIT License

Copyright (c) 2023 Overwolf Ltd.

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
#include "cfcore_premium_mods_impl.h"
#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <services/settings_service.h>
#include <services/crypto_service/crypto_service.h>
#include <api/cfcore_api.h>

#include <api/models/model_responses/premium_mods_check_request_response.h>
#include <api/models/model_responses/get_mod_file_download_url_response.h>
#include <api/models/model_responses/owned_premium_mods_response.h>

#include <JsonUtilities.h>

namespace cfcore {

// See method: |FixUnexpectedFieldNameCasing|
const TArray<FString> kUnexpectedFieldNameCasingTable = {
  TEXT("data"),
  TEXT("url"), // uRL -> url
  TEXT("securedParams"),
};

#define VERIFY_SIGNATURE(opt_err, model, delegate) { \
  if (opt_err.IsSet()) { \
    delegate.ExecuteIfBound({}, FCFCoreError(*opt_err)); \
    return; \
  } \
\
  FString payload; \
  if (!GetPayload(model, payload)) { \
    auto code = ECFCoreErrorCodes::FailedToVerifySignature; \
    delegate.ExecuteIfBound({}, FCFCoreError(code)); \
    return; \
  } \
\
  if (!IsSignatureVerified(payload, signature)) { \
    auto code = ECFCoreErrorCodes::FailedToVerifySignature; \
    delegate.ExecuteIfBound({}, FCFCoreError(code)); \
    return; \
  } \
\
  delegate.ExecuteIfBound(model, {}); \
}

};

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCorePremiumModsImpl::CFCorePremiumModsImpl(
  TSharedPtr<ISettingsService> settings_service,
  TSharedPtr<ICryptoService> crypto_service,
  TSharedPtr<ICFCoreApi> cfcore_api,
  TSharedPtr<IPremiumModPurchasePollingService> polling_service) :
  settings_service_(settings_service),
  crypto_service_(crypto_service),
  cfcore_api_(cfcore_api),
  polling_service_(polling_service) {
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::OverridePublicKey(const FString& public_key_pem) {
  public_key_pem_override_ = public_key_pem;
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::GetPremiumMods(FGetPremiumModsDelegate delegate) {
  cfcore_api_->Authorized()->GetMyPremiumMods(
    ICFCoreApiAuthorized::FGetMyPreimumModsDelegate::CreateLambda(
      [this, delegate](const TOptional<TArray<int64>>& opt_owned_mods,
                       const FString& signature,
                       const TOptional<FCFCoreApiResponseError>& opt_err) {

        VERIFY_SIGNATURE(opt_err, *opt_owned_mods, delegate);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::GetPremiumModsV2(
  FGetPremiumModsV2Delegate delegate) {

  cfcore_api_->Authorized()->GetMyPremiumModsV2(
    ICFCoreApiAuthorized::FGetMyPremiumModsV2Delegate::CreateLambda(
      [this, delegate](const TOptional<FOwnedPremiumMods>& opt_owned_mods,
                       const FString& signature,
                       const TOptional<FCFCoreApiResponseError>& opt_err) {

        VERIFY_SIGNATURE(opt_err, *opt_owned_mods, delegate);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::CheckMods(
  const TArray<int64>& mod_ids,
  FPremiumModsCheckDelegate delegate) {

  cfcore_api_->Authorized()->PremiumModsCheck(
    mod_ids,
    ICFCoreApiAuthorized::FPremiumModsCheckDelegate::CreateLambda(
      [this, delegate](const TOptional<TArray<int64>>& opt_owned_mods,
                       const FString& signature,
                       const TOptional<FCFCoreApiResponseError>& opt_err) {

        VERIFY_SIGNATURE(opt_err, *opt_owned_mods, delegate);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::CheckDlc(
  const TArray<FString>& entitlement_ids,
  FCheckDlcDelegate delegate) {

  cfcore_api_->Authorized()->CheckDlc(
    entitlement_ids,
    ICFCoreApiAuthorized::FCheckDlcDelegate::CreateLambda(
      [this, delegate](const TOptional<TArray<int64>>& opt_owned_mods,
                       const FString& signature,
                       const TOptional<FCFCoreApiResponseError>& opt_err) {

        VERIFY_SIGNATURE(opt_err, *opt_owned_mods, delegate);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::GetModFileDownloadUrl(
  int64 mod_id,
  int64 file_id,
  FGetModFileDownloadUrlDelegate delegate) {

  cfcore_api_->GetModFileDownloadUrl(
    mod_id,
    file_id,
    ICFCoreApi::FGetModFileDownloadUrlDelegate::CreateLambda(
      [this, delegate](
        const TOptional<FModFileDownloadUrl>& opt_download_url,
        const FString& signature,
        const TOptional<FCFCoreApiResponseError>& opt_err) {

        VERIFY_SIGNATURE(opt_err, *opt_download_url, delegate);
      })
  );
}


// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::PollModPurchase(const FPollPurchaseParams& params,
  FPollPurchaseDelegate delegate) {

  polling_service_->Start(
    params,
    IPremiumModPurchasePollingService::FCheckModsFunc::CreateRaw(
      this, &CFCorePremiumModsImpl::CheckMods),
    delegate);
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::StopPurchasePolling() {
  polling_service_->Stop();
}

// -----------------------------------------------------------------------------
bool CFCorePremiumModsImpl::IsSignatureVerified(const FString& payload,
                                                const FString& signature) {
  const FString& pub_key = GetPublicKey();
  if (pub_key.IsEmpty()) {
    return true;
  }

  if (signature.IsEmpty()) {
    CFCORE_LOG(Error, TEXT("Missing a signature from server"));
    return false;
  }

  if (!crypto_service_->DigestVerifyPS256(pub_key, signature, payload)) {
    CFCORE_LOG(Error, TEXT("Failed to validate payload"));
    CFCORE_LOG(Error, TEXT("Payload: %s"), *payload);
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
const FString& CFCorePremiumModsImpl::GetPublicKey() const {
  if (public_key_pem_override_.IsEmpty()) {
    return settings_service_->GetSettings().premiumMods.publicKeyPem;
  }

  return public_key_pem_override_;
}

// -----------------------------------------------------------------------------
bool CFCorePremiumModsImpl::GetPayload(const TArray<int64>& mod_ids,
                                       OUT FString& payload) {

  FPremiumModsCheckRequestResponse response;
  response.data = mod_ids;

  TSharedPtr<FJsonObject> json_object =
    FJsonObjectConverter::UStructToJsonObject(response, 0, 0, nullptr);
  if (!json_object.IsValid()) {
    return false;
  }

  TSharedRef<FJsonObject> json = json_object.ToSharedRef();
  RemoveSignatureField(json);

  if (!ToStringPayload(json, payload)) {
    CFCORE_LOG(Error, TEXT("Failed to get payload"));
    return false;
  }

  payload = FixUnexpectedFieldNameCasing(payload);

  return true;
}

// -----------------------------------------------------------------------------
bool CFCorePremiumModsImpl::GetPayload(
  const FModFileDownloadUrl& download_url,
  OUT FString& payload) {

  FGetModFileDownloadUrlResponse reponse;
  reponse.data = download_url;

  TSharedPtr<FJsonObject> json_object =
    FJsonObjectConverter::UStructToJsonObject(reponse, 0, 0, nullptr);
  if (!json_object.IsValid()) {
    return false;
  }

  TSharedRef<FJsonObject> json = json_object.ToSharedRef();
  RemoveSignatureField(json);

  if (!ToStringPayload(json, payload)) {
    CFCORE_LOG(Error, TEXT("Failed to get payload"));
    return false;
  }

  payload = FixUnexpectedFieldNameCasing(payload);

  return true;
}

// -----------------------------------------------------------------------------
bool CFCorePremiumModsImpl::GetPayload(
  const FOwnedPremiumMods& owned_mods,
  OUT FString& payload) {

  FOwnedPremiumModsResponse response;
  response.data = owned_mods;

  TSharedPtr<FJsonObject> json_object =
    FJsonObjectConverter::UStructToJsonObject(response, 0, 0, nullptr);
  if (!json_object.IsValid()) {
    return false;
  }

  TSharedRef<FJsonObject> json = json_object.ToSharedRef();
  RemoveSignatureField(json);

  if (!ToStringPayload(json, payload)) {
    CFCORE_LOG(Error, TEXT("Failed to get payload"));
    return false;
  }

  payload = FixUnexpectedFieldNameCasing(payload);

  return true;
}

// -----------------------------------------------------------------------------
void CFCorePremiumModsImpl::RemoveSignatureField(
  TSharedRef<FJsonObject>& json) {

  const FString& field =
    FPremiumModsCheckRequestResponse::GetSignatureFieldName();

  json->RemoveField(field);
}

// -----------------------------------------------------------------------------
bool CFCorePremiumModsImpl::ToStringPayload(TSharedRef<FJsonObject>& json,
                                            OUT FString& payload) {
  auto json_writer = TJsonWriterFactory<
    FString::ElementType,
    TCondensedJsonPrintPolicy<TCHAR>>::Create(&payload, 0);

  bool success = FJsonSerializer::Serialize(json, json_writer);
  json_writer->Close();

  return success;
}

// -----------------------------------------------------------------------------
// NOTE(twolf): We've noticed that JSON Serializing has unexpected results in
// how it converts its field names (this is probably because internally it is
// using FName and not FString, but this is not fully validated). The end
// result is that you might get different casing for the field names (e.g. url
// becomes uRL).
//
// We fix it by manually replacing all the field names to what we expect them
// to be.
FString CFCorePremiumModsImpl::FixUnexpectedFieldNameCasing(
  const FString& payload) {

  FString final_payload = payload;

  for (const FString& field_name : kUnexpectedFieldNameCasingTable) {
    FString field_as_json = FString::Format(TEXT("\"{0}\":"), {
      field_name
    });

    final_payload = final_payload.Replace(*field_as_json,
                                          *field_as_json,
                                          ESearchCase::IgnoreCase);
  }

  return final_payload;
}


