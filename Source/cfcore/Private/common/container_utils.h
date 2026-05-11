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
#include "Misc/StringBuilder.h"

namespace cfcore {

// -----------------------------------------------------------------------------
class ContainerUtils {
public:
  // ---------------------------------------------------------------------------
  template<typename T>
  static TArray<T> Dedupe(const TArray<T>& array) {
    TSet<T> unique;

    for (const T& val : array) {
      unique.Add(val);
    }

    return unique.Array();
  }

  // ---------------------------------------------------------------------------
  template<typename T>
  static TArray<T> Remove(const TArray<T>& base, const TArray<T>& removals) {
    TSet<T> lookup;
    for (const T& val : removals) {
      lookup.Add(val);
    }

    TSet<T> unique;
    for (const T& val : base) {
      if (!lookup.Contains(val)) {
        unique.Add(val);
      }
    }

    return unique.Array();
  }

// ---------------------------------------------------------------------------
  template<typename T>
  static void Join(const TArray<T>& array, OUT FString& ouput) {
    TStringBuilder<1024> builder;

    for (int i = 0; i < array.Num(); ++i) {
      builder << array[i];
      if (i + 1 < array.Num()) {
        builder << ", ";
      }
    }

    ouput = builder;
  }

  // ---------------------------------------------------------------------------
  static int32 TCHARToChar(const FString& str,
                           OUT TArray<uint8>& out,
                           int32 max_len) {
    int32 num_bytes = 0;
    const TCHAR* char_pos = *str;

    while (*char_pos && num_bytes < max_len)
    {
      out[num_bytes] = (uint8)(*char_pos);
      char_pos++;
      ++num_bytes;
    }

    return num_bytes;
  }

  // ---------------------------------------------------------------------------
  template<typename T>
  static TSet<T> AsSet(const TArray<T>& array) {
    TSet<T> unique;

    for (const T& val : array) {
      unique.Add(val);
    }

    return unique;
  }

  // ---------------------------------------------------------------------------
  template<typename K, typename V>
  static TSet<V> AsSet(const TMap<K, V>& map) {
    TSet<V> unique;

    for (const auto& iter : map) {
      unique.Add(iter.Value);
    }

    return unique;
  }

  // ---------------------------------------------------------------------------
  // Usage:
  //   auto as_map = ContainerUtils::AsMap(arr, &FInstalledMods::id);
  template<typename T, typename KeyType>
  static TMap<KeyType, T> AsMap(const TArray<T>& array, KeyType T::* member) {
    TMap<KeyType, T> result;

    for (const auto& item : array) {
      KeyType id = item.*member;
      result.Add(id, item);
    }

    return MoveTemp(result);
  }

  // ---------------------------------------------------------------------------
  template<typename T>
  static void Shuffle(TArray<T>& array) {
    int32 last_index = array.Num() - 1;

    for (int32 i = 0; i <= last_index; ++i) {
      int32 swap_index = FMath::RandRange(i, last_index);
      if (i != swap_index) {
        array.Swap(i, swap_index);
      }
    }
  }
};

} // namespace cfcore