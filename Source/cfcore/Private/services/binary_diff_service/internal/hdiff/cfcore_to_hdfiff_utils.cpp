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
#include "cfcore_to_hdfiff_utils.h"

#include <services/filesystem/persistent_file/persistent_binary_file_reader.h>
#include <services/filesystem/persistent_file/persistent_binary_file_writer.h>
#include <services/filesystem/persistent_file/persistent_binary_file.h>
#include <common/macros.h>

namespace cfcore {

// -----------------------------------------------------------------------------
static hpatch_BOOL ReadPersistentFileStream(
  IPersistentBinaryFileReader* file,
  hpatch_StreamPos_t from_position,
  unsigned char* out_data,
  unsigned char* out_data_end) {

  hpatch_size_t read_len = out_data_end - out_data;

#ifdef __RUN_MEM_SAFE_CHECK
  if (from_position > stream->streamSize)
    return _hpatch_FALSE;

  if (read_len > (hpatch_StreamPos_t)(stream->streamSize - from_position))
    return _hpatch_FALSE;
#endif

  TArrayView64<uint8> view(out_data, read_len);
  if (!file->Read(from_position, read_len, view)) {
    return hpatch_FALSE;
  }

  return hpatch_TRUE;
}

// -----------------------------------------------------------------------------
static hpatch_BOOL ReadPersistentFileStream(
  const hpatch_TStreamInput* stream,
  hpatch_StreamPos_t from_position,
  unsigned char* out_data,
  unsigned char* out_data_end) {

  auto file = (IPersistentBinaryFileReader*)stream->streamImport;
  return ReadPersistentFileStream(file, from_position, out_data, out_data_end);
}

// -----------------------------------------------------------------------------
static hpatch_BOOL ReadWritedPersistentFileStream(
  const struct hpatch_TStreamOutput* stream,
  hpatch_StreamPos_t from_position,
  unsigned char* out_data,
  unsigned char* out_data_end) {

  auto file = (IPersistentBinaryFileReader*)stream->streamImport;
  return ReadPersistentFileStream(file, from_position, out_data, out_data_end);
}

// -----------------------------------------------------------------------------
static hpatch_BOOL WritePersistentFileStream(
  const hpatch_TStreamOutput* stream,
  hpatch_StreamPos_t to_position,
  const unsigned char* data,
  const unsigned char* data_end) {

  auto file = (IPersistentBinaryFileWriter*)stream->streamImport;
  hpatch_size_t write_len = data_end - data;

#ifdef __RUN_MEM_SAFE_CHECK
  if (writeToPos > stream->streamSize)
    return _hpatch_FALSE;

  if (writeLen > (hpatch_StreamPos_t)(stream->streamSize - writeToPos))
    return _hpatch_FALSE;
#endif

  if (!file->Write(/*to_position, */data, write_len)) {
    return hpatch_FALSE;
  }

  return hpatch_TRUE;
}

};

using namespace cfcore;

// -----------------------------------------------------------------------------
// static
void CFCoreToHDiffUtils::FileToStreamInput(
  TSharedPtr<IPersistentBinaryFileReader> persistent_binary_file,
  hpatch_TStreamInput& out_input_stream) {

  out_input_stream.streamImport = (void*)(persistent_binary_file.Get());
  out_input_stream.streamSize = persistent_binary_file->TotalSizeInBytes();
  out_input_stream.read = ReadPersistentFileStream;
}

// -----------------------------------------------------------------------------
// static
void CFCoreToHDiffUtils::FileToStreamOutput(
  TSharedPtr<IPersistentBinaryFileWriter> persistent_binary_file_writer,
  hpatch_StreamPos_t stream_size,
  hpatch_TStreamOutput& out_output_stream) {

  out_output_stream.streamImport = (void*)(persistent_binary_file_writer.Get());
  out_output_stream.streamSize = stream_size;
  // NOTE(twolf): We don't need read_writed, as far as I tested
  out_output_stream.read_writed = nullptr; // ReadWritedPersistentFileStream;
  out_output_stream.write = WritePersistentFileStream;
}
