/* MIT License

Copyright (c) 2023 Overwolf Ltd.

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
#include "mods_validation_service_impl.h"
#include <common/macros.h>
#include <common/future_utils.h>
#include <common/container_utils.h>

#include <services/logger/logger_macros.h>

#include <services/directories_service.h>
#include <services/filesystem/file_system_service.h>
#include <services/settings_service.h>
#include <library/services/installed_mods_info/installed_mods_info_service.h>
#include <api/cfcore_api.h>
#include <api/services/feature_phasing_service.h>

#include <Misc/Paths.h>

namespace cfcore {

  const FString kReasonInstalledFileInvalid = TEXT("Installed file not valid");
  const FString kReasonCorruptedFiles = TEXT("Corrupted mod files");
}

using namespace cfcore;

// -----------------------------------------------------------------------------
ModsValidationServiceImpl::ModsValidationServiceImpl(
  TSharedRef<IDirectoriesService> dirs_service,
  TSharedRef<IFileSystemService> file_system_service,
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
  TSharedRef<IFingerprintsService> fingerprints_service,
  TSharedRef<ICFCoreApi> cfcore_api,
  TSharedRef<IFeaturePhasingService> feature_phasing_service,
  TSharedRef<ISettingsService> settings_service,
  TSharedRef<IDelegateBroadcaster<IModsValidationServiceDelegate>> delegates) :
  dirs_service_(dirs_service),
  file_system_service_(file_system_service),
  installed_mods_info_service_(installed_mods_info_service),
  fingerprints_service_(fingerprints_service),
  cfcore_api_(cfcore_api),
  feature_phasing_service_(feature_phasing_service),
  settings_service_(settings_service),
  delegates_(delegates) {
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsValidationServiceImpl::PerformQuickValidation(
  TSharedRef<TArray<FInstalledMod>> installed_mods) {

  if (installed_mods->Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  bool file_size_validation = feature_phasing_service_->IsFeatureEnabledForUser(
    &FMePhasingData::modFileSizeValidation);

  auto futures = MakeShared<TArray<TFuture<bool>>>();
  for (const FInstalledMod& installed_mod : installed_mods.Get()) {
    if (!ShouldValidateMod(installed_mod)) {
      continue;
    }

    bool skip_validations = false;
    delegates_->ForEach([this, futures, &installed_mod, &skip_validations](
      const TSharedRef<IModsValidationServiceDelegate> delegate) {
        bool inner_skip_validations = false;
        futures->Add(delegate->OnPerformQuickValidation(
          installed_mod,
          inner_skip_validations));

        skip_validations |= inner_skip_validations;
      }
    );

    if (skip_validations) {
      continue;
    }

    futures->Add(PerformModulesExistValidation(installed_mod));
    if (file_size_validation) {
      futures->Add(PerformFileSizeValidation(installed_mod));
    }
  }

  auto future = FutureUtils::Parallel<bool>(futures);

  // NOTE: Keep installed_mods referenced as we pass references to the
  // validation functions
  future.Next([promise, installed_mods](TArray<bool>) {
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> ModsValidationServiceImpl::PerformValidation(
  const TArray<FInstalledMod>& installed_mods) {

  if (installed_mods.Num() == 0) {
    return MakeFulfilledPromise<FCFCoreError>(FCFCoreError()).GetFuture();
  }

  FCFCoreGetFilesFilter filter;
  for (const FInstalledMod& installed_mod : installed_mods) {
    filter.fileIds.Add(installed_mod.installedFile.id);
  }

  SHAREABLE_PROMISE(FCFCoreError, promise);

  cfcore_api_->GetFiles(filter, ICFCoreApi::FGetFilesDelegate::CreateLambda(
    [this, installed_mods, promise](
      const TOptional<TArray<FFile>>& opt_files,
      const TOptional<FCFCoreApiResponseError>& opt_err) {

      if (opt_err.IsSet()) {
        promise->SetValue(FCFCoreError(opt_err));
        return;
      }

      TMap<int64, FFile> files_map = ContainerUtils::AsMap(*opt_files,
                                                           &FFile::id);

      auto futures = MakeShared<TArray<TFuture<bool>>>();
      for (const FInstalledMod& installed_mod : installed_mods) {
        FFile server_file_details = {0};

        int64 file_id = installed_mod.installedFile.id;
        if (files_map.Contains(file_id)) {
          server_file_details = files_map[file_id];
        }

        futures->Add(PerformModValidation(installed_mod, server_file_details));
      }

      auto future = FutureUtils::Parallel<bool>(futures);
      future.Next([promise](TArray<bool>) {
        promise->SetValue(FCFCoreError());
      });
    })
  );

  return promise->GetFuture();

}

// -----------------------------------------------------------------------------
TFuture<bool> ModsValidationServiceImpl::PerformModulesExistValidation(
  const FInstalledMod& mod) {

  SHAREABLE_PROMISE(bool, promise);

  if (!IsInstalledFileValid(mod)) {
    SetModInvalid(mod.details.id, kReasonInstalledFileInvalid, promise);
    return promise->GetFuture();
  }

  if (mod.installedFile.modules.Num() == 0) {
    promise->SetValue(true);
    return promise->GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<bool>>>();

  FString mods_dir = FPaths::Combine(dirs_service_->GetModsRootDirectory(),
                                     mod.pathOnDisk);

  for (const FFileModule& module : mod.installedFile.modules) {
    auto path = MakeShared<FString>(FPaths::Combine(mods_dir, module.name));
    futures->Add(file_system_service_->PathExists(MoveTemp(path)));
  }

  int64 mod_id = mod.details.id;
  auto future = FutureUtils::Parallel<bool>(futures);
  future.Next([this, promise, mod_id](TArray<bool> existingModules) {
    for (int i = 0; i < existingModules.Num(); ++i) {
      if (existingModules[i]) {
        continue;
      }

      SetModInvalid(mod_id, TEXT("Missing module"), promise);
      return;
    }

    promise->SetValue(true);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> ModsValidationServiceImpl::PerformModValidation(
  const FInstalledMod& mod,
  const FFile& server_file_details) {

  SHAREABLE_PROMISE(bool, promise);

  if (mod.installedFile.id != server_file_details.id) {
    SetModInvalid(mod.details.id, TEXT("Server details missing"), promise);
    return promise->GetFuture();
  }

  if (!IsInstalledFileValid(mod)) {
    SetModInvalid(mod.details.id, kReasonInstalledFileInvalid, promise);
    return promise->GetFuture();
  }

  auto futures = MakeShared<TArray<TFuture<bool>>>();

  auto fingerprints = MakeShared<ModuleFingerprintArray>();
  int64 mod_id = mod.details.id;
  const TArray<FFileModule>& modules = server_file_details.modules;

  fingerprints_service_->CalcFingerprints(mod, modules, fingerprints)
    .Next([this, mod_id, promise, fingerprints](FCFCoreError err) {

    if (err.isError) {
      SetModInvalid(mod_id, TEXT("Error calculating fingerprints"), promise);
      return;
    }

    bool contains_invalid = ContainsInvalidFingerprint(mod_id, fingerprints);
    if (contains_invalid) {
      SetModInvalid(mod_id, TEXT("Invalid fingerprint for module"), promise);
      return;
    }

    SetModValid(mod_id, "Valid fingerprint detected", promise);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
bool ModsValidationServiceImpl::ContainsInvalidFingerprint(
  int64 mod_id,
  TSharedRef<ModuleFingerprintArray> fingerprints) {

  bool has_invalid_fingerprint = fingerprints->ContainsByPredicate(
    [mod_id](const FModuleFingerprint& fingerprint) {

    if (!fingerprint.is_valid) {
      CFCORE_LOG(
        Error,
        TEXT("Invalid module fingerprint=%lld, module=%s, modId=%lld"),
        fingerprint.fingerprint,
        *fingerprint.module.name,
        mod_id);

      return true;
    }

    return false;
  });

  return has_invalid_fingerprint;
}

// -----------------------------------------------------------------------------
bool ModsValidationServiceImpl::IsInstalledFileValid(const FInstalledMod& mod) {
  if ((mod.installedFile.id == 0) ||
      (mod.installedFile.modId != mod.details.id)) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
void ModsValidationServiceImpl::SetModInvalid(
  int64 mod_id,
  const FString& reason,
  TSharedPtr<TPromise<bool>> promise) {

  auto status = EInstalledModStatus::Invalid;

  CFCORE_LOG(Warning,
             TEXT("Set installed mod status invalid (%lld) - %s"),
             mod_id,
             *reason);

  struct FInternalInstalledModInfoProperties props;
  props.modId = mod_id;
  props.status = status;
  TArray<FInternalInstalledModInfoProperties> props_arr{ props };
  auto f = installed_mods_info_service_->SetInstalledProperties(props_arr);
  f.Next([promise](FCFCoreError) {
    promise->SetValue(true);
  });
}

// -----------------------------------------------------------------------------
void ModsValidationServiceImpl::SetModValid(
  int64 mod_id,
  const FString& reason,
  TSharedPtr<TPromise<bool>> promise) {

  TOptional<FInstalledMod> installed_mod =
    installed_mods_info_service_->GetInstalledMod(mod_id);
  if (!installed_mod.IsSet()) {
    CFCORE_LOG(Log, TEXT("Failed get mod for setting status (%lld)"), mod_id);
    promise->SetValue(true);
    return;
  }

  CFCORE_LOG(Log,
             TEXT("Set installed mod status valid (%lld) (%s)"),
             mod_id,
             *reason);

  static TSet<EInstalledModStatus> StatusesForUpdating {
    // In case we installed successfully but never managed to update the status
    // EInstalledModStatus::Pending,
    EInstalledModStatus::Invalid
  };

  if (!StatusesForUpdating.Contains(installed_mod->status)) {
    promise->SetValue(true);
    return;
  }

  auto status = EInstalledModStatus::Normal;
  auto oldId = installed_mod->installedFile.id;
  auto newId = installed_mod->latestUpdatedFile.id;

  if (oldId != newId) {
    status = EInstalledModStatus::OutOfDate;
  }

  struct FInternalInstalledModInfoProperties props;
  props.modId = mod_id;
  props.status = status;
  TArray<FInternalInstalledModInfoProperties> props_arr{ props };
  auto f = installed_mods_info_service_->SetInstalledProperties(props_arr);
  f.Next([promise](FCFCoreError) {
    promise->SetValue(true);
  });
}

// -----------------------------------------------------------------------------
bool ModsValidationServiceImpl::IsValidFingerprint(const FFileModule& module,
                                                   uint64 fingerprint) {

  if (module.fingerprint == fingerprint) {
    return true;
  }

  CFCORE_LOG(
    Error,
    TEXT("Invalid fingerprint detected (%lld != %lld) for module: %s"),
    module.fingerprint,
    fingerprint,
    *module.name);

  return false;
}

// -----------------------------------------------------------------------------
TFuture<bool> ModsValidationServiceImpl::PerformFileSizeValidation(
  const FInstalledMod& mod) {

  if (mod.installedFile.fileSizeOnDisk == 0) {
    return MakeFulfilledPromise<bool>(true).GetFuture();
  }

  SHAREABLE_PROMISE(bool, promise);

  if (!IsInstalledFileValid(mod)) {
    SetModInvalid(mod.details.id, kReasonInstalledFileInvalid, promise);
    return promise->GetFuture();
  }

  auto mod_files = MakeShared<TArray<FString>>();
  auto future = GetModFiles(mod, mod_files);
  future.Next([this, mod, mod_files, promise](bool success) {
    if (!success) {
      promise->SetValue(true);
      return;
    }

    auto future = GetModFileStats(mod_files);
    future.Next([this, mod, promise](TArray<FFileStatData> file_stats) {

      int64 total_file_size_on_disk = 0;
      for (const FFileStatData& stat : file_stats) {
        total_file_size_on_disk += stat.FileSize;
      }

      if (total_file_size_on_disk != mod.installedFile.fileSizeOnDisk) {
        SetModInvalid(mod.details.id, kReasonCorruptedFiles, promise);
        return;
      }

      promise->SetValue(true);
      return;
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<bool> ModsValidationServiceImpl::GetModFiles(
  const FInstalledMod& mod,
  TSharedRef<TArray<FString>> mod_files) {

  auto mod_directory = FPaths::Combine(dirs_service_->GetModsRootDirectory(),
    mod.pathOnDisk);

  return file_system_service_->GetFilesInDirectoryRecursive(
    MakeShared<FString>(mod_directory),
    MakeShared<FString>(),
    mod_files);
}

// -----------------------------------------------------------------------------
TFuture<TArray<FFileStatData>> ModsValidationServiceImpl::GetModFileStats(
  TSharedRef<TArray<FString>> mod_files) {

  auto stat_futures = MakeShared<TArray<TFuture<FFileStatData>>>();
  for (const FString& file : *mod_files) {
    if (IsIgnoredFile(file)) {
      continue;
    }

    stat_futures->Add(file_system_service_->GetStat(MakeShared<FString>(file)));
  }

  return FutureUtils::Parallel<FFileStatData>(stat_futures);
}

// -----------------------------------------------------------------------------
bool ModsValidationServiceImpl::IsIgnoredFile(const FString& filename) {
  FString lower_case_filename = FPaths::GetCleanFilename(filename).ToLower();

  return settings_service_->GetSettings()
                            .ignoredDynamicModFiles
                            .Contains(lower_case_filename);
}

// -----------------------------------------------------------------------------
bool ModsValidationServiceImpl::ShouldValidateMod(const FInstalledMod& mod) {
  return (mod.status != EInstalledModStatus::Invalid);
}
