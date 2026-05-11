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
#include "cfcore_initialization_options.h"
#include "enums/mods_directory_mode.h"
#include "cfcore_settings_throttling.h"
#include "cfcore_settings_premium_mods.h"
#include "cfcore_settings_logger.h"
#include "cfcore_settings_analytics.h"
#include "cfcore_settings_unmanaged_mods.h"
#include "cfcore_settings_subscriptions.h"
#include "cfcore_settings_downloads.h"

#include <api/models/enums/external_auth_provider.h>

#include "cfcore_settings.generated.h"

namespace cfcore {

const int32 kCFCoreDefaultMaxConcurrentInstallation = 3;
const FString kSpecialFolderUserDirectory = TEXT("%USER_DIR%");
const FString kSpecialFolderUserSettingsDirectory = TEXT("%USER_SETTINGS_DIR%");
const FString kSpecialFolderProjectDirectory = TEXT("%PROJECT_DIR%");
const FString kSpecialFolderProjectSavedDirectory = TEXT("%PROJECT_SAVED_DIR%");

};

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FCFCoreSettings {
  GENERATED_BODY()

  /**
   * Contains options that are passed by the game in SDK initialization.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreInitializationOptions initOptions;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FString defaultLanguage;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  int64 gameId = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FString apiKey;

  /**
   * Set the provider (e.g. Steam, Epic etc.)
   * This is important when you support premium mods on multiple providers and
   * platforms.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  ECFCoreExternalAuthProvider provider = ECFCoreExternalAuthProvider::None;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  int32 maxConcurrentInstallations =
    cfcore::kCFCoreDefaultMaxConcurrentInstallation;

  /**
   * %USER_DIR%/[game name]/mods (e.g. MyDocuments/game/mods)
   * See: |kSpecialFolderUserDirectory|, |kSpecialFolderUserSettingsDirectory|,
   *|kSpecialFolderProjectDirectory| and |kSpecialFolderProjectSavedDirectory|
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FString modsDirectory;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  EModsDirectoryMode modsDirectoryMode = EModsDirectoryMode::CFCore;

  /**
   * %USER_SETTINGS_DIR%/[game name]/cfcore/user_data (e.g. appdata/local)
   * See: |kSpecialFolderUserDirectory|, |kSpecialFolderUserSettingsDirectory|,
   * |kSpecialFolderProjectDirectory| and |kSpecialFolderProjectSavedDirectory|
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FString userDataDirectory;

  /**
   * If this is a game server, set it to true so that we better identify the
   * platform (e.g. windows_server and not just windows)
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  bool isServer = false;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  bool isServerPcOnly = false;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsThrottling throttling;

  /**
   * This is only relevant if your game supports paid mods. In order to support
   * paid mods, please contact us.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsPremiumMods premiumMods;

  /**
   *
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsLogger logger;

  /**
   * Mods that belong to this list of category ids, will support dynamic
   * content - mainly for checking ownership of dynamic content that is premium.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  TSet<int64> dynamicContentCategoryIds;

  /**
   * Allows disabling Performance and Stability analytics, and allows enabling
   * User Engagement analytics.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsAnalytics analytics;

  /**
   * The following files will be ignored when calculating fingerprints and total
   * size of a mod - these are usually files that are dynamically created during
   * runtime and aren't part of the original mod.
   * Note - Keep filenames in lower case.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  TSet<FString> ignoredDynamicModFiles{ "assetregistry.bin" };

  /**
   * Unmanaged mods are mods that exist in the user's mods directory but weren't
   * installed properly via the plugin - usually, these mods don't exist on
   * CurseForge. This is useful for allowing creators to play with their
   * creations prior to uploading them to the CurseForge servers.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsUnmanagedMods unmanagedMods;

  /**
   * Subscriptions is the ability of the user to subscribe to mods, and have
   * them auto-managed by the plugin.
   * Make sure your game supports subscriptions by contacting us.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsSubscriptions subscriptions;


  /**
   * Contains settings related to chunked downloads.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsDownloads downloads;
};
