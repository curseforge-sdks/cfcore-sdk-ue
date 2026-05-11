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
#include "api/common/cfcore_api_response_error.h"
#include "cfcore_error.generated.h"

// -----------------------------------------------------------------------------
UENUM(BlueprintType)
enum class ECFCoreErrorCodes : uint8 {
  None = 0,
  ApiError,
  FileSystemError,
  FailedToInitialize,
  AlreadyInitialized,
  NotImplemented,
  /**
   * Missing mod id or game id is invalid
   */
  InvalidModParams,
  /**
   * Passed an uninstalled mod to a function that expects the mod to be
   * installed - or the mod is corrupted.
   */
  InstalledModNotFound,
  InstallCancelled,
  DownloadCancelled,
  UploadCancelled,
  MissingModsDirectory,
  MissingModsDirectoryMode,
  MissingUserDataDirectory,
  FailedToLoadModsStateFromDisk,
  FailedToSaveModsStateToDisk,
  MissingLatestFileInformation,
  MissingFileInformation,
  /**
   * Given FFile doesn't belong to FCFCoreMod
   */
  FileNotBelongingToMod,
  /**
   * Couldn't match platform files from the server files (ClientServerLibrary)
   */
  NoPlatformFilesMatched,
  /**
   * One or more mods are missing (related to ClientServerLibrary)
   */
  MissingInstalledMods,
  /**
   * One or more mods are set to unavailable on the CF website
   */
  DetectedUnavailableMod,
  ModAlreadyBeingInstalled,
  FailedToDownloadFile,
  DownloadedFileHasInvalidHash,
  FailedDeletingOutputDirectory,
  FailedDeletingOutputFile,
  FailedToUnzip,
  FailedToMoveModDirectory,
  FailedSettingAuthToken,
  UserNotAuthenticated,
  FailedToCancelAction,
  MissingParameter,
  /**
   * Server response signature (e.g. premium mods check) couldn't be verified.
   */
  FailedToVerifySignature,
  /**
   * One or more mods are not owned by the user (they will need to purchase them
   * in order to use them)
   */
  ModsNotOwnedByUser,
  /**
   * Couldn't retrieve mod(s) details from server - for example, if you have a
   * non-standard mod installed that is not on the CurseForge servers.
   */
  MissingModsDetails,
  BindiffInvalidHeader,
  BindiffFailedToApplyPatch,
  /**
   * Used internally when a specific mod installation strategy isn't supported
   * for a given request. There is no need to handle this error by the game.
   */
  UnsupportedStrategy,
  /**
   * Usually means the server returned data that this SDK version doesn't
   * support. There is no need to handle this error by the game.
   */
  UnsupportedSchemaVersion,
  UnsupportedDiffMethod,
  FailedToZip,
  AlreadySentUserLogs,
  /**
   * Another subscription sync operation is already in progress.
   * Wait for it to complete before starting a new sync.
   */
  SubscriptionSyncInProgress,
  /**
   * PollModPurchase reached the maximum allowed polling duration without
   * detecting ownership.
   */
  PurchasePollingTimeout,
  /**
   * PollModPurchase was stopped manually via StopPurchasePolling before
   * ownership was detected.
   */
  PurchasePollingStopped
};

// -----------------------------------------------------------------------------
// TODO(twolf): Improve this struct - default descriptions can be standard based
// on the error code
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreError {
  GENERATED_BODY()

public:
  FCFCoreError();
  FCFCoreError(ECFCoreErrorCodes error_code);
  FCFCoreError(const TOptional<FCFCoreError>& err);
  FCFCoreError(const TOptional<FCFCoreApiResponseError>& err);

public:
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool isError = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  ECFCoreErrorCodes code = ECFCoreErrorCodes::None;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FCFCoreApiResponseError apiError;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString description;

private:
  void ClearError();
};

