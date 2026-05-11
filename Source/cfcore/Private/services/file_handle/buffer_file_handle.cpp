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
#include "buffer_file_handle.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
BufferFileHandle::BufferFileHandle(TSharedPtr<IBuffer> buffer) :
  buffer_(buffer),
  current_pos_(0) {
}

// -----------------------------------------------------------------------------
BufferFileHandle::~BufferFileHandle() {
}

// -----------------------------------------------------------------------------
int64 BufferFileHandle::Tell() {
  return current_pos_;
}

// -----------------------------------------------------------------------------
bool BufferFileHandle::Seek(int64 new_position) {
  if (new_position < 0 || new_position > buffer_->GetLength())
    return false;

  current_pos_ = new_position;
  return true;
}

// -----------------------------------------------------------------------------
bool BufferFileHandle::SeekFromEnd(int64 new_pos_relative_to_end/*= 0*/) {
  if (new_pos_relative_to_end < 0 ||
    new_pos_relative_to_end > buffer_->GetLength())
    return false;

  current_pos_ = buffer_->GetLength() - new_pos_relative_to_end;
  return true;
}

// -----------------------------------------------------------------------------
bool BufferFileHandle::Read(uint8* destination, int64 bytes_to_read) {
  if (bytes_to_read <= 0 ||
      (current_pos_ + bytes_to_read) > buffer_->GetLength())
    return false;

  const uint8* data = buffer_->GetData();
  FMemory::Memcpy(destination, &data[current_pos_], bytes_to_read);
  current_pos_ += bytes_to_read;
  return true;
}

// -----------------------------------------------------------------------------
bool BufferFileHandle::Write(const uint8* source, int64 bytes_to_write) {
  /// not support
  return false;
}

// -----------------------------------------------------------------------------
bool BufferFileHandle::Flush(const bool should_full_flush/*=false*/) {
  /// not support
  return false;
}

// -----------------------------------------------------------------------------
bool BufferFileHandle::Truncate(int64 new_size) {
  /// not support
  return false;
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
// -----------------------------------------------------------------------------
bool BufferFileHandle::ReadAt(uint8* destination,
  int64 bytes_to_read,
  int64 offset) {

  if (bytes_to_read <= 0) {
    return false;
  }

  // TODO(twolf): Assure we shouldn't update the current position
  //if (!Seek(offset)) {
  //  return false;
  //}

  const uint8* data = buffer_->GetData();
  FMemory::Memcpy(destination, &data[offset], bytes_to_read);
  current_pos_ += bytes_to_read;
  return true;
}
#endif
