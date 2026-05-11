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
#include <CoreMinimal.h>
#include <Containers/Map.h>

#include "mods_installation_queue_service.h"

#include <models/cfcore_settings.h>

namespace cfcore {
class ISettingsService;

// -----------------------------------------------------------------------------
class ModsInstallationQueueServiceImpl :
  public IModsInstallationQueueService {
public:
  ModsInstallationQueueServiceImpl(
    TSharedRef<ISettingsService> settings_service);

// IModsInstallationQueueService
public:
  virtual void Enqueue(FInstallModRequestPtr request) override;
  virtual bool MoveNextToInProgress(FInstallModRequestPtr& out_request) override;
  virtual void Remove(FInstallModRequestPtr request) override;

  virtual void GetAllRequests(
    TArray<FInstallModRequestPtr>& out_requests) override;

  virtual bool PeekByModId(int64 mod_id,
                           FInstallModRequestPtr& out_request) override;

private:
  int32 GetMaxConccurentInstallations();
  bool SetInProgress(FInstallModRequestPtr request);

private:
  TSharedRef<ISettingsService> settings_service_;

  TArray<FInstallModRequestPtr> installation_requests_;
  TMap<int64, FInstallModRequestPtr> installations_in_progress_;
};

} // namespace cfcore