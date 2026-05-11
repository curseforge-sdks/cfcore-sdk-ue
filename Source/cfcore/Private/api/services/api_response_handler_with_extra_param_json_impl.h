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
#include "api_response_handler.h"

#include <common/macros.h>

#include <Json.h>
#include <JsonUtilities.h>

namespace cfcore {

// -----------------------------------------------------------------------------
template<typename ResponseModel,
         typename DelegateThreeParamsWithError,
         typename ExtraParamType>
class ApiResponseHandlerWithExtraParamJsonImpl : public IApiResponseHandler {
public:
  // ---------------------------------------------------------------------------
  ApiResponseHandlerWithExtraParamJsonImpl(
    FName extra_param_name,
    DelegateThreeParamsWithError delegate) :
    extra_param_name_(extra_param_name),
    delegate_(delegate) {
  }

// IApiResponseHandler
public:
  // ---------------------------------------------------------------------------
  virtual void HandleProgress(const HttpProgress& progress) {
  }

  // ---------------------------------------------------------------------------
  virtual void HandleResponse(const HttpResponse& http_response) override {
    ResponseModel response_model;
    bool success = FJsonObjectConverter::JsonObjectStringToUStruct(
      http_response.body,
      &response_model,
      0,
      0);

    if (!success) {
      FCFCoreApiResponseError error;
      error.failedToParseServerResponse = true;
      error.description = kApiResponseErrorFailedToParseJsonResponse;
      HandleApiResponseError(error);
      return;
    }

    UStruct* model_struct = ResponseModel::StaticStruct();
    FProperty* property = model_struct->FindPropertyByName(extra_param_name_);

    if (!property) {
      FCFCoreApiResponseError error;
      error.failedToParseServerResponse = true;
      error.description = TEXT("missing property");
      HandleApiResponseError(error);
      return;
    }

    ExtraParamType* val_ptr =
      property->ContainerPtrToValuePtr<ExtraParamType>(&response_model);

    delegate_.ExecuteIfBound(MoveTempIfPossible(response_model.data),
                             *val_ptr,
                             {});
  }

  // ---------------------------------------------------------------------------
  virtual void HandleApiResponseError(
    const FCFCoreApiResponseError& error) override {
    delegate_.ExecuteIfBound({}, {}, error);
  }

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ApiResponseHandlerWithExtraParamJsonImpl);

private:
  FName extra_param_name_;
  DelegateThreeParamsWithError delegate_;
};

} // namespace cfcore