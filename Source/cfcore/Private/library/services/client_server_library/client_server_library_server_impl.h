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
#include <library/services/client_server_library/client_server_library_shared_impl.h>
#include <api/cfcore_api.h>

#include <api/models/mod.h>

#include <common/cfcore_error.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IClientServerLibraryServer {
public:
  virtual ~IClientServerLibraryServer() = default;

public:
  virtual void AssureServerModsUpdated(
    const FAssureServerModsUpdatedParams& params,
    ICFCoreClientServerLibrary::FModsUpdateProgressDelegate on_progress,
    ICFCoreClientServerLibrary::FModsUpdateDelegate on_updated) = 0;
};

// -----------------------------------------------------------------------------
class ClientServerLibraryServerImpl : public IClientServerLibraryServer {
public:
  using ModFilePair = TTuple<FCFCoreMod, FFile>;
  using GetModFilePairsResult = TTuple<FCFCoreError, TArray<ModFilePair>>;

  using GetInstalledModsResult =
    TTuple<FCFCoreError, TOptional<TArray<FInstalledMod>>>;

  ClientServerLibraryServerImpl(TSharedPtr<IClientServerLibraryShared> shared,
                                TSharedPtr<ICFCoreLibrary> cfcore_library,
                                TSharedPtr<ICFCoreApi> cfcore_api);
  virtual ~ClientServerLibraryServerImpl() = default;

// IClientServerLibraryServer
public:
  virtual void AssureServerModsUpdated(
    const FAssureServerModsUpdatedParams& params,
    ICFCoreClientServerLibrary::FModsUpdateProgressDelegate on_progress,
    ICFCoreClientServerLibrary::FModsUpdateDelegate on_updated) override;

private:
  TFuture<GetModFilePairsResult> GetModFilePairs(
    TSharedRef<TArray<FCFCoreMod>> mods);

  TFuture<FCFCoreError> HandleUnavailableMods(
    TSharedRef<TArray<FCFCoreMod>> mods);
  bool IsModAvailableForServer(const FCFCoreMod& mod);

  TFuture<FCFCoreError> EnsureModUninstalled(int64 mod_id);

private:
  TSharedPtr<IClientServerLibraryShared> shared_;
  TSharedPtr<ICFCoreLibrary> cfcore_library_;
  TSharedPtr<ICFCoreApi> cfcore_api_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IClientServerLibraryServer)
Expose_TNameOf(cfcore::ClientServerLibraryServerImpl)