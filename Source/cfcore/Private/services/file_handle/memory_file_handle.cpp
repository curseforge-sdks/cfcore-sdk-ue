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
#include "memory_file_handle.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
MemoryFileHandle::MemoryFileHandle(int64 allocated_size) : current_pos_(0) {
  data_.SetNumUninitialized(allocated_size);
}

// -----------------------------------------------------------------------------
MemoryFileHandle::~MemoryFileHandle() {
}

// -----------------------------------------------------------------------------
int64 MemoryFileHandle::Tell() {
  return current_pos_;
}

// -----------------------------------------------------------------------------
bool MemoryFileHandle::Seek(int64 new_position) {
  if (new_position < 0 || new_position > data_.Num())
    return false;

  current_pos_ = new_position;
  return true;
}

// -----------------------------------------------------------------------------
bool MemoryFileHandle::SeekFromEnd(int64 new_pos_relative_to_end/*= 0*/) {
  if (new_pos_relative_to_end < 0 ||
    new_pos_relative_to_end > data_.Num())
    return false;

  current_pos_ = data_.Num() - new_pos_relative_to_end;
  return true;
}

// -----------------------------------------------------------------------------
bool MemoryFileHandle::Read(uint8* destination, int64 bytes_to_read) {
  if (bytes_to_read <= 0 || (current_pos_ + bytes_to_read) > data_.Num())
    return false;

  FMemory::Memcpy(destination, &data_[current_pos_], bytes_to_read);
  current_pos_ += bytes_to_read;
  return true;
}

// -----------------------------------------------------------------------------
bool MemoryFileHandle::Write(const uint8* source, int64 bytes_to_write) {
  /// not support
  return false;
}

// -----------------------------------------------------------------------------
bool MemoryFileHandle::Flush(const bool should_full_flush/*=false*/) {
  /// not support
  return false;
}

// -----------------------------------------------------------------------------
bool MemoryFileHandle::Truncate(int64 new_size) {
  /// not support
  return false;
}