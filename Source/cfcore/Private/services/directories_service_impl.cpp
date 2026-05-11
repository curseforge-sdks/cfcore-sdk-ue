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
#include "directories_service_impl.h"

#include <Misc/Paths.h>

namespace cfcore {
const TCHAR kLogsDirName[] = TEXT("logs");
}

using namespace cfcore;

// -----------------------------------------------------------------------------
DirectoriesServiceImpl::DirectoriesServiceImpl(
  TSharedPtr<ISettingsService> settings_service) :
  settings_service_(settings_service) {

  settings_service_->OnSettingsUpdated().AddLambda([this]() {
    InitializeMembers();
  });

  InitializeMembers();
}

// -----------------------------------------------------------------------------
void DirectoriesServiceImpl::InitializeMembers() {
  auto settings = settings_service_->GetSettings();

  FString mods_dir = settings.modsDirectory;
  normalized_root_mods_dir_ = ReplaceDirectoryTokens(mods_dir);

  normalized_relative_mods_dir_.Empty();
  if (settings.modsDirectoryMode == EModsDirectoryMode::CFCore) {
    normalized_relative_mods_dir_ = FString::FromInt(settings.gameId);
  }

  mods_dir = FPaths::Combine(mods_dir, normalized_relative_mods_dir_);
  normalized_mods_dir_ = ReplaceDirectoryTokens(mods_dir);

  FString user_data_dir = settings_service_->GetSettings().userDataDirectory;
  normalized_user_data_dir_ = ReplaceDirectoryTokens(user_data_dir);

  normalized_user_logs_dir_ =
    ReplaceDirectoryTokens(FPaths::Combine(user_data_dir, kLogsDirName));

  updated_delegate_.Broadcast();
}

// -----------------------------------------------------------------------------
FString DirectoriesServiceImpl::ReplaceDirectoryTokens(
  const FString& directory) {

  FString ret_dir = directory;

  static TMap<FString, FString> tokens_map = {
    {kSpecialFolderUserDirectory, FPlatformProcess::UserDir()},
    {kSpecialFolderUserSettingsDirectory, FPlatformProcess::UserSettingsDir()},
    {kSpecialFolderProjectDirectory, FPaths::ProjectDir()},
    {kSpecialFolderProjectSavedDirectory, FPaths::ProjectSavedDir()}
  };

  ESearchCase::Type ignore_case = ESearchCase::Type::IgnoreCase;

  for (auto token : tokens_map) {
    ret_dir = ret_dir.Replace(*token.Key, *token.Value, ignore_case);
  }

  FPaths::MakeStandardFilename(ret_dir);
  FixDoubleSlashes(ret_dir);

  return ret_dir;
}

// -----------------------------------------------------------------------------
void DirectoriesServiceImpl::FixDoubleSlashes(FString& directory) {
  directory.ReplaceInline(TEXT("//"), TEXT("/"), ESearchCase::CaseSensitive);
}