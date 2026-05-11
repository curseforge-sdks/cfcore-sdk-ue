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
#include <models/cfcore_settings.h>
#include <models/cfcore_updatable_settings.h>
#include <common/cfcore_error.h>
#include "models/internal_settings.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class ISettingsService {
public:
  DECLARE_MULTICAST_DELEGATE(FSettingsUpdated);
  DECLARE_MULTICAST_DELEGATE_OneParam(FUpdatableSettingsUpdated,
                                      const FCFCoreUpdatableSettings&);

public:
  virtual ~ISettingsService() = default;

public:
  virtual FSettingsUpdated& OnSettingsUpdated() = 0;
  virtual FUpdatableSettingsUpdated& OnUpdatableSettingsUpdated() = 0;

  virtual TOptional<FCFCoreError> ValidateSettings(
    const FCFCoreSettings& settings) = 0;
  virtual void SetSettings(const FCFCoreSettings& settings) = 0;
  virtual void UpdateUpdatableSettings(
    const FCFCoreUpdatableSettings& updatable_settings) = 0;

  virtual const FCFCoreSettings& GetSettings() = 0;
  virtual const FInternalSettings& GetInternalSettings() = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::ISettingsService)
