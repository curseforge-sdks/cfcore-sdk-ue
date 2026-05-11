/* MIT License

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
SOFTWARE. */
#include "installed_mods_info_system_service_impl.h"
#include <common/constants.h>
#include <common/macros.h>

#include <services/logger/logger_macros.h>
#include <Json.h>
#include <JsonUtilities.h>

namespace cfcore {

using InstallModsInfoResult = IInstalledModsInfoService::InstallModsInfoResult;

}

using namespace cfcore;

// -----------------------------------------------------------------------------
InstalledModsInfoSystemServiceImpl::InstalledModsInfoSystemServiceImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IDirectoriesService> directories_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<TModsInfoBroadcaster> delegates) :
  settings_service_(settings_service),
  directories_service_(directories_service),
  file_system_service_(file_system_service),
  cfcore_api_(cfcore_api),
  delegates_(delegates) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoSystemServiceImpl::Initialize() {
  SetLocalLibraryFile();
  return LoadInstalledModsOnDisk();
}

// -----------------------------------------------------------------------------
TOptional<FInstalledMod> InstalledModsInfoSystemServiceImpl::GetInstalledMod(
  int64 mod_id) {

  FInstalledMod* installed_mod = GetInstalledModRefByModId(mod_id);
  if (!installed_mod) {
    return {};
  }

  return *installed_mod;
}

// -----------------------------------------------------------------------------
TOptional<FInstalledMod> InstalledModsInfoSystemServiceImpl::GetInstalledMod(
  const FGuid& id) {

  FInstalledMod* installed_mod = GetInstalledModRefById(id);
  if (!installed_mod) {
    return {};
  }

  return *installed_mod;
}

