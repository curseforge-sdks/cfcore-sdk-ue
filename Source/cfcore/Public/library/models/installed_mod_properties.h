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

#include "installed_mod_properties.generated.h"

// -----------------------------------------------------------------------------
// Used to update specific properties of an installed mod so that they persist
// across sessions
USTRUCT(BlueprintType)
struct CFCORE_API FInstalledModProperties {
  GENERATED_BODY()

  /**
   * id will take precedence over modId if both are set.
   * This is the internal (guid) id given to the locally installed mod and is
   * the only way to identify unmanaged mods.
   *
   * You may also use this for managed mods.
   *
   * Leave this id UNSET if you want modId to be used.
  */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  FGuid id;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  int64 modId = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  bool enabled = true;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "cfcore")
  int32 loadOrder = MAX_int32;
};
