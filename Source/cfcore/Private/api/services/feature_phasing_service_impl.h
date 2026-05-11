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
#pragma once
#include <CoreMinimal.h>
#include <common/macros.h>

#include "feature_phasing_service.h"

#include <api/cfcore_api_delegates_dispatcher.h>
#include <api/cfcore_api_authorized_delegates.h>

namespace cfcore {
class IUserPhasingPercentService;

// -----------------------------------------------------------------------------
class FeaturePhasingServiceImpl : public IFeaturePhasingService {
public:
  FeaturePhasingServiceImpl(
    TSharedRef<IUserPhasingPercentService> user_phasing_percent_service,
    TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
      authorized_delegates_dispatcher);

// IFeaturePhasingService
public:
  virtual bool IsFeatureEnabledForUser(
    int32 FMePhasingData::* phasing_field) override;


private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(FeaturePhasingServiceImpl);

  void LogPhasingData();
  void RegisterCFCoreApiDispatcherDelegates(
    TSharedRef<CFCoreApiDelegatesDispatcher<CFCoreApiAuthorizedDelegates>>
      authorized_delegates_dispatcher);

  bool IsFeatureDisabled(int32 phasing_value);
  FString GetPhasingFieldName(int32 FMePhasingData::* phasing_field);

private:
  TSharedRef<IUserPhasingPercentService> user_phasing_percent_service_;
  TSharedRef<CFCoreApiAuthorizedDelegates> delegates_;
  TSharedRef<FMePhasingData> phasing_data_;
};

} // namespace cfcore