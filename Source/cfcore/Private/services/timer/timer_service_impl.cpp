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
#include "timer_service_impl.h"

#include <services/lifetime/lifetime_service.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
TimerServiceImpl::TimerServiceImpl(
  TSharedPtr<ILifetimeService> lifetime_service) :
  lifetime_service_(lifetime_service),
  is_shutting_down_(false) {

  on_shutdown_handle_ = lifetime_service->OnShutdown().AddLambda([this]() {
    is_shutting_down_ = true;
    Ticker::GetCoreTicker().RemoveTicker(ticker_handler_);
    // TODO(apliscov): Consider removing pending delegates
    delegates_.Empty();
  });

  ticker_handler_ = Ticker::GetCoreTicker().AddTicker(
    FTickerDelegate::CreateLambda([this](float delta) {

    if (is_shutting_down_) {
      return false;
    }

    HandleTick(delta);
    return true;
  }));
}

// -----------------------------------------------------------------------------
FDelegateHandle TimerServiceImpl::Delay(const FTimerDelegate& delegate,
                                        uint64_t time_in_ms) {

  return AddTimer(delegate, time_in_ms);
}

// -----------------------------------------------------------------------------
bool TimerServiceImpl::CancelDelay(FDelegateHandle handle) {
  for (int32 index = 0; index < delegates_.Num(); ++index) {
    if (delegates_[index].delegate.GetHandle() == handle) {
      delegates_.RemoveAt(index);
      return true;
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
void TimerServiceImpl::HandleTick(float delta) {
  int32 index = 0;

  while (index < delegates_.Num()) {
    TimerDelegateInfo& info = delegates_[index];

    FTimespan delta_time = FDateTime::Now() - info.start_timestamp;
    double delta_passed = delta_time.GetTotalMilliseconds();

    if (delta_passed < info.delay_time_in_ms) {
      index++;
      continue;
    }

    info.delegate.ExecuteIfBound();

    delegates_.RemoveAt(index);
  }
}

// -----------------------------------------------------------------------------
FDelegateHandle TimerServiceImpl::AddTimer(const FTimerDelegate& delegate,
                                           uint64_t time_in_ms) {

  TimerDelegateInfo info;
  info.delegate = delegate;
  info.delay_time_in_ms = time_in_ms;
  info.start_timestamp = FDateTime::Now();
  delegates_.Add(info);

  return delegate.GetHandle();
}