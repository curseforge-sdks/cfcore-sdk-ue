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
#include "url_search_params.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
void UrlSearchParams::Append(const FString& key, const FString& value) {
  search_params_.Add(key, value);
}

// -----------------------------------------------------------------------------
void UrlSearchParams::Append(const FString& key, int64 value) {
   search_params_.Add(key, FString::Printf(TEXT("%lld"), value));
}

// -----------------------------------------------------------------------------
void UrlSearchParams::Append(const FString& key, int32 value) {
  search_params_.Add(key, FString::FromInt(value));
}

// -----------------------------------------------------------------------------
FString UrlSearchParams::ToString() {
  TStringBuilder<1024> builder;

  int32 counter = search_params_.Num() - 1;

  for (const TPair<FString, FString>& pair : search_params_) {
    builder.Append(FString::Format(TEXT("{0}={1}"), { pair.Key, pair.Value }));

    if (counter > 0) {
      builder.Append(TEXT("&"));
    }

    counter--;
  }

  return builder.ToString();
}
