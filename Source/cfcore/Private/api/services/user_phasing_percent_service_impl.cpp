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
#include "user_phasing_percent_service_impl.h"

#include <services/user_context/shared_context_service.h>
#include <services/logger/logger_macros.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
UserPhasingPercentServiceImpl::UserPhasingPercentServiceImpl(
  TSharedRef<ISharedContextService> shared_context_service) :
  shared_context_service_(shared_context_service) {
}

// -----------------------------------------------------------------------------
int32 UserPhasingPercentServiceImpl::GetUserPhasingPercent(
  const FString& salt) {

  FSharedContextInfo info = shared_context_service_->SharedContextInfo();

  if (info.machineId.IsEmpty()) {
    CFCORE_LOG(Error, TEXT("No machine id was found"));
    return -1;
  }

  FString value = info.machineId + salt;
  uint32 hash = GetTypeHash(value);

  return (hash % 100) + 1;
}
