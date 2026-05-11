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
#include <common/macros.h>
#include "cfcore_authentication_internal.h"
#include <authentication/cfcore_authentication.h>

#include <api/cfcore_api.h>
#include <services/user_context/user_context_service.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class CFCoreAuthenticationImpl :
  public ICFCoreAuthentication,
  public ICFCoreAuthenticationInternal,
  public TSharedFromThis<CFCoreAuthenticationImpl> {

public:
  CFCoreAuthenticationImpl(
    TSharedPtr<ICFCoreApi> cfcore_api,
    TSharedPtr<IUserContextService> user_context_service);
  virtual ~CFCoreAuthenticationImpl() = default;

// ICFCoreAuthenticationInternal
public:
  virtual TSharedPtr<ICFCoreAuthentication> GetAuthentication() override;
  virtual TFuture<FCFCoreError> Initialize() override;

// ICFCoreAuthentication
public:
  virtual bool IsAuthenticated() override;

public:
  virtual void GetTerms(FGetTermsDelegate delegate) override;

  virtual void SendSecurityCode(const FString& email,
                                FSendSecurityCodeDelegate delegate) override;

  virtual void GenerateAuthToken(const FString& email,
                                 int32 security_code,
                                 FGenerateAuthTokenDelegate delegate) override;

  virtual void GenerateAuthTokenByExternalProvider(
      ECFCoreExternalAuthProvider provider,
      const FString& external_token,
      const FExternalAuthAdditionalInfo& additional_info,
      FGenerateAuthTokenDelegate delegate) override;

  virtual void Logout(FLogoutDelegate delegate) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CFCoreAuthenticationImpl);

  void HandleAuthToken(TOptional<FAuthToken> auth_token,
                       const TOptional<FCFCoreApiResponseError>& opt_err,
                       FGenerateAuthTokenDelegate delegate);
  void ResetAuthToken();

  virtual TFuture<bool> CheckAuthToken();

private:
  TSharedPtr<ICFCoreApi> cfcore_api_;
  TSharedPtr<IUserContextService> user_context_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreAuthenticationImpl)