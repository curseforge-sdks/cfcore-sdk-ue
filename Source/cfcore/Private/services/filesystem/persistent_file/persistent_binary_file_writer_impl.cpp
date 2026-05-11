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
#include "persistent_binary_file_writer_impl.h"
#include <services/io_throttling/io_throttling_service.h>

#include <HAL/FileManager.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentBinaryFileWriterImpl::PersistentBinaryFileWriterImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service) :
  io_throttling_service_(io_throttling_service) {
}

// -----------------------------------------------------------------------------
PersistentBinaryFileWriterImpl::~PersistentBinaryFileWriterImpl() {
  CloseFile();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterImpl::Open(const FString & filename) {
  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  file_handle_.Reset(platfrm_file.OpenWrite(*filename, true));
  return IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterImpl::IsValid() const {
  if (!file_handle_.IsValid()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterImpl::Write(const uint8* data,
                                           int64 length_in_bytes) {

  if (!IsValid()) {
    return false;
  }

  return WriteBufferWithThrottling(data, length_in_bytes);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterImpl::Write(int64 offset,
                                           const uint8* data,
                                           int64 length_in_bytes) {

  if (!IsValid()) {
    return false;
  }


  int64 current_pos = file_handle_->Tell();
  file_handle_->Seek(offset);
  bool success = WriteBufferWithThrottling(data, length_in_bytes);
  file_handle_->Seek(current_pos);

  return success;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterImpl::Flush() {
  if (!IsValid()) {
    return false;
  }

  return file_handle_->Flush(true);
}

// -----------------------------------------------------------------------------
int64 PersistentBinaryFileWriterImpl::TotalSizeInBytes() const {
  if (!IsValid()) {
    return -1;
  }

  return file_handle_->Size();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileWriterImpl::CloseFile() {
  Flush();

  file_handle_.Reset();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileWriterImpl::WriteBufferWithThrottling(
  const uint8* data,
  int64 length) {

  int64 written_bytes = 0;
  int64 left_to_write = length - written_bytes;
  bool success = true;

  while (success && (left_to_write > 0)) {
    int64 bytes_to_write =
      io_throttling_service_->SleepForNextQuotaInBytes(left_to_write);

    success = file_handle_->Write(&data[written_bytes], bytes_to_write);
    if (success) {
      success = file_handle_->Flush();
    }

    written_bytes += bytes_to_write;
    left_to_write = length - written_bytes;
  }

  return success;
}
