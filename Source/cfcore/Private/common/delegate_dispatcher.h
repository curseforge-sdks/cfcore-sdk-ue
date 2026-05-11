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
#include "CoreMinimal.h"

#include "delegate_registrar.h"
#include "delegate_broadcaster.h"

namespace cfcore {

// -----------------------------------------------------------------------------
template <typename T>
class DelegateDispatcher :
  public IDelegateRegistrar<T>,
  public IDelegateBroadcaster<T> {

  // IDelegateRegistrar<T>
public:
  // ---------------------------------------------------------------------------
  virtual void Register(
    FGuid id,
    TSharedRef<T> delegate) override {

    UnRegister(id, delegate);
    delegates_.Add(id, delegate);
  }

  // ---------------------------------------------------------------------------
  virtual void UnRegister(
    FGuid id,
    TSharedRef<T> delegate) override {

    if (!delegates_.Contains(id)) {
      return;
    }

    if (delegates_[id] != delegate) {
      return;
    }

    delegates_.Remove(id);
  }

  // IDelegateBroadcaster<T>
public:
  // ---------------------------------------------------------------------------
  virtual void ForEach(
    const TFunction<void(TSharedRef<T>)>& listener)
      override {

    for (auto& pair : delegates_) {
      listener(pair.Value);
    }
  }

  // ---------------------------------------------------------------------------
  virtual void ForEach(
    const TFunction<void(TSharedRef<T>, bool&)>& listener)
      override {

    bool break_loop = false;
    for (auto& pair : delegates_) {
      listener(pair.Value, break_loop);

      if (break_loop) {
        break;
      }
    }
  }

private:
  TMap<FGuid, TSharedRef<T>> delegates_;
};

}; // namespace cfcore