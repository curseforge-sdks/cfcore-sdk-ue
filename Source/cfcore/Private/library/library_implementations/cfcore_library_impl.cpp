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
#include "cfcore_library_impl.h"
#include <services/logger/logger_macros.h>
#include <common/macros.h>
#include <common/future_utils.h>
#include <common/container_utils.h>
#include <common/constants.h>

#include "library/services/installed_mods_info/get_installed_mods_service.h"
#include "library/services/installed_mods_info/installed_mods_info_service.h"
#include "library/services/mods_installation_service.h"
#include "library/services/mods_validation/mods_validation_service.h"
#include "library/services/dynamic_content_service.h"
#include "analytics/cfcore_analytics_internal.h"

#include <library/cfcore_client_server_library.h>
#include <library/services/mods_cleanup/mods_cleanup_service.h>
#include <library/services/installed_mods_synchronize_with_server_service.h>
#include <library/services/unmanaged_mods_service.h>

#include <library/common/cfcore_per_user_library_utils.h>
#include <library/common/cfcore_library_macros.h>
#include "library/services/install_analytics_tagger/install_analytics_tagger.h"

namespace cfcore {
using InstallModsInfoResult = IInstalledModsInfoService::InstallModsInfoResult;
using InstallResult = IModsInstallationService::InstallResult;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
CFCoreLibraryImpl::CFCoreLibraryImpl(
  TSharedRef<IInstalledModsInfoService> installed_mods_info_service,
  TSharedRef<IGetInstalledModsService> get_installed_mods_service,
  TSharedRef<IModsInstallationService> mods_inst_srv,
  TSharedRef<IModsValidationService> mods_validation_service,
  TSharedRef<IDynamicContentService> dynamic_content_service,
  TSharedRef<IModsCleanupService> mods_cleanup_service,
  TSharedRef<IInstalledModsSynchronizeWithServerService> mods_sync_service,
  TSharedRef<IUnmanagedModsService> unmanaged_mods_service,
  TSharedRef<ICFCoreAnalyticsInternal> cfcore_analytics_internal,
  TSharedRef<IDelegateBroadcaster<ICFCoreLibraryDelegate>> delegates,
  TSharedRef<InstallAnalyticsTaggerArray> install_analytics_taggers) :
  installed_mods_info_service_(installed_mods_info_service),
  get_installed_mods_service_(get_installed_mods_service),
  mods_installation_service_(mods_inst_srv),
  mods_validation_service_(mods_validation_service),
  dynamic_content_service_(dynamic_content_service),
  mods_cleanup_service_(mods_cleanup_service),
  mods_sync_service_(mods_sync_service),
  unmanaged_mods_service_(unmanaged_mods_service),
  cfcore_analytics_internal_(cfcore_analytics_internal),
  install_analytics_taggers_(install_analytics_taggers),
  delegates_(delegates) {
}

// -----------------------------------------------------------------------------
TSharedPtr<ICFCoreLibrary> CFCoreLibraryImpl::GetLibrary() {
  return this->AsShared();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreLibraryImpl::Initialize() {
  check(IsInGameThread());

  auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>(
  TArray<TSequenceFunction<FCFCoreError>> {
    [this]() { return installed_mods_info_service_->Initialize(); },
    [this]() { return mods_installation_service_->Initialize(); },
    [this]() { return unmanaged_mods_service_->Initialize(); },
    [this]() { return mods_cleanup_service_->Run(FModsCleanupOptions()); },
    [this]() { return PerformInstalledModsValidations(); }
  });

  return FutureUtils::SequenceAll(functions, &FCFCoreError::isError);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::SetClientServerLibrary(
  TSharedPtr<ICFCoreClientServerLibrary> client_server_library) {

  cfcore_client_server_library_ = client_server_library;
}

// -----------------------------------------------------------------------------
ICFCoreLibrary::FModInstallProgressDelegate&
CFCoreLibraryImpl::OnModInstallProgress() {
  CFCORE_LIBRARY_MARK_UNSUPPORTED_DELEGATE;
  return dummy_mod_install_progress_delegate_;
}

// -----------------------------------------------------------------------------
ICFCoreLibrary::FModInstalledDelegate& CFCoreLibraryImpl::OnModInstalled() {
  CFCORE_LIBRARY_MARK_UNSUPPORTED_DELEGATE;
  return dummy_mod_installed_delegate_;
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::SyncWithServer(FSyncWithServerDelegate delegate) {
  check(IsInGameThread());

  mods_sync_service_->Sync().Next([delegate](FCFCoreError err) {
    if (err.isError) {
      delegate.ExecuteIfBound(err);
      return;
    }

    delegate.ExecuteIfBound({});
  });
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::GetInstalledMods(FGetInstalledModsDelegate delegate) {
  check(IsInGameThread());

  TArray<FInstalledMod> installed_mods;
  get_installed_mods_service_->GetUserInstalledMods(installed_mods);

  TArray<FInstalledMod> visible_mods = FilterOutHiddenMods(installed_mods);
  delegate.ExecuteIfBound(MoveTemp(visible_mods));
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::GetSystemInstalledMods(
  FGetInstalledModsDelegate delegate) {

  TArray<FInstalledMod> installed_mods;
  get_installed_mods_service_->GetSystemInstalledMods(installed_mods);

  TArray<FInstalledMod> visible_mods = FilterOutHiddenMods(installed_mods);
  delegate.ExecuteIfBound(MoveTemp(visible_mods));
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::GetModsDirInfo(FGetModsDirInfoDelegate delegate) {
  check(IsInGameThread());

  auto mods_dir_info = MakeShared<FModsDirInfo>();
  auto future = mods_installation_service_->GetModsDirInfo(mods_dir_info);

  future.Next([delegate, mods_dir_info](FCFCoreError err) {
    if (err.isError) {
      delegate.ExecuteIfBound({}, err);
    } else {
      delegate.ExecuteIfBound(*mods_dir_info, {});
    }
  });
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::Install(FCFCoreMod mod,
                                FInstallProgressDelegate on_progrs,
                                FInstalledDelegate on_installed) {

  CFCORE_LIBRARY_MARK_UNSUPPORTED_INSTALL;
  Install(mod, FFile(), FInstallModAdditionalParams(), on_progrs, on_installed);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::Install(const FCFCoreMod& mod,
                                const FFile& file,
                                FInstallProgressDelegate on_progress,
                                FInstalledDelegate on_installed) {

  CFCORE_LIBRARY_MARK_UNSUPPORTED_INSTALL;
  Install(mod, file, FInstallModAdditionalParams(), on_progress, on_installed);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::Install(
  const FCFCoreMod& mod,
  const FFile& file,
  const FInstallModAdditionalParams& inst_params,
  FInstallProgressDelegate on_progress,
  FInstalledDelegate on_installed) {

  auto params = MakeShared<FInstallModParams>();
  params->mod = mod;
  params->file = file;
  params->additional_params = inst_params;
  params->on_progress = on_progress;
  params->on_installed = on_installed;

  HandleInstall(params);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::CancelInstallation(int64 mod_id,
                                           FCancelInstallDelegate delegate) {
  TOptional<FInstalledMod> opt_mod =
      installed_mods_info_service_->GetInstalledMod(mod_id);

  if (!opt_mod.IsSet()) {
    auto code = ECFCoreErrorCodes::InstalledModNotFound;
    delegate.ExecuteIfBound(FCFCoreError(code));
    return;
  }

  CFCORE_LOG(Log,
             TEXT("Request to Cancel mod '%s' (modId=%lld, fileId=%lld)"),
             *opt_mod->details.name,
             mod_id,
             opt_mod->installedFile.id);

  FCFCoreError err;
  if (!mods_installation_service_->CancelInstall(opt_mod.GetValue())) {
    err = FCFCoreError(ECFCoreErrorCodes::FailedToCancelAction);
  }

  delegate.ExecuteIfBound(err);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::Uninstall(int64 mod_id, FUninstallDelegate delegate) {
  TOptional<FInstalledMod> opt_mod =
    installed_mods_info_service_->GetInstalledMod(mod_id);

  if (!opt_mod.IsSet()) {
    auto code = ECFCoreErrorCodes::InstalledModNotFound;
    delegate.ExecuteIfBound({}, FCFCoreError(code));
    return;
  }

  // TODO(apliscov): Consider adding a guard against uninstalling unmanaged
  // mods. Unmanaged mods have details.id == 0 so they are unlikely to be
  // reached here in practice, except if mod_id 0 is passed to Uninstall.

  CFCORE_LOG(Log,
             TEXT("Request to Uninstall mod '%s' (modId=%lld, fileId=%lld)"),
             *opt_mod->details.name,
             mod_id,
             opt_mod->installedFile.id);

  FInstalledMod& installed_mod = opt_mod.GetValue();

  TFuture<FCFCoreError> future;
  if (installed_mod.users.Num() > 1) {
    future = MakeFulfilledPromise<FCFCoreError>().GetFuture();
  } else {
    future = SetUninstalled(installed_mod);
  }

  future.Next([this, installed_mod, delegate](FCFCoreError) {
    // NOTE(apliscov): if SetUninstalled returns an error, we still
    // proceed to uninstall. This can be handled at a later time, at the next
    // library init, without halting this uninstall.
    StartUninstall(installed_mod, delegate);
  });
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::PerformModsValidation(
  const TArray<FInstalledMod>& installed_mods_to_validate,
  FPerformModsValidationDelegate delegate) {

  auto validation_mods =
    MakeShared<TArray<FInstalledMod>>(installed_mods_to_validate);
  mods_validation_service_->PerformValidation(*validation_mods)
    .Next([this, validation_mods, delegate](FCFCoreError) {

    TArray<FInstalledMod> invalid = GetInvalidInstalledMods(*validation_mods);
    delegate.ExecuteIfBound(invalid, {});
  });
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::UpdateInstalledModsProperties(
  const TArray<FInstalledModProperties>& installed_mod_properties,
  FUpdateInstalledModsPropertiesDelegate delegate) {

  TArray<FInternalInstalledModInfoProperties> internal_properties;
  for (const FInstalledModProperties& props : installed_mod_properties) {
    FInternalInstalledModInfoProperties internal_props;

    if (props.id.IsValid()) {
      internal_props.id = props.id;
    }

    internal_props.modId = props.modId;
    internal_props.enabled = props.enabled;
    internal_props.loadOrder = props.loadOrder;
    internal_properties.Add(internal_props);
  }

  installed_mods_info_service_->SetInstalledProperties(internal_properties)
    .Next([this, delegate](FCFCoreError err) {

    TOptional<FCFCoreError> opt_err;
    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to update installed mod properties"));
      opt_err = err;
    }

    delegate.ExecuteIfBound(opt_err);
  });
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::CleanTempDir(FCleanTempDirDelegate delegate) {
  FModsCleanupOptions options;
  options.force_cleanup = true;

  mods_cleanup_service_->Run(options).Next([delegate](FCFCoreError err) {
    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to clean temp dir"));
      delegate.ExecuteIfBound(err);
      return;
    }

    delegate.ExecuteIfBound({});
  });
}

// -----------------------------------------------------------------------------
FCFCoreError CFCoreLibraryImpl::ValidateInstallRequest(
  const FCFCoreMod& mod,
  const FFile* file_to_install,
  const FInstallModAdditionalParams& additional_params) {

  if (!file_to_install) {
    CFCORE_LOG(Error, TEXT("Mod (%lld) has no latest file"), mod.id);
    auto err_code = ECFCoreErrorCodes::MissingLatestFileInformation;
    return FCFCoreError(err_code);
  }

  if (file_to_install->downloadUrl.IsEmpty()) {
    CFCORE_LOG(Error,
               TEXT("File (%lld) for Mod (%lld) has no url"),
               file_to_install->id,
               mod.id);

    auto err_code = ECFCoreErrorCodes::MissingFileInformation;
    return FCFCoreError(err_code);
  }

  if (file_to_install->modId != mod.id) {
    CFCORE_LOG(Error,
               TEXT("File (%lld) doesn't belong to Mod (%lld)"),
               file_to_install->id,
               mod.id);

    auto err_code = ECFCoreErrorCodes::FileNotBelongingToMod;
    return FCFCoreError(err_code);
  }

  return FCFCoreError();
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::HandleInstall(
  TSharedRef<FInstallModParams> inst_params) {

  auto functions = MakeShared<TArray<TSequenceFunction<FHandleInstallContext>>>(
    TArray<TSequenceFunction<FHandleInstallContext>> {
    [this, inst_params]() { return HandleInstallDynamicContent(inst_params); },
    [this, inst_params]() { return SetInstalling(inst_params); },
    [this, inst_params]() { return StartInstall(inst_params); }
  });

  auto stop_member = &FHandleInstallContext::should_stop;
  auto future = FutureUtils::SequenceAll(functions, stop_member);

  future.Next([this, inst_params](FHandleInstallContext last_context) {
    SendModInstallAnalytics(last_context, inst_params);

    if (last_context.error.isError) {
      CFCORE_LOG(Error,
        TEXT("Failed to install mod (%lld), error code=%d"),
        inst_params->mod.id,
        static_cast<int32>(last_context.error.code));

      inst_params->on_installed.ExecuteIfBound({}, last_context.error);
      return;
    }

    inst_params->on_installed.ExecuteIfBound(last_context.installed_mod, {});
    return;
  });
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreLibraryImpl::PerformInstalledModsValidations() {
  auto installed = MakeShared<TArray<FInstalledMod>>();
  get_installed_mods_service_->GetSystemInstalledMods(installed.Get());

  auto futures = MakeShared<TArray<TFuture<FCFCoreError>>>();
  futures->Add(mods_validation_service_->PerformQuickValidation(
    installed));
  futures->Add(dynamic_content_service_->CheckPremiumOwnership());

  SHAREABLE_PROMISE(FCFCoreError, promise);

  auto future = FutureUtils::Parallel<FCFCoreError>(futures);
  future.Next([this, promise](TArray<FCFCoreError> errors) {
    promise->SetValue(FCFCoreError());
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FHandleInstallContext> CFCoreLibraryImpl::HandleInstallDynamicContent(
  TSharedRef<FInstallModParams> inst_params) {

  SHAREABLE_PROMISE(FHandleInstallContext, promise);

  dynamic_content_service_->HandleInstall(inst_params)
    .Next([promise, inst_params](TOptional<FInstalledMod> opt_installed_mod) {

    FHandleInstallContext context;

    if (opt_installed_mod.IsSet()) {
      CFCORE_LOG(Log,
        TEXT("Finished installing mod (%lld) early after "
             "dynamic content check"),
        inst_params->mod.id);
      context.should_stop = true;
    }

    promise->SetValue(context);
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FHandleInstallContext> CFCoreLibraryImpl::SetInstalling(
  TSharedRef<FInstallModParams> install_params) {

  SHAREABLE_PROMISE(FHandleInstallContext, promise);

  installed_mods_info_service_->SetInstalling(install_params)
    .Next([install_params, promise](InstallModsInfoResult res) {

    FHandleInstallContext context;
    context.error = res.Key;

    if (res.Key.isError) {
      context.should_stop = true;
      promise->SetValue(context);
      install_params->on_installed.ExecuteIfBound({}, res.Key);
      return;
    }

    auto file = install_params->file;
    if (!UserLibUtils::RequiresInstall(res.Value, file)) {
      int64 mod_id = install_params->mod.id;
      CFCORE_LOG(Log, TEXT("Mod (%lld) does not require install"), mod_id);
      CFCORE_LOG(Log, TEXT("Finished installing mod (%lld) early"), mod_id);

      context.should_stop = true;
      promise->SetValue(context);
      install_params->on_installed.ExecuteIfBound(res.Value, {});
      return;
    }

    promise->SetValue(context);
  });

  return promise->GetFuture();
}

// TODO(apliscov): Reduce function size
// -----------------------------------------------------------------------------
TFuture<FHandleInstallContext> CFCoreLibraryImpl::StartInstall(
  TSharedRef<FInstallModParams> inst_params) {

  TOptional<FInstalledMod> opt_installed_mod =
    installed_mods_info_service_->GetInstalledMod(inst_params->mod.id);

  if (!opt_installed_mod.IsSet()) {
    FCFCoreError err(ECFCoreErrorCodes::InstalledModNotFound);
    FHandleInstallContext context{ true, err, {} };
    CFCORE_LOG(Error,
      TEXT("Failed to start install - mod (%lld) does not exist"),
      inst_params->mod.id);
    return MakeFulfilledPromise<FHandleInstallContext>(context).GetFuture();
  }

  const FInstalledMod& installed_mod = opt_installed_mod.GetValue();
  SHAREABLE_PROMISE(FHandleInstallContext, promise);

  auto future = mods_installation_service_->Install(
    installed_mod,
    inst_params->additional_params,
    IModsInstallationService::FModsInstallationProgressDelegate::CreateLambda(
      [inst_params](const FLibraryProgress& progress) {

      inst_params->on_progress.ExecuteIfBound(progress);
    })
  );

  future.Next(
    [this, installed_mod, inst_params, promise](InstallResult install_res) {

    const FCFCoreError& err = install_res.error;
    const FInstalledMod& updated_inst_mod =
      install_res.error.isError ? installed_mod : install_res.installed_mod;

    UpdateInstalledModState(err, updated_inst_mod, inst_params).Next(
      [updated_inst_mod,
       err,
       inst_params,
       promise,
       install_res](InstallModsInfoResult res) {

      if (res.Key.isError) {
        CFCORE_LOG(Error,
                   TEXT("Failed updating state (id=%lld, code=%d)"),
                   updated_inst_mod.details.id,
                   res.Key.code);
      }

      FLibraryProgress lib_progress = { 0 };
      lib_progress.modId = updated_inst_mod.details.id;
      lib_progress.fileId = updated_inst_mod.latestUpdatedFile.id;

      if (err.isError) {
        lib_progress.state = ELibraryProgressState::FailedToComplete;
      } else {
        lib_progress.state = ELibraryProgressState::SuccessfullyCompleted;
      }

      inst_params->on_progress.ExecuteIfBound(lib_progress);
      FHandleInstallContext context{
        true,
        err,
        res.Value,
        install_res.analytics_params
      };
      promise->SetValue(context);
    });
  });

  return promise->GetFuture();
}

// -----------------------------------------------------------------------------
TFuture<FCFCoreError> CFCoreLibraryImpl::SetUninstalled(
  FInstalledMod& installed_mod) {

  auto state_uninst = EInstalledModStatus::Uninstalled;

  FInternalInstalledModInfoProperties props;
  props.modId = installed_mod.details.id;
  props.status = state_uninst;
  TArray<FInternalInstalledModInfoProperties> props_arr{ props };

  return installed_mods_info_service_->SetInstalledProperties(props_arr);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::StartUninstall(
  const FInstalledMod& installed_mod,
  FUninstallDelegate delegate) {

  int64 mod_id = installed_mod.details.id;

  TFuture<FCFCoreError> future;
  if (UserLibUtils::GetRefCount(installed_mod) <= 1) {
    future = mods_installation_service_->Uninstall(installed_mod);
  } else {
    future = MakeFulfilledPromise<FCFCoreError>().GetFuture();
  }

  future.Next([this, mod_id, delegate](FCFCoreError err) {
    if (err.isError) {
      CFCORE_LOG(Error, TEXT("Failed to uninstall mod (%lld)"), mod_id);
      delegate.ExecuteIfBound({}, err);
      return;
    }

    installed_mods_info_service_->Uninstall(mod_id)
      .Next([this, mod_id, delegate](InstallModsInfoResult res) {

      if (res.Key.isError) {
        delegate.ExecuteIfBound(res.Value, res.Key);
        return;
      }

      CFCORE_LOG(Log, TEXT("Successfully uninstalled mod %lld"), mod_id);

      delegate.ExecuteIfBound(res.Value, {});
    });
  });
}

// -----------------------------------------------------------------------------
TArray<FInstalledMod> CFCoreLibraryImpl::GetInvalidInstalledMods(
  const TArray<FInstalledMod>& filter) {

  auto validation_mod_ids = MakeShared<TSet<int64>>();
  for (const FInstalledMod& installed_mod : filter) {
    validation_mod_ids->Add(installed_mod.details.id);
  }

  auto all_mods = MakeShared<TArray<FInstalledMod>>();
  get_installed_mods_service_->GetSystemInstalledMods(*all_mods);

  TArray<FInstalledMod> invalid_mods = all_mods->FilterByPredicate(
    [validation_mod_ids](const FInstalledMod& mod) {

    if (mod.status != EInstalledModStatus::Invalid) {
      return false;
    }

    return validation_mod_ids->Contains(mod.details.id);
  });

  return MoveTemp(invalid_mods);
}

// -----------------------------------------------------------------------------
TArray<FInstalledMod> CFCoreLibraryImpl::FilterOutHiddenMods(
  const TArray<FInstalledMod>& installed_mods) const {

  TArray<FInstalledMod> visible_mods;

  for (const FInstalledMod& installed_mod : installed_mods) {
    if (!ShouldHideInstalledMod(installed_mod)) {
      visible_mods.Add(installed_mod);
    }
  }

  return MoveTemp(visible_mods);
}

// -----------------------------------------------------------------------------
bool CFCoreLibraryImpl::ShouldHideInstalledMod(
  const FInstalledMod& installed_mod) const {

  bool should_hide_mod = false;

  delegates_->ForEach([&installed_mod, &should_hide_mod](
    const TSharedRef<ICFCoreLibraryDelegate> delegate,
    bool& out_break) {

    if (delegate->OnHideInstalledMod(installed_mod)) {
      should_hide_mod = out_break = true;
    }
  });

  return should_hide_mod;
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::SendModInstallAnalytics(
  const FHandleInstallContext& context,
  TSharedRef<FInstallModParams> inst_params) {

  FModInstallAnalyticsParams analytics_params = context.analytics_params;

  FillModInstallOrigin(analytics_params, inst_params->additional_params);

  analytics_params.modId = inst_params->mod.id;
  analytics_params.success = !context.error.isError;
  analytics_params.cancelled = WasInstallationCancelled(context.error);

  for (const auto& tagger : *install_analytics_taggers_) {
    tagger->Tag(context, analytics_params);
  }

  cfcore_analytics_internal_->SendModInstall(analytics_params);
}

// -----------------------------------------------------------------------------
TFuture<InstallModsInfoResult> CFCoreLibraryImpl::UpdateInstalledModState(
  const FCFCoreError& err,
  const FInstalledMod& installed_mod,
  TSharedRef<FInstallModParams> inst_params) {

  TFuture<InstallModsInfoResult> future;
  if (err.isError) {
    return installed_mods_info_service_->SetInstallFailed(installed_mod);
  }

  CFCORE_LOG(
    Log,
    TEXT("Successfully installed mod '%s' (modId=%lld, fileId=%lld)"),
    *inst_params->mod.name,
    inst_params->mod.id,
    inst_params->file.id);

  return installed_mods_info_service_->SetInstalled(
    installed_mod,
    inst_params->additional_params);
}

// -----------------------------------------------------------------------------
void CFCoreLibraryImpl::FillModInstallOrigin(
  FModInstallAnalyticsParams& analytics_params,
  const FInstallModAdditionalParams& extra_params) {

  EModInstallOrigin origin = extra_params.origin;
  if (analytics_params.wasResumed) {
    origin = EModInstallOrigin::ResumeDownload;
  }

  ENUM_TO_STR(origin_str, origin);
  analytics_params.origin = origin_str;
}

// -----------------------------------------------------------------------------
bool CFCoreLibraryImpl::WasInstallationCancelled(
  const FCFCoreError& err) const {

  if (!err.isError) {
    return false;
  }

  if (err.code != ECFCoreErrorCodes::InstallCancelled) {
    return false;
  }

  return true;
}
