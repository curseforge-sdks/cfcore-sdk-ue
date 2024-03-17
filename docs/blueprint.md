## The Blueprint interface

You access the Blueprint interface via the CFCoreSubsystem module:

![cfcoresubsystem](/docs/assets/images/cfcoresubsystem.jpg)

### Initializing the SDK

- Add a CFCoreSubsystem node and connect it to the Initialize method
- Use a Make Settings from Project Config node to set the initialization
settings as set in the plugin settings
- If we fail to initialize, the On Error delegate will trigger
- Otherwise, if we succeed, the On Initialized delegate will trigger - use a
Custom Event node to handle

![blueprint initialize](/docs/assets/images/blueprint_initialize.jpg)

> **_NOTE:_** To set the plugin settings - select Edit > Project Settings...
> from the main menu of Unreal - then search for CFCore
>
> ![plugin settings](/docs/assets/images/plugin_settings.jpg)
>

The following escape tokens are supported in the two directory fields:
- %USER_DIR% - FPlatformProcess::UserDir()
- %USER_SETTINGS_DIR% - FPlatformProcess::UserSettingsDir()
- %PROJECT_DIR% - FPaths::ProjectDir()
- %PROJECT_SAVED_DIR% - FPaths::ProjectSavedDir()

### Getting installed mods

- Add a CFCoreSubsystem node and connect it to the Get Installed Mods method
- Add a For Each Loop on the array and if the mod doesn't have the Pending (or Invalid) status, you can use it (e.g. print it's name)

![blueprint get isntalled mods](/docs/assets/images/blueprint_get_installed_mods.jpg)

### Querying server mods information

- Add a CFCoreSubsystem node and connect it to the Search Mods Info method
- Use a Make Search Mods Filter node to set the search filters - setting values
to 0, empty strings or Enum None/Any will set them to the defaults
- Use a Make Api Request Pagination if you want to get the next page results
- If the query fails - we'll get an OnError event
- If the query succeeds - we'll get an OnResults event with an array of mod
information - we can then use a for...each to access each result

![blueprint querying mods](/docs/assets/images/blueprint_querying_mods.jpg)

### Installing a mod

> **NOTE:** The Library API is not yet implemented in the current release

- Get mod information for the mod you wish to install - using the a method such
as Search Mods Info or Get Mod Info By Id
- Use the Install Mod method and pass the above mentioned mod to the node
- You may listen on the progress - via the OnProgress delegate
- If the mod is installed successfully - you'll receive the On Installed event

![blueprint install mod](/docs/assets/images/blueprint_install_mod.jpg)
