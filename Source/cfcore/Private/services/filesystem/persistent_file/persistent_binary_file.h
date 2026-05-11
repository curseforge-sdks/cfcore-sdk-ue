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

namespace cfcore {

// -----------------------------------------------------------------------------
// This interface should be used only on non-game threads!
class IPersistentBinaryFile {
public:
  virtual ~IPersistentBinaryFile() = default;

public:
  virtual bool OpenRead(const FString& filename) = 0;
  virtual bool OpenWrite(const FString& filename) = 0;
  virtual bool OpenReadWrite(const FString& filename) = 0;

  /**
   * Can the persistent file be used?
   */
  virtual bool IsValid() const = 0;

  /**
   */
  virtual bool Write(const uint8* data, int64 length_in_bytes) = 0;

  /**
   * Shouldn't be used often - only if you know what you are doing.
   */
  virtual bool Flush() = 0;

  /**
   */
  virtual bool Read(int64 file_offset_in_bytes,
                    int64 bytes_to_read,
                    TArrayView64<uint8>& buffer) = 0;

  /**
   * Get the total size (in bytes) of the file
   */
  virtual int64 TotalSizeInBytes() const = 0;
};

} // namespace cfcore

Expose_TNameOf(cfcore::IPersistentBinaryFile)