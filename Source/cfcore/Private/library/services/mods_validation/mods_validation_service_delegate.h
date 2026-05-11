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

#include <library/models/installed_mod.h>
#include <common/delegate_dispatcher.h>
#include <common/delegate_registrar.h>
#include <common/delegate_broadcaster.h>

namespace cfcore {

// -----------------------------------------------------------------------------
class IModsValidationServiceDelegate {
public:
  virtual ~IModsValidationServiceDelegate() = default;

public:
  virtual TFuture<bool> OnPerformQuickValidation(
    const FInstalledMod& installed_mod,
    bool& out_skip_other_validations) = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::DelegateDispatcher<cfcore::IModsValidationServiceDelegate>)
Expose_TNameOf(cfcore::IDelegateRegistrar<cfcore::IModsValidationServiceDelegate>)
Expose_TNameOf(cfcore::IDelegateBroadcaster<cfcore::IModsValidationServiceDelegate>)