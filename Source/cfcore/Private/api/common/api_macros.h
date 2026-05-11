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

#include <api/services/api_response_handler_empty_impl.h>
#include <api/services/api_response_handler_json_impl.h>
#include <api/services/api_response_handler_with_extra_param_json_impl.h>
#include <api/services/api_response_handler_with_progress_impl.h>

namespace cfcore {

#define CREATE_EMPTY_RESPONSE_HANDLER(delegate_type, delegate) \
  MakeShareable(new ApiResponseHandlerEmptyImpl<delegate_type>(delegate));

#define CREATE_RESPONSE_HANDLER(model_response_type, delegate_type, delegate) \
  MakeShareable(new ApiResponseHandlerJsonImpl<model_response_type, \
                                               delegate_type>(delegate))

#define CREATE_RESPONSE_HANDLER_WITH_PROGRESS(model_response_type, \
                                              delegate_type, \
                                              delegate, \
                                              progress_delegate) \
  MakeShareable( \
    new ApiResponseHandlerWithProgressImpl<model_response_type, delegate_type>(\
      CREATE_RESPONSE_HANDLER(model_response_type, delegate_type, delegate), \
      progress_delegate))

// NOTE(twolf): We use this macro to detect if a model doesn't have an expected
// field. This can happen if the field was changed.
#define ASSURE_FIELD_EXISTS(model_response_type, field) { \
  model_response_type temp; \
  (void)temp.field; \
}

#define CREATE_RESPONSE_HANDLER_WITH_PAGINATION(model_response_type, \
                                                delegate_type, \
                                                delegate) \
  MakeShareable(new ApiResponseHandlerWithExtraParamJsonImpl< \
    model_response_type, delegate_type, FCFCoreApiResponsePagination>( \
      "pagination", delegate)); \
  ASSURE_FIELD_EXISTS(model_response_type, pagination)


// TODO(apliscov): Use a method like GetSignatureFieldName() instead of the
// hard coded "sig".
#define CREATE_RESPONSE_HANDLER_WITH_SIGNATURE(model_response_type, \
                                               delegate_type, \
                                               delegate) \
  MakeShareable(new ApiResponseHandlerWithExtraParamJsonImpl< \
    model_response_type, delegate_type, FString>("sig", delegate)); \
  ASSURE_FIELD_EXISTS(model_response_type, sig)

} // namespace cfcore