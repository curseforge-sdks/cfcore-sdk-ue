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
#include "unzip_buffer_handle.h"

#include <common/path_utils.h>

#include <services/file_handle/buffer_file_handle.h>
#include <services/buffer/buffer_impl.h>
#include <services/buffer/buffer.h>
#include "zip_platform_file.h"

#include <HAL/PlatformFile.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
TSharedPtr<UnzipBufferHandle> UnzipBufferHandle::FromFile(
  const FString& uncompressed_filename,
  const TCHAR* password/*= nullptr*/) {

  // TODO(twolf): This is a quick patch as we do not want to access file system
  // objects from outside of the IFileSystemService - however, this code should
  // be running on it's own thread and not the game/main thread.
  check(!IsInGameThread());

  IPlatformFile& platfrm_file = FPlatformFileManager::Get().GetPlatformFile();
  TSharedPtr<IFileHandle> file_handle =
    MakeShareable<IFileHandle>(platfrm_file.OpenRead(*uncompressed_filename));

  return UnzipBufferHandle::FromFileHandle(file_handle);
}

// -----------------------------------------------------------------------------
TSharedPtr<UnzipBufferHandle> UnzipBufferHandle::FromBuffer(
  TSharedPtr<IBuffer> uncompressed_buffer,
  const TCHAR* password/*= nullptr*/) {

  // TODO(twolf): Use ClassFactory
  TSharedPtr<BufferFileHandle> buffer_file_handle =
    MakeShareable(new BufferFileHandle(uncompressed_buffer));

  return UnzipBufferHandle::FromFileHandle(buffer_file_handle);
}

// -----------------------------------------------------------------------------
TSharedPtr<UnzipBufferHandle> UnzipBufferHandle::FromFileHandle(
  TSharedPtr<IFileHandle> file_handle,
  const TCHAR* password/*= nullptr*/) {

  zlib_filefunc_def zlib_file_fun_defs;
  zlib_file_fun_defs.zopen_file = &ZipPlatformFile::zlib_open_file;
  zlib_file_fun_defs.zread_file = &ZipPlatformFile::zlib_read_file;
  zlib_file_fun_defs.zwrite_file = &ZipPlatformFile::zlib_write_file;
  zlib_file_fun_defs.ztell_file = &ZipPlatformFile::zlib_tell_file;
  zlib_file_fun_defs.zseek_file = &ZipPlatformFile::zlib_seek_file;
  zlib_file_fun_defs.zclose_file = &ZipPlatformFile::zlib_close_file;
  zlib_file_fun_defs.zerror_file = &ZipPlatformFile::zlib_testerror_file;

  zlib_filefunc64_def zlib_file_fun64_defs;
  zlib_file_fun64_defs.zopen64_file = &ZipPlatformFile::zlib_open64_file;
  zlib_file_fun64_defs.ztell64_file = &ZipPlatformFile::zlib_tell64_file;
  zlib_file_fun64_defs.zseek64_file = &ZipPlatformFile::zlib_seek64_file;

  zlib_file_fun64_defs.zread_file = &ZipPlatformFile::zlib_read_file;
  zlib_file_fun64_defs.zwrite_file = &ZipPlatformFile::zlib_write_file;
  zlib_file_fun64_defs.zclose_file = &ZipPlatformFile::zlib_close_file;
  zlib_file_fun64_defs.zerror_file = &ZipPlatformFile::zlib_testerror_file;

  zlib_file_fun64_defs.opaque = zlib_file_fun_defs.opaque =
    (void*)file_handle.Get();
  unzFile zlib_handle = cfcore_unzOpen2_64(nullptr,
                                           &zlib_file_fun_defs,
                                           &zlib_file_fun64_defs);
  if (!zlib_handle) {
    return nullptr;
  }

  TSharedPtr<UnzipBufferHandle> handle = MakeShareable(
    new UnzipBufferHandle(zlib_handle, file_handle));

  TMap<FString, FZLibFileInfo> zlib_file_info_map;

  /// scan the zip file
  int32 zlib_res = cfcore_unzGoToFirstFile(zlib_handle);
  while (zlib_res == UNZ_OK) {
    FZLibFileInfo zlib_file_info;
    zlib_res = cfcore_unzGetCurrentFileInfo64(zlib_handle,
                                              &zlib_file_info.info,
                                              nullptr,
                                              0,
                                              nullptr,
                                              0,
                                              nullptr,
                                              0);
    if (zlib_res != UNZ_OK)
      break;

    if (zlib_file_info.info.size_filename <= 0)
      break;

    TArray<char> zlib_filename_temp;
    zlib_filename_temp.SetNumUninitialized(
      zlib_file_info.info.size_filename + 1
    );

    zlib_filename_temp[zlib_file_info.info.size_filename] = '\0';

    zlib_res = cfcore_unzGetCurrentFileInfo64(zlib_handle,
                                              &zlib_file_info.info,
                                              zlib_filename_temp.GetData(),
                                              zlib_file_info.info.size_filename,
                                              nullptr,
                                              0,
                                              nullptr,
                                              0);
    if (zlib_res != UNZ_OK)
      break;

    zlib_res = cfcore_unzGetFilePos64(zlib_handle, &zlib_file_info.pos);
    if (zlib_res != UNZ_OK)
      break;

    const FString zlib_filename = UTF8_TO_TCHAR(zlib_filename_temp.GetData());
    zlib_file_info.is_directory = zlib_filename.EndsWith(TEXT("/"));

    handle->AddFileInfo(zlib_filename, zlib_file_info);

    zlib_res = cfcore_unzGoToNextFile(zlib_handle);
  }

  return handle;
}

