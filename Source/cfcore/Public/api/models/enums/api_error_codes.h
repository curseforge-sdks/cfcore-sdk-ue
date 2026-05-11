/*MIT License

Copyright (c) 2022 Overwolf Ltd.

Permission is hereby granted; free of charge; to any person obtaining a copy
of this software and associated documentation files (the "Software"); to deal
in the Software without restriction; including without limitation the rights
to use; copy; modify; merge; publish; distribute; sublicense; and/or sell
copies of the Software; and to permit persons to whom the Software is
furnished to do so; subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND; EXPRESS OR
IMPLIED; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM; DAMAGES OR OTHER
LIABILITY; WHETHER IN AN ACTION OF CONTRACT; TORT OR OTHERWISE; ARISING FROM;
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#pragma once
#include <CoreMinimal.h>

// -----------------------------------------------------------------------------
namespace cfcore {

/**
 * These are the supported API errors - they are not ordered in any meaningful
 * manner - so it is best to use them as a reference.
 *
 * We are working on adding them to our documentations.
 */
namespace api_error_codes {

/**
  * No error
  */
const int32 kNone = 0;

/**
  * When trying upload a file for a mod (or to update a mod) that doesn't exist
  */
const int32 kNoSuchMod = 1100;
const int32 kPLACEHOLDER_1101 = 1101;

/**
  * When trying to create a mod with an unknown GameId
  */
const int32 kNoSuchGame = 1102;

/**
  * - When an uploaded mod file failed - internal server error
  * - When uploading a manually cooked file for an unknown sourceFileId
  */
const int32 kNoSuchModFile = 1103;

/**
  * When trying to upload a mod file to the wrong GameId (i.e. to a mod
  belonging to a different game than the one initialized with the SDK)
  */
const int32 kModGameMismatch = 1104;
const int32 kModFileSourceMismatch = 1105;
const int32 kNotSourceModFile = 1106;

/**
 * Only the source file author (the person who uploaded the source file) can
 * upload cooked files associated to that source.
 */
const int32 kModFileAuthorMismatch = 1107;

/**
 * The current user is not allowed to upload/update a mod
 */
const int32 kUserNotAllowedForMod = 1110;
const int32 kDeletedUserSuspected = 1111;
const int32 kModIsNotValid = 1120;

/**
  * When trying to create a new mod with a name that is already taken
  */
const int32 kModNameAlreadyExists = 1121;

/**
 * The class (classId) was configured to not allow uploads
 */
const int32 kNoUploadToClass = 1130;
const int32 kFileIsStillProccessing = 1131;

/**
 * Exceeded max size of file - contact us if you need this changed
 */
const int32 kFileSizeTooBig = 1132;

/**
 * Missing filename for uploaded file
 */
const int32 kNoFileName = 1133;
const int32 kNoAdditionalFiles = 1134;
const int32 kParentFileDoesNotExist = 1135;
const int32 kGameVersionsNotAllowedForAddonFiles = 1136;

/**
 * Unknown game version (see https://docs.curseforge.com/#get-versions-v2)
 * and try to fix the version id
 */
const int32 kNoSuchGameVersion = 1137;

/**
 * Unknown game version (see https://docs.curseforge.com/#get-versions-v2)
 * and try to fix the version id
 */
const int32 kInvalidGameVersion = 1138;

/**
 * The game version is not allowed for the given class (classId)
 */
const int32 kGameVersionNotMappedToCategory = 1139;
const int32 kNoChildFileType = 1140;

/**
 * User is not allowed to upload membership files (CurseForge Pro)
 */
const int32 kUserCannotUploadMemberAccessOnlyContent = 1141;

/**
 * The current user doesn't have privlieges to upload a file for a mod
 */
const int32 kUserCannotUploadFiles = 1142;
const int32 kSourceFileNotInCookingStatus = 1143;
const int32 kNotAllSourceFilesReady = 1144;
const int32 kSourceFileAlreadyHasPlatform = 1145;
const int32 kPlatformNotAllowed = 1160;

} // namespace api_error_codes
} // namespace cfcore