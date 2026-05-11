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
#include "delta_diff_utils.h"

#include <JsonUtilities.h>

#include "services/logger/logger_macros.h"

namespace cfcore {

const TSet<ECFCoreDiffMethod> kSupportedDiffMethods {
  ECFCoreDiffMethod::GBSDIF42
};

const TSet<int> kSupportedVersions {
  1, // Original version
  2, // Added support for methods
  3  // Added support for newFilename
};

}


using namespace cfcore;

// ---------------------------------------------------------------------------
bool DeltaDiffUtils::TryParsingExtraJsonData(
  FInstallModRequestPtr request,
  OUT FFileDeltaDiffExtraData& out_extra_data) {

  if (!request->state.file_delta_diff_info.IsValid()) {
    CFCORE_LOG(Error,
      TEXT("Missing delta diff file info (modId=%lld)"),
      request->modId);
    return false;
  }

  FFileDeltaDiff diff_info = *(request->state.file_delta_diff_info.Get());
  return TryParsingExtraJsonData(diff_info, out_extra_data);
}

// ---------------------------------------------------------------------------
bool DeltaDiffUtils::TryParsingExtraJsonData(
  FFileDeltaDiff delta_diff_info,
  OUT FFileDeltaDiffExtraData& out_extra_data) {

  bool success = FJsonObjectConverter::JsonObjectStringToUStruct(
    delta_diff_info.extraJsonData,
    &out_extra_data,
    0,
    0);

  if (!success) {
    CFCORE_LOG(
      Error,
      TEXT("Failed parsing delta diff file extra data: %s (modId=%lld)"),
      *delta_diff_info.extraJsonData,
      delta_diff_info.modId);
    return false;
  }

  return true;

}

// -----------------------------------------------------------------------------
bool DeltaDiffUtils::IsDiffMethodSupported(
  FFileDeltaDiffExtraData extra_data,
  TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map) {

  if (extra_data.version == 1) {
    return true;
  }

  if (!kSupportedVersions.Contains(extra_data.version)) {
    return false;
  }

  if (extra_data.files.Num() == 0) {
    return false;
  }

  for (const FFileDeltaDiffExtraDataFileInfo& file_info : extra_data.files) {
    if (!diff_method_to_service_map->Contains(file_info.method)) {
      return false;
    }
  }

  return true;
}
