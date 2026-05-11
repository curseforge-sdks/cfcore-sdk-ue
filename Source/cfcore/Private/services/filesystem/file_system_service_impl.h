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
#include <Misc/QueuedThreadPool.h>

#include "file_system_service.h"
#include <class_factory.h>

namespace cfcore {
class IIOThrottlingService;
class IThreadPoolsService;
class IPersistentLogFile;
class IPersistentBinaryFile;
class IPersistentBinaryFileBuffered;

template<typename InterfaceType>
class IPersistentBinaryFileInternal;
template<typename InterfaceType>
class IPersistentBinaryFileDecoratorInternal;

// TODO(twolf): Refactoring is needed here - move all the persistent file
// functionality into it's own service (e.g. PersistentFileFactory)
class IPersistentBinaryFileReader;
class IPersistentBinaryFileWriter;

using PersistentBinFileReaderInternal =
  IPersistentBinaryFileInternal<IPersistentBinaryFileReader>;
using PersistBinFileReaderInternalFactory =
  ClassFactory<PersistentBinFileReaderInternal>;

using PersistentBinFileWriterInternal =
  IPersistentBinaryFileInternal<IPersistentBinaryFileWriter>;
using PersistBinFileWriterInternalFactory =
  ClassFactory<PersistentBinFileWriterInternal>;

using PersistentBinFileReaderBufferedInternal =
  IPersistentBinaryFileDecoratorInternal<IPersistentBinaryFileReader>;
using PersistBinFileReaderBufferredInternalFactory =
  ClassFactory<PersistentBinFileReaderBufferedInternal>;

using PersistentBinFileWriterBufferedInternal =
  IPersistentBinaryFileDecoratorInternal<IPersistentBinaryFileWriter>;
using PersistBinFileWriterBufferredInternalFactory =
  ClassFactory<PersistentBinFileWriterBufferedInternal>;

using PersistBinFileBuffFactory = ClassFactory<IPersistentBinaryFileBuffered>;

// -----------------------------------------------------------------------------
class FileSystemServiceImpl : public IFileSystemService {
public:
  FileSystemServiceImpl(
    TSharedPtr<IIOThrottlingService> io_throttling_service,
    TSharedPtr<IThreadPoolsService> thread_pools_service,
    TSharedPtr<ClassFactory<IPersistentLogFile>> persistent_log_file_factory,
    TSharedPtr<PersistBinFileReaderInternalFactory> prstnt_bin_file_r_intrnl,
    TSharedPtr<PersistBinFileWriterInternalFactory> prstnt_bin_file_w_intrnl,
    TSharedPtr<PersistBinFileReaderBufferredInternalFactory>
      persistnt_bin_file_reader_buffered_intrnl_factory,
    TSharedPtr<PersistBinFileWriterBufferredInternalFactory>
      persistnt_bin_file_writer_buffered_intrnl_factory,
    TSharedPtr<PersistBinFileBuffFactory> persistnt_bin_file_buffered_factory);

// IFileSystemService
public:
  virtual TFuture<FFileStatData> GetStat(TSharedRef<FString> path) override;
  virtual TFuture<bool> FileExists(TSharedRef<FString> filename) override;
  virtual TFuture<bool> PathExists(TSharedRef<FString> path) override;

  virtual TFuture<bool> DeleteFile(TSharedRef<FString> filename) override;
  virtual TFuture<bool> DeleteDirectory(TSharedRef<FString> directory) override;
  virtual TFuture<EFileSystemError> CreateDirectory(
    const FString& directory) override;

  virtual TFuture<EFileSystemError> ReadBuffer(
    TSharedRef<FString> filename,
    int64 file_offset_in_bytes,
    int64 bytes_to_read,
    TSharedRef<TArray<uint8>> buffer) override;

  virtual TFuture<TTuple<EFileSystemError, FString>> ReadText(
    const FString& filename) override;

  virtual TFuture<ReadBufferRes> ReadBuffer(const FString& filename) override;

  virtual TFuture<ReadBuffer64Res> ReadBuffer64(
    TSharedRef<FString> filename) override;

  virtual TFuture<EFileSystemError> WriteText(const FString& filename,
                                              const FString& text) override;

