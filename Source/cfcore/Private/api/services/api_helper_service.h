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
#include <services/http/www_form.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IApiHelperService {
public:
	virtual ~IApiHelperService() = default;

public:
	virtual void PerformGet(const FString& url,
													TSharedPtr<IApiResponseHandler> handler) = 0;
	virtual void PerformPost(const FString& url,
													 const FString& json_data,
													 TSharedPtr<IApiResponseHandler> handler) = 0;
	virtual int64 PerformPost(const FString& url,
													  const WWWForm& www_form,
													  TSharedPtr<IApiResponseHandler> handler) = 0;
	virtual void PerformPut(const FString& url,
													const FString& json_data,
													TSharedPtr<IApiResponseHandler> handler) = 0;
	virtual int64 PerformPut(const FString& url,
													 const WWWForm& www_form,
													 TSharedPtr<IApiResponseHandler> handler) = 0;
	virtual void PerformDelete(const FString& url,
														 TSharedPtr<IApiResponseHandler> handler) = 0;

	/**
	 * Can be called on values returned from PerformPost/PerformPut etc...
	 */
	virtual void CancelFileOperation(int64 operation_id) = 0;

	virtual void GetApiRequestHeaders(TMap<FString, FString>& headers) = 0;

	virtual TOptional<FCFCoreApiResponseError> GetApiResponseError(
		const HttpResponse& http_response) = 0;
};

}; // namespace cfcore

Expose_TNameOf(cfcore::IApiHelperService)