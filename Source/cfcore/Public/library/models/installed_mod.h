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
#include <library/models/enums/installed_mod_status.h>
#include <library/models/library_progress.h>
#include <api/models/mod.h>
#include <api/models/file.h>

#include "installed_mod_download_info.h"

#include "installed_mod.generated.h"

// -----------------------------------------------------------------------------
// Represents a mod that is either being installed or already installed locally
// on the user's device
USTRUCT(BlueprintType)
struct CFCORE_API FInstalledMod {
  GENERATED_BODY()

  FInstalledMod() : id(FGuid()) {}

  /**
   * Factory that returns a new FInstalledMod with a freshly generated id
   */
  static FInstalledMod CreateNew() {
    FInstalledMod instance;
    instance.id = FGuid::NewGuid();
    return instance;
  }

  /**
   * An internal id given to the installed mod
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FGuid id;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime dateInstalled = 0;

  /**
   * When was the mod file updated/installed
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FDateTime dateUpdated = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  EInstalledModStatus status = EInstalledModStatus::Normal;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FString pathOnDisk;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool enabled = true;

  /**
   * When this is true, it means the mod wasn't installed from the CurseForge
   * servers and the SDK wasn't able to detect the mod - usually, this means the
   * mod doesn't exist on the CurseForge servers.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool unmanaged = false;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FCFCoreMod details;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FFile installedFile;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FFile latestUpdatedFile;

  /**
   * Dynamic content are mods that are downloaded automatically by the game and
   * aren't treated as "installed" until the user manually installs them.
   * This is useful for sharing custom cosmetics between players on a server.
   *
   * When set to true, treat mod as a dynamic content, otherwise (when false),
   * treat it as a user installed mod.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool dynamicContent = false;

  /**
   * Optionally, the mod's load order, allowing the game to load mods in a
   * specific order.
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 loadOrder = MAX_int32;

  /**
   * The status of this mod prior to installation. Neither persists to disk nor
   * exposed to blueprints. 
   */
  EInstalledModStatus preInstallStatus = EInstalledModStatus::Normal;

  /**
   * During installation, this field contains relevant progress information.
   * Does not persist to disk.
   */
  UPROPERTY(BlueprintReadOnly, Transient, Category = "Models")
  FLibraryProgress installProgress;

  /**
   * A set of all users that have this mod installed. Not exposed to blueprints.
   */
  UPROPERTY()
  TSet<FString> users;

  /**
   * Keeps information about a downloaded file that might be resumable
   */
  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FInstalledModDownloadInfo downloadInfo;
};