// -----------------------------------------------------------------------------
void InstalledModsInfoSystemServiceImpl::GetInstalledMods(
  TArray<FInstalledMod>& out_installed_mods) {
  out_installed_mods.Empty();

  for (const FInstalledMod& installed_mod : installed_mods_.installedMods) {
    out_installed_mods.Add(installed_mod);
  }

  out_installed_mods.Sort([](const FInstalledMod& a, const FInstalledMod& b) {
    return a.loadOrder < b.loadOrder;
  });
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoSystemServiceImpl::SetInstalling(
  TSharedRef<FInstallModParams> install_params) {

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);

  if (!IsModValid(install_params->mod)) {
    auto code = ECFCoreErrorCodes::InvalidModParams;
    InstallModsInfoResult result{FCFCoreError(code), FInstalledMod()};
    promise->SetValue(result);
    return promise->GetFuture();
  }

  FInstalledMod* installed_mod = GetInstalledModRefByModId(
    install_params->mod.id);
  if (!installed_mod) {
    FInstalledMod updated_installed_mod = FInstalledMod::CreateNew();
    updated_installed_mod.details = install_params->mod;
    updated_installed_mod.enabled = true;
    installed_mods_.installedMods.Add(updated_installed_mod);
  }

  installed_mod = GetInstalledModRefByModId(install_params->mod.id);
  installed_mod->preInstallStatus = installed_mod->status;
  installed_mod->status = EInstalledModStatus::Pending;
  installed_mod->latestUpdatedFile = install_params->file;

  SetAdditionalParams(*installed_mod, install_params->additional_params);

  SaveChangesAndTrigger(*installed_mod, promise);
  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoSystemServiceImpl::SetInstalled(
  const FInstalledMod& installed_mod,
  const FInstallModAdditionalParams& additional_params) {

  int64 id = installed_mod.details.id;
  FInstalledMod* stored_mod = GetInstalledModRefByModId(id);
  if (!stored_mod) {
    CFCORE_LOG(Error, TEXT("Couldn't find installed mod (%lld)"), id);
    return CreateError(ECFCoreErrorCodes::InstalledModNotFound, installed_mod);
  }

  stored_mod->status = EInstalledModStatus::Normal;
  stored_mod->pathOnDisk = installed_mod.pathOnDisk;
  stored_mod->installedFile = installed_mod.latestUpdatedFile;
  stored_mod->dateInstalled = stored_mod->dateUpdated = FDateTime::Now();

  SetAdditionalParams(*stored_mod, additional_params);

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);
  SaveChangesAndTrigger(*stored_mod, promise);
  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult>
InstalledModsInfoSystemServiceImpl::SetInstallFailed(
  const FInstalledMod& installed_mod) {

  int64 id = installed_mod.details.id;

  FInstalledMod* stored_mod = GetInstalledModRefByModId(id);
  if (!stored_mod) {
    CFCORE_LOG(Error, TEXT("Couldn't find installed mod (%lld)"), id);
    return CreateError(ECFCoreErrorCodes::InstalledModNotFound, installed_mod);
  }

  bool keep_installed_mod = false;
  delegates_->ForEach([stored_mod  = *stored_mod, &keep_installed_mod](
    TSharedRef<IInstalledModsInfoServiceDelegate> listener, bool& break_loop) {

    listener->OnSetInstallFailed(stored_mod, keep_installed_mod);
    if (keep_installed_mod) {
      break_loop = true;
    }
  });

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);

  if (!WasPreviouslyInstalled(*stored_mod)) {
    if (keep_installed_mod) {
      InstallModsInfoResult result{ FCFCoreError(), installed_mod };
      promise->SetValue(result);
      return promise->GetFuture();
    }

    FInstalledMod copy = *stored_mod;
    RemoveInstalledModByModId(id);
    SaveChangesAndTrigger(copy, promise);
    return promise->GetFuture();
  }

  stored_mod->status = EInstalledModStatus::Normal;
  if (stored_mod->installedFile.id < stored_mod->latestUpdatedFile.id) {
    stored_mod->status = EInstalledModStatus::OutOfDate;
  }

  stored_mod->dateUpdated = FDateTime::Now();
  SaveChangesAndTrigger(*stored_mod, promise);
  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoSystemServiceImpl::SetInstalledProperties(
  const TArray<FInternalInstalledModInfoProperties>& properties) {

  if (properties.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  for (const FInternalInstalledModInfoProperties& props : properties) {
    FInstalledMod* stored_mod = nullptr;
    if (props.id.IsSet() && props.id->IsValid()) {
      stored_mod = GetInstalledModRefById(*props.id);
    } else {
      stored_mod = GetInstalledModRefByModId(props.modId);
    }

    if (!stored_mod) {
      CFCORE_LOG(Error, TEXT("Couldn't find installed mod"));
      auto code = ECFCoreErrorCodes::InstalledModNotFound;
      return MakeFulfilledPromise<FCFCoreError>(FCFCoreError(code)).GetFuture();
    }

    FillInstalledModProperties(stored_mod, props);
  }

  return SaveInstalledModsToDisk();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoSystemServiceImpl::SetUnmanagedInstalledMods(
  const TArray<FInstalledMod>& installed_mods) {

  if (installed_mods.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  for (const FInstalledMod& installed_mod : installed_mods) {
    FInstalledMod* existing_mod = GetInstalledModRefById(installed_mod.id);
    if (existing_mod) {
      existing_mod->status = installed_mod.status;
      continue;
    }

    installed_mods_.installedMods.Add(installed_mod);
  }

  return SaveInstalledModsToDisk();
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoSystemServiceImpl::Uninstall(
  int64 mod_id) {

  int64 id = mod_id;
  FInstalledMod* installed_mod = GetInstalledModRefByModId(id);
  if (!installed_mod) {
    CFCORE_LOG(Error, TEXT("Couldn't find installed mod (%lld)"), id);
    return CreateError(ECFCoreErrorCodes::InstalledModNotFound,
                       FInstalledMod());
  }


  FInstalledMod installed_mod_copy = *installed_mod;
  RemoveInstalledModByModId(id);

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);
  SaveChangesAndTrigger(installed_mod_copy, promise);

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoSystemServiceImpl::Uninstall(
  const FGuid& id) {

  FInstalledMod* installed_mod = GetInstalledModRefById(id);
  if (!installed_mod) {
    CFCORE_LOG(Error, TEXT("Couldn't find installed mod (%lld)"), id);
    return CreateError(ECFCoreErrorCodes::InstalledModNotFound,
                       FInstalledMod());
  }

  FInstalledMod installed_mod_copy = *installed_mod;
  RemoveInstalledModById(id);

  SHAREABLE_PROMISE(InstallModsInfoResult, promise);
  SaveChangesAndTrigger(installed_mod_copy, promise);

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
// TODO(twolf): Consider alpha/beta/release parameters
const FFile* InstalledModsInfoSystemServiceImpl::GetLatestFile(
  const FCFCoreMod& mod) {

  if (mod.mainFileId <= 0) {
    return nullptr;
  }

  if (mod.latestFiles.Num() == 0) {
    return nullptr;
  }

  const FFile* main_file = GetMainFile(mod);
  return main_file;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> InstalledModsInfoSystemServiceImpl::UpdateModDetails(
  const TArray<FCFCoreMod>& mods_details) {

  for (const FCFCoreMod& detail : mods_details) {
    FInstalledMod* stored_mod = GetInstalledModRefByModId(detail.id);
    if (!stored_mod) {
      continue;
    }

    UpdateInstalledMod(*stored_mod, detail);
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);
  SaveInstalledModsToDisk().Next([promise](FCFCoreError err) {
    promise->SetValue(err);
  });

  return promise->GetFuture();

}

// -----------------------------------------------------------------------------
void InstalledModsInfoSystemServiceImpl::SetLocalLibraryFile() {
  FString user_data_dir = directories_service_->GetUserDataDirectory();
  int64 game_id = settings_service_->GetSettings().gameId;

  local_library_file_ = FPaths::Combine(user_data_dir,
                                        FString::FromInt(game_id),
                                        kLibraryFilename);
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoSystemServiceImpl::LoadInstalledModsOnDisk() {
  auto future = file_system_service_->ReadText(local_library_file_);
  return future.Next([this](TTuple<EFileSystemError, FString> result) {
    installed_mods_.installedMods.Empty();

    if (result.Key != EFileSystemError::None) {
      // NOTE(twolf): We do not want to fail the entire initialization just for
      // not being able to load the library file from disk
      CFCORE_LOG(Warning, TEXT("Couldn't load mods library from disk"));
      return FCFCoreError();
    }

    FString json_data = result.Value;
    if (json_data.IsEmpty()) {
      return FCFCoreError();
    }

    bool success = FJsonObjectConverter::JsonObjectStringToUStruct(
      json_data,
      &installed_mods_,
      0,
      0);

    if (!success) {
      CFCORE_LOG(Warning, TEXT("Failed to deserialize installed from disk"));
    }

    return FCFCoreError();
  });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
InstalledModsInfoSystemServiceImpl::SaveInstalledModsToDisk() {
  FString json;
  bool stat = FJsonObjectConverter::UStructToJsonObjectString(
    installed_mods_,
    json,
    0/*CheckFlags*/,
    0/*SkipFlags*/,
    0/*Indent*/,
    nullptr/*ExportCb*/,
    false/*bPrettyPrint*/);

  if (!stat) {
    TPromise<FCFCoreError> promise(nullptr);
    FCFCoreError err(ECFCoreErrorCodes::FailedToSaveModsStateToDisk);
    err.description = TEXT("Failed to serialize installed mods");
    promise.SetValue(err);
    return promise.GetFuture();
  }

  auto future = file_system_service_->WriteText(local_library_file_, json);
  return future.Next([this](EFileSystemError result) {
    if (result != EFileSystemError::None) {
      CFCORE_LOG(Warning, TEXT("Failed to save installed mods to disk"));
      return FCFCoreError(ECFCoreErrorCodes::FailedToSaveModsStateToDisk);
    }

    return FCFCoreError();
  });
}

// -----------------------------------------------------------------------------
void InstalledModsInfoSystemServiceImpl::UpdateInstalledMod(
  FInstalledMod& installed_mod,
  const FCFCoreMod& updated_mod) {

  installed_mod.details = updated_mod;

  const FFile* latest_file = GetLatestFile(updated_mod);
  if (latest_file == nullptr) {
    return;
  }

  if (latest_file->id == installed_mod.installedFile.id) {
    return;
  }

  installed_mod.latestUpdatedFile = *latest_file;
  installed_mod.dateUpdated = FDateTime::Now();

  // NOTE(twolf): We do not want to override some states - e.g. Invalid, which
  // can cause the user to try to run the game with an Invalid mod - because
  // it only shows it is OutOfDate
  if (installed_mod.status == EInstalledModStatus::Normal) {
    installed_mod.status = EInstalledModStatus::OutOfDate;
  }
}

// -----------------------------------------------------------------------------
const FFile*
InstalledModsInfoSystemServiceImpl::GetMainFile(const FCFCoreMod& mod) {
  const FFile* main_file = mod.latestFiles.FindByPredicate(
    [mod](const FFile& file) {
      return (mod.mainFileId == file.id);
    });

  return main_file;
}

// -----------------------------------------------------------------------------
bool InstalledModsInfoSystemServiceImpl::IsModValid(const FCFCoreMod& mod) {
  if (mod.id == 0) {
    return false;
  }

  if (mod.gameId != settings_service_->GetSettings().gameId) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
FInstalledMod* InstalledModsInfoSystemServiceImpl::GetInstalledModRefById(
  const FGuid& id) {

  FInstalledMod* installed_mod = nullptr;
  installed_mod = installed_mods_.installedMods.FindByPredicate(
    [id](const FInstalledMod& installed_mod) {
      return (installed_mod.id == id);
    });

  return installed_mod;
}

// -----------------------------------------------------------------------------
// NOTE(twolf): This returns a pointer reference to the installed mod - so if
// someone removes the item from the installed_mods_.installedMods array, it
// will cause the pointer to point to junk
FInstalledMod* InstalledModsInfoSystemServiceImpl::GetInstalledModRefByModId(
  int64 mod_id) {

  FInstalledMod* installed_mod = nullptr;
  installed_mod = installed_mods_.installedMods.FindByPredicate(
    [mod_id](const FInstalledMod& installed_mod) {
      return (installed_mod.details.id == mod_id);
    });

  return installed_mod;
}

// -----------------------------------------------------------------------------
bool InstalledModsInfoSystemServiceImpl::RemoveInstalledModByModId(int64 id) {
  for (int i = 0; i < installed_mods_.installedMods.Num(); ++i) {
    if (installed_mods_.installedMods[i].details.id == id) {
      installed_mods_.installedMods.RemoveAt(i);
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
bool
InstalledModsInfoSystemServiceImpl::RemoveInstalledModById(const FGuid& id) {
  for (int i = 0; i < installed_mods_.installedMods.Num(); ++i) {
    if (installed_mods_.installedMods[i].id == id) {
      installed_mods_.installedMods.RemoveAt(i);
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
bool InstalledModsInfoSystemServiceImpl::WasPreviouslyInstalled(
  const FInstalledMod& installed_mod) {
  return installed_mod.installedFile.id > 0;
}

// -----------------------------------------------------------------------------
void InstalledModsInfoSystemServiceImpl::SaveChangesAndTrigger(
  const FInstalledMod& installed_mod,
  TSharedPtr<TPromise<InstallModsInfoResult>> promise) {

  auto future = SaveInstalledModsToDisk();
  future.Next([promise, installed_mod](FCFCoreError err) {
    if (err.isError) {
      InstallModsInfoResult result{ err, installed_mod };
      promise->SetValue(result);
      return;
    }

    InstallModsInfoResult result{ FCFCoreError(), installed_mod };
    promise->SetValue(result);
  });
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> InstalledModsInfoSystemServiceImpl::CreateError(
  ECFCoreErrorCodes code,
  const FInstalledMod& installed_mod) {

  TPromise<InstallModsInfoResult> promise(nullptr);
  InstallModsInfoResult result{FCFCoreError(code), installed_mod};
  promise.SetValue(result);
  return promise.GetFuture();
}

// -----------------------------------------------------------------------------
void InstalledModsInfoSystemServiceImpl::SetAdditionalParams(
  FInstalledMod& installed_mod,
  const FInstallModAdditionalParams& additional_params) {

  installed_mod.dynamicContent = additional_params.dynamicContent;
}

// -----------------------------------------------------------------------------
void InstalledModsInfoSystemServiceImpl::FillInstalledModProperties(
  FInstalledMod* stored_mod,
  const FInternalInstalledModInfoProperties& props) {

  if (props.status.IsSet()) {
    stored_mod->status = *props.status;
  }

  if (props.dynamicContent.IsSet()) {
    stored_mod->dynamicContent = *props.dynamicContent;
  }

  if (props.enabled.IsSet()) {
    stored_mod->enabled = *props.enabled;
  }

  if (props.loadOrder.IsSet()) {
    stored_mod->loadOrder = *props.loadOrder;
  }

  if (props.users.IsSet()) {
    stored_mod->users = *props.users;
  }

  if (props.downloadInfo.IsSet()) {
    stored_mod->downloadInfo = *props.downloadInfo;
  }
}
