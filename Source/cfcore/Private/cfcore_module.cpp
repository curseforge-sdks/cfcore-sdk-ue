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
#include "cfcore_module.h"
#include "common/cfcore_log.h"
#include "editor/cfcore_editor_settings.h"
#include "cfcore_context.h"

#if WITH_EDITOR
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#include <ISettingsContainer.h>
#endif

#define LOCTEXT_NAMESPACE "FCFCoreModule"

using namespace cfcore;

// -----------------------------------------------------------------------------
void FCFCoreModule::StartupModule() {
  RegisterSettings();
}

// -----------------------------------------------------------------------------
void FCFCoreModule::ShutdownModule() {
  CFCoreContext::GetInstance()->Uninitialize(
    ICFCore::FUninitializeDelegate::CreateLambda(
      [](TOptional<FCFCoreError> err_opt) {
      })
  );

  UnregisterSettings();
}

// -----------------------------------------------------------------------------
void FCFCoreModule::RegisterSettings() {
#if WITH_EDITOR
  ISettingsModule* settings_module =
    FModuleManager::GetModulePtr<ISettingsModule>("Settings");

  if (!settings_module) {
    return;
  }

  // Create the new category
  ISettingsContainerPtr settings_container =
    settings_module->GetContainer("Project");

  // Register the settings
  ISettingsSectionPtr SettingsSection =
    settings_module->RegisterSettings(
      "Project",
      "Plugins",
      "CFCore",
      LOCTEXT("RuntimeGeneralSettingsName", "CFCore"),
      LOCTEXT("RuntimeGeneralSettingsDescription", "Configuration for CFCore"),
      GetMutableDefault<UCFCoreEditorSettings>());
#endif
}

// -----------------------------------------------------------------------------
void FCFCoreModule::UnregisterSettings() {
#if WITH_EDITOR
  ISettingsModule* settings_module =
    FModuleManager::GetModulePtr<ISettingsModule>("Settings");

  if (settings_module) {
    settings_module->UnregisterSettings("Project", "Plugins", "CFCore");
  }
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCFCoreModule, cfcore)
