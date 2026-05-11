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
#include "settings_service.h"
#include <common/macros.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class SettingsServiceImpl : public ISettingsService {
public:
  SettingsServiceImpl(const FInternalSettings& internal_settings);

public:
  void Initialize(const FCFCoreSettings& settings);

// ISettingsService
public:
  virtual FSettingsUpdated& OnSettingsUpdated() override {
    return settings_updated_delegate_;
  }

  virtual FUpdatableSettingsUpdated& OnUpdatableSettingsUpdated() override {
    return updatable_settings_updated_delegate_;
  }

  virtual TOptional<FCFCoreError> ValidateSettings(
    const FCFCoreSettings& settings) override;

  virtual void SetSettings(const FCFCoreSettings& settings) override;
  virtual void UpdateUpdatableSettings(
    const FCFCoreUpdatableSettings& updatable_settings) override;

  virtual const FCFCoreSettings& GetSettings() override {
    return settings_;
  }

  virtual const FInternalSettings& GetInternalSettings() override {
    return internal_settings_;
  }

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SettingsServiceImpl);

  void SetApiBaseUrl();
  bool TryUpdateDefaultLanguage(
    const FCFCoreUpdatableSettings& updatable_settings);
  void LogSettingWithoutSecrets();
  void LogInternalSettingWithoutSecrets();
  FString Obfuscate(const FString& field);

private:
  FCFCoreSettings settings_;
  FInternalSettings internal_settings_;

  FSettingsUpdated settings_updated_delegate_;
  FUpdatableSettingsUpdated updatable_settings_updated_delegate_;
};

} // namespace cfcore
