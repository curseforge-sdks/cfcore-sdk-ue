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
#include "enums/external_auth_provider.h"

#include "initiate_purchase_request.generated.h"

/**
 * Used when purchasing mods on consoles
 */
USTRUCT(BlueprintType)
struct CFCORE_API FInitiatePurchaseRequest {
  GENERATED_BODY()

  /**
   * The mod id being purchased
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 modId = 0;

  /**
   * The token's provider (e.g. xbl)
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreExternalAuthProvider provider = ECFCoreExternalAuthProvider::None;

  /**
   * The user's token on the device - see |ECFCoreExternalAuthProvider|
   * NOTE: This is the platform token, not the SDK's token.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString token;
};
