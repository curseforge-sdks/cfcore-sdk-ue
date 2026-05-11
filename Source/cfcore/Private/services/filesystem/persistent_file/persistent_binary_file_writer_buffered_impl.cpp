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
#include "persistent_binary_file_writer_buffered_impl.h"
#include <services/io_throttling/io_throttling_service.h>

#include <HAL/FileManager.h>

namespace cfcore {
}

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentBinaryFileWriterBufferedImpl::PersistentBinaryFileWriterBufferedImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service) :
  io_throttling_service_(io_throttling_service),
  current_write_buffer_index_(0) {

  stats_ = { 0 };
}

// -----------------------------------------------------------------------------
PersistentBinaryFileWriterBufferedImpl::
~PersistentBinaryFileWriterBufferedImpl() {
  CloseFile();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileWriterBufferedImpl::Initialize(
  TSharedRef<IPersistentBinaryFileWriter> decorated,
  struct BufferedFileOptions& options) {

  decorated_writer_ = decorated;
  write_buffer_size_in_bytes_ =
    options.buffer_page_size_in_bytes * options.max_buffer_pages;
  current_write_buffer_index_ = 0;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterBufferedImpl::Open(const FString & filename) {
  if (!decorated_writer_.IsValid()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterBufferedImpl::IsValid() const {
  return decorated_writer_.IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterBufferedImpl::Write(const uint8* data,
                                                   int64 length_in_bytes) {

  if (!IsValid()) {
    return false;
  }

  auto start_full = FDateTime::UtcNow();

  if (stats_.max_bytes_to_write < (uint64)length_in_bytes) {
    stats_.max_bytes_to_write = length_in_bytes;
  }

  if (write_buffer_.Num() == 0) {
    AllocateWriteBuffer();
  }

  int64 left_over_size = length_in_bytes;

  while (left_over_size > 0) {
    int64 buf_free_size =
      write_buffer_size_in_bytes_ - current_write_buffer_index_;
    int64 write_size = FMath::Min(buf_free_size, left_over_size);
    left_over_size -= write_size;

    uint8_t* raw_buffer = write_buffer_.GetData() + current_write_buffer_index_;
    FMemory::Memcpy(raw_buffer, data, write_size);
    current_write_buffer_index_ += write_size;

    if (current_write_buffer_index_ >= write_buffer_size_in_bytes_) {
      if (!WriteBufferToDisk()) {
        return false;
      }
    }

    data += write_size;
  }

  auto end_full = FDateTime::UtcNow();
  auto timing = end_full - start_full;
  stats_.seconds_full_write += timing.GetTotalSeconds();

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterBufferedImpl::Write(int64 offset,
  const uint8* data,
  int64 length_in_bytes) {

  // NOTE(twolf): Consider a better implementation for this
  return decorated_writer_->Write(offset, data, length_in_bytes);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterBufferedImpl::Flush() {
  WriteBufferToDisk();
  return decorated_writer_->Flush();
}

// -----------------------------------------------------------------------------
int64 PersistentBinaryFileWriterBufferedImpl::TotalSizeInBytes() const {
  return decorated_writer_->TotalSizeInBytes();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileWriterBufferedImpl::CloseFile() {
  decorated_writer_.Reset();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileWriterBufferedImpl::AllocateWriteBuffer() {
  write_buffer_.AddUninitialized(write_buffer_size_in_bytes_);
  current_write_buffer_index_ = 0;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterBufferedImpl::WriteBufferToDisk() {
  if (current_write_buffer_index_ == 0) {
    return true;
  }

  auto start = FDateTime::UtcNow();

  bool success = decorated_writer_->Write(write_buffer_.GetData(),
                                          current_write_buffer_index_);
  current_write_buffer_index_ = 0;

  auto end = FDateTime::UtcNow();
  auto timing = end - start;
  stats_.seconds_write_to_disk += timing.GetTotalSeconds();

  return success;
}