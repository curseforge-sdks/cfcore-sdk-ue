/*MIT License

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
SOFTWARE.*/
#pragma once
#include "CoreMinimal.h"
#include <Async/Async.h>

#include <common/cfcore_error.h>

#include <api/models/mod.h>
#include <api/models/filters/get_mods_filter.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IInstalledModsSynchronizeWithServerService {
public:
  virtual ~IInstalledModsSynchronizeWithServerService() = default;

public:
  virtual TFuture<FCFCoreError> Sync() = 0;

  /**
   * This queries mods from the server but also updates the details of the
   * locally installed mods - convinient for keeping the local mods up to date
   * with the server whenever a module queries this information.
   */
  virtual TFuture<FCFCoreError> GetModDetailsFromServer(
    const FCFCoreGetModsFilter& filter,
    TSharedRef<TArray<FCFCoreMod>> out_mod_details) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IInstalledModsSynchronizeWithServerService)