  virtual TFuture<EFileSystemError> WriteBuffer(
    TSharedRef<FString> filename,
    const uint8* data,
    int64 length_in_bytes,
    int64 file_offset_in_bytes = 0) override;

  virtual TFuture<bool> MoveFile(const FString& src_filename,
                                 const FString& dest_filename) override;
  virtual TFuture<bool> MoveDirectory(const FString& src_dir,
                                      const FString& dest_dir) override;

  virtual TFuture<EFileSystemError> GetDiskSpaceInfo(
    TSharedRef<FString> path,
    IN OUT TSharedRef<DiskSpaceInfo> disk_space_info) override;

  virtual TFuture<bool> GetFilesInDirectory(
    TSharedRef<FString> directory,
    TSharedRef<FString> file_prefix,
    OUT TSharedRef<TArray<FString>> files) override;

  virtual TFuture<bool> GetFilesInDirectoryRecursive(
    TSharedRef<FString> directory,
    TSharedRef<FString> file_prefix,
    OUT TSharedRef<TArray<FString>> files) override;

  virtual TSharedPtr<IPersistentLogFile> CreatePersistentLogFile(
    const FString& filename) override;

  virtual TSharedPtr<IPersistentBinaryFileReader>
    CreatePersistentBinaryFileReader(const FString& filename) override;
  virtual TSharedPtr<IPersistentBinaryFileWriter>
    CreatePersistentBinaryFileWriter(const FString& filename) override;
  virtual TSharedPtr<IPersistentBinaryFileReader>
    CreatePersistentBinaryFileReaderBuffered(
      const FString& filename,
      struct BufferedFileOptions& options) override;
  virtual TSharedPtr<IPersistentBinaryFileWriter>
    CreatePersistentBinaryFileWriterBuffered(
      const FString& filename,
      struct BufferedFileOptions& options) override;


  virtual TSharedPtr<IPersistentBinaryFile> CreatePersistentBinaryFile(
    const FString& filename,
    bool read) override;

private:
  bool AssureFileDirectoryExists(const FString& filename);
  bool AssureDirectoryExists(const FString& directory);
  bool WriteBufferWithThrottling(const TUniquePtr<IFileHandle>& file_handle,
                                 const uint8* data,
                                 int64 length);

private:
  TSharedPtr<IIOThrottlingService> io_throttling_service_;
  TSharedPtr<FQueuedThreadPool> thread_pool_;
  TSharedPtr<ClassFactory<IPersistentLogFile>> persistent_log_file_factory_;
  TSharedPtr<PersistBinFileReaderInternalFactory>
    persistent_bin_file_reader_intrnl_factory_;
  TSharedPtr<PersistBinFileWriterInternalFactory>
    persistent_bin_file_writer_intrnl_factory_;
  TSharedPtr<PersistBinFileReaderBufferredInternalFactory>
    persistnt_bin_file_reader_buffered_factory_;
  TSharedPtr<PersistBinFileWriterBufferredInternalFactory>
    persistnt_bin_file_writer_buffered_factory_;

  TSharedPtr<PersistBinFileBuffFactory> persistent_bin_file_buffered_factory_;
};

} // namespace cfcore

Expose_TNameOf(cfcore::FileSystemServiceImpl)
Expose_TNameOf(cfcore::PersistentBinFileReaderInternal)
Expose_TNameOf(cfcore::PersistentBinFileWriterInternal)
Expose_TNameOf(cfcore::PersistentBinFileReaderBufferedInternal)
Expose_TNameOf(cfcore::PersistentBinFileWriterBufferedInternal)

Expose_TNameOf(cfcore::ClassFactory<cfcore::IPersistentLogFile>)
Expose_TNameOf(cfcore::ClassFactory<cfcore::PersistentBinFileReaderInternal>)
Expose_TNameOf(cfcore::ClassFactory<cfcore::PersistentBinFileWriterInternal>)
Expose_TNameOf(cfcore::ClassFactory<cfcore::PersistentBinFileReaderBufferedInternal>)
Expose_TNameOf(cfcore::ClassFactory<cfcore::PersistentBinFileWriterBufferedInternal>)
Expose_TNameOf(cfcore::ClassFactory<cfcore::IPersistentBinaryFile>)
Expose_TNameOf(cfcore::ClassFactory<cfcore::IPersistentBinaryFileBuffered>)