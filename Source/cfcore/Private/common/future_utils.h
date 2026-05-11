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
#include "CoreMinimal.h"
#include <Async/Future.h>

#include <common/macros.h>
#include <common/cfcore_error.h>
#include "models/future_utils_sequence_type.h"

namespace cfcore {

template<typename TFutureType>
using TSequenceFunction = TFunction<TFuture<TFutureType>()>;

// -----------------------------------------------------------------------------
class FutureUtils {
public:
  // ---------------------------------------------------------------------------
  // A JavScript Promise.all equivalent
  template<typename TFutureType>
  static TFuture<TArray<TFutureType>> Parallel(
    const TSharedPtr<TArray<TFuture<TFutureType>>> futures) {

    if (!futures.IsValid() || futures->Num() == 0) {
      return MakeFulfilledPromise<TArray<TFutureType>>().GetFuture();
    }

    SHAREABLE_PROMISE(TArray<TFutureType>, promise);

    int count = futures->Num();

    TSharedPtr<FThreadSafeCounter> completed = 
      MakeShareable(new FThreadSafeCounter(0));

    auto results = MakeShared<TArray<TFutureType>>();
    results->SetNum(count);

    int results_index = 0;

    for (TFuture<TFutureType>& future : *futures) {
      future.Next([=](TFutureType result) {

        TArray<TFutureType>& results_array = results.Get();
        results_array[results_index] = result;

        int32 completions = completed->Increment();
        if (completions != count) {
          return;
        }

        promise->SetValue(results.Get());
      });

      results_index++;
    };

    return promise->GetFuture();
  }

  // ---------------------------------------------------------------------------
  // Accepts functions returning futures of type FCFCoreError - the first to
  // fail will stop the sequence from running
  //
  // success_member - &FCFCoreError::isError
  template<typename TFutureType>
  static TFuture<TFutureType> SequenceAll(
    const TSharedRef<TArray<TSequenceFunction<TFutureType>>> functions,
    bool TFutureType::* success_member) {

    if (functions->Num() == 0) {
      return MakeFulfilledPromise<TFutureType>().GetFuture();
    }

    SHAREABLE_PROMISE(TFutureType, promise);

    RunNextSequenceAllFunction(functions, 0, success_member, promise);

    return promise->GetFuture();
  }

  // ---------------------------------------------------------------------------
  // Accepts functions returning futures of type FCFCoreError - the first to
  // succeed will stop the sequence from running
  //
  // Example:
  // auto functions = MakeShared<TArray<TSequenceFunction<FCFCoreError>>>();
  //
  // for (auto strategy : *installation_strategies_) {
  //  functions->Add([request, strategy]() {
  //    request->installation_strategy = strategy;
  //    return strategy->Install(request.ToSharedRef());
  //  });
  // }
  //
  // return FutureUtils::SequenceAny(functions, &FCFCoreError::isError);
  template<typename TFutureType>
  static TFuture<TFutureType> SequenceAny(
    const TSharedRef<TArray<TSequenceFunction<TFutureType>>> functions,
    bool TFutureType::* completion_member) {

    if (functions->Num() == 0) {
      return MakeFulfilledPromise<TFutureType>().GetFuture();
    }

    SHAREABLE_PROMISE(TFutureType, promise);

    RunNextSequenceAnyFunction(functions, 0, completion_member, promise);

    return promise->GetFuture();
  }

private:
  // ---------------------------------------------------------------------------
  template<typename TFutureType>
  static void RunNextSequenceAllFunction(
    const TSharedRef<TArray<TSequenceFunction<TFutureType>>> functions,
    uint32_t func_index,
    bool TFutureType::* success_member,
    TSharedRef<TPromise<TFutureType>> completion_promise) {

    if (IsIndexOutOfBounds(functions, func_index)) {
      completion_promise->SetValue(TFutureType());
      return;
    }

    auto current_function = (*functions)[func_index];
    current_function()
      .Next([functions,
             func_index,
             success_member,
             completion_promise](TFutureType result) {

     bool has_error = result.*success_member;

      if (has_error) {
        completion_promise->SetValue(result);
        return;
      }

      RunNextSequenceAllFunction(functions,
                                 func_index + 1,
                                 success_member,
                                 completion_promise);
    });
  }

  // ---------------------------------------------------------------------------
  template<typename TFutureType>
  static void RunNextSequenceAnyFunction(
    const TSharedRef<TArray<TSequenceFunction<TFutureType>>> functions,
    uint32_t func_index,
    bool TFutureType::* success_member,
    TSharedRef<TPromise<TFutureType>> completion_promise) {

    check(!IsIndexOutOfBounds(functions, func_index));

    auto current_function = (*functions)[func_index];
    current_function()
      .Next([functions,
             func_index,
             success_member,
             completion_promise](TFutureType result) {

     bool has_error = result.*success_member;

      if (!has_error) {
        completion_promise->SetValue(result);
        return;
      }

      if (IsLastIndex(functions, func_index)) {
        // We have an error - if this is the last item return the error
        completion_promise->SetValue(result);
        return;
      }

      RunNextSequenceAnyFunction(functions,
                                 func_index + 1,
                                 success_member,
                                 completion_promise);
    });
  }

  // ---------------------------------------------------------------------------
  template<typename TFutureType>
  static bool IsLastIndex(
    const TSharedRef<TArray<TSequenceFunction<TFutureType>>> functions,
    uint32_t index) {

    return IsIndexOutOfBounds(functions, index + 1);
  }

  // ---------------------------------------------------------------------------
  template<typename TFutureType>
  static bool IsIndexOutOfBounds(
    const TSharedRef<TArray<TSequenceFunction<TFutureType>>> functions,
    uint32_t index) {

    return ((uint32_t)(functions->Num()) <= index);
  }
};

} // namespace cfcore