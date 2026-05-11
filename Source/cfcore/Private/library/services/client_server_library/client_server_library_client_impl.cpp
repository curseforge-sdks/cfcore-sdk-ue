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
#include "client_server_library_client_impl.h"
#include <common/future_utils.h>
#include <common/container_utils.h>
#include <common/macros.h>
#include <common/constants.h>

#include <services/logger/logger_macros.h>
#include <premium/cfcore_premium_mods.h>
#include <library/services/dynamic_content_service.h>

namespace cfcore {
using QueryModsResult = IClientServerLibraryShared::QueryModsResult;

using MatchPlatformFilesResult =
  ClientServerLibraryClientImpl::MatchPlatformFilesResult;

using ModFilePair = ClientServerLibraryClientImpl::ModFilePair;
using GetModFilePairsResult =
  ClientServerLibraryClientImpl::GetModFilePairsResult;
using GetInstalledModsResult =
  ClientServerLibraryClientImpl::GetInstalledModsResult;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
ClientServerLibraryClientImpl::ClientServerLibraryClientImpl(
  TSharedRef<IClientServerLibraryShared> shared,
  TSharedRef<ICFCoreLibrary> cfcore_library,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<ICFCorePremiumMods> premium_mods,
  TSharedRef<IDynamicContentService> dynamic_content_service) :
  shared_(shared),
  cfcore_library_(cfcore_library),
  cfcore_api_(cfcore_api),
  premium_mods_(premium_mods),
  dynamic_content_service_(dynamic_content_service) {
}

// -----------------------------------------------------------------------------
void ClientServerLibraryClientImpl::AssureClientModsUpdated(
  const TArray<int64>& file_ids,
  ICFCoreClientServerLibrary::FModsUpdateProgressDelegate on_progress,
  ICFCoreClientServerLibrary::FModsUpdateDelegate on_updated) {

  if (file_ids.Num() == 0) {
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

  // Get platform files
  MatchPlatformFiles(file_ids)
    .Next([this, progress, updated](MatchPlatformFilesResult match_res) {

    if (match_res.Key.isError) {
      updated->ExecuteIfBound({}, match_res.Key);
      return;
    }

    auto files = MakeShared<TArray<FFile>>(MoveTemp(match_res.Value));
    if (files->Num() == 0) {
      auto err = FCFCoreError(ECFCoreErrorCodes::NoPlatformFilesMatched);
      updated->ExecuteIfBound({}, err);
      return;
    }

    // Get the <FCFCoreMod, FFile> pairs
    GetModFilePairs(MoveTemp(files))
      .Next([this, progress, updated](GetModFilePairsResult pairs_res) {

      FCFCoreError mod_file_pairs_err = pairs_res.Key;
      if (mod_file_pairs_err.isError) {
        updated->ExecuteIfBound({}, mod_file_pairs_err);
        return;
      }

      auto mod_file_pairs =
        MakeShared<TArray<ModFilePair>>(MoveTemp(pairs_res.Value));

      // If there are any premium mods, assure they are purchased by user
      AssurePremiumModsOwned(mod_file_pairs)
        .Next([this, progress, updated, mod_file_pairs](FCFCoreError err) {

        if (err.isError) {
          updated->ExecuteIfBound({}, err);
          return;
        }

        AssureDynamicContentPremiumModsOwned()
          .Next([this, progress, updated, mod_file_pairs](FCFCoreError err) {

          if (err.isError) {
            updated->ExecuteIfBound({}, err);
            return;
          }

          FInstallModAdditionalParams params;
          params.tracking.Add(kTrackingOrigin, kTrackingOriginJoinServer);
          params.origin = EModInstallOrigin::ServerModule;

          shared_->AssureModsUpdated(mod_file_pairs, params, progress, updated);
        });
      });
    });
  });
}

// -----------------------------------------------------------------------------
TFuture<MatchPlatformFilesResult>
ClientServerLibraryClientImpl::MatchPlatformFiles(
  const TArray<int64>& file_ids) {

  SHAREABLE_PROMISE(MatchPlatformFilesResult, promise);

  FMatchPlatformFilesFilter filter;
  filter.fileIds = file_ids;

  cfcore_api_->MatchPlatformFiles(
    filter,
    ICFCoreApi::FMatchPlatformFilesDelegate::CreateLambda(
      [promise](TOptional<TArray<FFile>> files,
                TOptional<FCFCoreApiResponseError> err) {

        MatchPlatformFilesResult res;

        if (err.IsSet()) {
          res.Key = FCFCoreError(err);
          CFCORE_LOG(Error,
                     TEXT("Error matching platform files: %s"),
                     *err->description);
        } else {
          CFCORE_LOG(Log,
                     TEXT("Matching platform files result: %d files"),
                     files->Num());

          res.Value = MoveTemp(files.GetValue());
        }

        promise->SetValue(MoveTemp(res));
      })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<GetModFilePairsResult> ClientServerLibraryClientImpl::GetModFilePairs(
  TSharedRef<TArray<FFile>> files) {

  SHAREABLE_PROMISE(GetModFilePairsResult, promise);

  // Extract the mod ids
  TArray<int64> mod_ids;
  auto mod_id_to_file_map = MakeShared<TMap<int64, const FFile*>>();

  for (const FFile& file : *files) {
    mod_ids.Add(file.modId);
    mod_id_to_file_map->Add(file.modId, &file);
  }

  FCFCoreGetModsFilter filter;
  filter.modIds = MoveTemp(mod_ids);

  // NOTE(twolf): We have to keep files referenced as we reference the internal
  // FFile
  shared_->QueryMods(filter)
    .Next([mod_id_to_file_map, files, promise](QueryModsResult query_res) {

    GetModFilePairsResult result;
    if (query_res.Key.isError) {
      result.Key = query_res.Key;
      promise->SetValue(result);
      return;
    }

    TArray<FCFCoreMod> mods = MoveTemp(query_res.Value.GetValue());

    auto& mod_file_pairs = result.Value;
    for (FCFCoreMod& mod : mods) {
      const FFile** file = mod_id_to_file_map->Find(mod.id);
      mod_file_pairs.Add(ModFilePair(MoveTemp(mod), **file));
    }

    promise->SetValue(MoveTemp(result));
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibraryClientImpl::AssurePremiumModsOwned(
  TSharedRef<TArray<ModFilePair>> mod_file_pairs) {

  TSet<int64> premium_mod_ids;
  FillPremiumModsForCheck(mod_file_pairs, premium_mod_ids);

  if (premium_mod_ids.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  auto expected_owned_mods = MakeShared<TArray<int64>>(premium_mod_ids.Array());
  return CheckPremiumMods(expected_owned_mods);
}

// -----------------------------------------------------------------------------
void ClientServerLibraryClientImpl::FillPremiumModsForCheck(
  TSharedRef<TArray<ModFilePair>> mod_file_pairs,
  OUT TSet<int64>& premium_mod_ids_to_check) {

  for (int i = 0; i < mod_file_pairs->Num(); ++i) {
    const ModFilePair& pair = (*mod_file_pairs)[i];
    const FCFCoreMod& mod = pair.Key;

    // NOTE(twolf): We intentionally send ALL mods for checking, because the
    // server knows which are premium and which aren't, this adds an extra layer
    // of protection (e.g. should the user edit the metadata or manipulate the
    // http response and treat a premium mod as a non-premium mod).
    if (!mod.premiumDetails.isFreemium) {
      premium_mod_ids_to_check.Add(mod.id);
    }
  }
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ClientServerLibraryClientImpl::CheckPremiumMods(
  TSharedRef<TArray<int64>> mod_ids) {

  SHAREABLE_PROMISE(FCFCoreError, promise);

  premium_mods_->CheckMods(
    *mod_ids,
    ICFCorePremiumMods::FPremiumModsCheckDelegate::CreateLambda(
      [promise, mod_ids](
        const TOptional<TArray<int64>>& opt_owned_mods,
        const TOptional<FCFCoreError>& opt_err) {

      if (opt_err.IsSet()) {
        promise->SetValue(opt_err);
        return;
      }

      TArray<int64> missing = ContainerUtils::Remove(*mod_ids, *opt_owned_mods);
      if (missing.Num() == 0) {
        promise->SetValue(FCFCoreError());
        return;
      }

      FString output;
      ContainerUtils::Join(missing, output);
      CFCORE_LOG(Error,
                  TEXT("Some premium mods are not owned by user: %s"),
                  *output);

      promise->SetValue(FCFCoreError(ECFCoreErrorCodes::ModsNotOwnedByUser));
    })
  );

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError>
ClientServerLibraryClientImpl::AssureDynamicContentPremiumModsOwned() {
  SHAREABLE_PROMISE(FCFCoreError, promise);

  dynamic_content_service_->CheckPremiumOwnership()
    .Next([promise](FCFCoreError err) {

    promise->SetValue(err);
  });

  return promise->GetFuture();
}
