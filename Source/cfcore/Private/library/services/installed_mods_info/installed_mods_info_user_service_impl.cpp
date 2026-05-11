/* MIT License

Copyright (c) 2025 Overwolf Ltd.

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
SOFTWARE. */
#include "installed_mods_info_user_service_impl.h"

#include <library/common/cfcore_per_user_library_utils.h>

#include <common/constants.h>
#include <common/macros.h>
#include <common/future_utils.h>
#include <common/container_utils.h>

#include <services/logger/logger_macros.h>
#include <services/user_context/user_context_service.h>
#include <services/settings_service.h>

#include <Json.h>
#include <JsonUtilities.h>

namespace cfcore {

using InstallModsInfoResult = IInstalledModsInfoService::InstallModsInfoResult;

}

using namespace cfcore;

// -----------------------------------------------------------------------------
InstalledModsInfoUserServiceImpl::InstalledModsInfoUserServiceImpl(
  TSharedRef<IInstalledModsInfoService> decorated_installed_mods_info,
  TSharedRef<IUserContextService> user_context_service,
  TSharedRef<ISettingsService> settings_service) :
  decorated_installed_mods_info_(decorated_installed_mods_info),
  user_context_service_(user_context_service),
  settings_service_(settings_service) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::Initialize() {
  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
    [this]() { return decorated_installed_mods_info_->Initialize(); },
    [this]() { return AssureLibraryMigrated(); },
    [this]() { return AssureStaleModsRemoved(); }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TOptional<FInstalledMod> InstalledModsInfoUserServiceImpl::GetInstalledMod(
  int64 mod_id) {

  TOptional<FInstalledMod> opt_installed_mod =
    decorated_installed_mods_info_->GetInstalledMod(mod_id);

  if (!opt_installed_mod) {
    return {};
  }

  FUserContextInfo user_info = user_context_service_->UserContextInfo();
  if (!UserLibUtils::ContainsMod(user_info, *opt_installed_mod)) {
    return {};
  }

  UserLibUtils::ApplyUserStateToInstalledMod(user_info, *opt_installed_mod);

  return opt_installed_mod;
}

// -----------------------------------------------------------------------------
TOptional<FInstalledMod> InstalledModsInfoUserServiceImpl::GetInstalledMod(
  const FGuid& id) {

  TOptional<FInstalledMod> opt_installed_mod =
    decorated_installed_mods_info_->GetInstalledMod(id);

  if (!opt_installed_mod) {
    return {};
  }

  FUserContextInfo user_info = user_context_service_->UserContextInfo();
  if (!UserLibUtils::ContainsMod(user_info, *opt_installed_mod)) {
    return {};
  }

  UserLibUtils::ApplyUserStateToInstalledMod(user_info, *opt_installed_mod);

  return opt_installed_mod;
}

// -----------------------------------------------------------------------------
void InstalledModsInfoUserServiceImpl::GetInstalledMods(
  TArray<FInstalledMod>& out_installed_mods) {

  out_installed_mods.Empty();

  TArray<FInstalledMod> system_installed_mods;
  decorated_installed_mods_info_->GetInstalledMods(system_installed_mods);

  FUserContextInfo user_info = user_context_service_->UserContextInfo();

  for (const FInstalledMod& installed_mod : system_installed_mods) {
    if (!UserLibUtils::ContainsMod(user_info, installed_mod)) {
      continue;
    }

    FInstalledMod installed_mod_copy = installed_mod;
    UserLibUtils::ApplyUserStateToInstalledMod(user_info, installed_mod_copy);

    out_installed_mods.Add(installed_mod_copy);
  }

  out_installed_mods.Sort([](const FInstalledMod& a, const FInstalledMod& b) {
    return a.loadOrder < b.loadOrder;
  });
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoUserServiceImpl::SetInstalling(
  TSharedRef<FInstallModParams> inst_params) {

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);

  TOptional<FInstalledMod> opt_inst_mod = GetInstalledMod(inst_params->mod.id);
  const FFile& file = inst_params->file;
  int64 mod_id = inst_params->mod.id;

  TFuture<InstallModsInfoResult> future;

  if (!opt_inst_mod || UserLibUtils::RequiresInstall(*opt_inst_mod, file)) {
    future = decorated_installed_mods_info_->SetInstalling(inst_params);
  } else {
    InstallModsInfoResult res{ FCFCoreError(), *opt_inst_mod };
    future = MakeFulfilledPromise<InstallModsInfoResult>(res).GetFuture();
  }

  future.Next([this, promise, mod_id](InstallModsInfoResult res) {
    if (res.Key.isError) {
      CFCORE_LOG(Error, TEXT("Failed set installing for mod (%lld)"), mod_id);
      promise->SetValue(res);
      return;
    }

    auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
    auto installed_mod_shared = MakeShared<FInstalledMod>(res.Value);

    functions->Add([this, installed_mod_shared]() {
      return LinkUserToMod(*installed_mod_shared);
    });

    if (!IsModRegistered(*installed_mod_shared)) {
      functions->Add([this, installed_mod_shared]() {
        return SetDefaultModState(*installed_mod_shared);
      });
    }

    auto future = FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
    future.Next([promise, res](FCFCoreError err) {
      promise->SetValue(InstallModsInfoResult{ err, res.Value });
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoUserServiceImpl::SetInstalled(
  const FInstalledMod& inst_mod,
  const FInstallModAdditionalParams& params) {

  return decorated_installed_mods_info_->SetInstalled(inst_mod, params);
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult>
InstalledModsInfoUserServiceImpl::SetInstallFailed(
  const FInstalledMod& installed_mod) {

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);

  FInstalledMod inst_mod_copy = installed_mod;

  UnlinkUserFromMod(inst_mod_copy)
    .Next([this, promise, inst_mod_copy](FCFCoreError err) {

    int64 mod_id = inst_mod_copy.details.id;
    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to unlink user from mod (%lld)"), mod_id);
      promise->SetValue(InstallModsInfoResult{ err, inst_mod_copy });
      return;
    }

    TOptional<FInstalledMod> opt_updated_mod =
      decorated_installed_mods_info_->GetInstalledMod(mod_id);

    if (!opt_updated_mod) {
      promise->SetValue(InstallModsInfoResult{ FCFCoreError(), inst_mod_copy });
      return;
    }

    FInstalledMod updated_mod = opt_updated_mod.GetValue();

    if (updated_mod.users.Num() > 0) {
      promise->SetValue(InstallModsInfoResult{ FCFCoreError(), updated_mod });
      return;
    }

    decorated_installed_mods_info_->SetInstallFailed(updated_mod)
      .Next([promise](InstallModsInfoResult res) {

      promise->SetValue(res);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::SetInstalledProperties(
  const TArray<FInternalInstalledModInfoProperties>& properties) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  decorated_installed_mods_info_->SetInstalledProperties(properties)
    .Next([this, properties, promise](FCFCoreError err) {

    if (err.isError) {
      CFCORE_LOG(Error,
        TEXT("Failed to set (system) installed mods properties"));
      promise->SetValue(err);
      return;
    }

    FUserContextInfo user_info = user_context_service_->UserContextInfo();
    UserLibUtils::ApplyPropsToUserMods(user_info, properties);

    user_context_service_->SetInstalledMods(user_info.installedMods)
      .Next([promise](FCFCoreError err) {

      if (err.isError) {
        CFCORE_LOG(Error, TEXT("Failed to set user installed mods properties"));
      }

      promise->SetValue(err);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoUserServiceImpl::SetUnmanagedInstalledMods(
  const TArray<FInstalledMod>& unmanaged_mods) {

  if (unmanaged_mods.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
      [this, unmanaged_mods]() {
        return decorated_installed_mods_info_->SetUnmanagedInstalledMods(
          unmanaged_mods);
      },
      [this, unmanaged_mods]() {
        return RegisterUnmanagedModsToUser(unmanaged_mods);
      },
      [this, unmanaged_mods]() {
        return RegisterUserToMods(unmanaged_mods);
      }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoUserServiceImpl::Uninstall(
  int64 mod_id) {

  TOptional<FInstalledMod> opt_inst_mod = GetInstalledMod(mod_id);

  if (!opt_inst_mod) {
    CFCORE_LOG(Error, TEXT("Couldn't find installed mod (%lld)"), mod_id);
    FCFCoreError err(ECFCoreErrorCodes::InstalledModNotFound);
    InstallModsInfoResult res{err, FInstalledMod()};
    return MakeFulfilledPromise<InstallModsInfoResult>(res).GetFuture();
  }

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);

  FInstalledMod installed_mod = opt_inst_mod.GetValue();

  UnlinkUserFromMod(installed_mod)
    .Next([this, promise, installed_mod, mod_id](FCFCoreError err) {

    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to unlink user from mod (%lld)"), mod_id);
      promise->SetValue(InstallModsInfoResult{ err, installed_mod });
      return;
    }

    TOptional<FInstalledMod> opt_updated_mod =
      decorated_installed_mods_info_->GetInstalledMod(mod_id);
    if (!opt_updated_mod) {
      CFCORE_LOG(Error, TEXT("Couldn't find installed mod after unlink (%lld)"),
        mod_id);
      FCFCoreError error(ECFCoreErrorCodes::InstalledModNotFound);
      promise->SetValue(InstallModsInfoResult{ error, FInstalledMod() });
      return;
    }

    FInstalledMod updated_mod = opt_updated_mod.GetValue();
    if (!UserLibUtils::RequiresUninstall(updated_mod)) {
      promise->SetValue(InstallModsInfoResult{ FCFCoreError(), updated_mod });
      return;
    }

    auto future = decorated_installed_mods_info_->Uninstall(mod_id);

    future.Next([promise](InstallModsInfoResult res) {
      promise->SetValue(res);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult>
InstalledModsInfoUserServiceImpl::Uninstall(const FGuid& id) {

  TOptional<FInstalledMod> opt_inst_mod = GetInstalledMod(id);
  if (!opt_inst_mod) {
    CFCORE_LOG(Error, TEXT("Couldn't find installed mod (%s)"), *id.ToString());
    FCFCoreError err(ECFCoreErrorCodes::InstalledModNotFound);
    InstallModsInfoResult res{ err, FInstalledMod() };
    return MakeFulfilledPromise<InstallModsInfoResult>(res).GetFuture();
  }

  return Uninstall(opt_inst_mod->details.id);
}

// -----------------------------------------------------------------------------
const FFile* InstalledModsInfoUserServiceImpl::GetLatestFile(
  const FCFCoreMod& mod) {

  return decorated_installed_mods_info_->GetLatestFile(mod);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::UpdateModDetails(
  const TArray<FCFCoreMod>& mods) {

  return decorated_installed_mods_info_->UpdateModDetails(mods);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoUserServiceImpl::AssureLibraryMigrated() {
  const FSharedContextInfo shared_context_info =
    user_context_service_->SharedContextInfo();

  if (shared_context_info.performedLibraryMigration) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  return MigrateLibrary();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoUserServiceImpl::AssureStaleModsRemoved() {

  TArray<FInstalledMod> installed_mods;
  decorated_installed_mods_info_->GetInstalledMods(installed_mods);

  FUserContextInfo user_info = user_context_service_->UserContextInfo();
  int32 removed_count = user_info.installedMods.RemoveAll(
    [installed_mods](FUserModState user_mod_state) {

    bool system_installed = installed_mods.ContainsByPredicate(
      [user_mod_state](const FInstalledMod& installed_mod) {

      return installed_mod.id == user_mod_state.id;
    });

    return !system_installed;
  });

  if (removed_count == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  return user_context_service_->SetInstalledMods(user_info.installedMods);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::MigrateLibrary() {
  auto installed_mods = MakeShared<TArray<FInstalledMod>>();
  decorated_installed_mods_info_->GetInstalledMods(*installed_mods);

  TArray<FUserModState> user_mods;
  for (const FInstalledMod& installed_mod : *installed_mods) {
    FUserModState state = FUserModState::FromInstalledMod(installed_mod);
    user_mods.Add(state);
  }

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
    TArray<TSequenceFunction<FCFCoreError>> {
    [this, user_mods]() {
      return user_context_service_->SetInstalledMods(user_mods);
    },
    [this, installed_mods]() {
      return RegisterUserToMods(*installed_mods);
    },
    [this]() {
      return user_context_service_->SetLibraryMigrationPerformed();
    }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::RegisterUserToMods(
  const TArray<FInstalledMod>& installed_mods) {

  TArray<FInternalInstalledModInfoProperties> props_array;

  for (const FInstalledMod& installed_mod : installed_mods) {
    TSet<FString> users = installed_mod.users;
    users.Add(GetUserContextId());

    FInternalInstalledModInfoProperties properties;
    properties.id = installed_mod.id;
    properties.modId = installed_mod.details.id;
    properties.users = users;
    props_array.Add(properties);
  }

  return SetInstalledProperties(props_array);
}

// -----------------------------------------------------------------------------
void InstalledModsInfoUserServiceImpl::ApplyUserStateToInstalledMod(
  FInstalledMod& installed_mod) {

  FUserContextInfo user_info = user_context_service_->UserContextInfo();

  if (UserLibUtils::ContainsMod(user_info, installed_mod)) {
    UserLibUtils::ApplyUserStateToInstalledMod(user_info, installed_mod);
  }
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::RegisterModToUser(
  const FInstalledMod& installed_mod) {

  if (IsModRegistered(installed_mod)) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  FUserContextInfo user_info = user_context_service_->UserContextInfo();
  FUserModState state = FUserModState::FromInstalledMod(installed_mod);
  user_info.installedMods.Add(state);
  return user_context_service_->SetInstalledMods(user_info.installedMods);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::UnregisterModFromUser(
  const FInstalledMod& installed_mod) {

  if (!IsModRegistered(installed_mod)) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  FUserContextInfo user_info = user_context_service_->UserContextInfo();
  user_info.installedMods.RemoveAll([installed_mod](FUserModState state) {
    return state.id == installed_mod.id;
  });

  return user_context_service_->SetInstalledMods(user_info.installedMods);
}

// -----------------------------------------------------------------------------
bool InstalledModsInfoUserServiceImpl::IsModRegistered(
  const FInstalledMod& installed_mod) {

  FUserContextInfo user_info = user_context_service_->UserContextInfo();
  return UserLibUtils::FindModById(user_info, installed_mod.id) != nullptr;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::RegisterUserToMod(
  FInstalledMod& installed_mod) {

  installed_mod.users.Add(GetUserContextId());
  return SaveInstalledModUsers(installed_mod);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::UnregisterUserFromMod(
  FInstalledMod& installed_mod) {

  installed_mod.users.Remove(GetUserContextId());
  return SaveInstalledModUsers(installed_mod);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::LinkUserToMod(
  FInstalledMod& installed_mod) {

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  auto installed_mod_shared = MakeShared<FInstalledMod>(installed_mod);

  functions->Add([this, installed_mod_shared]() {
    return RegisterModToUser(*installed_mod_shared);
  });

  functions->Add([this, installed_mod_shared]() {
    return RegisterUserToMod(*installed_mod_shared);
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::UnlinkUserFromMod(
  FInstalledMod& installed_mod) {

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  auto installed_mod_shared = MakeShared<FInstalledMod>(installed_mod);

  functions->Add([this, installed_mod_shared]() {
    return UnregisterModFromUser(*installed_mod_shared);
  });

  functions->Add([this, installed_mod_shared]() {
    return UnregisterUserFromMod(*installed_mod_shared);
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
FString InstalledModsInfoUserServiceImpl::GetUserContextId() {
  FString user_id = settings_service_->GetSettings().initOptions.userContextId;
  if (user_id.IsEmpty()) {
    return TEXT("local");
  }

  return user_id;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::SaveInstalledModUsers(
  const FInstalledMod& installed_mod) {

  FInternalInstalledModInfoProperties props;
  props.modId = installed_mod.details.id;
  props.users = installed_mod.users;

  TArray<FInternalInstalledModInfoProperties> props_arr{ props };
  return SetInstalledProperties(props_arr);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::SetDefaultModState(
  FInstalledMod& installed_mod) {

  TArray<FInternalInstalledModInfoProperties> props_array;

  TArray<FInstalledMod> sys_installed_mods;
  decorated_installed_mods_info_->GetInstalledMods(sys_installed_mods);

  UserLibUtils::ShiftInstalledMods(sys_installed_mods, props_array);
  UserLibUtils::PushModToFront(installed_mod, props_array);
  UserLibUtils::EnableMod(installed_mod, props_array);

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  auto props_array_shared =
    MakeShared<TArray<FInternalInstalledModInfoProperties>>(props_array);

  functions->Add([this, props_array_shared](){
    return SetUserModStates(*props_array_shared);
  });

  functions->Add([this, props_array_shared]() {
    return SetInstalledProperties(*props_array_shared);
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoUserServiceImpl::SetUserModStates(
  const TArray<FInternalInstalledModInfoProperties>& props_array) {

  FUserContextInfo user_info = user_context_service_->UserContextInfo();

  const TArray<FUserModState>& user_mods_modified =
    UserLibUtils::ApplyPropsToUserMods(user_info, props_array);

  return user_context_service_->SetInstalledMods(user_mods_modified);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoUserServiceImpl::RegisterUnmanagedModsToUser(
  const TArray<FInstalledMod>& unmanaged_mods) {

  FUserContextInfo user_info = user_context_service_->UserContextInfo();

  for (const FInstalledMod& installed_mod : unmanaged_mods) {
    if (UserLibUtils::ContainsMod(user_info, installed_mod)) {
      continue;
    }

    FUserModState state = FUserModState::FromInstalledMod(installed_mod);
    user_info.installedMods.Add(state);
  }

  return user_context_service_->SetInstalledMods(user_info.installedMods);
}
