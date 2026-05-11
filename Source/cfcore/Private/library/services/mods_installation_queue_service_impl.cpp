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
#include "mods_installation_queue_service_impl.h"

#include <services/settings_service.h>

namespace cfcore {
};

using namespace cfcore;

// -----------------------------------------------------------------------------
ModsInstallationQueueServiceImpl::ModsInstallationQueueServiceImpl(
  TSharedRef<ISettingsService> settings_service) :
  settings_service_(settings_service) {
}

// -----------------------------------------------------------------------------
void ModsInstallationQueueServiceImpl::Enqueue(
  FInstallModRequestPtr request) {

  request->inProgerss = false;
  installation_requests_.Add(request);
}

// -----------------------------------------------------------------------------
bool ModsInstallationQueueServiceImpl::MoveNextToInProgress(
  FInstallModRequestPtr& out_request) {

  if (installation_requests_.Num() == 0) {
    return false;
  }

  int32 in_progress = installations_in_progress_.Num();
  int32 max_concurrency = GetMaxConccurentInstallations();
  if (in_progress == max_concurrency) {
    return false;
  }

  out_request = installation_requests_[0];
  installation_requests_.RemoveAt(0);

  SetInProgress(out_request);

  return true;
}

// -----------------------------------------------------------------------------
void ModsInstallationQueueServiceImpl::Remove(FInstallModRequestPtr request) {
  int index = installation_requests_.IndexOfByPredicate(
    [request](const FInstallModRequestPtr& req) {
      return req->modId == request->modId;
    }
  );

  if (INDEX_NONE != index) {
    installation_requests_.RemoveAt(index);
    return;
  }

  if (!installations_in_progress_.Contains(request->modId)) {
    return;
  }

  installations_in_progress_.Remove(request->modId);
}

// -----------------------------------------------------------------------------
void ModsInstallationQueueServiceImpl::GetAllRequests(
  TArray<FInstallModRequestPtr>& out_requests) {

  for (const FInstallModRequestPtr& req : installation_requests_) {
    out_requests.Add(req);
  }

  for (const auto& pair : installations_in_progress_) {
    out_requests.Add(pair.Value);
  }
}

// -----------------------------------------------------------------------------
bool ModsInstallationQueueServiceImpl::PeekByModId(
  int64 mod_id,
  FInstallModRequestPtr& out_request) {

  FInstallModRequestPtr* req = installation_requests_.FindByPredicate(
    [mod_id](const FInstallModRequestPtr& req) {
      return req->modId == mod_id;
    }
  );

  if (req) {
    out_request = *req;
    return true;
  }

  if (!installations_in_progress_.Contains(mod_id)) {
    return false;
  }

  out_request = installations_in_progress_[mod_id];
  out_request->inProgerss = true;
  return true;
}

// -----------------------------------------------------------------------------
int32 ModsInstallationQueueServiceImpl::GetMaxConccurentInstallations() {
  const FCFCoreSettings& settings = settings_service_->GetSettings();
  int32 max_concurrency =
    FMath::Max<int32>(1, settings.maxConcurrentInstallations);

  return max_concurrency;
}

// -----------------------------------------------------------------------------
bool ModsInstallationQueueServiceImpl::SetInProgress(
  FInstallModRequestPtr request) {

  request->inProgerss = true;
  installations_in_progress_.Add(request->modId, request);

  return false;
}

