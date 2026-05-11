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

#include "installed_mod_status.generated.h"

// -----------------------------------------------------------------------------
UENUM(BlueprintType)
enum class EInstalledModStatus : uint8 {
  Pending = 0,
  OutOfDate = 1,
  Normal = 2,
  // This means the mod was deleted or modified
  Invalid = 3,
  // TODO(twolf): Add support for working copies (e.g. with .git folder)
  WorkingCopy = 4,
  Uploading = 5,
  Modified = 6,

  /**
   * Set when a mod was requested to be uninstalled, but failed in the middle,
   * this allows the game to ignore it and for the SDK to try and uninstall it
   * at a later time (e.g. next initialization)
   */
  Uninstalled = 7,
};
