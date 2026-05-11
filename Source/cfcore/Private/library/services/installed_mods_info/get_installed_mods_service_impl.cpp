/* MIT License

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
SOFTWARE. */
#include "get_installed_mods_service_impl.h"

#include <common/macros.h>

#include <library/services/installed_mods_info/installed_mods_info_service.h>

namespace cfcore {

}

using namespace cfcore;

// -----------------------------------------------------------------------------
GetInstalledModsServiceImpl::GetInstalledModsServiceImpl(
  TSharedRef<IInstalledModsInfoService> installed_mods_info_system_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_user_service) :
  installed_mods_info_system_service_(installed_mods_info_system_service),
  installed_mods_info_user_service_(installed_mods_info_user_service) {
}

// -----------------------------------------------------------------------------
void GetInstalledModsServiceImpl::GetUserInstalledMods(
  TArray<FInstalledMod>& out_installed_mods) {

  installed_mods_info_user_service_->GetInstalledMods(out_installed_mods);
}

// -----------------------------------------------------------------------------
void GetInstalledModsServiceImpl::GetSystemInstalledMods(
  TArray<FInstalledMod>& out_installed_mods) {

  installed_mods_info_system_service_->GetInstalledMods(out_installed_mods);
}
