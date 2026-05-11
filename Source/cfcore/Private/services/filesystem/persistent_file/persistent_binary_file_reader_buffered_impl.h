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
class PersistentBinaryFileReaderBufferedImpl :
  public IPersistentBinaryFileDecoratorInternal<IPersistentBinaryFileReader>,
  public IPersistentBinaryFileReader,
  public TSharedFromThis<PersistentBinaryFileReaderBufferedImpl> {

public:
  PersistentBinaryFileReaderBufferedImpl(
    TSharedPtr<IIOThrottlingService> io_throttling_service);
  virtual ~PersistentBinaryFileReaderBufferedImpl();

// IPersistentBinaryFileDecoratorInternal
public:
  virtual void Initialize(TSharedRef<IPersistentBinaryFileReader> decorated,
                          struct BufferedFileOptions& options) override;
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
  void FillBufferFromMemory(int64 bytes_to_read, int64 page_start_index, int64 page_end_index, int64 file_offset_in_bytes, TArrayView64<uint8>& out_buffer);
  virtual int64 TotalSizeInBytes() const override;

private:
  void CloseFile();
  void EvictLruPages(TSet<int16>& pages_to_load);
  bool LoadPagesToMemory(TSet<int16>& pages_to_load);

private:
  TSharedPtr<IIOThrottlingService> io_throttling_service_;
  TSharedPtr<IPersistentBinaryFileReader> decorated_reader_;
  struct BufferedFileOptions options_ = { 0 };

  typedef struct _FMemoryPage {
    bool empty = true;
    FDateTime last_accessed;
    TArray64<uint8> data;
  } FMemoryPage;

  TMap<int16, FMemoryPage> lru_pages_memory_;

  struct FStats {
    uint32 swaps = 0;
    uint32 hits = 0;
    uint32 max_pages_loaded = 0;
    uint64 max_bytes_to_read = 0;
    double seconds_full_reads = 0.0;
    double seconds_evictting = 0.0;
    double seconds_reading_from_disk = 0.0;
    double seconds_filling_buffers = 0.0;
  } stats_;
};

} // namespace cfcore