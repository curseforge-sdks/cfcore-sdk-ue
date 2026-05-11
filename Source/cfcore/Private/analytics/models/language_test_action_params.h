/*MIT License

Copyright (c) 2026 Overwolf Ltd.

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

#include "language_test_action_params.generated.h"

/**
 * Parameters for a Language Test Action (premium mods).
 */
USTRUCT(BlueprintType)
struct FLanguageTestActionParams {
  GENERATED_BODY()

  /**
   * ID of the mod.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 modId = 0;

  /**
   * Test or Control group.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString group;

  /**
   * Selected language.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString selectedLanguage;

  /**
   * Action performed - purchase or page view.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString action;

  /**
   * User owns the premium mod or not.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool ownsMod = false;
};