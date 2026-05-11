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
#include "persistent_binary_file_reader.h"

namespace cfcore {
class IIOThrottlingService;

// -----------------------------------------------------------------------------
class PersistentBinaryFileReaderImpl :
  public IPersistentBinaryFileInternal<IPersistentBinaryFileReader>,
  public IPersistentBinaryFileReader,
  public TSharedFromThis<PersistentBinaryFileReaderImpl> {

public:
  PersistentBinaryFileReaderImpl(
    TSharedPtr<IIOThrottlingService> io_throttling_service);
  virtual ~PersistentBinaryFileReaderImpl();

// IPersistentBinaryFileReaderInternal
public:
  virtual bool Open(const FString& filename) override;
  virtual TSharedRef<IPersistentBinaryFileReader> AsType() override {
    return AsShared();
  }

// IPersistentBinaryFileReader
public:
  virtual bool IsValid() const override;
  virtual bool Read(int64 file_offset_in_bytes,
                    int64 bytes_to_read,
                    TArrayView64<uint8>& out_buffer) override;
  virtual int64 TotalSizeInBytes() const override;

private:
  void CloseFile();

private:
  TSharedPtr<IIOThrottlingService> io_throttling_service_;
  TUniquePtr<IFileHandle> file_handle_;
};

} // namespace cfcore