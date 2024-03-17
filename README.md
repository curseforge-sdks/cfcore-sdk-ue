# cfcore-sdk-ue

The CurseForge Core SDK for Unreal Engine allows games to add mod management
support, in-game, with a simple API.

It is open source and uses the MIT license.

The SDK is a wrapper for working with the
[CFCore API](https://docs.curseforge.com) - allowing to browse mods for your
game, install mods locally, keep them up-to-date and more.

## Getting started

After creating an account on https://console.curseforge.com and setting up your
game (feel free to [contact us](mailto:CFcore@overwolf.com)):

- Download the latest [release](https://github.com/overwolf/eternal-sdk-ue/releases) zip file from our github repo and then extract the cfcore-sdk-ue folder's **CONTENTS**
into your game's plugins directory under: Plugins/cfcore.

  ![plugins folder](/docs/assets/images/plugins_folder.jpg)

- Restart your project and you should see a "Missing Modules" message box asking
you to rebuild the cfcore plugin - click yes to build

  ![missing modules](/docs/assets/images/missing_modules.jpg)

  ![build](/docs/assets/images/build.jpg)

- Make sure that the plugin is enabled via the Edit > Plugins menu (search for
cfcore)

  ![plugin enabled](/docs/assets/images/plugin_enabled.jpg)


> **_NOTE:_** If you want to use the SDK with C++, you might also need to
> manually add "cfcore" to your game's Build.cs file, under
> PublicDependencyModuleNames, save and then refresh the code via File >
> Refresh Visual Studio project menu option
>
> ![dependency module](/docs/assets/images/dependency_module.jpg)

## Detailed documentation

Please find more detailed docs under the [/docs](/docs/index.md) folder