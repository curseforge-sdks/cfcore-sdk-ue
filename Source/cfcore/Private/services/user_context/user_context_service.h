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
#include <CoreMinimal.h>
#include <Async/Future.h>

#include <common/cfcore_error.h>

#include <library/models/installed_mod.h>
#include "models/user_context_info.h"
#include "models/shared_context_info.h"

namespace cfcore {

// -----------------------------------------------------------------------------
// A service to keep track (read/write) of the current user's context - things
// like the user's authorization token and in the future other information.
class IUserContextService {
public:
  DECLARE_MULTICAST_DELEGATE(FUserContextServiceInitialized);

public:
  virtual ~IUserContextService() = default;

public:
  virtual FUserContextInfo UserContextInfo() = 0;
  virtual FSharedContextInfo SharedContextInfo() = 0;

public:
  virtual TFuture<FCFCoreError> Initialize() = 0;

  virtual TFuture<bool> SetAuthToken(const FString& auth_token) = 0;

  virtual TFuture<FCFCoreError> SetInstalledMods(
    const TArray<FUserModState>& user_mods) = 0;

  virtual TFuture<FCFCoreError> SetLibraryMigrationPerformed() = 0;

  virtual TFuture<FCFCoreError> SetSubscribedToInstalledMods() = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IUserContextService)