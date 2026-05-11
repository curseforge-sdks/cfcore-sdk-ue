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
#include "persistent_binary_file_reader_impl.h"
#include <services/io_throttling/io_throttling_service.h>

#include <HAL/FileManager.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentBinaryFileReaderImpl::PersistentBinaryFileReaderImpl(
  TSharedPtr<IIOThrottlingService> io_throttling_service) :
  io_throttling_service_(io_throttling_service) {

}

// -----------------------------------------------------------------------------
PersistentBinaryFileReaderImpl::~PersistentBinaryFileReaderImpl() {
  CloseFile();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderImpl::Open(const FString & filename) {
  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  file_handle_.Reset(platfrm_file.OpenRead(*filename));
  return IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderImpl::IsValid() const {
  if (!file_handle_.IsValid()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
bool PersistentBinaryFileReaderImpl::Read(int64 file_offset_in_bytes,
                                          int64 bytes_to_read,
                                          TArrayView64<uint8>& out_buffer) {

  if (!IsValid()) {
    return false;
  }

  if (!file_handle_->Seek(file_offset_in_bytes)) {
    // TODO(twolf): add warning log
    return false;
  }

  bool success = file_handle_->Read(out_buffer.GetData(), bytes_to_read);
  return success;
}

// -----------------------------------------------------------------------------
int64 PersistentBinaryFileReaderImpl::TotalSizeInBytes() const {
  if (!IsValid()) {
    return -1;
  }

  return file_handle_->Size();
}

// -----------------------------------------------------------------------------
void PersistentBinaryFileReaderImpl::CloseFile() {
  file_handle_.Reset();
}