// -----------------------------------------------------------------------------
UnzipBufferHandle::UnzipBufferHandle(
  unzFile zlib_handle,
  TSharedPtr<class IFileHandle> file_handle_ref) :
  zlib_handle_(zlib_handle),
  file_handle_ref_(file_handle_ref),
  zlib_file_info_map_() {
}

// -----------------------------------------------------------------------------
UnzipBufferHandle::~UnzipBufferHandle() {
  if (zlib_handle_) {
    cfcore_unzClose(zlib_handle_);
    zlib_handle_ = 0;
  }
}

// -----------------------------------------------------------------------------
void UnzipBufferHandle::AddFileInfo(const FString& filename,
                                  const FZLibFileInfo& info) {
  zlib_file_info_map_.Add(filename, info);
}

// -----------------------------------------------------------------------------
bool UnzipBufferHandle::FileExists(const TCHAR* filename) const {
  return zlib_file_info_map_.Contains(filename);
}

// -----------------------------------------------------------------------------
bool UnzipBufferHandle::DirectoryExists(const TCHAR* dir) const {
  const FString the_dir = UnzipBufferHandle::FormatAsDirectoryPath(dir);
  return zlib_file_info_map_.Contains(the_dir);
}

// -----------------------------------------------------------------------------
int64_t UnzipBufferHandle::FileSize(const TCHAR* filename) const {
  const FZLibFileInfo* zlib_file_info = zlib_file_info_map_.Find(filename);
  return zlib_file_info ? (int64_t)zlib_file_info->info.uncompressed_size : 0;
}

// -----------------------------------------------------------------------------
FDateTime UnzipBufferHandle::GetTimeStamp(const TCHAR* filename) const {
  return GetStatData(filename).CreationTime;
}

