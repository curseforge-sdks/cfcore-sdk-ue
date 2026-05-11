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
#include "enums/file_release_type.h"
#include "enums/file_status.h"
#include "file_hash.h"
#include "sortable_game_version.h"
#include "file_dependency.h"
#include "file_module.h"
#include "file_cooking_info.h"

#include "file.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FFile {
  GENERATED_BODY()

  /**
   * 0 is treated as uninstalled/non-existant
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 id = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 gameId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 modId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isAvailable = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString displayName;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString fileName;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreFileReleaseType releaseType = ECFCoreFileReleaseType::None;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreFileStatus fileStatus = ECFCoreFileStatus::None;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FFileHash> hashes;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime fileDate = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 fileLength = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 fileSizeOnDisk = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 downloadCount = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString downloadUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FString> gameVersions;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FSortableGameVersion> sortableGameVersions;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FFileDependency> dependencies;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool ExposeAsAlternative = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 parentProjectFileId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 alternateFileId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isServerPack = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 serverPackFileId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 fileFingerprint = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  TArray<FFileModule> modules;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FFileCookingInfo cookingInfo;
};
