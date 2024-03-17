## The C++ interface

You access the C++ interface via the |CFCoreContext| singleton class.

- CFCoreContext::GetInstance() returns ICFCore*
- CFCoreContext::GetInstance()->Api() returns ICFCoreApi*
- CFCoreContext::GetInstance()->Library() returns ICFCoreLibrary*
- CFCoreContext::GetInstance()->Authentication() returns ICFCoreAuthentication*

### Initializing the SDK

You'll need your game id and game api key from [CFCore Console](https://console.curseforge.com)

> **_NOTE:_** Uninitialzing is done in a similar manner (with the Uninitialized
> method)

```c++
#include <cfcore_context.h>
#include <editor/cfcore_bp_library.h>

...

const int32 kGameId = [YOUR GAME ID];
const TCHAR kGameApiKey[] = TEXT("[YOUR GAME API KEY]");

// How many concurrent installations can take place
const int32 kMaxConcurrentInstallations = 3;

...

void ATestGameMode::StartPlay() {
  Super::StartPlay();

  // FCFCoreSettings settings =
  //   UCFCoreBPLibrary::MakeSettingsFromProjectConfig();
  //
  // or:
  FCFCoreSettings settings;
  settings.gameId = kGameId;
  settings.apiKey = TEXT(kGameApiKey);
  settings.maxConcurrentInstallations = kMaxConcurrentInstallations;

  // Where extracted mods are saved locally - supporting the following escape
  // tokens:
  // %USER_DIR% - FPlatformProcess::UserDir()
  // %USER_SETTINGS_DIR% - FPlatformProcess::UserSettingsDir()
  // %PROJECT_DIR% - FPaths::ProjectDir()
  // %PROJECT_SAVED_DIR% - FPaths::ProjectSavedDir()
  settings.modsDirectory = TEXT("%USER_DIR%/demo_game/mods");

  // Where the user's settings (enabled mods, auth token etc...) will be saved
  settings.userDataDirectory = TEXT("%USER_DIR%/demo_game/mods/.eternal");

  CFCoreContext::GetInstance()->Initialize(
    settings,
    ICFCore::InitializeDelegate::CreateUObject(
      this,
      &ATestGameMode::OnCFCoreInitialized));
}

...

void ATestGameMode::OnCFCoreInitialized(TOptional<FCFCoreError> optional_err) {
  if (optional_err.IsSet()) {
    FCFCoreError error = err.GetValue();
    UE_LOG(LogTemp, Display, TEXT("Error to init: %s!"), *error.description);
    return;
  }

  ...
}

```

> **NOTE:** Use the |UCFCoreBPLibrary::MakeSettingsFromProjectConfig| static method to get the CFCore plugin from the Project Settings (Edit > Project Settings... from the menu)

### Get installed mods

```c++
#include <cfcore_context.h>

...

CFCoreContext::GetInstance()->Library()->GetInstalledMods(
  ICFCoreLibrary::FGetInstalledModsDelegate::CreateLambda(
    [](const TArray<FInstalledMod>& installed_mods) {

    for (FInstalledMod installed_mod : installed_mods) {
      if ((installed_mod.status != EInstalledModStatus::Pending) &&
          (installed_mod.status != EInstalledModStatus::Invalid) &&
          (installed_mod.enabled)) {
        UE_LOG(LogTemp, Display, TEXT("Loading %s"), installed_mod.pathOnDisk);
      }
    }
  })
);

```

### Querying server mods information

```c++
#include <cfcore_context.h>
#include <api/common/api_response_pagination.h>

...

void ATestGameMode::SearchMods() {
  FSearchModsFilter filter;
  filter.sortOrder = ESortOrder::Desc;
  filter.sortField = EModsSearchSortField::Popularity;
  // see https://docs.curseforge.com/#get-version-types
  filter.gameVersionTypeId = 517;
  filter.searchFilter = TEXT("deadly boss");

  FApiRequestPagination pagination;

  CFCoreContext::GetInstance()->Api()->SearchMods(
    filter,
    pagination,
    ICFCoreApi::FSearchModsDelegate::CreateLambda(
      [](TOptional<TArray<FCFCoreMod>> opt_mods,
         TOptional<FApiResponsePagination> opt_pagination,
         const TOptional<FCFCoreApiResponseError>& opt_err) {

        if (opt_err.IsSet()) {
          FString desc = opt_err.GetValue().description;
          UE_LOG(LogTemp, Display, TEXT("Error: %s"), *desc);
          return;
        }

        TArray<FCFCoreMod> mods = opt_mods.GetValue();
        for (const FCFCoreMod& mod : mods) {
          UE_LOG(LogTemp, Display, TEXT("Mod: %s (%d)"), *mod.name, mod.id);
        }
      })
  );
}
```

### Installing a mod

```c++
#include <cfcore_context.h>

...

void ATestGameMode::InstallMod() {
  TOptional<FCFCoreMod> opt_mod = QueryModById(3358);
  if (!opt_mod.IsSet()) {
    return;
  }

  CFCoreContext::GetInstance()->Library()->Install(
    opt_mod.GetValue(),
    ICFCoreLibrary::InstallProgressDelegate::CreateLambda(
      [](const FLibraryProgress& progress) {
        FString state = UEnum::GetDisplayValueAsText(progress.state).ToString();
        int32 progress = progress.dataTransfer.progress;
        UE_LOG(LogTemp, Display, TEXT("State: %s (%d%%)"), *state, progress);
      }
    ),
    ICFCoreLibrary::InstalledDelegate::CreateLambda(
      [](TOptional<FInstalledMod> opt_mod, const TOptional<FCFCoreError>& err) {
        if (err.IsSet()) {
          // ...
          return;
        }

        FInstalledMod installed_mod = opt_mod.GetValue();
        FString name = installed_mod.details.name;
        UE_LOG(LogTemp, Display, TEXT("Successfully installed: %s"), *name);
      }
    )
  );
}
```


### Authentication via Email

```c++
#include <cfcore_context.h>

...
void ATestGameMode::SendEmailCode(const FString& email) {
  CFCoreContext::GetInstance()->Authentication()->SendSecurityCode(
    email,
    ICFCoreAuthentication::FSendSecurityCodeDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          FString desc = opt_err.GetValue().description;
          UE_LOG(LogTemp,
                 Error,
                 TEXT("Error sending security code (%s)"),
                 *desc);
          return;
        }

        // handle successfully sending a request for code
      })
  );
}

...

// -----------------------------------------------------------------------------
void ATestGameMode::GetAuthToken(const FString& email, int32 security_code) {
  CFCoreContext::GetInstance()->Authentication()->GenerateAuthToken(
    email,
    security_code,
    ICFCoreAuthentication::FGenerateAuthTokenDelegate::CreateLambda(
      [=](const TOptional<FCFCoreError>& opt_err) {
        if (opt_err.IsSet()) {
          FString desc = opt_err.GetValue().description;
          UE_LOG(LogTemp,
                 Error,
                 TEXT("Error generating auth token (%s)"),
                 *desc);
          return;
        }

        // handle authentication (e.g. get user profile via api)
      })
  );
}
```