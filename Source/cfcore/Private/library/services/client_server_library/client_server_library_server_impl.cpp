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
#include "client_server_library_server_impl.h"
#include <common/future_utils.h>
#include <common/macros.h>

#include <services/logger/logger_macros.h>

#include <library/cfcore_library.h>

namespace cfcore {
using ModFilePair = ClientServerLibraryServerImpl::ModFilePair;
using GetModFilePairsResult =
  ClientServerLibraryServerImpl::GetModFilePairsResult;
using QueryModsResult = IClientServerLibraryShared::QueryModsResult;
using GetInstalledModsResult =
  ClientServerLibraryServerImpl::GetInstalledModsResult;

static const TCHAR kErrMsgDetectedUnavailMod[] =
  TEXT("Detected an unavailable mod: %s (%ld)! Ensuring mod is deleted...");
}

using namespace cfcore;

// -----------------------------------------------------------------------------
ClientServerLibraryServerImpl::ClientServerLibraryServerImpl(
  TSharedPtr<IClientServerLibraryShared> shared,
  TSharedPtr<ICFCoreLibrary> cfcore_library,
  TSharedPtr<ICFCoreApi> cfcore_api) : shared_(shared),
                                       cfcore_library_(cfcore_library),
                                       cfcore_api_(cfcore_api) {
}

// -----------------------------------------------------------------------------
void ClientServerLibraryServerImpl::AssureServerModsUpdated(
  const FAssureServerModsUpdatedParams& params,
  ICFCoreClientServerLibrary::FModsUpdateProgressDelegate on_progress,
  ICFCoreClientServerLibrary::FModsUpdateDelegate on_updated) {

  if ((params.modIds.Num() == 0) && (params.devModIds.Num() == 0)) {
    on_updated.ExecuteIfBound(TArray<FInstalledMod>{}, {});
    return;
  }

  FModsUpdateProgress mods_update_progress;
  mods_update_progress.state = EModsUpdateProgressState::Validating;
  on_progress.ExecuteIfBound(mods_update_progress, {}, {});

  auto progress =
    MakeShared<ICFCoreClientServerLibrary::FModsUpdateProgressDelegate>(
      MoveTemp(on_progress));
  auto updated =
    MakeShared<ICFCoreClientServerLibrary::FModsUpdateDelegate>(
      MoveTemp(on_updated));

  FCFCoreGetModsFilter filter;
  filter.modIds = params.modIds;
  filter.devModIds = params.devModIds;

  // Get mods info from server
  shared_->QueryMods(MoveTemp(filter))
    .Next([this, progress, updated](QueryModsResult query_res) {

    if (query_res.Key.isError) {
      updated->ExecuteIfBound({}, query_res.Key);
      return;
    }

    auto mods =
      MakeShared<TArray<FCFCoreMod>>(MoveTemp(query_res.Value.GetValue()));

    HandleUnavailableMods(mods)
      .Next([this, progress, updated, mods](FCFCoreError err) {

      if (err.isError) {
        updated->ExecuteIfBound({}, err);
        return;
      }

      // Get the <FCFCoreMod, FFile> pairs
      GetModFilePairs(mods)
        .Next([this, progress, updated](GetModFilePairsResult pairs_res) {

        FCFCoreError mod_file_pairs_err = pairs_res.Key;
        if (mod_file_pairs_err.isError) {
          updated->ExecuteIfBound({}, mod_file_pairs_err);
          return;
        }

        auto mod_file_pairs =
          MakeShared<TArray<ModFilePair>>(MoveTemp(pairs_res.Value));

        FInstallModAdditionalParams params;
        shared_->AssureModsUpdated(mod_file_pairs, params, progress, updated);
      });
    });
  });
}

// -----------------------------------------------------------------------------
// NOTE(twolf): We are using a TFuture signature because, we may want to assure
// we have the FFile object
TFuture<GetModFilePairsResult> ClientServerLibraryServerImpl::GetModFilePairs(
  TSharedRef<TArray<FCFCoreMod>> mods) {

  GetModFilePairsResult result;
  if (mods->Num() == 0) {
    result.Key = FCFCoreError();
    return MakeFulfilledPromise<GetModFilePairsResult>(result).GetFuture();
  }

  auto& mod_file_pairs = result.Value;
  for (FCFCoreMod& mod : *mods) {
    FFile* main_file = nullptr;

    for (FFile& file : mod.latestFiles) {
      if (file.id == mod.mainFileId) {
        main_file = &file;
        break;
      }
    }

    if (main_file != nullptr) {
      mod_file_pairs.Add(ModFilePair(mod, *main_file));
    } else {
      CFCORE_LOG(Error,
                 TEXT("Mod: %s (%ld) doesn't contain it's main file (%ld)!"),
                 *mod.name,
                 mod.id,
                 mod.mainFileId);
    }
  }

  return
    MakeFulfilledPromise<GetModFilePairsResult>(MoveTemp(result)).GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibraryServerImpl::HandleUnavailableMods(
  TSharedRef<TArray<FCFCoreMod>> mods) {

  if (mods->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto uninstall_futures = MakeShared<TArray<TFuture<FCFCoreError>>>();

  for (const FCFCoreMod& mod : *mods) {
    if (IsModAvailableForServer(mod)) {
      continue;
    }

    CFCORE_LOG(Error, kErrMsgDetectedUnavailMod, *mod.name, mod.id);
    uninstall_futures->Add(EnsureModUninstalled(mod.id));
  }

  if (uninstall_futures->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<FCFCoreError>(uninstall_futures);
  future.Next([promise](TArray<FCFCoreError>) {
    promise->SetValue(FCFCoreError(ECFCoreErrorCodes::DetectedUnavailableMod));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
// We have different critiria for considering if a mod is available or not
// available on a server - this function holds that logic
bool ClientServerLibraryServerImpl::IsModAvailableForServer(
  const FCFCoreMod& mod) {

  if (mod.isAvailable) {
    return true;
  }

  // NOTE(twolf): Right now, we don't want premium mods to be unavailble to
  // users who purchased them, even if the mod is unavailable
  if (mod.premiumDetails.isPremium) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibraryServerImpl::EnsureModUninstalled(
  int64 mod_id) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_library_->Uninstall(
    mod_id,
    ICFCoreLibrary::FUninstallDelegate::CreateLambda(
      [promise](TOptional<FInstalledMod>, const TOptional<FCFCoreError>&) {
        promise->SetValue(FCFCoreError());
      }));

  return promise->GetFuture();
}
