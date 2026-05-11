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
#include <common/cfcore_error.h>

// -----------------------------------------------------------------------------
FCFCoreError::FCFCoreError() {
  ClearError();
}

// -----------------------------------------------------------------------------
FCFCoreError::FCFCoreError(ECFCoreErrorCodes error_code) {
  code = error_code;
  isError = error_code != ECFCoreErrorCodes::None;
}

// -----------------------------------------------------------------------------
FCFCoreError::FCFCoreError(const TOptional<FCFCoreError>& err) {
  if (!err.IsSet()) {
    ClearError();
    return;
  }

  isError = true;
  auto error_val = err.GetValue();
  code = error_val.code;
  apiError = error_val.apiError;
  description = error_val.description;
}

// -----------------------------------------------------------------------------
FCFCoreError::FCFCoreError(const TOptional<FCFCoreApiResponseError>& err) {
  if (!err.IsSet()) {
    ClearError();
    return;
  }

  isError = true;
  code = ECFCoreErrorCodes::ApiError;

  auto error_val = err.GetValue();
  apiError = error_val;
  description = error_val.description;
}

// -----------------------------------------------------------------------------
void FCFCoreError::ClearError() {
  code = ECFCoreErrorCodes::None;
  isError = false;
}