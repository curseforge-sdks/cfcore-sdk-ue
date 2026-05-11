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
#include "persistent_binary_file_reader_buffered_impl.h"
#include <services/io_throttling/io_throttling_service.h>

#include <HAL/FileManager.h>

namespace cfcore {
}

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentBinaryFileReaderBufferedImpl::PersistentBinaryFileReaderBufferedImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service) :
  io_throttling_service_(io_throttling_service) {
}

// -----------------------------------------------------------------------------
PersistentBinaryFileReaderBufferedImpl::
~PersistentBinaryFileReaderBufferedImpl() {
  CloseFile();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileReaderBufferedImpl::Initialize(
  TSharedRef<IPersistentBinaryFileReader> decorated,
  struct BufferedFileOptions& options) {

  decorated_reader_ = decorated;
  options_ = options;

  stats_ = { 0 };
  lru_pages_memory_.Reset();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderBufferedImpl::Open(const FString & filename) {
  // Initialize();
  if (!decorated_reader_.IsValid()) {
    return false;
  }

  stats_ = {0};
  lru_pages_memory_.Reset();

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderBufferedImpl::IsValid() const {
  return decorated_reader_.IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderBufferedImpl::Read(
  int64 file_offset_in_bytes,
  int64 bytes_to_read,
  TArrayView64<uint8>& out_buffer) {

  if (bytes_to_read <= 0) {
    return true;
  }

  auto start_full = FDateTime::UtcNow();

  if (stats_.max_bytes_to_read < (uint64)bytes_to_read) {
    stats_.max_bytes_to_read = bytes_to_read;
  }

  // Get page indices from offset and size
  int64 page_start_index =
    file_offset_in_bytes / options_.buffer_page_size_in_bytes;

  int64 page_end_index =
    (file_offset_in_bytes + bytes_to_read - 1) / options_.buffer_page_size_in_bytes;

  TSet<int16> pages_to_load;
  auto now = FDateTime::UtcNow();
  for (int64 i = page_start_index; i <= page_end_index; ++i) {
    if (!lru_pages_memory_.Contains(i)) {
      pages_to_load.Add(i);
    } else {
      stats_.hits++;
      lru_pages_memory_[i].last_accessed = now;
    }
  }

  if (stats_.max_pages_loaded < (uint32)pages_to_load.Num()) {
    stats_.max_pages_loaded = pages_to_load.Num();
  }

  {
    auto start = FDateTime::UtcNow();

    EvictLruPages(pages_to_load);

    auto end = FDateTime::UtcNow();
    auto timing = end - start;
    stats_.seconds_evictting += timing.GetTotalSeconds();
  }

  {
    auto start = FDateTime::UtcNow();

    if (!LoadPagesToMemory(pages_to_load)) {
      return false;
    }

    auto end = FDateTime::UtcNow();
    auto timing = end - start;
    stats_.seconds_reading_from_disk += timing.GetTotalSeconds();
  }


  {
    auto start = FDateTime::UtcNow();

    FillBufferFromMemory(bytes_to_read,
                         page_start_index,
                         page_end_index,
                         file_offset_in_bytes,
                         out_buffer);

    auto end = FDateTime::UtcNow();
    auto timing = end - start;
    stats_.seconds_filling_buffers += timing.GetTotalSeconds();
  }

  auto end_full = FDateTime::UtcNow();
  auto timing = end_full - start_full;
  stats_.seconds_full_reads += timing.GetTotalSeconds();
  return true;
}

// -----------------------------------------------------------------------------
int64 PersistentBinaryFileReaderBufferedImpl::TotalSizeInBytes() const {
  return decorated_reader_->TotalSizeInBytes();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileReaderBufferedImpl::CloseFile() {
  decorated_reader_.Reset();
}


// -----------------------------------------------------------------------------
void PersistentBinaryFileReaderBufferedImpl::EvictLruPages(
  TSet<int16>& pages_to_load) {

  int32 pages_to_evict =
    (lru_pages_memory_.Num() + pages_to_load.Num()) - options_.max_buffer_pages;

  if (pages_to_evict <= 0) {
    return;
  }

  for (int32 i = 0; i < pages_to_evict; ++i) {
    int16 oldest_page = -1;
    FDateTime oldest_time = FDateTime::MaxValue();

    for (const auto& pair : lru_pages_memory_) {
      if (pair.Value.last_accessed < oldest_time) {
        oldest_time = pair.Value.last_accessed;
        oldest_page = pair.Key;
      }
    }

    if (oldest_page != -1) {
      lru_pages_memory_.Remove(oldest_page);
      stats_.swaps++;
    }
  }
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderBufferedImpl::LoadPagesToMemory(
  TSet<int16>& pages_to_load) {

  // Load the required pages
  for (int16 page_index : pages_to_load) {
    FMemoryPage& page = lru_pages_memory_.Add(page_index);
    page.data.AddUninitialized(options_.buffer_page_size_in_bytes);
    page.empty = false;
    page.last_accessed = FDateTime::UtcNow();
    int64 offset = page_index * options_.buffer_page_size_in_bytes;

    int64 total_size = decorated_reader_->TotalSizeInBytes();
    int64 bytes_to_read_for_page =
      FMath::Min(options_.buffer_page_size_in_bytes,
                 (uint64)(total_size - offset));

    TArrayView64<uint8> view(page.data.GetData(), page.data.Num());
    if (!decorated_reader_->Read(offset, bytes_to_read_for_page, view)) {
      return false;
    }
  }

  return true;
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileReaderBufferedImpl::FillBufferFromMemory(
  int64 bytes_to_read,
  int64 page_start_index,
  int64 page_end_index,
  int64 file_offset_in_bytes,
  TArrayView64<uint8>& out_buffer) {

  int64 bytes_left = bytes_to_read;
  bool read_first = false;

  for (int64 i = page_start_index; i <= page_end_index; ++i) {
    FMemoryPage& page = lru_pages_memory_[i];
    page.last_accessed = FDateTime::UtcNow();
    int64 page_offset = 0;

    if (!read_first) {
      page_offset =
        file_offset_in_bytes - (i * options_.buffer_page_size_in_bytes);
      read_first = true;
    }

    int64 bytes_to_copy = FMath::Min(
      options_.buffer_page_size_in_bytes - page_offset,
      (uint64)bytes_left);

    if (bytes_to_copy <= 0) {
      continue; // No data to copy from this page
    }

    FMemory::Memcpy(out_buffer.GetData() + (bytes_to_read - bytes_left),
      page.data.GetData() + page_offset,
      bytes_to_copy);

    bytes_left -= bytes_to_copy;
  }
}
