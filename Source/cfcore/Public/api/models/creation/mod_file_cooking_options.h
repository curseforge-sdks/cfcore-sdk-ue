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
#include <CoreMinimal.h>

#include <api/models/enums/auto_cooking_type.h>

#include "mod_file_cooking_options.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FModFileCookingOptions {
  GENERATED_BODY()
  /**
   * Set true when uploading a source file for cooking. The rest of the options
   * in this structure will be ignored when |isSourceFile| is set to false.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool isSourceFile = false;

  /**
   * Use this enum if you want to support PCOnly or manual cooking.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreAutoCookingType autoCookingType = ECFCoreAutoCookingType::All;

  /**
   * For cloud cooking.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString cookerVersion;
};
