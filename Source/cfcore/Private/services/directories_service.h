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
#include <api/models/file.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IDirectoriesService {
public:
  DECLARE_MULTICAST_DELEGATE(FDirectoriesUpdated);

public:
  virtual ~IDirectoriesService() = default;

public:
  /**
   * Delegate triggered when there is an update to the directories.
   */
  virtual FDirectoriesUpdated& OnUpdated() = 0;

  /**
   * Returns the directory where the mods are stored (the mod root directory)
   */
  virtual const FString GetModsDirectory() const = 0;

  /**
   * For some modes (e.g. CFCore) we keep an inner directory per game.
   * |GetRelativeModsDirectory| returns only the inner dir.
   */
  virtual const FString GetRelativeModsDirectory() const = 0;

  /**
   * Returns GetModsDirectory() - GetRelativeModsDirectory()
   */
  virtual const FString GetModsRootDirectory() const = 0;

  virtual const FString GetUserDataDirectory() const = 0;

  virtual const FString GetUserLogsDirectory() const = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IDirectoriesService)