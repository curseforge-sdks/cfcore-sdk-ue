/*MIT License

Copyright (c) 2025 Overwolf Ltd.

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

#include "cfcore_settings_subscriptions.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreSettingsSubscriptions {
  GENERATED_BODY()

  /**
   * The interval, in milliseconds, between automatic mod management runs.
   * Used when ICFCoreSubscription::EnableAutoManagement is called.
   * Valid range: 10000-600000 (10 seconds to 10 minutes).
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore",
    meta=(ClampMin = "10000", ClampMax = "600000",
          UIMin = "10000", UIMax = "600000"))
  int32 automaticModManagementIntervalMs = 30000;

  /**
   * Allows install actions as part of sync plans and auto management.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  bool allowInstallActions = true;

  /**
   * Allows uninstall actions as part of sync plans and auto management.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  bool allowUninstallActions = true;

  /**
   * Allows update actions as part of sync plans and auto management.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  bool allowUpdateActions  = true;

};
