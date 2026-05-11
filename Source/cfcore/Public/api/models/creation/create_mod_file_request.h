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

#include "mod_file_cooking_options.h"

#include <api/models/enums/changelog_markup_type.h>
#include <api/models/enums/file_release_type.h>

#include "create_mod_file_request.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCreateModFileRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreChangelogMarkupType changelogType = ECFCoreChangelogMarkupType::Text;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString changelog;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString filename;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString displayName;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TArray<int64> gameVersionIds;

  UPROPERTY(BlueprintReadWrite, Category = "Models")
  ECFCoreFileReleaseType releaseType = ECFCoreFileReleaseType::Release;

  /**
   * Use these options when uploading a source file for cooking.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FModFileCookingOptions cookingOptions;

  /**
   * This field is only mandatory for chunked uploads
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 fileLength = 0;

  /**
   * Whether the mod should be manually or automatically published after
   * moderation approval.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool isMarkedForManualRelease = false;
};
