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
#pragma once
#include "minizip/zip.h"

namespace ZipPlatformFile {
  // ---------------------------------------------------------------------------
  static inline long zlib_seek64_file(voidpf opaque,
                                      voidpf stream,
                                      ZPOS64_T offset,
                                      int origin) {
    if (!stream) {
      return -1;
    }

    IFileHandle* file_handle = (IFileHandle*)stream;
    switch (origin) {
    case ZLIB_FILEFUNC_SEEK_CUR:
      file_handle->Seek(file_handle->Tell() + (int64_t)offset);
      break;
    case ZLIB_FILEFUNC_SEEK_END:
      file_handle->SeekFromEnd((int64_t)offset);
      break;
    case ZLIB_FILEFUNC_SEEK_SET:
      file_handle->Seek((int64_t)offset);
      break;
    }

    return 0;
  }

  // ---------------------------------------------------------------------------
  static inline ZPOS64_T zlib_tell64_file(voidpf opaque, voidpf stream) {
    if (!stream) {
      return 0;
    }

    IFileHandle* file_handle = (IFileHandle*)stream;
    return file_handle->Tell();
  }

  // ---------------------------------------------------------------------------
  static inline voidpf zlib_open64_file(voidpf opaque,
                                        const void* filename,
                                        int mode) {
    if (!opaque) {
      return opaque;
    }

    IFileHandle* file_handle = (IFileHandle*)opaque;
    return file_handle;
  }

  // ---------------------------------------------------------------------------
  static inline voidpf zlib_open_file(voidpf opaque,
                                      const char* filename,
                                      int mode) {

    return zlib_open64_file(opaque, (const void*)filename, mode);
  }

  // ---------------------------------------------------------------------------
  static inline uLong zlib_read_file(voidpf opaque,
                                     voidpf stream,
                                     void* buf,
                                     uLong size) {
    if (!stream) {
      return 0;
    }

    IFileHandle* file_handle = (IFileHandle*)stream;
    const int64_t remain_size = file_handle->Size() - file_handle->Tell();
    if (remain_size <= 0) {
      return 0;
    }

    if ((int64_t)size > remain_size) {
      size = remain_size;
    }

    if (!file_handle->Read((uint8*)buf, size)) {
      return 0;
    }

    return size;
  }

  // ---------------------------------------------------------------------------
  static inline uLong zlib_write_file(voidpf opaque,
                                      voidpf stream,
                                      const void* buf,
                                      uLong size) {
    if (!stream) {
      return 0;
    }



    IFileHandle* file_handle = (IFileHandle*)stream;
    if (!file_handle->Write((const uint8*)buf, size)) {
      return 0;
    }

    return size;
  }

  // ---------------------------------------------------------------------------
  static inline long zlib_tell_file(voidpf opaque, voidpf stream) {
    return (long)zlib_tell64_file(opaque, stream);
  }

  // ---------------------------------------------------------------------------
  static inline long zlib_seek_file(voidpf opaque,
                                    voidpf stream,
                                    uLong offset,
                                    int origin) {

    return ZipPlatformFile::zlib_seek64_file(opaque, stream, offset, origin);
  }

  // ---------------------------------------------------------------------------
  static inline int zlib_close_file(voidpf opaque, voidpf stream) {
    return 0;
  }

  // ---------------------------------------------------------------------------
  static inline int zlib_testerror_file(voidpf opaque, voidpf stream) {
    return 0;
  }
}