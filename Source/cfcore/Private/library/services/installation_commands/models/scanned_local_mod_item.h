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
#include <models/enums/mods_directory_mode.h>

namespace cfcore {

// -----------------------------------------------------------------------------
// A locally installed mod that was scanned
struct FScannedLocalModItem {
  // The root path of the installed mod (relative to the mods dir)
  FString path;

  // The part of the path that is relative to the mods dir (e.g. CF/flat mode)
  FString relative_path_part;

  // If tue, this is a directory, otherwise, a file
  bool is_directory = true;

  // Detection mode - e.g. is this a CFCore mod structure or not
  EModsDirectoryMode directory_mode = EModsDirectoryMode::None;
};

}