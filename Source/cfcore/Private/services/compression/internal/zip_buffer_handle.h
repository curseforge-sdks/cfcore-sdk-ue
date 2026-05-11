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
#include <CoreMinimal.h>
#include <GenericPlatform/GenericPlatformFile.h>

#include "common/macros.h"

#include <services/filesystem/persistent_file/persistent_binary_file_writer.h>

#include "minizip/zip.h"

namespace cfcore {

class ZipBufferHandle {
public:
  virtual ~ZipBufferHandle();
public:
  static TSharedPtr<ZipBufferHandle> FromFile(
    const FString& uncompressed_filename,
    const TCHAR* password/*= nullptr*/);

  bool OpenNewEntry(const FString& entry_name,
                    int method,
                    int level,
                    bool zip64);

  bool Write(const uint8* data, int64 len);
  bool CloseEntry();

  bool Close();

private:
  static TSharedPtr<ZipBufferHandle> FromFileHandle(
    TSharedPtr<IFileHandle> file_handle);

  ZipBufferHandle(zipFile zlib_handle,
                       TSharedPtr<IFileHandle> file_handle);

  DISALLOW_COPY_AND_ASSIGN(ZipBufferHandle);

private:
  zipFile zlib_handle_;
  TSharedPtr<class IFileHandle> file_handle_ref_;
};

} // namespace cfcore