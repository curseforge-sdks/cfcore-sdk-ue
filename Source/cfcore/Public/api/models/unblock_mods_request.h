/*MIT License

Copyright (c) 2023 Overwolf Ltd.

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

#include "unblock_mods_request.generated.h"

// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct CFCORE_API FUnblockModsRequest {
  GENERATED_BODY()

  /**
   * Authors blocked by the user from within the in-game browser or website
  */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TArray<int64> blockedAuthors;

  /**
   * Mods blocked by the user from within the in-game browser or website
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TArray<int64> blockedMods;

  /**
   * Mods blocked by the user from within the context of a server (e.g. join
   * server screen)
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TArray<int64> blockedServerMods;

  /**
   * Servers blocked by the user
   */
  UPROPERTY(BlueprintReadWrite, Category = "Models")
  TArray<FString> blockedServers;

};
