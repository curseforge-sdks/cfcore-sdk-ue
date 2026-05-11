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
#include "persistent_binary_file_impl.h"
#include <services/io_throttling/io_throttling_service.h>

#include <HAL/FileManager.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentBinaryFileImpl::PersistentBinaryFileImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service) :
  io_throttling_service_(io_throttling_service) {

}

// -----------------------------------------------------------------------------
PersistentBinaryFileImpl::~PersistentBinaryFileImpl() {
  CloseFile();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::OpenRead(const FString& filename) {
  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  file_handle_.Reset(platfrm_file.OpenRead(*filename));
  return IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::OpenWrite(const FString & filename) {
  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  file_handle_.Reset(platfrm_file.OpenWrite(*filename, true));
  return IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::OpenReadWrite(const FString& filename) {
  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  file_handle_.Reset(platfrm_file.OpenWrite(*filename, true, true));
  return IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::IsValid() const {
  if (!file_handle_.IsValid()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::Write(const uint8* data, int64 length_in_bytes) {
  if (!IsValid()) {
    return false;
  }

  return WriteBufferWithThrottling(data, length_in_bytes);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::Flush() {
  if (!IsValid()) {
    return false;
  }

  return file_handle_->Flush(true);
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::Read(int64 file_offset_in_bytes,
                                    int64 bytes_to_read,
                                    TArrayView64<uint8>& buffer) {

  if (!IsValid()) {
    return false;
  }

  if (!file_handle_->Seek(file_offset_in_bytes)) {
    // TODO(twolf): add warning log
    return false;
  }

  bool success = file_handle_->Read(buffer.GetData(), bytes_to_read);
  return success;
}

// -----------------------------------------------------------------------------
int64 PersistentBinaryFileImpl::TotalSizeInBytes() const {
  if (!IsValid()) {
    return -1;
  }

  return file_handle_->Size();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileImpl::OpenFile(const FString& filename) {
  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  file_handle_.Reset(platfrm_file.OpenWrite(*filename, true));
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileImpl::CloseFile() {
  Flush();

  file_handle_.Reset();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileImpl::WriteBufferWithThrottling(const uint8* data,
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
