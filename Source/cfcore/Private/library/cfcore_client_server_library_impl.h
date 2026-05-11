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
#include <library/services/client_server_library/client_server_library_server_impl.h>
#include <library/services/client_server_library/client_server_library_client_impl.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class CFCoreClientServerLibraryImpl : public ICFCoreClientServerLibrary {
public:
  CFCoreClientServerLibraryImpl(TSharedPtr<IClientServerLibraryShared> shared,
                                TSharedPtr<IClientServerLibraryServer> server,
                                TSharedPtr<IClientServerLibraryClient> client);
  virtual ~CFCoreClientServerLibraryImpl() = default;
  CFCoreClientServerLibraryImpl(const CFCoreClientServerLibraryImpl&) = delete;
  CFCoreClientServerLibraryImpl(CFCoreClientServerLibraryImpl&&) = delete;
  void operator=(const CFCoreClientServerLibraryImpl&) = delete;
  CFCoreClientServerLibraryImpl& operator=(
    CFCoreClientServerLibraryImpl&&) = delete;

// ICFCoreClientServerLibrary
public:
  virtual void AssureServerModsUpdated(
    const FAssureServerModsUpdatedParams& params,
    FModsUpdateProgressDelegate on_progress,
    FModsUpdateDelegate on_updated) override;

  virtual void AssureClientModsUpdated(
    const TArray<int64>& server_file_ids,
    FModsUpdateProgressDelegate on_progress,
    FModsUpdateDelegate on_updated) override;

  virtual void GetInstalledMods(TSharedRef<TArray<int64>> mod_ids,
                                FGetInstalledModsDelegate on_success) override;

private:
  TSharedPtr<IClientServerLibraryShared> shared_;
  TSharedPtr<IClientServerLibraryServer> server_;
  TSharedPtr<IClientServerLibraryClient> client_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::CFCoreClientServerLibraryImpl)