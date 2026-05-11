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

#include <Misc/TypeContainer.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// Allows for creating factories for specific interfaces
template<class T>
class ClassFactory {
public:
  ClassFactory(TSharedPtr<TTypeContainer<>> container) : container_(container) {
    // NOTE(twolf): force create at least one instance so that the compiler will
    // not optimize and not include it - we experienced this in UE5.2 where if
    // we didn't force create here, then TNameOf<T>::GetName() would not exist
    // for T
    Create();
  }

public:
  TSharedRef<T> Create() {
    return container_->GetInstance<T>();
  }

private:
  TSharedPtr<TTypeContainer<>> container_;
};


} // namespace cfcore