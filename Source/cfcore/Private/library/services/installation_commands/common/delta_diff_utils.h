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
#include "CoreMinimal.h"

#include <library/services/models/install_mod_request.h>
#include <library/services/installation_commands/models/file_delta_diff_extra_data.h>
#include <library/services/installation_commands/models/diff_method_to_service_map.h>

namespace cfcore {

class DeltaDiffUtils {
public:
  static bool TryParsingExtraJsonData(
    FInstallModRequestPtr request,
    OUT FFileDeltaDiffExtraData& out_extra_data);

  static bool TryParsingExtraJsonData(
    FFileDeltaDiff delta_diff_info,
    OUT FFileDeltaDiffExtraData& out_extra_data);

  static bool IsDiffMethodSupported(
    FFileDeltaDiffExtraData extra_data,
    TSharedRef<DiffMethodToServiceMap> diff_method_to_service_map);
};

} // namespace cfcore