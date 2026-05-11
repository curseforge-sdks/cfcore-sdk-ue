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
#include <CoreMinimal.h>
#include "persistent_binary_file_internal.h"
#include "persistent_binary_file_writer.h"

namespace cfcore {
class IIOThrottlingService;

// -----------------------------------------------------------------------------
class PersistentBinaryFileWriterBufferedImpl :
  public IPersistentBinaryFileDecoratorInternal<IPersistentBinaryFileWriter>,
  public IPersistentBinaryFileWriter,
  public TSharedFromThis<PersistentBinaryFileWriterBufferedImpl> {

public:
  PersistentBinaryFileWriterBufferedImpl(
    TSharedPtr<IIOThrottlingService> io_throttling_service);
  virtual ~PersistentBinaryFileWriterBufferedImpl();

// IPersistentBinaryFileDecoratorInternal
public:
  virtual void Initialize(TSharedRef<IPersistentBinaryFileWriter> decorated,
                          struct BufferedFileOptions& options) override;
  virtual bool Open(const FString& filename) override;

  virtual TSharedRef<IPersistentBinaryFileWriter> AsType() override {
    return AsShared();
  }

// IPersistentBinaryFileReader
public:
  virtual bool IsValid() const override;
  virtual bool Write(const uint8* data, int64 length_in_bytes) override;
  virtual bool Write(int64 offset,
                     const uint8* data,
                     int64 length_in_bytes) override;
  virtual bool Flush() override;
  virtual int64 TotalSizeInBytes() const override;

private:
  void CloseFile();
  void AllocateWriteBuffer();
  bool WriteBufferToDisk();

private:
  TSharedPtr<IIOThrottlingService> io_throttling_service_;
  TSharedPtr<IPersistentBinaryFileWriter> decorated_writer_;

  TArray64<uint8> write_buffer_;
  uint64 current_write_buffer_index_;
  uint64 write_buffer_size_in_bytes_ = 0;

  struct FStats {
    uint64 max_bytes_to_write = 0;
    double seconds_full_write = 0.0;
    double seconds_write_to_disk = 0.0;
  } stats_;
};

} // namespace cfcore