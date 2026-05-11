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
#include "create_mod_file_request_serializer.h"

#include <common/macros.h>

using namespace cfcore;

// -----------------------------------------------------------------------------
// static
void CreateModFileRequestSerializer::Serialize(FCreateModFileRequest request,
                                               WWWForm& www_form) {

  ENUM_TO_STR(str_changelog_type, request.changelogType);
  www_form.AddField("changelogType", str_changelog_type);

  www_form.AddField("changelog", request.changelog);
  www_form.AddField("filename", request.filename);

  if (!request.displayName.IsEmpty()) {
    www_form.AddField("displayName", request.displayName);
  }

  ENUM_TO_STR(str_release_type, request.releaseType);
  www_form.AddField("releaseType", str_release_type);

  SerializeGameVersions(request, www_form);
  SerializeCookingOptions(request, www_form);

  www_form.AddField("fileLength", request.fileLength);

  www_form.AddField("isMarkedForManualRelease",
                    request.isMarkedForManualRelease);
}

// -----------------------------------------------------------------------------
// static
void CreateModFileRequestSerializer::SerializeGameVersions(
  FCreateModFileRequest request,
  WWWForm& www_form) {

  for (const int64& gameVersionId : request.gameVersionIds) {
    www_form.AddField("gameVersionIds", gameVersionId);
  }

  if (request.gameVersionIds.Num() == 0) {
    // -1 will take the latest
    www_form.AddField("gameVersionIds", -1);
  }
}

// -----------------------------------------------------------------------------
// static
void CreateModFileRequestSerializer::SerializeCookingOptions(
  FCreateModFileRequest request,
  WWWForm& www_form) {

  if (!request.cookingOptions.isSourceFile) {
    return;
  }

  www_form.AddField("cookingOptions", request.cookingOptions);
}
