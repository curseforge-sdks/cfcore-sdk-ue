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

#include <common/container_utils.h>

namespace cfcore {

// -----------------------------------------------------------------------------
template <typename TDelegates>
class CFCoreApiDelegatesDispatcher {
public:
  virtual ~CFCoreApiDelegatesDispatcher() = default;

public:
  // ---------------------------------------------------------------------------
  void Register(FGuid id, TSharedRef<TDelegates> delegates) {
    delegates_.Add(id, delegates);
    delegates_as_set_.Add(delegates);
  }

  // ---------------------------------------------------------------------------
  TSet<TSharedRef<TDelegates>> Delegates() {
    return delegates_as_set_;
  }

  // ---------------------------------------------------------------------------
  void Unregister(FGuid id) {
    delegates_as_set_.Remove(delegates_[id]);
    delegates_.Remove(id);
  }

private:
  TMap<FGuid, TSharedRef<TDelegates>> delegates_;
  TSet<TSharedRef<TDelegates>> delegates_as_set_;
};

} // namespace cfcore