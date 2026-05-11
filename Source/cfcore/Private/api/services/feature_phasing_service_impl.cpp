/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include "feature_phasing_service_impl.h"
#include <services/logger/logger_macros.h>
#include "user_phasing_percent_service.h"

#include <JsonUtilities.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
FeaturePhasingServiceImpl::FeaturePhasingServiceImpl(
  TSharedRef<IUserPhasingPercentService> user_phasing_percent_service,
  TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
    authorized_delegates_dispatcher) :
  user_phasing_percent_service_(user_phasing_percent_service),
  delegates_(MakeShared<CFCoreApiAuthorizedDelegates>()),
  phasing_data_(MakeShared<FMePhasingData>()) {

  RegisterCFCoreApiDispatcherDelegates(authorized_delegates_dispatcher);
}

// -----------------------------------------------------------------------------
bool FeaturePhasingServiceImpl::IsFeatureEnabledForUser(
  int32 FMePhasingData::* phasing_field) {

  int32 server_phasing_val = (*phasing_data_).*phasing_field;
  if (IsFeatureDisabled(server_phasing_val)) {
    return false;
  }

  FString field_name = GetPhasingFieldName(phasing_field);

  // By passing the feature name as a salt value - we assure that the same users
  // won't ALWAYS fall into the phasing %
  int32 user_phasing_val =
    user_phasing_percent_service_->GetUserPhasingPercent(field_name);

  if (user_phasing_val <= server_phasing_val) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
void FeaturePhasingServiceImpl::LogPhasingData() {
  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(*phasing_data_, json_data);
  CFCORE_LOG(Log, TEXT("New phasing data receieved:\n%s"), *json_data);
}

// -----------------------------------------------------------------------------
void FeaturePhasingServiceImpl::RegisterCFCoreApiDispatcherDelegates(
  TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
  authorized_delegates_dispatcher) {

  delegates_->get_me = ICFCoreApiAuthorized::FGetMeDelegate::CreateLambda(
    [this](const TOptional<FMe>& opt_me,
           const TOptional<FCFCoreApiResponseError>& opt_err) {

    phasing_data_ = MakeShared<FMePhasingData>();
    if (opt_me.IsSet()) {
      *phasing_data_ = opt_me->phasingData;
    }

    LogPhasingData();
  });

  authorized_delegates_dispatcher->Register(FGuid::NewGuid(), delegates_);
}

// -----------------------------------------------------------------------------
bool FeaturePhasingServiceImpl::IsFeatureDisabled(int32 phasing_value) {
  return phasing_value == 0;
}

// -----------------------------------------------------------------------------
FString FeaturePhasingServiceImpl::GetPhasingFieldName(
  int32 FMePhasingData::* phasing_field) {

  UStruct* struct_type = FMePhasingData::StaticStruct();
  for (TFieldIterator<FProperty> iter(struct_type); iter; ++iter) {
    FProperty* property = *iter;

    auto value_ptr = property->ContainerPtrToValuePtr<FMePhasingData>(
      &(*phasing_data_));

    if (value_ptr == (FMePhasingData*)&((*phasing_data_).*phasing_field)) {
      FString prop_name = property->GetName();
      return prop_name;
    }
  }

  return "";
}