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
#include <Misc/EngineVersionComparison.h>

#include <services/buffer/buffer.h>

namespace cfcore {

// ---------------------------------------------------------------------------
class BufferFileHandle : public IFileHandle {
public:
  BufferFileHandle(TSharedPtr<IBuffer> buffer);
  virtual ~BufferFileHandle();

public:
  virtual int64 Tell() override;
  virtual bool Seek(int64 new_position) override;
  virtual bool SeekFromEnd(int64 new_pos_relative_to_end = 0) override;
  virtual bool Read(uint8* destination, int64 bytes_to_read) override;
  virtual bool Write(const uint8* source, int64 bytes_to_write) override;
  virtual bool Flush(const bool should_full_flush = false) override;
  virtual bool Truncate(int64 new_size) override;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
  virtual bool ReadAt(uint8* destination,
                      int64 bytes_to_read,
                      int64 offset) override;
#endif

private:
  TSharedPtr<IBuffer> buffer_;
  int64 current_pos_;
};

} // namespace cfcore