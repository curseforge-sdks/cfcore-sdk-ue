## Design decisions

Here are some of our design decisions for the SDK:

- Support both Blueprint and C++ interfaces
- Use native UE APIs and types (for performance, simplicity and portability)
- All functionality should be asynchronous
- Keep the implementation as simple as possible, allowing developers to easily
maintain their own fork or contribute to the code-base

## Threading model

The assumption is that all calls to the SDK are performed on the game's main
thread. We utilize this to refrain from using locks.

For intensive CPU functionality (compression) or IO (read/write to disk) we use
separate threads so that we do not block the game loop.

To make things predictable and more human-error-proof, the rule of thumb is that
every async operation will callback on the game's main thread.

## The SDK plugins

Currently, the SDK contains a 3 plugins:
- cfcore - the business logic plugin - search, install and manage mods locally,
without the user interface.
- cfcore_ui - the user interface mod browser based on UMG.
- [cfeditor](https://github.com/curseforge-sdks/cf-ue-editor-plugin) - UE Editor
plugin that allows for the integration of game editors with CurseForge.

### The cfcore plugin

The SDK module contains the following parts:

- **API** - for querying game and mods information - see https://docs.curseforge.com
- **Library** - for managing the local mods library (installing, updating,
validating etc...)
- **Authentication** - for easy authentication of players with the CurseForge
Core backend (e.g. for voting, reporting etc...)
- **Creation** - for creating mods and uploading mod file revisions
- **PremiumMods** - for getting information about premium mods and allowing the
user to purchase a mod

The API supports 2 development "flavors":

- Blueprint - Using the **CFCoreSubsystem** you can interact with all the API
sections via the UE Blueprint visual scripting system - see
[blueprint](blueprint.md)
- C++ - Using the |CFCoreContext| singleton class - you can access the |ICFCore|
interface and interact with all the above mentioned sections - see [c++](cpp.md)

### The cfcore_ui plugin

We provide another plugin (cfcore_ui), a UMG in-game mod browser that you can
use to add support for browsing and managing mods inside the game.
The relevant docs are found in that project.

## Supporting mods in your game (getting started)

In order to use the CFCore SDK, your game must first, progrmatically, support
mods (e.g. support scripting mods, texture mods etc...).

Your game should also decide in which folder mods are stored on the user's disk
and at which point the game loads installed mods from the disk.

Some examples can be: When the user clicks the Start Play from the main
menu, when loading a new world, right before a game match starts etc...

In order to know which mods are ready to be loaded (e.g. fully installed and
enabled by the current user), use the GetInstalledMods method of the Library
section (or Get Installed Mods via blueprints).
This method returns an array of all the installed mods relevant to the user's
context - each installed mod includes properties that can help decide if the mod
is ready for loading:
- **status** - where Pending, Invalid means the mod cannot be loaded
- **enabled** - whether or not the user enabled/disabled this mod
- **pathOnDisk** - where to load the mod from, relative to the modding folder

Currently, there are two folder modes that are supported by the SDK:
 - CFCore - mods installed by the SDK will reside in sub-folders with the name of [modId]_[fileId] and then the contents of the mod will be inside
 - Flat - mods installed by the SDK will reside directly under the designated modding folder set by the game.

It might not be a good idea to use Flat directly inside the game's mods folder,
as you might not have control over mods that are enabled/disabled or premium
mods - should you work with the UE's mods APIs.

The method of storing mods on disk is decided in the CFCore settings, under the
modsDirectoryMode setting.

## Authentication

Some features require the user to be authenticated - these include: cross
device mod subscriptions, rating, reporting, uploading creations etc...

> **NOTE:** Some of the above mentioned featurs are still in the works

We currently support the following authentication methods:
 - **Email** - Users enter their email and send a request to the server. The server sends an email with a secret code to the user, which can then use it to generate an authentication token.
 - **Steam seamless authentication** - A game running from Steam can get an
encrypted app ticket for the user (via the Steam Online Subsystem) and then
send it to the server to generate an authentication token.
This also requires a setup on the Eternal console, so please contact us should
you choose to use it.
- **PlayStation Network seamless authentication** - A game running on PlaySation
can silently authenticate the user.
- **Xbox Live seamless authentication** - A game running on Xbox
can silently authenticate the user.

More providers will be added soon.

In order to enable authentication for your game, please use the Authentication
section of the CurseForge for studios console (https://console.curseforge.com).

Some providers require additional information - this is explained in the
console. For further questions about authentication, please contact us.

When a user performs a seamless authentication - an anonymous user
will be created on CurseForge. In order to associate the anonymous user with
an actual CurseForge user - please direct the user to:
https://www.curseforge.com/account/connected-accounts.

## Premium Mods

A game can support premium mods - meaning paid mods.

To get started - contact us and we'll enable it in the console.

Use the CFCore.PremiumMods interface to implement support for paid mods in your
game.

For extra security, all premium mods check can be signed by the server with a
private key and validated on the client with a public key. To enable this, setup
the private key on the console (under the UGC Monetization settings screen) and
set the matching public key in the cfcore settings (premiumMods).

> **NOTE:** It is even safer to use the ICFCorePremiumMods.OverridePublicKey
> instead of setting it in the Project Settings, so that user's can't override
> it. You'll need to store the public key in-memory.


> **NOTE:** Creating a private/public key pair with openssl:
> 1. openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048
> 2. openssl rsa -pubout -in private_key.pem -out public_key.pem


- A premium mod will have the FCFCoreMod.premiumDetails field filled with
information such as the price and currency for the mod.
- When a game match/session starts - use the ICFCorePremiumMods.CheckMods api
("Premium Mods Check" BP function). This requires the user to be online.
- When joining a server with premium mods - the SDK will perform the validation
for you - via AssureClientModsUpdated ("Assure Client Mods Updated" in BP).
- On PC, for purchasing a mod - call the
ICFCoreApi.ICFCoreApiAuthorized.GeneratePremiumCheckoutUrl and open the user's
default browser to the url returned. You'll need to poll (with a 2 sec timer)
the CheckMods api so that you know when the user is ready to install the mod.
- On console, fur purchasing a mod - call the
ICFCoreApi.ICFCoreApiAuthorized.InitiatePurchase to get the relevant details
for purchasing on the user device (keep track of the transaction id returned)
and once the local purchase is completed, call the
ICFCoreApi.ICFCoreApiAuthorized.FinalizePurchase function to complete the
purchase

## Logging

The cfcore plugin has some internal logs that are being written via the standard
UE_LOG macro. However, most games do not always enable UE_LOG to be written to
disk when shipping.

The plugin logs are important to have in production, so that we can analyze and
troubleshoot issues to users that might have problems installing mods.

This is why the plugin supports a setting to save log files on disk via the
cfcore settings (in the project settings).

If enabled, you can control the max size of a single log file and the max number
of old (historic) logs to save on disk.

## Install Mod Additional Parameters

Read the following section to understand the different mod installation
parameters you can use when calling the BP "Install Mod Extended" function or
the c++ ICFCoreLibrary::Install function (the additional params override one).

### tracking

This key-value map allows sending tracking events to the download. We use this
for creating custom partner reports with additional annotations. For example,
how many mod installations are perfomed by the user via the Browser UI versus
how many mod installations were performed as part of the user joining a server.

### throttle_download_kbps

This parameter allows throttling the download of mods to a certain kilobytes per
second rate. The unit of this field is kb - so 100 = 100KB = 102,400 bytes.

When set to 0 (default) there is not throttling.

You might want to use this if performing mod installations in the background,
while the game is running, and you don't want to hurt the user's ping.

### dynamicContent

Dynamic content means mods (usually cosmetics) which are downloaded
automatically by the game so that all users can share visuals.

A usecase for dynamic content is when in a multiplayer session, one player has a
skin/cosmetic and we want all other players to be able to see the skin as played
by the user who owns it. In this case, the game can start downloading the
relevant mod (for this skin) so that it resides on all player's computers but is
not considered installed - meaning they cannot use the skin, only view those who
actually own it.

When FInstalledMod.dynamicContent is true, it means the user has the mod
installed but does not own it or didn't manually request to install it.

If later the user decides to purchase/install it (depending on the mod being
premium or not) - the reguler installation functions (BP or C++) will assure the
mod is switched from have FInstalledMod.dynamicContent being true to false.

> **NOTE:** If you plan on hosting premium dynamic content - it is important to
> fill in the |dynamicContentCategoryIds| cfcore setting - so that the SDK will
> enforce a premium ownership check for dynamic content (in case a user
> manipulates the local library metadata files on their device).
