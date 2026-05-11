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

#include "enums/diff_method.h"

#include "file_delta_diff_extra_data_file_info.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FFileDeltaDiffExtraDataFileInfo {
  GENERATED_BODY()

  /**
   * Relative path of the mod, where the patch and old file exist + where the
   * new file should exist.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString path;

  /**
   * The filename of the patch file.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString patchFilename;

  /**
   * The name of the actual mod file (which is both the old and new filename)
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString originalFilename;

  /**
   * Supported from version 3 schema - this is the target filename
   * (old + patch -> new). Should be ignored in older schemas
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString newFilename;

  /**
   * Supported from version 3 schema
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString newPath;

  /**
   * The method by which the delta diff was created (e.g. bsdiff, hdiff).
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreDiffMethod method = ECFCoreDiffMethod::None;
};
