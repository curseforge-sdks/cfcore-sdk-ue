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
#include "client_server_library_shared_impl.h"
#include <common/container_utils.h>
#include <common/future_utils.h>

#include <services/logger/logger_macros.h>

#include <library/cfcore_library.h>
#include <services/settings_service.h>
#include <library/services/installed_mods_synchronize_with_server_service.h>
#include <library/services/mods_validation/mods_validation_service.h>


namespace cfcore {
using QueryModsResult = ClientServerLibrarySharedImpl::QueryModsResult;
using GetInstalledModsResult =
  ClientServerLibrarySharedImpl::GetInstalledModsResult;
using ModFilePair = IClientServerLibraryShared::ModFilePair;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
ClientServerLibrarySharedImpl::ClientServerLibrarySharedImpl(
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<ICFCoreLibrary> cfcore_library,
  TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service,
  TSharedRef<IModsValidationService> mods_validation_service) :
  settings_service_(settings_service),
  cfcore_library_(cfcore_library),
  mods_sync_service_(mods_sync_service),
  mods_validation_service_(mods_validation_service) {
}

// -----------------------------------------------------------------------------
TFuture<QueryModsResult> ClientServerLibrarySharedImpl::QueryMods(
  const FCFCoreGetModsFilter& filter) {

  SHAREABLE_PROMISE(QueryModsResult, promise);

  bool filterPcOnly = false;
  if (settings_service_->GetSettings().isServer) {
    filterPcOnly = settings_service_->GetSettings().isServerPcOnly;
  }

  FCFCoreGetModsFilter normalized_filter;
  normalized_filter.devModIds = ContainerUtils::Dedupe(filter.devModIds);

  // By removing devModIds from modIds - we give precedence to devModIds - which
  // is what we want
  normalized_filter.modIds = ContainerUtils::Remove(
    filter.modIds,
    normalized_filter.devModIds);

  normalized_filter.filterPcOnly = filterPcOnly;

  auto mods = MakeShared<TArray<FCFCoreMod>>();
  mods_sync_service_->GetModDetailsFromServer(normalized_filter,mods)
    .Next([promise, mods](FCFCoreError err) {

    QueryModsResult res;
    res.Key = err;
    res.Value = MoveTemp(*mods);

    promise->SetValue(MoveTemp(res));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<TArray<FInstalledMod>>
ClientServerLibrarySharedImpl::GetInstalledByIdsWithValidations(
  TSharedRef<TArray<ModFilePair>> mods_file_pairs) {

  SHAREABLE_PROMISE(TArray<FInstalledMod>, promise);

  auto mod_ids = MakeShared<TArray<int64>>();
  for (const ModFilePair& mod_file_pair : *mods_file_pairs) {
    mod_ids->Add(mod_file_pair.Key.id);
  }

  // Get locally installed mods from the given ids
  GetInstalledByIds(mod_ids).Next([this, mod_ids, mods_file_pairs, promise](
    TArray<FInstalledMod> mods) {

    auto installed_mods = MakeShared<TArray<FInstalledMod>>(MoveTemp(mods));

    // Check the local disk status of each installed mod (saves it internally)
    PerformModsValidation(MoveTemp(installed_mods), mods_file_pairs)
      .Next([this, promise, mod_ids](FCFCoreError) {

      // Again, this time with the updated validation statuses
      GetInstalledByIds(mod_ids).Next([promise](TArray<FInstalledMod> mods) {
        promise->SetValue(MoveTemp(mods));
      });
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<TArray<FInstalledMod>>
ClientServerLibrarySharedImpl::GetInstalledByIds(
  TSharedRef<TArray<int64>> mod_ids) {

  SHAREABLE_PROMISE(TArray<FInstalledMod>, promise);

  cfcore_library_->GetInstalledMods(
    ICFCoreLibrary::FGetInstalledModsDelegate::CreateLambda(
      [mod_ids, promise](const TArray<FInstalledMod>& installed_mods) {

        TArray<FInstalledMod> mods_to_return = installed_mods.FilterByPredicate(
          [mod_ids](const FInstalledMod& installed_mod) {

            int32 index;
            return mod_ids->Find(installed_mod.details.id, index);
          });

        promise->SetValue(MoveTemp(mods_to_return));
      }));

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool ClientServerLibrarySharedImpl::ShouldInstallModByState(
  const FInstalledMod* installed_mod) {

  static const TArray<EInstalledModStatus> requires_install_statuses{
    EInstalledModStatus::Invalid,
    EInstalledModStatus::Pending,
    EInstalledModStatus::Modified,
  };

  int32 index;
  if (requires_install_statuses.Find(installed_mod->status, index)) {
    FString status =
      UEnum::GetDisplayValueAsText(installed_mod->status).ToString();

    CFCORE_LOG(Log,
               TEXT("Mod: %s (%d) requires a fix with status: %s"),
               *installed_mod->details.name,
               installed_mod->details.id,
               *status);

    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibrarySharedImpl::PerformModsValidation(
  TSharedRef<TArray<FInstalledMod>> installed_mods,
  TSharedRef<TArray<ModFilePair>> mods_file_pairs) {

  // NOTE(twolf): The following code is not really necessary because of how
  // we implement |QueryMods| which saves the freshly retrieved mod details in
  // the local installed mods cache. The reason we have this code is to make it
  // clear to the reviewer that the installed_mods contains up-to-date server
  // details about each mod.
  TMap<int64, FCFCoreMod> mod_details_map;
  for (const ModFilePair& mod_file_pair : (*mods_file_pairs)) {
    const FCFCoreMod& mod = mod_file_pair.Key;
    mod_details_map.Add(mod.id, mod);
  }

  for (FInstalledMod& installed_mod : *installed_mods) {
    const FCFCoreMod* details = mod_details_map.Find(installed_mod.details.id);
    if (details == nullptr) {
      continue;
    }

    installed_mod.details = *details;
  }

  return mods_validation_service_->PerformQuickValidation(installed_mods);
  // PerformFingerprintValidations(installed_mods));
}

// -----------------------------------------------------------------------------
void ClientServerLibrarySharedImpl::AssureModsUpdated(
  TSharedRef<TArray<ModFilePair>> mod_file_pairs,
  const FInstallModAdditionalParams& additional_params,
  TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> on_prgs,
  TSharedRef<ICFCoreClientServerLibrary::FModsUpdateDelegate> on_update) {

  GetModsToInstall(mod_file_pairs).Next([this,
                                         mod_file_pairs,
                                         additional_params,
                                         on_prgs,
                                         on_update](TArray<ModFilePair> inst) {

    // Perform installations
    InstallMods(MoveTemp(inst), additional_params, on_prgs)
      .Next([this, on_update, mod_file_pairs](FCFCoreError err) {

      if (err.isError) {
        on_update->ExecuteIfBound({}, err);
        return;
      }

      // Validate final state
      ValidateModsInstallation(mod_file_pairs)
        .Next([on_update](GetInstalledModsResult res) {

        if (res.Key.isError) {
          on_update->ExecuteIfBound({}, res.Key);
          return;
        }

        on_update->ExecuteIfBound(MoveTemp(res.Value.GetValue()), {});
      });
    });
  });
}

// -----------------------------------------------------------------------------
TFuture<TArray<ModFilePair>> ClientServerLibrarySharedImpl::GetModsToInstall(
  TSharedRef<TArray<ModFilePair>> mods_file_pairs) {

  SHAREABLE_PROMISE(TArray<ModFilePair>, promise);

  GetInstalledByIdsWithValidations(mods_file_pairs)
    .Next([this, promise, mods_file_pairs](TArray<FInstalledMod> installed) {

    TArray<ModFilePair> mods_to_install;

    for (const ModFilePair& mod_file_pair : *mods_file_pairs) {
      if (installed.Num() == 0) {
        mods_to_install.Add(mod_file_pair);
        continue;
      }

      const FInstalledMod* installed_mod = installed.FindByPredicate(
        [mod_file_pair](const FInstalledMod& intalled_mod) {
          return (intalled_mod.details.id == mod_file_pair.Key.id);
        });

      if (ShouldInstallModByFile(mod_file_pair.Value, installed_mod)) {
        mods_to_install.Add(mod_file_pair);
      } else {
        CFCORE_LOG(Log,
                   TEXT("No need to update existing mod: %s (%d)"),
                   *installed_mod->details.name,
                   installed_mod->details.id);
      }
    }

    promise->SetValue(MoveTemp(mods_to_install));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibrarySharedImpl::InstallMods(
  const TArray<ModFilePair>& mod_file_pairs,
  const FInstallModAdditionalParams& additional_params,
  TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> progrs) {

  if (mod_file_pairs.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  for (const ModFilePair& mod_file_pair : mod_file_pairs) {
    futures->Add(InstallMod(mod_file_pair, additional_params, progrs));
  }

  auto future = FutureUtils::Parallel<FCFCoreError>(futures);
  future.Next([promise](TArray<FCFCoreError> errors) {
    for (const FCFCoreError& err : errors) {
      if (err.isError) {
        promise->SetValue(err);
        return;
      }
    }

    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}


// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibrarySharedImpl::InstallMod(
  const ModFilePair& mod_file_pair,
  const FInstallModAdditionalParams& additional_params,
  TSharedRef<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate> progrs) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  FModsUpdateProgress mods_update_progress;
  mods_update_progress.state = EModsUpdateProgressState::Installing;

  const FCFCoreMod& mod = mod_file_pair.Key;
  const FFile& file = mod_file_pair.Value;

  cfcore_library_->Install(
    mod,
    file,
    additional_params,
    ICFCoreLibrary::FInstallProgressDelegate::CreateLambda(
      [progrs, mods_update_progress, mod](const FLibraryProgress& progress) {
        (*progrs).ExecuteIfBound(mods_update_progress, progress, mod);
      }),
    ICFCoreLibrary::FInstalledDelegate::CreateLambda(
      [promise](TOptional<FInstalledMod>, const TOptional<FCFCoreError>& err) {
        promise->SetValue(FCFCoreError(err));
      }));

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<GetInstalledModsResult>
ClientServerLibrarySharedImpl::ValidateModsInstallation(
  TSharedRef<TArray<ModFilePair>> mod_file_pairs) {

  SHAREABLE_PROMISE(GetInstalledModsResult, promise);

  auto mod_ids = MakeShared<TArray<int64>>();
  for (const ModFilePair& mod_file_pair : *mod_file_pairs) {
    const FFile& file = mod_file_pair.Value;
    mod_ids->Add(file.modId);
  }

  GetInstalledByIds(MoveTemp(mod_ids))
    .Next([this, promise, mod_file_pairs](TArray<FInstalledMod> mods) {

    GetInstalledModsResult res;

    if (mods.Num() == 0 || mods.Num() != mod_file_pairs->Num()) {
      CFCORE_LOG(Log,
                 TEXT("Missing installed mods - (%d/%d)"),
                 mods.Num(),
                 mod_file_pairs->Num());

      ECFCoreErrorCodes code = ECFCoreErrorCodes::MissingInstalledMods;
      res.Key = FCFCoreError(code);
      promise->SetValue(res);
      return;
    }

    for (const ModFilePair& mod_file_pair : *mod_file_pairs) {
      const FInstalledMod* installed_mod = mods.FindByPredicate(
        [mod_file_pair](const FInstalledMod& intalled_mod) {
          return (intalled_mod.details.id == mod_file_pair.Key.id);
        });

      if (ShouldInstallModByFile(mod_file_pair.Value, installed_mod)) {
        ECFCoreErrorCodes code = ECFCoreErrorCodes::MissingInstalledMods;
        res.Key = FCFCoreError(code);
        promise->SetValue(res);
        return;
      }

      CFCORE_LOG(Log,
                 TEXT("Mod valid: %s (%d)"),
                 *installed_mod->details.name,
                 installed_mod->details.id);
    }

    res.Value = MoveTemp(mods);
    promise->SetValue(MoveTemp(res));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool ClientServerLibrarySharedImpl::ShouldInstallModByFile(
  const FFile& file,
  const FInstalledMod* installed_mod) {

  if (!installed_mod || installed_mod->installedFile.id == 0) {
    // new mod to install
    CFCORE_LOG(Log, TEXT("Mod: (%lld) doesn't exist"), file.modId);
    return true;
  }

  if (file.id != installed_mod->installedFile.id) {
    // different version required
    CFCORE_LOG(
      Log,
      TEXT("Mod: %s (%lld) requires upgrade/downgrade (%lld -> %lld)"),
      *installed_mod->details.name,
      installed_mod->details.id,
      installed_mod->installedFile.id,
      file.id);

    return true;
  }

  return ShouldInstallModByState(installed_mod);
}