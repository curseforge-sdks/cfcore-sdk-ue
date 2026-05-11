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
#include "zip_buffer_handle.h"
#include "zip_platform_file.h"

using namespace cfcore;

// -----------------------------------------------------------------------------
ZipBufferHandle::ZipBufferHandle(
  zipFile zlib_handle,
  TSharedPtr<IFileHandle> file_handle_ref) :
  zlib_handle_(zlib_handle),
  file_handle_ref_(file_handle_ref) {
}


// -----------------------------------------------------------------------------
TSharedPtr<ZipBufferHandle> ZipBufferHandle::FromFile(
  const FString& uncompressed_filename,
  const TCHAR* password/*= nullptr*/) {

  check(!IsInGameThread());

  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  IFileHandle* raw_file_handle_ptr = platfrm_file.OpenWrite(
    *uncompressed_filename,
    false,
    true);

  TSharedPtr<IFileHandle> file_handle = MakeShareable<IFileHandle>(
    raw_file_handle_ptr);

  return ZipBufferHandle::FromFileHandle(file_handle);
}

// -----------------------------------------------------------------------------
TSharedPtr<ZipBufferHandle> ZipBufferHandle::FromFileHandle(
  TSharedPtr<IFileHandle> file_handle) {

  zlib_filefunc64_def zlib_file_fun64_defs;
  zlib_file_fun64_defs.zopen64_file = &ZipPlatformFile::zlib_open64_file;
  zlib_file_fun64_defs.ztell64_file = &ZipPlatformFile::zlib_tell64_file;
  zlib_file_fun64_defs.zseek64_file = &ZipPlatformFile::zlib_seek64_file;

  zlib_file_fun64_defs.zread_file = &ZipPlatformFile::zlib_read_file;
  zlib_file_fun64_defs.zwrite_file = &ZipPlatformFile::zlib_write_file;
  zlib_file_fun64_defs.zclose_file = &ZipPlatformFile::zlib_close_file;
  zlib_file_fun64_defs.zerror_file = &ZipPlatformFile::zlib_testerror_file;

  zlib_file_fun64_defs.opaque = (void*)file_handle.Get();

  zipFile zlib_handle = cfcore_zipOpen2_64(nullptr,
                                           APPEND_STATUS_CREATE,
                                           nullptr,
                                           &zlib_file_fun64_defs);

  TSharedPtr<ZipBufferHandle> handle = MakeShareable(
    new ZipBufferHandle(zlib_handle, file_handle));

  return handle;
}

// -----------------------------------------------------------------------------
bool ZipBufferHandle::OpenNewEntry(const FString& entry_name,
                                        int method,
                                        int level,
                                        bool zip64) {
  // TODO(apliscov): set timestamps here
  zip_fileinfo info{};
  const FTCHARToUTF8 name_utf8(*entry_name);

  int zr = cfcore_zipOpenNewFileInZip64(zlib_handle_,
                                        name_utf8.Get(),
                                        &info,
                                        nullptr,
                                        0,
                                        nullptr,
                                        0,
                                        nullptr,
                                        method,
                                        level,
                                        zip64 ? 1 : 0);
  return zr == ZIP_OK;
}

// -----------------------------------------------------------------------------
bool ZipBufferHandle::Write(const uint8* data, int64 len) {
  if (cfcore_zipWriteInFileInZip(zlib_handle_, data, (unsigned)len) == ZIP_OK) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
bool ZipBufferHandle::CloseEntry() {
  if (cfcore_zipCloseFileInZip(zlib_handle_) == ZIP_OK) {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
bool ZipBufferHandle::Close() {
  if (zlib_handle_ != nullptr) {
    const int rc = cfcore_zipClose(zlib_handle_, nullptr);
    zlib_handle_ = nullptr;

    return (rc == ZIP_OK);
  }

  return true;
}
// -----------------------------------------------------------------------------
ZipBufferHandle::~ZipBufferHandle() {
  Close();
}