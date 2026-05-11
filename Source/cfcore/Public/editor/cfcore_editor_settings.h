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
#include <models/cfcore_initialization_options.h>
#include <models/enums/mods_directory_mode.h>
#include <models/cfcore_settings_throttling.h>
#include <models/cfcore_settings_premium_mods.h>
#include <models/cfcore_settings_logger.h>
#include <models/cfcore_settings_analytics.h>
#include <models/cfcore_settings_unmanaged_mods.h>
#include <models/cfcore_settings_subscriptions.h>
#include <models/cfcore_settings_downloads.h>

#include <api/models/enums/external_auth_provider.h>

#include "cfcore_editor_settings.generated.h"

// -----------------------------------------------------------------------------
// A settings adapter for the UE editor
UCLASS(config = Game, defaultconfig)
class CFCORE_API UCFCoreEditorSettings : public UObject {
  GENERATED_BODY()
public:
  UCFCoreEditorSettings(const FObjectInitializer& ObjectInitializer);

  /**
   * Contains options that are passed by the game in SDK initialization.
   */
  UPROPERTY(config, EditAnywhere, Category="cfcore")
  FCFCoreInitializationOptions initOptions;

  UPROPERTY(config, EditAnywhere, Category="cfcore")
  FString defaultLanguage = TEXT("en");

  UPROPERTY(config, EditAnywhere, Category="cfcore")
  int64 gameId;

  UPROPERTY(config, EditAnywhere, Category="cfcore")
  FString apiKey;

  /**
   * Set the provider (e.g. Steam, Epic etc.)
   * This is important when you support premium mods on multiple providers and
   * platforms.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  ECFCoreExternalAuthProvider provider = ECFCoreExternalAuthProvider::None;

  UPROPERTY(config, EditAnywhere, Category="cfcore")
  int32 maxConcurrentInstallations = 3;

  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  FString modsDirectory;

  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  EModsDirectoryMode modsDirectoryMode = EModsDirectoryMode::CFCore;

  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  FString userDataDirectory;

  /**
   * If this is a game server, set it to true so that we better identify the
   * platform (e.g. windows_server and not just windows)
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  bool isServer = false;

  /**
   * Relevant only when |isServer| is true.
   * Set this if your server only supports PC clients (e.g. non-console)
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  bool isServerPcOnly = false;

  /**
   * IO related throttling - usually not something you'd use unless you know
   * what you're doing
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsThrottling throttling;

  /**
   * This is only relevant if your game supports paid mods. In order to support
   * paid mods, please contact us.
  */
  UPROPERTY(config, EditAnywhere, Category = "cfcore", meta = (MultiLine = "true"))
  FCFCoreSettingsPremiumMods premiumMods;

  UPROPERTY(config, EditAnywhere, Category = "cfcore", meta = (MultiLine = "true"))
  FCFCoreSettingsLogger logger;

  /**
   * Mods that belong to this list of class ids, will support dynamic content -
   * mainly for checking ownership of dynamic content that is premium.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  TSet<int64> dynamicContentCategoryIds;

  /**
   * Allows disabling Performance and Stability analytics, and allows enabling
   * User Engagement analytics.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore", meta = (MultiLine = "true"))
  FCFCoreSettingsAnalytics analytics;

  /**
   * The following files will be ignored when calculating fingerprints and total
   * size of a mod - these are usually files that are dynamically created during
   * runtime and aren't part of the original mod.
   * Note - Keep filenames in lower case.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  TSet<FString> ignoredDynamicModFiles{ "assetregistry.bin" };

  /**
   * Unmanaged mods are mods that exist in the user's mods directory but weren't
   * installed properly via the plugin - usually, these mods don't exist on
   * CurseForge. This is useful for allowing creators to play with their
   * creations prior to uploading them to the CurseForge servers.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore", meta = (MultiLine = "true"))
  FCFCoreSettingsUnmanagedMods unmanagedMods;

  /**
   * Subscriptions is the ability of the user to subscribe to mods, and have
   * them auto-managed by the plugin.
   * Make sure you game supports subscriptions by contacting us.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsSubscriptions subscriptions;

  /**
   * Contains settings related to chunked downloads.
   */
  UPROPERTY(config, EditAnywhere, Category = "cfcore")
  FCFCoreSettingsDownloads downloads;
};
