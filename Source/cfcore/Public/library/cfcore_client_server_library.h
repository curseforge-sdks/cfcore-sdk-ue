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
#include <common/cfcore_error.h>
#include <api/models/mod.h>
#include <library/models/installed_mod.h>
#include <library/models/mods_update_progress.h>
#include <library/models/library_progress.h>
#include <library/models/assure_server_mods_updated_params.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// Use this interface if you want to manage the local mods library for a client-
// server game session.
//
// Server - A modded server that starts and wants to assure that it has the
// latest versions of mods required to start.
//
// Client - A client that connets to a modded serverand synchronizes it's mod
// library with what the mod versions that the server has
class ICFCoreClientServerLibrary {
public:
  DECLARE_DELEGATE_ThreeParams(FModsUpdateProgressDelegate, const FModsUpdateProgress&, const TOptional<FLibraryProgress>&, const TOptional<FCFCoreMod>&);
  DECLARE_DELEGATE_TwoParams(FModsUpdateDelegate, TOptional<TArray<FInstalledMod>>, TOptional<FCFCoreError>);
  DECLARE_DELEGATE_OneParam(FGetInstalledModsDelegate, TArray<FInstalledMod>);
  DECLARE_DELEGATE_OneParam(FCancelCurrentUpdateDelegate, TOptional<FCFCoreError>);
  DECLARE_DELEGATE_OneParam(FClearDiskSpaceDelegate, TOptional<FCFCoreError>);

public:
  virtual ~ICFCoreClientServerLibrary() = default;

public:
  /**
   * Don't forget to set the Settings with isServer = true (optionally, set
   * PcOnly = true) before calling this function
   */
  virtual void AssureServerModsUpdated(
    const FAssureServerModsUpdatedParams& params,
    FModsUpdateProgressDelegate on_progress,
    FModsUpdateDelegate on_updated) = 0;

  virtual void AssureClientModsUpdated(
    const TArray<int64>& server_file_ids,
    FModsUpdateProgressDelegate on_progress,
    FModsUpdateDelegate on_updated) = 0;

  virtual void GetInstalledMods(TSharedRef<TArray<int64>> mod_ids,
                                FGetInstalledModsDelegate on_success) = 0;

  // virtual void CancelUpdates(FCancelCurrentUpdateDelegate delegate) = 0;
  // virtual void ClearDiskSpace(FClearDiskSpaceDelegate delegate) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ICFCoreClientServerLibrary)