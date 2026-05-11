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
#include "enums/entitlment_campaign_type.h"

#include "entitlement_campaign.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FEntitlementCampaign {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 id = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 gameId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString name;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime createdAt = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime expiresAt = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreEntitlmentCampaignType type = ECFCoreEntitlmentCampaignType::Unknown;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isDeleted = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 codeLength = 0;

  /**
   * Json data
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString extraData;
};
