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
#include "persistent_binary_file_buffered_impl.h"

namespace cfcore {
// NOTE(twolf): This is currently optimized for bsdiff - reading from the old
// file, when large, can be very slow due to random access.
// TODO(twolf): Consider passing these sizes as parameters to the
// OpenRead/OpenWrite functions.
constexpr uint64 kPersistentBufferReadSizeInBytes = 1024 * 1024 * 1024;
constexpr uint64 kPersistentBufferWriteSizeInBytes = 100 * 1024 * 1024;
}

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentBinaryFileBufferedImpl::PersistentBinaryFileBufferedImpl(
  TSharedPtr<IPersistentBinaryFile> persistent_binary_file) :
  persistent_binary_file_(persistent_binary_file),
  current_write_buffer_index_(0),
  current_read_file_offset_(-1) {
}

// -----------------------------------------------------------------------------
PersistentBinaryFileBufferedImpl::~PersistentBinaryFileBufferedImpl() {
  Flush();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::OpenRead(const FString& filename) {
  Initialize();
  return persistent_binary_file_->OpenRead(filename);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::OpenWrite(const FString& filename) {
  Initialize();
  return persistent_binary_file_->OpenWrite(filename);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::OpenReadWrite(const FString& filename) {
  Initialize();
  return persistent_binary_file_->OpenReadWrite(filename);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::IsValid() const {
  return persistent_binary_file_->IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::Write(const uint8* data,
                                             int64 length_in_bytes) {
  if (!IsValid()) {
    return false;
  }

  if (write_buffer_.Num() == 0) {
    AllocateWriteBuffer();
  }

  int64 left_over_size = length_in_bytes;

  while (left_over_size > 0) {
    int64 buf_free_size =
      kPersistentBufferWriteSizeInBytes - current_write_buffer_index_;
    int64 write_size = FMath::Min(buf_free_size, left_over_size);
    left_over_size -= write_size;

    uint8_t* raw_buffer = write_buffer_.GetData() + current_write_buffer_index_;
    FMemory::Memcpy(raw_buffer, data, write_size);
    current_write_buffer_index_ += write_size;

    if (current_write_buffer_index_ >= kPersistentBufferWriteSizeInBytes) {
      WriteBufferToDisk();
    }

    data += write_size;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::Flush() {
  WriteBufferToDisk();
  return persistent_binary_file_->Flush();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::Read(int64 file_offset_in_bytes,
                                            int64 bytes_to_read,
                                            TArrayView64<uint8>& buffer) {

  if (bytes_to_read >= kPersistentBufferReadSizeInBytes) {
    return persistent_binary_file_->Read(file_offset_in_bytes,
                                         bytes_to_read,
                                         buffer);
  }

  if (CanReadFromBuffer(file_offset_in_bytes, bytes_to_read)) {
    int64 in_mem_offset = file_offset_in_bytes - current_read_file_offset_;
    FMemory::Memcpy(buffer.GetData(),
                    read_buffer_.GetData() + in_mem_offset,
                    bytes_to_read);
    return true;
  }

  if (!FillReadBuffer(file_offset_in_bytes)) {
    return false;
  }

  FMemory::Memcpy(buffer.GetData(),
                  read_buffer_.GetData(),
                  bytes_to_read);

  return true;
}

// -----------------------------------------------------------------------------
int64 PersistentBinaryFileBufferedImpl::TotalSizeInBytes() const {
  return persistent_binary_file_->TotalSizeInBytes();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileBufferedImpl::Initialize() {
  write_buffer_.Reset(0);
  current_write_buffer_index_ = 0;

  read_buffer_.Reset(0);
  current_read_file_offset_ = -1;
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileBufferedImpl::AllocateWriteBuffer() {
  write_buffer_.AddUninitialized(kPersistentBufferWriteSizeInBytes);
  current_write_buffer_index_ = 0;
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileBufferedImpl::WriteBufferToDisk() {
  if (current_write_buffer_index_ == 0) {
    return;
  }

  persistent_binary_file_->Write(write_buffer_.GetData(),
                                 current_write_buffer_index_);
  current_write_buffer_index_ = 0;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::CanReadFromBuffer(
  int64 file_offset_in_bytes,
  int64 bytes_to_read) {

  if (current_read_file_offset_ == -1) {
    return false;
  }

  if (file_offset_in_bytes < current_read_file_offset_) {
    return false;
  }

  if ((file_offset_in_bytes + bytes_to_read) >
    (current_read_file_offset_ + read_buffer_.Num())) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileBufferedImpl::FillReadBuffer(
  int64 file_offset_in_bytes) {

  if (read_buffer_.Num() == 0) {
    AllocateReadBuffer();
  }

  int64 max_bytes_to_read =
    persistent_binary_file_->TotalSizeInBytes() - file_offset_in_bytes;

  int64 in_mem_bytes_to_read = FMath::Min(kPersistentBufferReadSizeInBytes,
                                          (uint64)max_bytes_to_read);

  TArrayView64<uint8> view(read_buffer_.GetData(), read_buffer_.Num());
  if (!persistent_binary_file_->Read(file_offset_in_bytes,
                                     in_mem_bytes_to_read,
                                     view)) {
    return false;
  }

  current_read_file_offset_ = file_offset_in_bytes;
  return true;
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileBufferedImpl::AllocateReadBuffer() {
  read_buffer_.AddUninitialized(kPersistentBufferReadSizeInBytes);
  current_read_file_offset_ = -1;
}
