/*MIT License

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
SOFTWARE.*/
#include "io_throttling_service_impl.h"

using namespace cfcore;

namespace cfcore {
const float kSecond =  1;

// Protection so that no accidence happen (e.g. the rate is so low that it takes
// an hour to write a mod to disk)
const int64 kMinDiskWriteBytesPerSec = 2621440;
};

// -----------------------------------------------------------------------------
IOThrottlingServiceImpl::IOThrottlingServiceImpl(
  TSharedPtr<ISettingsService> settings_service) :
  settings_service_(settings_service),
  last_quota_start_time_seconds_(FPlatformTime::Seconds()),
  bytes_written_current_second_(0) {

  FPlatformTime::InitTiming();
}

// -----------------------------------------------------------------------------
int64 IOThrottlingServiceImpl::SleepForNextQuotaInBytes(int64 requested_bytes) {
  int64 bytes_per_sec =
    settings_service_->GetSettings().throttling.diskWriteBytesPerSec;

  if (bytes_per_sec == 0) {
    return requested_bytes;
  }

  // Add our min protection - see comment on |kMinDiskWriteBytesPerSec|
  bytes_per_sec = FMath::Max(bytes_per_sec, kMinDiskWriteBytesPerSec);

  FScopeLock scope_lock(&critical_section_);

  double passed = FPlatformTime::Seconds() - last_quota_start_time_seconds_;
  if (passed > 1) {
    bytes_written_current_second_ = 0;
  }

  int64 bytes_allowed = bytes_per_sec - bytes_written_current_second_;
  if (bytes_allowed == 0) {
    FPlatformProcess::Sleep(kSecond);
    bytes_written_current_second_ = 0;
    bytes_allowed = bytes_per_sec;
  }

  bytes_allowed = FMath::Min(requested_bytes, bytes_allowed);
  bytes_written_current_second_ += bytes_allowed;
  last_quota_start_time_seconds_ = FPlatformTime::Seconds();

  return bytes_allowed;
}
