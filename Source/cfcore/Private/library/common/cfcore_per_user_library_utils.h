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
#include <Misc/Optional.h>

#include "library/services/models/internal_installed_mod_info_properties.h"
#include <library/models/user_mod_state.h>
#include <library/models/installed_mod.h>
#include "services/user_context/models/user_context_info.h"
#include <api/models/file.h>

#include <common/cfcore_error.h>

namespace cfcore {

class UserLibUtils {
public:
  static bool RequiresInstall(const FInstalledMod& installed_mod,
                              const FFile& file);

  static bool RequiresUninstall(const FInstalledMod& installed_mod);

  static uint32 GetRefCount(const FInstalledMod& installed_mod);

  static void ShiftInstalledMods(
    const TArray<FInstalledMod>& sys_installed_mods,
    IN OUT TArray<FInternalInstalledModInfoProperties>& props_array);

  static void PushModToFront(
    const FInstalledMod& installed_mod,
    IN OUT TArray<FInternalInstalledModInfoProperties>& props_array);

  static void EnableMod(
    const FInstalledMod& installed_mod,
    IN OUT TArray<FInternalInstalledModInfoProperties>& props_array);

  static void ApplyUserStateToInstalledMod(
    FUserContextInfo& user_info,
    FInstalledMod& installed_mod);

  static TArray<FUserModState> ApplyPropsToUserMods(
    FUserContextInfo& user_info,
    const TArray<FInternalInstalledModInfoProperties>& props_array);

  static FUserModState* FindModById(FUserContextInfo& user_info,
                                    const FGuid& id);
  static FUserModState* FindModByModId(FUserContextInfo& user_info,
                                       int64 mod_id);

  static bool ContainsMod(FUserContextInfo& user_info,
                          const FInstalledMod& installed_mod);
};

} // namespace cfcore