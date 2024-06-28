#pragma once

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// We don't like UNICODE steered macros! 
// We will always use the UNICODE A versions of these functions!
#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef messagebox
#undef message_box
#endif

namespace influx::platform
{
	inline void set_console_colour_attribute(e_console_colour colour)
	{
		::HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
		::SetConsoleTextAttribute(hConsole, static_cast<int>(colour));
	}

#pragma region files
	inline string get_current_directory()
	{
		wchar_t buff[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, buff);

		return to_string(buff);
	}

	inline void set_current_directory(const string& path)
	{
		::SetCurrentDirectory(to_wstring(path).c_str());
	}
#pragma endregion
}