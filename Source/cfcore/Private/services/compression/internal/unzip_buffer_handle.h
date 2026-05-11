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
#pragma once
#include <CoreMinimal.h>
#include <GenericPlatform/GenericPlatformFile.h>

#include "common/macros.h"

// NOTE(twolf): These two definitions means we can't use password for encryption
// and unencryption of zip files. This is helpful for cross platform (PS)
// support and isn't something we use anyway.
#define NOCRYPT
#define NOUNCRYPT
#include "minizip/unzip.h"

namespace cfcore {
class IBuffer;

// -----------------------------------------------------------------------------
class UnzipBufferHandle {
public:
  struct FZLibFileInfo {
    unz_file_info64 info;
    unz64_file_pos pos;
    bool is_directory;
    FDateTime GetDateTime() const;
  };

public:
  static TSharedPtr<UnzipBufferHandle> FromFile(
    const FString& uncompressed_filename,
    const TCHAR* password = nullptr);

  static TSharedPtr<UnzipBufferHandle> FromBuffer(
    TSharedPtr<IBuffer> uncompressed_buffer,
    const TCHAR* password = nullptr);

public:
  virtual ~UnzipBufferHandle();

public:
  FORCEINLINE bool IsValid() const { return (zlib_handle_ != 0); }
  FORCEINLINE const TMap<FString, FZLibFileInfo>& GetZipInfos() const {
    return zlib_file_info_map_;
  }

public:
  bool FileExists(const TCHAR* filename) const;
  bool DirectoryExists(const TCHAR* directory) const;
  int64_t FileSize(const TCHAR* filename) const;
  FDateTime GetTimeStamp(const TCHAR* filename) const;
  TSharedPtr<IBuffer> GetBuffer(const TCHAR* filename);
  FFileStatData GetStatData(const TCHAR* filename_or_directory) const;

  // NOTE(twolf): Use these trio methods to efficiently stream a file from the
  // zip to disk
  bool OpenCurrentFile(const TCHAR* filename);
  // > 0 - number of written bytes
  // == 0 - reached EOF (UNZ_EOF)
  // < 0 - error occured (see unzip.h under #define UNZ_OK)
  int64_t ReadCurrentFile(OUT TArray<uint8>& buffer);
  bool CloseCurrentFile();

private:
  static TSharedPtr<UnzipBufferHandle> FromFileHandle(
    TSharedPtr<IFileHandle> file_handle,
    const TCHAR* password = nullptr);

  UnzipBufferHandle(unzFile zlib_handle,
                  TSharedPtr<class IFileHandle> file_handle_ref);
  DISALLOW_COPY_AND_ASSIGN(UnzipBufferHandle);

  void AddFileInfo(const FString& filename, const FZLibFileInfo& info);

private:
  unzFile zlib_handle_;
  // To keep the handle alive
  TSharedPtr<class IFileHandle> file_handle_ref_;
  FString password_;

  TMap<FString, FZLibFileInfo> zlib_file_info_map_;

public:
  static FString FormatAsDirectoryPath(const TCHAR* Filename);
};

} // namespace cfcore