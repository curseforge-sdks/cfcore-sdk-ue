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

namespace cfcore {

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;    \
  void operator=(const TypeName&) = delete; \

// Disallow copying a type, and only provide move construction and move
// assignment. Especially useful for move-only structs.
#define MOVE_ONLY_NO_DEFAULT_CONSTRUCTOR(TypeName)       \
  TypeName(TypeName&&) = default;            \
  TypeName& operator=(TypeName&&) = default;

#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete; \
  MOVE_ONLY_NO_DEFAULT_CONSTRUCTOR(TypeName); \
  DISALLOW_COPY_AND_ASSIGN(TypeName);

#define SHAREABLE_PROMISE(ResultType, variable) \
  auto variable = MakeShared<TPromise<ResultType>>();

// Sets the promise's value on the game thread - so any future.Next call will
// run in the context of the game thread.
//
// NOTE(twolf): Because we are capturing parameters, the value MUST be an
// l-value.
#define SET_SHAREABLE_PROMISE_VALUE_ON_GAME_THREAD(promise, lvalue) \
  AsyncTask(ENamedThreads::GameThread, [promise, lvalue]() { \
    promise->SetValue(lvalue); \
  });

// NOTE(twolf): We assure a string copy here, so the resulting string variable
// won't have any dependencies on temporary variables - might be a solution to
// some crashes we see in production.
#define ENUM_TO_STR(str_variable, enum_val) \
  FString str_variable; \
  { \
    FString temp = UEnum::GetDisplayValueAsText(enum_val).ToString(); \
    str_variable = temp.Replace(TEXT(" "), TEXT("")); \
  }

} // namespace cfcore
