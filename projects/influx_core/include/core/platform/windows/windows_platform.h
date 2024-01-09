#pragma once

namespace influx::platform
{
	inline void set_console_colour_attribute(e_console_colour colour)
	{
		HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
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