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
#include "cfcore_per_user_library_utils.h"

#include "common/constants.h"
#include "services/logger/logger_macros.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
bool UserLibUtils::RequiresInstall(const FInstalledMod& installed_mod,
                                   const FFile& file) {

  if (installed_mod.installedFile.id != file.id) {
    return true;
  }

  TSet<EInstalledModStatus> invalid_status{
    EInstalledModStatus::Invalid,
    EInstalledModStatus::Pending,
    EInstalledModStatus::Uninstalled
  };

  if (invalid_status.Contains(installed_mod.status)) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
bool UserLibUtils::RequiresUninstall(const FInstalledMod& installed_mod) {
  return installed_mod.users.Num() <= 0;
}

// -----------------------------------------------------------------------------
uint32 UserLibUtils::GetRefCount(
  const FInstalledMod& installed_mod) {

  return installed_mod.users.Num();
}

// -----------------------------------------------------------------------------
void UserLibUtils::ShiftInstalledMods(
  const TArray<FInstalledMod>& sys_installed_mods,
  IN OUT TArray<FInternalInstalledModInfoProperties>& props_array) {

  for (auto sys_installed_mod : sys_installed_mods) {
    if (sys_installed_mod.loadOrder == MAX_int32) {
      continue;
    }

    FInternalInstalledModInfoProperties properties;
    properties.loadOrder = sys_installed_mod.loadOrder + 1;
    properties.modId = sys_installed_mod.details.id;
    props_array.Add(properties);
  }

  return;
}

// -----------------------------------------------------------------------------
void UserLibUtils::PushModToFront(
  const FInstalledMod& installed_mod,
  IN OUT TArray<FInternalInstalledModInfoProperties>& props_array) {

  FInternalInstalledModInfoProperties props;
  props.id = installed_mod.id;
  props.modId = installed_mod.details.id;
  props.loadOrder = 0;
  props_array.Add(props);

  return;
}

// -----------------------------------------------------------------------------
void UserLibUtils::EnableMod(
  const FInstalledMod& installed_mod,
  IN OUT TArray<FInternalInstalledModInfoProperties>& props_array) {

  FInternalInstalledModInfoProperties props;
  props.id = installed_mod.id;
  props.modId = installed_mod.details.id;
  props.enabled = true;
  props_array.Add(props);

  return;
}

// -----------------------------------------------------------------------------
void UserLibUtils::ApplyUserStateToInstalledMod(
  FUserContextInfo& user_info,
  FInstalledMod& installed_mod) {

  FUserModState* user_state = UserLibUtils::FindModById(user_info,
                                                        installed_mod.id);
  if (!user_state) {
    return;
  }

  installed_mod.enabled = user_state->enabled;
  installed_mod.loadOrder = user_state->loadOrder;
  installed_mod.dynamicContent = user_state->dynamicContent;
}

// -----------------------------------------------------------------------------
TArray<FUserModState> UserLibUtils::ApplyPropsToUserMods(
  FUserContextInfo& user_info,
  const TArray<FInternalInstalledModInfoProperties>& props_array) {

  for (const FInternalInstalledModInfoProperties& properties : props_array) {
    FUserModState* user_mod = nullptr;
    if (properties.id.IsSet()) {
      user_mod = FindModById(user_info, properties.id.GetValue());
    } else {
      user_mod = FindModByModId(user_info, properties.modId);
    }

    if (!user_mod) {
      continue;
    }

    if (properties.enabled.IsSet()) {
      user_mod->enabled = properties.enabled.GetValue();
    }

    if (properties.loadOrder.IsSet()) {
      user_mod->loadOrder = properties.loadOrder.GetValue();
    }

    if (properties.dynamicContent.IsSet()) {
      user_mod->dynamicContent = properties.dynamicContent.GetValue();
    }
  }

  return user_info.installedMods;
}

// -----------------------------------------------------------------------------
FUserModState* UserLibUtils::FindModById(FUserContextInfo& user_info,
                                         const FGuid& id) {

  return user_info.installedMods.FindByPredicate(
    [id](const FUserModState& state) {
      return state.id == id;
    }
  );
}

// -----------------------------------------------------------------------------
FUserModState* UserLibUtils::FindModByModId(FUserContextInfo& user_info,
                                            int64 mod_id) {

  return user_info.installedMods.FindByPredicate(
    [mod_id](const FUserModState& state) {
      return state.modId == mod_id;
    }
  );
}

// -----------------------------------------------------------------------------
bool UserLibUtils::ContainsMod(FUserContextInfo& user_info,
                               const FInstalledMod& installed_mod) {

  return FindModById(user_info, installed_mod.id) != nullptr;
}