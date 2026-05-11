/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include <Async/Future.h>
#include <library/models/installed_mod.h>
#include <library/services/models/module_fingerprint.h>
#include <common/cfcore_error.h>

namespace cfcore {
/**
 * A map between an FFileModule and it's calculated fingerprint
 */
typedef TArray<struct FModuleFingerprint> ModuleFingerprintArray;

// -----------------------------------------------------------------------------
class IFingerprintsService {
public:
  virtual ~IFingerprintsService() = default;

public:
  /**
   * Given an installed mod, calculate and return all of its fingerprints.
   *
   * By accepting server_modules_detail - we allow the caller to get server-side
   * details about expected fingerprints and thus creating a more secure scan
   * of fingerprints (e.g. assuming the user edits the local metadata file)
   */
  virtual TFuture<FCFCoreError> CalcFingerprints(
    FInstalledMod installed_mod,
    const TArray<FFileModule>& server_modules_detail,
    OUT TSharedRef<ModuleFingerprintArray> fingerprints) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IFingerprintsService)