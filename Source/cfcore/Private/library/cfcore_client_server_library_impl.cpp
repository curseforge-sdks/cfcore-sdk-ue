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
#include "cfcore_client_server_library_impl.h"
#include <common/cfcore_log.h>
#include <common/container_utils.h>
#include <common/future_utils.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreClientServerLibraryImpl::CFCoreClientServerLibraryImpl(
  TSharedPtr<IClientServerLibraryShared> shared,
  TSharedPtr<IClientServerLibraryServer> server,
  TSharedPtr<IClientServerLibraryClient> client) : shared_(shared),
                                                   server_(server),
                                                   client_(client) {
}

// -----------------------------------------------------------------------------
// Server
void CFCoreClientServerLibraryImpl::AssureServerModsUpdated(
  const FAssureServerModsUpdatedParams& params,
  FModsUpdateProgressDelegate on_progress,
  FModsUpdateDelegate on_updated) {

  return server_->AssureServerModsUpdated(params, on_progress, on_updated);
}

// -----------------------------------------------------------------------------
void CFCoreClientServerLibraryImpl::AssureClientModsUpdated(
  const TArray<int64>& file_ids,
  FModsUpdateProgressDelegate on_progress,
  FModsUpdateDelegate on_updated) {

  return client_->AssureClientModsUpdated(file_ids, on_progress, on_updated);
}

// -----------------------------------------------------------------------------
void CFCoreClientServerLibraryImpl::GetInstalledMods(
  TSharedRef<TArray<int64>> mod_ids,
  FGetInstalledModsDelegate on_success) {

  shared_->GetInstalledByIds(mod_ids)
    .Next([on_success](TArray<FInstalledMod> mods) {

    on_success.ExecuteIfBound(MoveTemp(mods));
  });
}
