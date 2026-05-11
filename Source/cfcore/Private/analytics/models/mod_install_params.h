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
#include "mod_install_params.generated.h"

/**
 * Parameters for a Mod Installation Analytic.
 */
USTRUCT(BlueprintType)
struct CFCORE_API FModInstallAnalyticsParams {
  GENERATED_BODY()

  /**
   * ID of the mod.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  int64 modId = 0;

  /**
   * Distinguishes between an update and an install.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool isUpdate = false;

  /**
   * Installation success or failure.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool success = false;

  /**
   * Installation was cancelled, either by user or by shutting down the SDK.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool cancelled = false;

  /**
   * Indicates whether the mod has been downloaded or already existed in the
   * system library.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool isDownload = false;

  /**
   * Installation was an update, using delta-diff.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool isDiff = false;

  /**
   * True if a delta-diff update has failed and reverted to a full download.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool postDiffFailed = false;

  /**
   * True if the download was a resumed one from a previous incomplete download
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool wasResumed = false;

  /**
   * Tracks the source of installation, e.g. mod page, server module, dynamic
   * content, subscription, or resume.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  FString origin = TEXT("");

  /**
   * True if this install was initiated as a Try Before You Buy trial.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  bool isTBYB = false;
};