/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include "timer_service.h"
#include <common/macros.h>
#include "models/timer_delegate_info.h"

#include <common/ue_compatibility_ticker.h>

namespace cfcore {
class ILifetimeService;

// -----------------------------------------------------------------------------
class TimerServiceImpl : public ITimerService {
public:
  TimerServiceImpl(TSharedPtr<ILifetimeService> lifetime_service);

// ITimerService
public:
  virtual FDelegateHandle Delay(const FTimerDelegate& delegate,
                                uint64_t time_in_ms) override;

  virtual bool CancelDelay(FDelegateHandle handle) override;

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(TimerServiceImpl);

private:
  void HandleTick(float delta);

  FDelegateHandle AddTimer(const FTimerDelegate& delegate,
                           uint64_t time_in_ms);

private:
  TSharedPtr<ILifetimeService> lifetime_service_;

  FDelegateHandle on_shutdown_handle_;
  TickerDelegate ticker_handler_;
  bool is_shutting_down_;
  TArray<TimerDelegateInfo> delegates_;
};

} // namespace cfcore