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
#include <common/cfcore_error.h>
#include <api/models/terms/terms.h>
#include <api/models/external_auth_additional_info.h>
#include <api/models/enums/external_auth_provider.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ICFCoreAuthentication {
public:
  DECLARE_DELEGATE_TwoParams(FGetTermsDelegate, const TOptional<FTerms>&, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FSendSecurityCodeDelegate, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FGenerateAuthTokenDelegate, const TOptional<FCFCoreError>&);
  DECLARE_DELEGATE_OneParam(FLogoutDelegate, const TOptional<FCFCoreError>&);

public:
  virtual ~ICFCoreAuthentication() = default;

public:
  virtual bool IsAuthenticated() = 0;

public:
  virtual void GetTerms(FGetTermsDelegate delegate) = 0;

  virtual void SendSecurityCode(const FString& email,
                                FSendSecurityCodeDelegate delegate) = 0;

  virtual void GenerateAuthToken(const FString& email,
                                 int32 security_code,
                                 FGenerateAuthTokenDelegate delegate) = 0;

  virtual void GenerateAuthTokenByExternalProvider(
    ECFCoreExternalAuthProvider provider,
    const FString& external_token,
    const FExternalAuthAdditionalInfo& additional_info,
    FGenerateAuthTokenDelegate delegate) = 0;

  virtual void Logout(FLogoutDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreAuthentication)