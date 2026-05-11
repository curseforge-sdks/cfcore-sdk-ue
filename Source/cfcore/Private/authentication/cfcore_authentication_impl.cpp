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
#include "cfcore_authentication_impl.h"
#include <services/logger/logger_macros.h>
#include "common/macros.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreAuthenticationImpl::CFCoreAuthenticationImpl(
  TSharedPtr<ICFCoreApi> cfcore_api,
  TSharedPtr<IUserContextService> user_context_service) :
  cfcore_api_(cfcore_api),
  user_context_service_(user_context_service) {
}

// -----------------------------------------------------------------------------
TSharedPtr<ICFCoreAuthentication>
  CFCoreAuthenticationImpl::GetAuthentication() {
  return this->AsShared();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreAuthenticationImpl::Initialize() {
  SHAREABLE_PROMISE(FCFCoreError, promise);

  check(IsInGameThread());

  CheckAuthToken().Next([this, promise](bool) {
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool CFCoreAuthenticationImpl::IsAuthenticated() {
  FString auth_token = user_context_service_->UserContextInfo().authToken;
  return !auth_token.IsEmpty();
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::GetTerms(FGetTermsDelegate delegate) {
  cfcore_api_->Authentication()->GetTerms(
    ICFCoreApiAuthentication::FGetTermsDelegate::CreateLambda(
      [delegate](const TOptional<FTerms>& opt_terms,
                 const TOptional<FCFCoreApiResponseError>& err) {
        TOptional<FCFCoreError> opt_err;

        if (err.IsSet()) {
          FString desc = err.GetValue().description;
          CFCORE_LOG(Error, TEXT("Failed to get terms (%s)"), *desc);
          opt_err = FCFCoreError(err);
        }

        delegate.ExecuteIfBound(opt_terms, opt_err);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::SendSecurityCode(
  const FString& email,
  FSendSecurityCodeDelegate delegate) {

  cfcore_api_->Authentication()->SendSecurityCode(
    email,
    ICFCoreApiAuthentication::FSendSecurityCodeDelegate::CreateLambda(
      [delegate](const TOptional<FCFCoreApiResponseError>& err) {
        TOptional<FCFCoreError> opt_err;

        if (err.IsSet()) {
          FString desc = err.GetValue().description;
          CFCORE_LOG(Error, TEXT("Failed sending code (%s)"), *desc);
          opt_err = FCFCoreError(err);
        }

        delegate.ExecuteIfBound(opt_err);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::GenerateAuthToken(
  const FString& email,
  int32 security_code,
  FGenerateAuthTokenDelegate delegate) {

  ResetAuthToken();

  cfcore_api_->Authentication()->GetAuthToken(
    email,
    security_code,
    ICFCoreApiAuthentication::FGetAuthTokenDelegate::CreateLambda(
      [this, delegate](TOptional<FAuthToken> auth_token,
                       const TOptional<FCFCoreApiResponseError>& opt_err) {

        HandleAuthToken(auth_token, opt_err, delegate);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::GenerateAuthTokenByExternalProvider(
  ECFCoreExternalAuthProvider provider,
  const FString& external_token,
  const FExternalAuthAdditionalInfo& additional_info,
  FGenerateAuthTokenDelegate delegate) {

  ResetAuthToken();

  cfcore_api_->Authentication()->GetAuthTokenByExternalProvider(
    provider,
    external_token,
    additional_info,
    ICFCoreApiAuthentication::FGetAuthTokenDelegate::CreateLambda(
      [this, delegate](TOptional<FAuthToken> auth_token,
                       const TOptional<FCFCoreApiResponseError>& opt_err) {

        HandleAuthToken(auth_token, opt_err, delegate);
      })
  );
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::Logout(FLogoutDelegate delegate) {
  ResetAuthToken();
  delegate.ExecuteIfBound({});
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::HandleAuthToken(
  TOptional<FAuthToken> auth_token,
  const TOptional<FCFCoreApiResponseError>& opt_err,
  FGenerateAuthTokenDelegate delegate) {

  if (opt_err.IsSet()) {
    FString desc = opt_err.GetValue().description;
    CFCORE_LOG(Error, TEXT("Failed getting token (%s)"), *desc);
    delegate.ExecuteIfBound(FCFCoreError(opt_err));
    return;
  }

  FString token = auth_token.GetValue().token;
  auto future = user_context_service_->SetAuthToken(token);
  future.Next([delegate](bool success) {
    if (success) {
      CFCORE_LOG(Log, TEXT("User successfully authenticated"));
      delegate.ExecuteIfBound({});
      return;
    }

    CFCORE_LOG(Error, TEXT("Failed setting auth token"));
    auto code = ECFCoreErrorCodes::FailedSettingAuthToken;
    delegate.ExecuteIfBound(FCFCoreError(code));
  });
}

// -----------------------------------------------------------------------------
void CFCoreAuthenticationImpl::ResetAuthToken() {
  user_context_service_->SetAuthToken(FString());
}

// -----------------------------------------------------------------------------
TFuture<bool> CFCoreAuthenticationImpl::CheckAuthToken() {
  SHAREABLE_PROMISE(bool, promise);

  if (!IsAuthenticated()) {
    promise->SetValue(true);
    return promise->GetFuture();
  }

  cfcore_api_->Authorized()->GetMe(
    ICFCoreApiAuthorized::FGetMeDelegate::CreateLambda(
      [this, promise](const TOptional<FMe>& opt_me,
                      const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet() && opt_err->tokenExpired) {
          CFCORE_LOG(Warning, TEXT("Auth token has expired"));
          ResetAuthToken();
        }

        promise->SetValue(true);
      })
  );

  return promise->GetFuture();
}
