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

#include "external_auth_provider.generated.h"

// -----------------------------------------------------------------------------
// NOTE: Most providers require a setup on the console
UENUM(BlueprintType)
enum class ECFCoreExternalAuthProvider : uint8 {
  None = 0,
  /**
   * Steam
   *
   * Get a session ticket by calling ISteamUser::GetAuthSessionTicket and pass
   * a base64 representation of the session ticket as the |external_token|.
   */
  Steam = 1,
  /**
   * PlayStation Network
   *
   * Get an access token using an authorization code and pass that as the
   * |external_token|.
   * You may also include an environment value under |additional_info|.
   */
  PSN = 2,
  /**
   * Xbox Live
   *
   * Get an access token using an XSTS token.
   */
  XBL = 3,
  /**
   * Warner Brothers
   */
  WB = 4,
  /**
   * Epic Games
   *
   * Pass the JWT token as the |external_token|. The JWT token can be obtained
   * by using the EOS SDK to call the EOS_Auth_CopyIdToken function.
   *
   * See https://dev.epicgames.com/docs/epic-account-services/auth/auth-interface#retrieving-an-id-token-for-user
   */
  Epic = 5,
  /**
   * GOG Galaxy
   *
   * Request an encrypted app ticket via galaxy::api::User()->RequestEncryptedAppTicket()
   * and pass a base64 representation of the raw ticket bytes as the |external_token|.
   * Requires the Encrypted App Ticket Key to be configured in your game's
   * authentication section on https://console.curseforge.com
   *
   * See https://docs.gog.com/sdk-encrypted-tickets/
   */
  GOG = 6,
};
