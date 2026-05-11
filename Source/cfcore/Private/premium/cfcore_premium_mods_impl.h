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
#pragma once
#include "CoreMinimal.h"
#include <premium/cfcore_premium_mods.h>

#include <premium/services/premium_mod_purchase_polling_service.h>

class FJsonObject;

namespace cfcore {
class ISettingsService;
class ICryptoService;
class ICFCoreApi;

// -----------------------------------------------------------------------------
class CFCorePremiumModsImpl : public ICFCorePremiumMods {
public:
  CFCorePremiumModsImpl(
    TSharedPtr<ISettingsService> settings_service,
    TSharedPtr<ICryptoService> crypto_service,
    TSharedPtr<ICFCoreApi> cfcore_api,
    TSharedPtr<IPremiumModPurchasePollingService> polling_service);
  virtual ~CFCorePremiumModsImpl() = default;

// ICFCorePremiumMods
public:
  virtual void OverridePublicKey(const FString& public_key_pem) override;
  virtual void GetPremiumMods(FGetPremiumModsDelegate delegate) override;
  virtual void GetPremiumModsV2(FGetPremiumModsV2Delegate delegate) override;
  virtual void CheckMods(const TArray<int64>& mod_ids,
                         FPremiumModsCheckDelegate delegate) override;
  virtual void CheckDlc(const TArray<FString>& entitlement_ids,
                        FCheckDlcDelegate delegate) override;
  virtual void GetModFileDownloadUrl(
    int64 mod_id,
    int64 file_id,
    FGetModFileDownloadUrlDelegate delegate) override;
  virtual void PollModPurchase(const FPollPurchaseParams& params,
                               FPollPurchaseDelegate delegate) override;
  virtual void StopPurchasePolling() override;

private:
  bool IsSignatureVerified(const FString& payload,
                           const FString& signature);

  const FString& GetPublicKey() const;

  bool GetPayload(const TArray<int64>& mod_ids, OUT FString& payload);
  bool GetPayload(const FModFileDownloadUrl& download_url,
                  OUT FString& payload);
  bool GetPayload(const FOwnedPremiumMods& owned_mods, OUT FString& payload);
  void RemoveSignatureField(TSharedRef<FJsonObject>& json);
  bool ToStringPayload(TSharedRef<FJsonObject>& json, OUT FString& payload);

  FString FixUnexpectedFieldNameCasing(const FString& payload);

private:
  TSharedPtr<ISettingsService> settings_service_;
  TSharedPtr<ICryptoService> crypto_service_;
  TSharedPtr<ICFCoreApi> cfcore_api_;
  TSharedPtr<IPremiumModPurchasePollingService> polling_service_;

  FString public_key_pem_override_;
};

} // namespace cfcore