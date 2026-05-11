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
#include "persistent_log_file_impl.h"
#include <Misc/OutputDeviceArchiveWrapper.h>
#include <HAL/FileManager.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
PersistentLogFileImpl::~PersistentLogFileImpl() {
  CloseFile();
}

// -----------------------------------------------------------------------------
bool PersistentLogFileImpl::Initialize(const FString& filename) {
  OpenFile(filename);
  return IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentLogFileImpl::IsValid() const {
  return archive_.IsValid() && report_.IsValid();
}

// -----------------------------------------------------------------------------
bool PersistentLogFileImpl::WriteLine(const FString& line) {
  if (!IsValid()) {
    return false;
  }

  report_->Log(line);
  report_->Flush();
  return true;
}

// -----------------------------------------------------------------------------
int64 PersistentLogFileImpl::TotalSizeInBytes() const {
  if (!IsValid()) {
    return -1;
  }

  return archive_->TotalSize();
}

// -----------------------------------------------------------------------------
void PersistentLogFileImpl::OpenFile(const FString& filename) {
  archive_ =
    TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*filename));
  report_ = TUniquePtr<FOutputDeviceArchiveWrapper>(
    new FOutputDeviceArchiveWrapper(archive_.Get()));
}

// -----------------------------------------------------------------------------
void PersistentLogFileImpl::CloseFile() {
  if (report_.IsValid()) {
    report_->TearDown();
    report_.Reset();
  }

  if (archive_.IsValid()) {
    archive_->Close();
    archive_.Reset();
  }
}