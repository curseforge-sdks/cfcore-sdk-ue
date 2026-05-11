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
#include <CoreMinimal.h>
#include "installed_mod.h"
#include "user_mod_state.generated.h"

// -----------------------------------------------------------------------------
// A struct that saves user-specific installed mod data, and persists to disk.

USTRUCT(BlueprintType)
struct FUserModState {
  GENERATED_BODY()

  FUserModState() : id(FGuid()) {}

  static FUserModState FromInstalledMod(
    const struct FInstalledMod& InstalledMod);

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  FGuid id;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int64 modId = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool enabled = true;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  int32 loadOrder = MAX_int32;

  UPROPERTY(BlueprintReadOnly, Category = "Models")
  bool dynamicContent = false;
};

inline FUserModState FUserModState::FromInstalledMod(
  const FInstalledMod& InstalledMod) {

  FUserModState state;
  state.id = InstalledMod.id;
  state.modId = InstalledMod.details.id;
  state.enabled = InstalledMod.enabled;
  state.loadOrder = InstalledMod.loadOrder;
  state.dynamicContent = InstalledMod.dynamicContent;
  return state;
}
