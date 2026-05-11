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
#include "persistent_binary_file.h"

namespace cfcore {
class IIOThrottlingService;

// -----------------------------------------------------------------------------
class PersistentBinaryFileImpl : public IPersistentBinaryFile {
public:
  PersistentBinaryFileImpl(
    TSharedPtr<IIOThrottlingService> io_throttling_service);
  virtual ~PersistentBinaryFileImpl();

// IPersistentBinaryFile
public:
  virtual bool OpenRead(const FString& filename) override;
  virtual bool OpenWrite(const FString& filename) override;
  virtual bool OpenReadWrite(const FString& filename) override;

  virtual bool IsValid() const override;
  virtual bool Write(const uint8* data, int64 length_in_bytes) override;
  virtual bool Flush() override;
  virtual bool Read(int64 file_offset_in_bytes,
                    int64 bytes_to_read,
                    TArrayView64<uint8>& buffer) override;
  virtual int64 TotalSizeInBytes() const override;

private:
  void OpenFile(const FString& filename);
  void CloseFile();
  bool WriteBufferWithThrottling(const uint8* data, int64 length);

private:
  TSharedPtr<IIOThrottlingService> io_throttling_service_;
  TUniquePtr<IFileHandle> file_handle_;
};

} // namespace cfcore