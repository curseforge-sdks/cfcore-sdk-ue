/*MIT License

Copyright (c) 2024 Overwolf Ltd.

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
#include "startup_info_logging_impl_win.h"
#include <services/logger/logger_macros.h>

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace cfcore {
#if PLATFORM_WINDOWS
	static bool DoesRegKeyExist(const Windows::HKEY InKey, const TCHAR* InSubKey);
#endif
}

using namespace cfcore;

// -----------------------------------------------------------------------------
void StartupInfoLoggingImplWin::Run() {
	PrintWindowsVersion();
	PrintRavExistance();
}

// -----------------------------------------------------------------------------
void StartupInfoLoggingImplWin::PrintWindowsVersion() {
#if PLATFORM_WINDOWS
	FString os = FWindowsPlatformMisc::GetOSVersion();
	CFCORE_LOG(Log, TEXT("Detected OS Windows: %s"), *os);
#endif
}

// -----------------------------------------------------------------------------
void StartupInfoLoggingImplWin::PrintRavExistance() {
#if PLATFORM_WINDOWS
	bool rav_exists = DoesRegKeyExist(HKEY_LOCAL_MACHINE,
																		TEXT("SOFTWARE\\ReasonLabs"));

	if (!rav_exists) {
		return;
	}

	CFCORE_LOG(Warning,
						 TEXT("Detected RAV (anti-virus) - might block downloads!"));
#endif
}

#if PLATFORM_WINDOWS

// -----------------------------------------------------------------------------
bool cfcore::DoesRegKeyExist(const Windows::HKEY InKey, const TCHAR* InSubKey) {
	for (int32 reg_index = 0; reg_index < 2; ++reg_index) {
		HKEY Key = 0;
		const uint32 flags = (reg_index == 0) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY;

		LSTATUS status = RegOpenKeyEx(InKey, InSubKey, 0, KEY_READ | flags, &Key);
		if (ERROR_SUCCESS == status) {
			RegCloseKey(Key);
			return true;
		}
	}

	return false;
}

#endif
