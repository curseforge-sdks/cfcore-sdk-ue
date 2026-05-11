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
#include <Async/Future.h>
#include <Misc/Paths.h>

#include <services/buffer/buffer.h>
#include <common/cfcore_error.h>

namespace cfcore {
typedef TTuple<FCFCoreError, FString> HashResult;

// -----------------------------------------------------------------------------
class IHashingService {
public:
  virtual ~IHashingService() = default;

public:
  virtual FString Md5(TSharedPtr<IBuffer> buffer) = 0;
  virtual FString Sha1(TSharedPtr<IBuffer> buffer) = 0;
  virtual uint64 Murmur2(TSharedPtr<IBuffer> buffer) = 0;

  virtual TFuture<HashResult> Md5(TSharedRef<FString> filename) = 0;
  virtual TFuture<HashResult> Sha1(TSharedRef<FString> filename) = 0;
  virtual TFuture<HashResult> Crc32(TSharedRef<FString> filename) = 0;
  virtual TFuture<HashResult> Murmur2(TSharedRef<FString> filename) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IHashingService)