// -----------------------------------------------------------------------------
TSharedPtr<IBuffer> UnzipBufferHandle::GetBuffer(const TCHAR* filename) {
  const FZLibFileInfo* zlib_file_info = zlib_file_info_map_.Find(filename);
  if (!zlib_file_info)
    return nullptr;

  unz64_file_pos target_pos = zlib_file_info->pos;
  int err = cfcore_unzGoToFilePos64(zlib_handle_, &target_pos);
  if (err != UNZ_OK)
    return nullptr;

  if ((password_.IsEmpty() && cfcore_unzOpenCurrentFile(zlib_handle_) != UNZ_OK) ||
    (!password_.IsEmpty() && cfcore_unzOpenCurrentFilePassword(zlib_handle_, TCHAR_TO_UTF8(*password_)) != UNZ_OK))
    return nullptr;

  int64_t allocated_size = (int64_t)zlib_file_info->info.uncompressed_size;
  TSharedPtr<TArray64<uint8>> buffer = MakeShareable(new TArray64<uint8>());
  buffer->SetNumUninitialized(allocated_size);

  ZPOS64_T result = cfcore_unzReadCurrentFile(
    zlib_handle_,
    (voidp)buffer->GetData(),
    zlib_file_info->info.uncompressed_size);

  cfcore_unzCloseCurrentFile(zlib_handle_);

  if (result != zlib_file_info->info.uncompressed_size) {
    return nullptr;
  }

  return MakeShared<BufferImpl>(buffer);
}

// -----------------------------------------------------------------------------
FFileStatData UnzipBufferHandle::GetStatData(const TCHAR* filename_or_dir) const {
  const FZLibFileInfo* zlib_file_info =
    zlib_file_info_map_.Find(filename_or_dir);
  if (!zlib_file_info) {
    /// try find as a directory
    const FString dir = UnzipBufferHandle::FormatAsDirectoryPath(filename_or_dir);
    zlib_file_info = zlib_file_info_map_.Find(dir);
  }

  if (!zlib_file_info)
    return FFileStatData();

  const FDateTime file_date_time = zlib_file_info->GetDateTime();
  return FFileStatData(file_date_time,
                       file_date_time,
                       file_date_time,
                       (int64_t)zlib_file_info->info.uncompressed_size,
                       zlib_file_info->is_directory,
                       true);
}

// -----------------------------------------------------------------------------
FDateTime UnzipBufferHandle::FZLibFileInfo::GetDateTime() const {
  return FDateTime((int32)info.tmu_date.tm_year,
                   (int32)info.tmu_date.tm_mon + 1,
                   (int32)info.tmu_date.tm_mday,
                   (int32)info.tmu_date.tm_hour,
                   (int32)info.tmu_date.tm_min,
                   (int32)info.tmu_date.tm_sec);
}

// -----------------------------------------------------------------------------
FString UnzipBufferHandle::FormatAsDirectoryPath(const TCHAR* filename) {
  FString dir = PathUtils::NormalizePath(filename);

  if (!dir.EndsWith(TEXT("/"))) {
    dir.Append(TEXT("/"));
  }

  return dir;
}

// -----------------------------------------------------------------------------
bool UnzipBufferHandle::OpenCurrentFile(const TCHAR* filename) {
  const FZLibFileInfo* zlib_file_info = zlib_file_info_map_.Find(filename);
  if (!zlib_file_info) {
    return false;
  }

  unz64_file_pos target_pos = zlib_file_info->pos;
  int err = cfcore_unzGoToFilePos64(zlib_handle_, &target_pos);
  if (err != UNZ_OK) {
    return false;
  }

  if (password_.IsEmpty()) {
    if (cfcore_unzOpenCurrentFile(zlib_handle_) != UNZ_OK) {
      return false;
    }
  } else { // !password_.IsEmpty()
    if (cfcore_unzOpenCurrentFilePassword(zlib_handle_, TCHAR_TO_UTF8(*password_)) != UNZ_OK) {
      return false;
    }
  }

  return true;
}

// -----------------------------------------------------------------------------
int64_t UnzipBufferHandle::ReadCurrentFile(OUT TArray<uint8>& buffer) {
  int64_t result = cfcore_unzReadCurrentFile(zlib_handle_,
                                             (voidp)buffer.GetData(),
                                             buffer.Num());

  return result;
}

// -----------------------------------------------------------------------------
bool UnzipBufferHandle::CloseCurrentFile() {
  return cfcore_unzCloseCurrentFile(zlib_handle_) == UNZ_OK;
}