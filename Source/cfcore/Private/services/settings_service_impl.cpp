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
#include "settings_service_impl.h"
#include <services/logger/logger_macros.h>
#include <common/constants.h>

#include <JsonUtilities.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
SettingsServiceImpl::SettingsServiceImpl(const FInternalSettings& settings) :
  internal_settings_(settings) {
}

// -----------------------------------------------------------------------------
TOptional<FCFCoreError> SettingsServiceImpl::ValidateSettings(
  const FCFCoreSettings& settings) {
  if (settings.modsDirectory.IsEmpty()) {
    return FCFCoreError(ECFCoreErrorCodes::MissingModsDirectory);
  }

  if (settings.modsDirectoryMode == EModsDirectoryMode::None) {
    return FCFCoreError(ECFCoreErrorCodes::MissingModsDirectoryMode);
  }

  if (settings.userDataDirectory.IsEmpty()) {
    return FCFCoreError(ECFCoreErrorCodes::MissingUserDataDirectory);
  }

  return {};
}

// -----------------------------------------------------------------------------
void SettingsServiceImpl::SetSettings(const FCFCoreSettings& settings) {
  settings_ = settings;

  SetApiBaseUrl();

  LogSettingWithoutSecrets();
  settings_updated_delegate_.Broadcast();
  LogInternalSettingWithoutSecrets();
};

// -----------------------------------------------------------------------------
void SettingsServiceImpl::UpdateUpdatableSettings(
  const FCFCoreUpdatableSettings& updatable_settings) {

  bool settings_changed = TryUpdateDefaultLanguage(updatable_settings);

  if (!settings_changed) {
    return;
  }

  updatable_settings_updated_delegate_.Broadcast(updatable_settings);
}

// -----------------------------------------------------------------------------
void SettingsServiceImpl::SetApiBaseUrl() {
  internal_settings_.apiBaseUrl = FString::Format(kApiBaseUrl, {
    settings_.gameId
  });
}

// -----------------------------------------------------------------------------
bool SettingsServiceImpl::TryUpdateDefaultLanguage(
  const FCFCoreUpdatableSettings& updatable_settings) {

  if (!updatable_settings.updateDefaultLanguage) {
    return false;
  }

  if (settings_.defaultLanguage == updatable_settings.defaultLanguage) {
    return false;
  }

  settings_.defaultLanguage = updatable_settings.defaultLanguage;
  return true;
}

// -----------------------------------------------------------------------------
void SettingsServiceImpl::LogSettingWithoutSecrets() {
  FCFCoreSettings copy_of_settings(settings_);

  copy_of_settings.apiKey = Obfuscate(settings_.apiKey);
  copy_of_settings.premiumMods.publicKeyPem =
    Obfuscate(settings_.premiumMods.publicKeyPem);

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(copy_of_settings, json_data);
  CFCORE_LOG(Log, TEXT("SetSettings called:\n%s"), *json_data);
}

// -----------------------------------------------------------------------------
void SettingsServiceImpl::LogInternalSettingWithoutSecrets() {
  FInternalSettings copy_of_settings(internal_settings_);

  FString json_data;
  FJsonObjectConverter::UStructToJsonObjectString(copy_of_settings, json_data);
  CFCORE_LOG(Log, TEXT("InternalSettings called:\n%s"), *json_data);
}

// -----------------------------------------------------------------------------
FString SettingsServiceImpl::Obfuscate(const FString& field) {
  int32 length = field.Len();
  FString obfuscated_field = "*****";

  if (length > 4) {
    obfuscated_field += field.Mid(length - 4);
  }

  return obfuscated_field;
}
