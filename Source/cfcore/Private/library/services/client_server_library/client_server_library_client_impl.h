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
#include "CoreMinimal.h"
#include <library/cfcore_client_server_library.h>
#include <library/cfcore_library.h>
#include <library/services/client_server_library/client_server_library_shared_impl.h>

#include <api/cfcore_api.h>

#include <api/models/mod.h>
#include <api/models/file.h>

#include <common/cfcore_error.h>

namespace cfcore {
class ICFCorePremiumMods;
class IDynamicContentService;

// -----------------------------------------------------------------------------
class IClientServerLibraryClient {
public:
  virtual ~IClientServerLibraryClient() = default;

public:
  virtual void AssureClientModsUpdated(
    const TArray<int64>& server_file_ids,
    ICFCoreClientServerLibrary::FModsUpdateProgressDelegate on_progress,
    ICFCoreClientServerLibrary::FModsUpdateDelegate on_updated) = 0;
};

// -----------------------------------------------------------------------------
class ClientServerLibraryClientImpl : public IClientServerLibraryClient {
public:
  using MatchPlatformFilesResult = TTuple<FCFCoreError, TArray<FFile>>;

  using ModFilePair = TTuple<FCFCoreMod, FFile>;
  using GetModFilePairsResult = TTuple<FCFCoreError, TArray<ModFilePair>>;
  using GetInstalledModsResult =
    TTuple<FCFCoreError, TOptional<TArray<FInstalledMod>>>;

  ClientServerLibraryClientImpl(
    TSharedRef<IClientServerLibraryShared> shared,
    TSharedRef<ICFCoreLibrary> cfcore_library,
    TSharedRef<ICFCoreApi> cfcore_api,
    TSharedRef<ICFCorePremiumMods> premium_mods,
    TSharedRef<IDynamicContentService> dynamic_content_service);
  virtual ~ClientServerLibraryClientImpl() = default;

// IClientServerLibraryClient
public:
  virtual void AssureClientModsUpdated(
    const TArray<int64>& server_file_ids,
    ICFCoreClientServerLibrary::FModsUpdateProgressDelegate on_progress,
    ICFCoreClientServerLibrary::FModsUpdateDelegate on_updated) override;

private:
  TFuture<MatchPlatformFilesResult> MatchPlatformFiles(
    const TArray<int64>& file_ids);

  TFuture<GetModFilePairsResult> GetModFilePairs(
    TSharedRef<TArray<FFile>> files);

  TFuture<FCFCoreError> AssurePremiumModsOwned(
    TSharedRef<TArray<ModFilePair>> mod_file_pairs);

  void FillPremiumModsForCheck(TSharedRef<TArray<ModFilePair>> mod_file_pairs,
                               OUT TSet<int64>& premium_mod_ids_to_check);

  TFuture<FCFCoreError> CheckPremiumMods(TSharedRef<TArray<int64>> mod_ids);

  TFuture<FCFCoreError> AssureDynamicContentPremiumModsOwned();

private:
  TSharedRef<IClientServerLibraryShared> shared_;
  TSharedRef<ICFCoreLibrary> cfcore_library_;
  TSharedRef<ICFCoreApi> cfcore_api_;
  TSharedRef<ICFCorePremiumMods> premium_mods_;
  TSharedRef<IDynamicContentService> dynamic_content_service_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IClientServerLibraryClient)
Expose_TNameOf(cfcore::ClientServerLibraryClientImpl)