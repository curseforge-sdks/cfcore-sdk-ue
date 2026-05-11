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
#pragma once
#include <CoreMinimal.h>

#include "thread_pools_service.h"
#include <common/macros.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class ThreadPoolsServiceImpl : public IThreadPoolsService {
public:
  ThreadPoolsServiceImpl(TSharedPtr<FQueuedThreadPool> logger,
                         TSharedPtr<FQueuedThreadPool> io,
                         TSharedPtr<FQueuedThreadPool> compression) :
    logger_(logger),
    io_(io),
    compression_(compression) {
  }

// IThreadPoolsService
public:
  virtual TSharedPtr<FQueuedThreadPool> Logger() const {
    return logger_;
  };

  virtual TSharedPtr<FQueuedThreadPool> IO() const {
    return io_;
  };

  virtual TSharedPtr<FQueuedThreadPool> Compression() const {
    return compression_;
  };

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ThreadPoolsServiceImpl);

  TSharedPtr<FQueuedThreadPool> logger_;
  TSharedPtr<FQueuedThreadPool> io_;
  TSharedPtr<FQueuedThreadPool> compression_;
};

} // namespace cfcore