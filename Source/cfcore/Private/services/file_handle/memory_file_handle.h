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
#include <GenericPlatform/GenericPlatformFile.h>

namespace cfcore {

// ---------------------------------------------------------------------------
class MemoryFileHandle : public IFileHandle {
public:
  MemoryFileHandle(int64 allocated_size);
  virtual ~MemoryFileHandle();

public:
  const uint8* GetData() const {
    return data_.GetData();
  }

// IFileHandle
public:
  virtual int64 Tell() override;
  virtual bool Seek(int64 new_position) override;
  virtual bool SeekFromEnd(int64 new_pos_relative_to_end = 0) override;
  virtual bool Read(uint8* destination, int64 bytes_to_read) override;
  virtual bool Write(const uint8* source, int64 bytes_to_write) override;
  virtual bool Flush(const bool should_full_flush = false) override;
  virtual bool Truncate(int64 new_size) override;

private:
  TArray64<uint8> data_;
  int64 current_pos_;
};

} // namespace cfcore