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
#include "directories_service.h"
#include "settings_service.h"
#include <common/macros.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class DirectoriesServiceImpl : public IDirectoriesService {
public:
  DirectoriesServiceImpl(TSharedPtr<ISettingsService> settings_service);

// IDirectoriesService
public:
  virtual FDirectoriesUpdated& OnUpdated() override {
    return updated_delegate_;
  }

  virtual const FString GetModsDirectory() const override {
    return normalized_mods_dir_;
  }

  virtual const FString GetRelativeModsDirectory() const override {
    return normalized_relative_mods_dir_;
  }

  virtual const FString GetModsRootDirectory() const override {
    return normalized_root_mods_dir_;
  }

  virtual const FString GetUserDataDirectory() const override {
    return normalized_user_data_dir_;
  }

  virtual const FString GetUserLogsDirectory() const override {
    return normalized_user_logs_dir_;
  }


private:
  void InitializeMembers();
  FString ReplaceDirectoryTokens(const FString& directory);
  void FixDoubleSlashes(FString& directory);

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(DirectoriesServiceImpl);

  TSharedPtr<ISettingsService> settings_service_;
  FString normalized_mods_dir_;
  FString normalized_relative_mods_dir_;
  FString normalized_root_mods_dir_;
  FString normalized_user_data_dir_;
  FString normalized_user_logs_dir_;

  FDirectoriesUpdated updated_delegate_;
};

} // namespace cfcore